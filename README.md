# ⚡ Energy Meter V3 — Smart 3-Socket Extension Box

An advanced IoT-enabled 3-socket smart extension box built on the **ESP32** microcontroller. It provides real-time AC voltage, current, real power, energy consumption (kWh), and **Nigerian Naira (₦)** cost calculations based on the **Ibadan Electricity Distribution Company (IBEDC)** Service-Based Tariff (SBT) band structure.

Developed by **Precious "Dunma"** — August 2026.

---

## 🌟 Key Features

- **🔌 Multi-Channel Real-Time Sensing**:
  - **Shared ZMPT101B** AC Voltage Sensor for mains voltage monitoring.
  - **4x ACS712 Current Sensors** tracking total input current + individual current for Socket 1, Socket 2, and Socket 3.
  - Time-aligned 4kHz AC waveform sampling (~200ms per capture) for accurate real power ($P = \frac{1}{N} \sum v \cdot i$) and power factor calculations.

- **🇳🇬 IBEDC Service-Based Tariff (SBT) Engine**:
  - Full support for all 5 IBEDC tariff bands including statutory **7.5% Value Added Tax (VAT)**:
    - **Band A** (20+ hrs/day): **₦225.21 / kWh** (Base: ₦209.50)
    - **Band B** (16+ hrs/day): **₦67.73 / kWh** (Base: ₦63.00)
    - **Band C** (12+ hrs/day): **₦53.75 / kWh** (Base: ₦50.00)
    - **Band D** (8+ hrs/day): **₦46.23 / kWh** (Base: ₦43.00)
    - **Band E** (4+ hrs/day): **₦43.00 / kWh** (Base: ₦40.00)
  - Tariff selection persists across power reboots using ESP32 NVS Flash (`Preferences`).

- **🌐 Embedded Local Web Dashboard (Hosted inside ESP32)**:
  - **100% Free & Unlimited** (No third-party cloud fees or monthly message limits).
  - Modern **Dark Mode Glassmorphism UI** served directly from ESP32 memory at `http://<ESP32-IP>`.
  - **Real-Time 1s Auto-Refresh** via JSON API (`/api/data`).
  - Interactive controls: Socket relay toggle switches, live tariff band dropdown, energy reset, and voltage calibration tool.

- **📱 Blynk IoT Cloud Integration**:
  - Full mobile dashboard support with virtual pins for live voltage, current, power, energy, Naira cost, relay switches, band selector, energy reset, and calibration input.

- **🔄 GitHub Over-The-Air (OTA) Updates**:
  - Wireless firmware updates directly from GitHub releases (`ppeterz/EnergyMeterV3`).
  - **Auto-Version Check**: Checks `version.txt` on boot or Wi-Fi connect.
  - **Live Progress Counter**: Displays real-time download percentage (`0%` → `100%`) on the 16x2 LCD display.
  - **4-Second Button Hold Trigger**: Press and hold physical button for 4 seconds to trigger update check anytime.

- **🛡️ Zero-Current Noise Suppressor**:
  - Clamps baseline ACS712 sensor noise below **80mA** or **5W** to `0.00A / 0.0W / ₦0.00`. Prevents false energy/cost accumulation on empty sockets.

- **🎯 Per-Socket Calibration & NVS Persistence**:
  - Store independent calibration scaling factors for Voltage and each individual current sensor in ESP32 non-volatile storage.

- **💡 LCD Power Saver & Multi-Gesture Button**:
  - LCD backlight auto-dims after 60s of inactivity to prolong display life and save energy.
  - **Single Push Button Gestures**:
    - **Quick Click (< 1.5s)**: Navigate LCD screens (Overview → Socket 1 → Socket 2 → Socket 3).
    - **Medium Hold (1.5s)**: Toggle relay of currently visible socket (or all sockets on Overview screen).
    - **Long Hold (4.0s)**: Check & trigger GitHub OTA update.
    - **Hold at Boot**: Wipe saved Wi-Fi and re-open `EnergyMonitor-Setup` hotspot.

- **📡 Wi-Fi Watchdog & Wi-Fi Manager**:
  - No hardcoded Wi-Fi credentials! Opens captive portal `EnergyMonitor-Setup` on first boot.
  - Non-blocking background watchdog checks and restores Wi-Fi and Blynk links every 15 seconds without freezing measurement loops.

---

## 🛠️ Hardware Requirements & Pinout Mapping

