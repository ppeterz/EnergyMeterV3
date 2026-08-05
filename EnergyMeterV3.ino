/*
  Energy Meter V3 -- 3-socket extension
  Precious "Dunma" -- Internal Defense, 4 August 2026

  Commands over Serial (115200 baud):
    1 / 2 / 3   toggle that socket's relay
    r           reset all energy counters
    c           calibrate voltage against a real multimeter reading
    b           cycle IBEDC tariff band (A→B→C→D→E→A)

  WIFI SETUP (no hardcoded credentials):
    First boot (or after a WiFi reset): the ESP32 opens its own WiFi hotspot
    named "EnergyMonitor-Setup". Connect a phone/laptop to it -- a setup
    page should open automatically (or go to 192.168.4.1 in a browser),
    pick your real network from the scanned list, enter its password, done.
    That's saved to flash and reused automatically on every future boot.

    To reconnect to a DIFFERENT network later (e.g. moving from home to the
    defense venue): hold the push button down while powering on. This wipes
    the saved WiFi and reopens the "EnergyMonitor-Setup" hotspot so you can
    pick the new network. Everything else (sensors, relays, LCD) keeps
    working normally even with no WiFi connected at all -- Blynk is the
    only thing that goes idle.

  BEFORE FLASHING:
    - Fill in your Blynk auth token below (WiFi no longer needs filling in).
    - VOLT_SCALE / CURRENT_SCALE in Config.h are fallback defaults only --
      your calibrated VOLT_SCALE from the 'c' command is already saved to
      flash from your single-channel testing and will be loaded automatically.
    - CURRENT_SCALE is currently shared across all 4 current channels. If
      your 3 socket ACS712 units read noticeably differently from each
      other, that's the next thing worth splitting out -- not done here to
      keep this change focused on the socket-count extension you asked for.

  NEW LIBRARY NEEDED: WiFiManager (by tzapu) -- install via Library Manager.
*/

#define BLYNK_TEMPLATE_ID   "TMPL2SE9pzkuP"
#define BLYNK_TEMPLATE_NAME "Energy Monitor"
#define BLYNK_AUTH_TOKEN    "4uPlGiQPU-errl4_KNACE2Cs6H9tjo2D"

// Shrink Blynk flash footprint
#define BLYNK_NO_BUILTIN
#define BLYNK_SEND_THROTTLE 0

#include <WiFi.h>
#include <WiFiManager.h>
#include <BlynkSimpleEsp32.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <HTTPUpdate.h>
#include "Config.h"
#include "Sensor.h"
#include "MeasurementEngine.h"
#include "Measurement.h"
#include "Relay.h"
#include "Logger.h"
#include "Command.h"
#include "Energy.h"
#include "Calibration.h"
#include "Display.h"
#include "Button.h"
#include "Tariff.h"
#include "DSP.h"

WiFiManager wm;
bool wifiConnected = false;

Sensor sensor;
MeasurementEngine measurementEngine;
Relay relay[NUM_SOCKETS];
Logger logger;
Command command;
Energy energy[NUM_CHANNELS];
Display display;
Button button;

Measurement measurements[NUM_CHANNELS];

const int relayPins[NUM_SOCKETS]       = { PIN_RELAY1, PIN_RELAY2, PIN_RELAY3 };
const char* energyKeys[NUM_CHANNELS]   = { "eIn", "e1", "e2", "e3" };

int currentScreen = 0;                    // 0 = input, 1..NUM_SOCKETS = sockets
unsigned long lastScreenChangeMs = 0;
const unsigned long HOME_SCREEN_MS   = 10000;  // home screen (overview) stays 10 seconds
const unsigned long SOCKET_SCREEN_MS = 3000;   // socket screens cycle every 3 seconds

unsigned long lastMeasureMs = 0;
const unsigned long MEASURE_INTERVAL_MS = 1000;   // one full capture+compute cycle per second

unsigned long lastBlynkPushMs = 0;
const unsigned long BLYNK_PUSH_INTERVAL_MS = 30000;   // 30 seconds — saves quota for demo

unsigned long lastEnergySaveMs = 0;
const unsigned long ENERGY_SAVE_INTERVAL_MS = 60000;

DSP dsp;

// ---------------- Blynk: relay & control handlers ----------------
// Bind 3 switch widgets to V0, V1, V2 in your Blynk template.
BLYNK_WRITE(V0) { display.resetActivityTimer(); param.asInt() ? relay[0].on() : relay[0].off(); }
BLYNK_WRITE(V1) { display.resetActivityTimer(); param.asInt() ? relay[1].on() : relay[1].off(); }
BLYNK_WRITE(V2) { display.resetActivityTimer(); param.asInt() ? relay[2].on() : relay[2].off(); }