| Component | ESP32 GPIO Pin | Description |
|-----------|----------------|-------------|
| **ZMPT101B Voltage Sensor** | `GPIO 36 (VP)` | Shared mains AC voltage sensing |
| **ACS712 Current Sensor (Total Input)** | `GPIO 39 (VN)` | Total input current measurement |
| **ACS712 Current Sensor (Socket 1)** | `GPIO 35` | Socket 1 current measurement |
| **ACS712 Current Sensor (Socket 2)** | `GPIO 32` | Socket 2 current measurement |
| **ACS712 Current Sensor (Socket 3)** | `GPIO 33` | Socket 3 current measurement |
| **Relay Module (Socket 1)** | `GPIO 16` | Active LOW relay control |
| **Relay Module (Socket 2)** | `GPIO 17` | Active LOW relay control |
| **Relay Module (Socket 3)** | `GPIO 18` | Active LOW relay control |
| **Push Button** | `GPIO 13` | Multi-gesture input (internal pull-up) |
| **16x2 I2C LCD Display (SDA)** | `GPIO 21` | I2C Data line |
| **16x2 I2C LCD Display (SCL)** | `GPIO 22` | I2C Clock line |

---

## 💻 Software Dependencies & Libraries

Install the following libraries in Arduino IDE:

1. **ESP32 Board Package** (v3.x by Espressif Systems)
2. **WiFiManager** (by tzapu)
3. **Blynk** (by Volodymyr Shymanskyy)
4. **LiquidCrystal_I2C** (by Frank de Brabander)

*Built-in ESP32 core libraries used:* `WiFi`, `WebServer`, `HTTPClient`, `HTTPUpdate`, `WiFiClientSecure`, `Preferences`, `Wire`.

---

## 🚀 Flashing & First Time Setup

### 1. Configure Partition Scheme in Arduino IDE (CRITICAL for OTA)
Before uploading, set your partition scheme in Arduino IDE:
- **Tools → Partition Scheme → "Minimal SPIFFS (1.9MB APP with OTA/128KB SPIFFS)"**

### 2. Flash Firmware via USB
1. Connect your ESP32 to your computer via USB.
2. Select your ESP32 board and COM port.
3. Click **Upload**.

### 3. Connect to Wi-Fi
1. On first boot, the ESP32 opens a Wi-Fi hotspot named **`EnergyMonitor-Setup`**.
2. Connect your smartphone or laptop to `EnergyMonitor-Setup`.
3. The setup page will open automatically (or visit `192.168.4.1` in your browser).
4. Select your Wi-Fi network, enter the password, and click **Save**.

### 4. Access Local Web Dashboard
- Open any browser on your Wi-Fi network and navigate to:  
  `http://<ESP32-IP>` (e.g. `http://192.168.1.50`).

---

## 📲 Blynk Virtual Pin Mapping

| Virtual Pin | Data Type | Widget Type | Description |
|-------------|-----------|-------------|-------------|
| **V0** | Integer (0/1) | Switch | Relay Socket 1 Control |
| **V1** | Integer (0/1) | Switch | Relay Socket 2 Control |
| **V2** | Integer (0/1) | Switch | Relay Socket 3 Control |
| **V3** | Integer (0-4) | Dropdown / Segmented | IBEDC Tariff Band Selector (0=A, 1=B, 2=C, 3=D, 4=E) |
| **V4** | Integer (0/1) | Push Button | Reset All Energy Counters |
| **V5** | Double | Numeric Input | Voltage Calibration Input (e.g. 225.0) |
| **V10** | Double | Gauge / Value | Shared RMS Voltage (V) |
| **V11 – V14** | Double | Value Display | Current for Input & Sockets 1–3 (A) |
| **V16 – V19** | Double | Value Display | Real Power for Input & Sockets 1–3 (W) |
| **V21 – V24** | Double | Value Display | Energy for Input & Sockets 1–3 (kWh) |
| **V26 – V29** | Double | Value Display | Cost for Input & Sockets 1–3 (₦) |
| **V30** | Double | Value Display | Total Cost across all sockets (₦) |

---

## ⌨️ Serial Monitor Commands (115200 Baud)

| Command | Action |
|---------|--------|
| `1` / `2` / `3` | Toggle Relay 1, 2, or 3 |
| `r` | Reset all energy counters (kWh and ₦) |
| `c` | Calibrate voltage against multimeter reading (e.g. enter `c` then `225.0`) |
| `b` | Cycle IBEDC Tariff Band (A → B → C → D → E → A) |

---

## 📦 How to Create & Push GitHub OTA Updates

1. **Update Code & Version**:
   - In `Config.h`: set `#define FIRMWARE_VERSION "3.1.1"`
   - In `version.txt`: set `3.1.1`
2. **Export Binary**:
   - In Arduino IDE: **Sketch → Export Compiled Binary** (`Ctrl + Alt + S`).
3. **Copy to Root**:
   - Copy `build/esp32.esp32.esp32/EnergyMeterV3.ino.bin` to `firmware.bin` in root folder.
4. **Push to GitHub**:
   - Commit and push `firmware.bin` and `version.txt` to repository `ppeterz/EnergyMeterV3`.
5. **Trigger OTA**:
   - Hold the physical button for 4 seconds OR restart the ESP32. The LCD will show `Downloading... 0% -> 100%` and reboot into the new firmware!

---

## 📜 License & Credits

Designed & Developed by **Pelumi Peter"** — 2026.  
Open-source under the **MIT License**. Free for academic, personal, and commercial adaptation.