// V3 = Tariff Band selector (0=Band A, 1=Band B, 2=Band C, 3=Band D, 4=Band E)
BLYNK_WRITE(V3) {
    display.resetActivityTimer();
    int b = param.asInt();
    if (b >= 0 && b < NUM_BANDS) {
        tariff.setBand(static_cast<TariffBand>(b));
        Serial.printf("Blynk: Tariff set to Band %s (N%.2f/kWh)\n", tariff.getBandName(), tariff.getRate());
        display.show(currentScreen, measurements, energy);
    }
}

// V4 = Reset energy counters (Push Button widget)
BLYNK_WRITE(V4) {
    display.resetActivityTimer();
    if (param.asInt() == 1) {
        for (int c = 0; c < NUM_CHANNELS; c++) energy[c].reset();
        Serial.println("Blynk: All energy counters reset");
        display.show(currentScreen, measurements, energy);
    }
}

// V5 = Calibrate Voltage (Numeric Input widget, e.g. send 225.0)
BLYNK_WRITE(V5) {
    display.resetActivityTimer();
    float knownVoltage = param.asFloat();
    if (knownVoltage > 50.0f && knownVoltage < 350.0f) {
        sensor.capture();
        float vOffset = dsp.mean(sensor.voltageBuffer);
        float vrmsADC = dsp.rms(sensor.voltageBuffer, vOffset);
        if (vrmsADC >= 1.0f) {
            float newScale = knownVoltage / vrmsADC;
            calibration.setVoltScale(newScale);
            Serial.printf("Blynk: Calibrated. Known V=%.1fV, VOLT_SCALE=%.6f\n", knownVoltage, newScale);
            display.show(currentScreen, measurements, energy);
        } else {
            Serial.println("Blynk: Calibration failed - ADC reading too low.");
        }
    }
}

// Check GitHub for new firmware version automatically
void checkForGitHubUpdate()
{
    if (!wifiConnected)
    {
        display.showMessage("GitHub OTA", "No WiFi Conn!");
        delay(1500);
        return;
    }

    Serial.println("----------------------------------------");
    Serial.printf("Checking GitHub for updates... (Current: v%s)\n", FIRMWARE_VERSION);
    display.showMessage("Checking GitHub", "For FW Updates");

    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

    if (http.begin(client, GITHUB_VERSION_URL))
    {
        int httpCode = http.GET();
        if (httpCode == HTTP_CODE_OK)
        {
            String remoteVersion = http.getString();
            remoteVersion.trim();

            Serial.printf("GitHub Remote Version: '%s'\n", remoteVersion.c_str());

            if (remoteVersion != FIRMWARE_VERSION && remoteVersion.length() > 0 && remoteVersion != "your-username/your-repo")
            {
                Serial.printf("New version v%s found! Auto-downloading firmware from GitHub...\n", remoteVersion.c_str());
                char line2[17];
                snprintf(line2, sizeof(line2), "v%s -> v%s", FIRMWARE_VERSION, remoteVersion.c_str());
                display.showMessage("New FW Found!", line2);
                delay(1500);
                http.end();
                performGitHubOTA(GITHUB_FIRMWARE_URL);
                return;
            }
            else
            {
                Serial.println("Firmware is up to date.");
                display.showMessage("Firmware Up2Date", "No New Version");
                delay(2000);
            }
        }
        else
        {
            Serial.printf("Version check HTTP result: %d\n", httpCode);
            display.showMessage("Check Failed!", "HTTP Error");
            delay(1500);
        }
        http.end();
    }
    else
    {
        Serial.println("Unable to connect to GitHub version URL.");
        display.showMessage("Check Failed!", "Connection Error");
        delay(1500);
    }
}

// Helper function for Remote GitHub OTA
void performGitHubOTA(const String& url) {
    if (!wifiConnected) {
        Serial.println("GitHub OTA: WiFi not connected!");
        display.showMessage("Update Failed!", "WiFi Disconnect");
        delay(1500);
        return;
    }
    Serial.println("----------------------------------------");
    Serial.println("Starting Remote GitHub OTA Update...");
    Serial.print("Downloading from: ");
    Serial.println(url);

    display.showMessage("Updating FW...", "Connecting...");

    WiFiClientSecure client;
    client.setInsecure(); // Bypass SSL certificate validation for GitHub
    httpUpdate.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

    // Live progress callback: displays percentage on LCD while downloading
    httpUpdate.onProgress([](int current, int total) {
        if (total > 0) {
            int percent = (current * 100) / total;
            char line2[17];
            snprintf(line2, sizeof(line2), "Progress: %3d%%", percent);
            display.showMessage("Downloading...", line2);
            Serial.printf("OTA Progress: %d%%\n", percent);
        }
    });

    t_httpUpdate_return ret = httpUpdate.update(client, url);

    switch (ret) {
        case HTTP_UPDATE_FAILED:
            Serial.printf("GitHub OTA FAILED! Error (%d): %s\n", 
                          httpUpdate.getLastError(), 
                          httpUpdate.getLastErrorString().c_str());
            display.showMessage("Update Failed!", "Try Again Later");
            delay(2000);
            break;
        case HTTP_UPDATE_NO_UPDATES:
            Serial.println("GitHub OTA: No new firmware found.");
            display.showMessage("No New Firmware", "Up To Date");
            delay(2000);
            break;
        case HTTP_UPDATE_OK:
            Serial.println("GitHub OTA SUCCESSFUL! Rebooting into new firmware...");
            display.showMessage("Update Success!", "Rebooting...");
            delay(1500);
            break;
    }
}

// V6 = Remote GitHub OTA (Text Input / Terminal with URL, or Push Button to check update)
BLYNK_WRITE(V6) {
    String input = param.asString();
    if (input.startsWith("http://") || input.startsWith("https://")) {
        performGitHubOTA(input);
    } else if (param.asInt() == 1) {
        checkForGitHubUpdate();
    }
}

// Datastreams to create in your Blynk template:
//   V0-V2   = Relay 1, 2, 3 control switches
//   V3      = Tariff Band selection (0..4)
//   V4      = Energy Reset button
//   V5      = Voltage Calibration input
//   V10     = voltage (shared)
//   V11-V14 = current (input, sock1, sock2, sock3)
//   V16-V19 = real power (input, sock1, sock2, sock3)
//   V21-V24 = energy kWh (input, sock1, sock2, sock3)
//   V26-V29 = cost Naira (input, sock1, sock2, sock3)
//   V30     = total cost Naira (sockets only)
void pushToBlynk()
{
    Blynk.virtualWrite(V3, static_cast<int>(tariff.getBand()));
    Blynk.virtualWrite(V10, measurements[0].voltage);
    float totalNaira = 0;
    for (int c = 0; c < NUM_CHANNELS; c++)
    {
        Blynk.virtualWrite(V11 + c, measurements[c].current);
        Blynk.virtualWrite(V16 + c, measurements[c].realPower);
        Blynk.virtualWrite(V21 + c, energy[c].getkWh());
        Blynk.virtualWrite(V26 + c, energy[c].getNaira());
        if (c > 0) totalNaira += energy[c].getNaira();
    }
    Blynk.virtualWrite(V30, totalNaira);
}

void advanceScreen()
{
    currentScreen = (currentScreen + 1) % NUM_CHANNELS;
    lastScreenChangeMs = millis();
    Serial.printf("Display: switched to screen %d (%s)\n", currentScreen, currentScreen == 0 ? "Input Overview" : "Socket");
    display.show(currentScreen, measurements, energy);
}

void setup()
{
    Serial.begin(115200);

    calibration.begin();      // loads your already-saved VOLT_SCALE from flash
    tariff.begin();           // loads saved IBEDC tariff band from flash
    sensor.begin();

    for (int i = 0; i < NUM_SOCKETS; i++) relay[i].begin(relayPins[i]);
    for (int c = 0; c < NUM_CHANNELS; c++) energy[c].begin(energyKeys[c]);

    button.begin(PIN_BUTTON);
    display.begin();

    Serial.println("Energy Meter V3 -- 1/2/3=toggle  r=reset  c=calibrate  b=tariff band");
    Serial.printf("Tariff: Band %s @ N%.2f/kWh (inc. 7.5%% VAT)\n",
                  tariff.getBandName(), tariff.getRate());

    // Hold the button down while powering on to wipe saved WiFi and force
    // the setup hotspot to reopen (e.g. moving to a different location).
    delay(50);   // let the pin settle before reading it
    if (digitalRead(PIN_BUTTON) == LOW)
    {
        Serial.println("Button held at boot -- clearing saved WiFi.");
        wm.resetSettings();
    }

    WiFi.mode(WIFI_STA);
    wm.setConfigPortalTimeout(180);   // give up after 3 min and continue offline, don't block the demo forever
    wifiConnected = wm.autoConnect("EnergyMonitor-Setup");

    if (wifiConnected)
    {
        Serial.print("WiFi connected: ");
        Serial.println(WiFi.SSID());
        Blynk.config(BLYNK_AUTH_TOKEN);
        Blynk.connect();

        // Automatically check GitHub for updates on boot/Wi-Fi connect
        checkForGitHubUpdate();
    }
    else
    {
        Serial.println("No WiFi configured -- continuing offline. Sensors, relays, and the LCD still work normally.");
    }

    lastScreenChangeMs = millis();
}

unsigned long lastWatchdogMs = 0;
const unsigned long WATCHDOG_INTERVAL_MS = 15000;  // Check connection status every 15s

void loop()
{
    if (wifiConnected) Blynk.run();

    command.process(relay, energy, sensor);

    // Smart Wi-Fi & Blynk Auto-Reconnect Watchdog
    unsigned long now = millis();
    if (wifiConnected && (now - lastWatchdogMs >= WATCHDOG_INTERVAL_MS))
    {
        lastWatchdogMs = now;
        if (WiFi.status() != WL_CONNECTED)
        {
            Serial.println("Watchdog: Wi-Fi connection lost! Reconnecting...");
            WiFi.reconnect();
        }
        else if (!Blynk.connected())
        {
            Serial.println("Watchdog: Blynk disconnected! Reconnecting...");
            Blynk.connect(2000);
        }
    }

    // 1.5s Hold -> Toggle relay of current socket (or all relays if on Overview)
    if (button.heldFor(1500, 4000))
    {
        display.resetActivityTimer();
        if (currentScreen == 0)
        {
            bool anyOn = relay[0].isOn() || relay[1].isOn() || relay[2].isOn();
            for (int i = 0; i < NUM_SOCKETS; i++)
            {
                if (anyOn) relay[i].off(); else relay[i].on();
                if (Blynk.connected()) Blynk.virtualWrite(V0 + i, relay[i].isOn());
            }
            display.showMessage("All Relays", anyOn ? "Turned OFF" : "Turned ON");
            Serial.printf("Button: Toggled ALL relays -> %s\n", anyOn ? "OFF" : "ON");
        }
        else
        {
            int idx = currentScreen - 1;
            relay[idx].toggle();
            if (Blynk.connected()) Blynk.virtualWrite(V0 + idx, relay[idx].isOn());
            char title[17];
            snprintf(title, sizeof(title), "Socket %d Relay", idx + 1);
            display.showMessage(title, relay[idx].isOn() ? "Turned ON" : "Turned OFF");
            Serial.printf("Button: Toggled Socket %d Relay -> %s\n", idx + 1, relay[idx].isOn() ? "ON" : "OFF");
        }
        delay(1200);
    }

    if (button.held(4000))
    {
        display.resetActivityTimer();
        Serial.println("Button held for 4s -> Triggering GitHub Update Check!");
        display.showMessage("Hold Detected!", "Checking GitHub");
        delay(1000);
        checkForGitHubUpdate();
    }

    if (button.pressed())
    {
        display.resetActivityTimer();
        advanceScreen();               // manual jump + resets the auto-cycle timer
    }

    {
        unsigned long cycleDuration = (currentScreen == 0) ? HOME_SCREEN_MS : SOCKET_SCREEN_MS;
        if (millis() - lastScreenChangeMs >= cycleDuration) advanceScreen();
    }

    // LCD Backlight Power Saver (turns off backlight after 60s of inactivity)
    display.checkPowerSave(60000);

    if (now - lastMeasureMs >= MEASURE_INTERVAL_MS)
    {
        lastMeasureMs = now;
        measurementEngine.calculate(sensor, measurements);   // ~200ms blocking capture, see Sensor.cpp note
        for (int c = 0; c < NUM_CHANNELS; c++) energy[c].update(measurements[c].realPower);

        logger.print(measurements, energy);
        display.show(currentScreen, measurements, energy);   // refresh numbers on whatever screen is showing
    }

    if (now - lastBlynkPushMs >= BLYNK_PUSH_INTERVAL_MS)
    {
        lastBlynkPushMs = now;
        if (Blynk.connected()) pushToBlynk();
    }

    if (now - lastEnergySaveMs >= ENERGY_SAVE_INTERVAL_MS)
    {
        lastEnergySaveMs = now;
        for (int c = 0; c < NUM_CHANNELS; c++) energy[c].save();
    }
}
