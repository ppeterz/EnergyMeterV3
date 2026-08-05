#include "WebDashboard.h"
#include "Tariff.h"
#include "Calibration.h"
#include "DSP.h"
#include "Sensor.h"

extern Sensor sensor;
extern DSP dsp;

WebDashboard webDashboard;

static Measurement* currentMeasurements = nullptr;
static Energy* currentEnergy = nullptr;
static Relay* currentRelays = nullptr;

static const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Energy Meter V3 Dashboard</title>
    <style>
        :root {
            --bg-color: #0f172a;
            --card-bg: rgba(30, 41, 59, 0.7);
            --border-color: rgba(255, 255, 255, 0.1);
            --accent-cyan: #06b6d4;
            --accent-green: #10b981;
            --accent-amber: #f59e0b;
            --accent-red: #ef4444;
            --text-primary: #f8fafc;
            --text-secondary: #94a3b8;
        }
        * { box-sizing: border-box; margin: 0; padding: 0; font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; }
        body { background: var(--bg-color); color: var(--text-primary); padding: 16px; min-height: 100vh; }
        .container { max-width: 1000px; margin: 0 auto; }
        
        header { display: flex; justify-content: space-between; align-items: center; padding: 16px 0; border-bottom: 1px solid var(--border-color); margin-bottom: 24px; }
        .logo-title { display: flex; align-items: center; gap: 12px; }
        .logo-icon { width: 36px; height: 36px; background: linear-gradient(135deg, var(--accent-cyan), var(--accent-green)); borderRadius: 8px; display: flex; align-items: center; justify-content: center; font-weight: bold; font-size: 20px; }
        h1 { font-size: 1.4rem; font-weight: 700; }
        .subtitle { font-size: 0.85rem; color: var(--text-secondary); }
        .status-badge { padding: 6px 12px; border-radius: 20px; font-size: 0.8rem; font-weight: 600; background: rgba(16, 185, 129, 0.15); color: var(--accent-green); border: 1px solid rgba(16, 185, 129, 0.3); }

        .overview-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(220px, 1fr)); gap: 16px; margin-bottom: 24px; }
        .card { background: var(--card-bg); border: 1px solid var(--border-color); border-radius: 14px; padding: 18px; backdrop-filter: blur(10px); }
        .card-header { font-size: 0.85rem; color: var(--text-secondary); margin-bottom: 8px; text-transform: uppercase; letter-spacing: 0.5px; }
        .card-value { font-size: 1.8rem; font-weight: 700; color: var(--text-primary); }
        .unit { font-size: 1rem; font-weight: 400; color: var(--text-secondary); margin-left: 4px; }
        
        .section-title { font-size: 1.1rem; margin: 24px 0 14px; color: var(--accent-cyan); display: flex; align-items: center; gap: 8px; }
        
        .sockets-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(280px, 1fr)); gap: 16px; margin-bottom: 24px; }
        .socket-card { background: var(--card-bg); border: 1px solid var(--border-color); border-radius: 14px; padding: 20px; position: relative; }
        .socket-top { display: flex; justify-content: space-between; align-items: center; margin-bottom: 16px; }
        .socket-name { font-weight: 600; font-size: 1.1rem; }
        
        /* Switch Toggle */
        .switch { position: relative; display: inline-block; width: 50px; height: 26px; }
        .switch input { opacity: 0; width: 0; height: 0; }
        .slider { position: absolute; cursor: pointer; top: 0; left: 0; right: 0; bottom: 0; background-color: #334155; transition: .3s; border-radius: 26px; }
        .slider:before { position: absolute; content: ""; height: 18px; width: 18px; left: 4px; bottom: 4px; background-color: white; transition: .3s; border-radius: 50%; }
        input:checked + .slider { background-color: var(--accent-green); }
        input:checked + .slider:before { transform: translateX(24px); }

        .metrics-row { display: grid; grid-template-columns: 1fr 1fr; gap: 12px; margin-bottom: 12px; }
        .metric-item { background: rgba(15, 23, 42, 0.4); padding: 10px; border-radius: 8px; }
        .metric-label { font-size: 0.75rem; color: var(--text-secondary); }
        .metric-val { font-size: 1.1rem; font-weight: 600; margin-top: 2px; }

        .controls-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(280px, 1fr)); gap: 16px; }
        .btn { width: 100%; padding: 12px; border: none; border-radius: 8px; font-weight: 600; cursor: pointer; transition: background 0.2s; font-size: 0.95rem; }
        .btn-danger { background: rgba(239, 68, 68, 0.2); color: var(--accent-red); border: 1px solid rgba(239, 68, 68, 0.4); }
        .btn-danger:hover { background: rgba(239, 68, 68, 0.3); }
        .btn-primary { background: var(--accent-cyan); color: #000; font-weight: 700; }
        .btn-primary:hover { opacity: 0.9; }

        select, input[type="number"] { width: 100%; padding: 10px 12px; background: rgba(15, 23, 42, 0.6); border: 1px solid var(--border-color); border-radius: 8px; color: var(--text-primary); font-size: 0.95rem; margin-bottom: 12px; }
    </style>
</head>
<body>
    <div class="container">
        <header>
            <div class="logo-title">
                <div class="logo-icon">⚡</div>
                <div>
                    <h1>Energy Monitor V3</h1>
                    <div class="subtitle">Embedded ESP32 Local Dashboard</div>
                </div>
            </div>
            <div class="status-badge" id="conn-status">LIVE CONNECTED</div>
        </header>

        <!-- Overview Cards -->
        <div class="overview-grid">
            <div class="card">
                <div class="card-header">RMS Voltage</div>
                <div class="card-value"><span id="v-volts">0.0</span><span class="unit">V</span></div>
            </div>
            <div class="card">
                <div class="card-header">Total Input Power</div>
                <div class="card-value" style="color: var(--accent-cyan)"><span id="v-power">0</span><span class="unit">W</span></div>
            </div>
            <div class="card">
                <div class="card-header">Total Naira Cost</div>
                <div class="card-value" style="color: var(--accent-green)">₦<span id="v-cost">0.00</span></div>
            </div>
            <div class="card">
                <div class="card-header">Tariff Band</div>
                <div class="card-value" style="color: var(--accent-amber)">Band <span id="v-band">A</span></div>
                <div class="subtitle" id="v-rate">₦225.21 / kWh</div>
            </div>
        </div>

        <!-- Sockets Grid -->
        <div class="section-title">🔌 Socket Controls & Metering</div>
        <div class="sockets-grid">
            <!-- Socket 1 -->
            <div class="socket-card">
                <div class="socket-top">
                    <span class="socket-name">Socket 1</span>
                    <label class="switch">
                        <input type="checkbox" id="relay-1" onchange="toggleRelay(1, this.checked)">
                        <span class="slider"></span>
                    </label>
                </div>
                <div class="metrics-row">
                    <div class="metric-item"><div class="metric-label">Current</div><div class="metric-val"><span id="s1-i">0.00</span> A</div></div>
                    <div class="metric-item"><div class="metric-label">Power</div><div class="metric-val"><span id="s1-p">0</span> W</div></div>
                </div>
                <div class="metrics-row">
                    <div class="metric-item"><div class="metric-label">Energy</div><div class="metric-val"><span id="s1-kwh">0.000</span> kWh</div></div>
                    <div class="metric-item"><div class="metric-label">Cost</div><div class="metric-val" style="color: var(--accent-green)">₦<span id="s1-cost">0.00</span></div></div>
                </div>
            </div>

            <!-- Socket 2 -->
            <div class="socket-card">
                <div class="socket-top">
                    <span class="socket-name">Socket 2</span>
                    <label class="switch">
                        <input type="checkbox" id="relay-2" onchange="toggleRelay(2, this.checked)">
                        <span class="slider"></span>
                    </label>
                </div>
                <div class="metrics-row">
                    <div class="metric-item"><div class="metric-label">Current</div><div class="metric-val"><span id="s2-i">0.00</span> A</div></div>
                    <div class="metric-item"><div class="metric-label">Power</div><div class="metric-val"><span id="s2-p">0</span> W</div></div>
                </div>
                <div class="metrics-row">
                    <div class="metric-item"><div class="metric-label">Energy</div><div class="metric-val"><span id="s2-kwh">0.000</span> kWh</div></div>
                    <div class="metric-item"><div class="metric-label">Cost</div><div class="metric-val" style="color: var(--accent-green)">₦<span id="s2-cost">0.00</span></div></div>
                </div>
            </div>

            <!-- Socket 3 -->
            <div class="socket-card">
                <div class="socket-top">
                    <span class="socket-name">Socket 3</span>
                    <label class="switch">
                        <input type="checkbox" id="relay-3" onchange="toggleRelay(3, this.checked)">
                        <span class="slider"></span>
                    </label>
                </div>
                <div class="metrics-row">
                    <div class="metric-item"><div class="metric-label">Current</div><div class="metric-val"><span id="s3-i">0.00</span> A</div></div>
                    <div class="metric-item"><div class="metric-label">Power</div><div class="metric-val"><span id="s3-p">0</span> W</div></div>
                </div>
                <div class="metrics-row">
                    <div class="metric-item"><div class="metric-label">Energy</div><div class="metric-val"><span id="s3-kwh">0.000</span> kWh</div></div>
                    <div class="metric-item"><div class="metric-label">Cost</div><div class="metric-val" style="color: var(--accent-green)">₦<span id="s3-cost">0.00</span></div></div>
                </div>
            </div>
        </div>

        <!-- Controls Section -->
        <div class="section-title">⚙️ Meter Settings & Calibration</div>
        <div class="controls-grid">
            <div class="card">
                <div class="card-header">Change Tariff Band</div>
                <select id="tariff-select" onchange="changeBand(this.value)">
                    <option value="0">Band A (₦225.21 / kWh)</option>
                    <option value="1">Band B (₦67.73 / kWh)</option>
                    <option value="2">Band C (₦53.75 / kWh)</option>
                    <option value="3">Band D (₦46.23 / kWh)</option>
                    <option value="4">Band E (₦43.00 / kWh)</option>
                </select>
                <div class="subtitle">Saves band to ESP32 flash immediately</div>
            </div>

            <div class="card">
                <div class="card-header">Calibrate Voltage</div>
                <input type="number" id="cal-v" placeholder="Multimeter Voltage e.g. 225.0" step="0.1">
                <button class="btn btn-primary" onclick="calibrateVolt()">Calibrate Voltage</button>
            </div>

            <div class="card">
                <div class="card-header">Reset Energy Counters</div>
                <div class="subtitle" style="margin-bottom: 12px">Resets all kWh and Naira totals to zero</div>
                <button class="btn btn-danger" onclick="resetEnergy()">Reset All Counters</button>
            </div>
        </div>
    </div>

    <script>
        function updateData() {
            fetch('/api/data')
                .then(res => res.json())
                .then(d => {
                    document.getElementById('v-volts').innerText = d.voltage.toFixed(1);
                    document.getElementById('v-power').innerText = Math.round(d.powerIn);
                    document.getElementById('v-cost').innerText = d.totalNaira.toFixed(2);
                    document.getElementById('v-band').innerText = d.bandName;
                    document.getElementById('v-rate').innerText = '₦' + d.bandRate.toFixed(2) + ' / kWh';
                    document.getElementById('tariff-select').value = d.band;

                    // Sockets
                    for (let i = 1; i <= 3; i++) {
                        let s = d.sockets[i-1];
                        document.getElementById(`s${i}-i`).innerText = s.current.toFixed(2);
                        document.getElementById(`s${i}-p`).innerText = Math.round(s.power);
                        document.getElementById(`s${i}-kwh`).innerText = s.kwh.toFixed(3);
                        document.getElementById(`s${i}-cost`).innerText = s.naira.toFixed(2);
                        document.getElementById(`relay-${i}`).checked = s.relay;
                    }
                })
                .catch(err => console.error(err));
        }

        function toggleRelay(id, state) {
            fetch(`/api/relay?id=${id}&state=${state ? 1 : 0}`, { method: 'POST' });
        }

        function changeBand(b) {
            fetch(`/api/tariff?band=${b}`, { method: 'POST' });
        }

        function resetEnergy() {
            if (confirm('Are you sure you want to reset all energy counters?')) {
                fetch('/api/reset', { method: 'POST' }).then(() => updateData());
            }
        }

        function calibrateVolt() {
            let v = parseFloat(document.getElementById('cal-v').value);
            if (v > 50 && v < 350) {
                fetch(`/api/calibrate?v=${v}`, { method: 'POST' })
                    .then(res => res.text())
                    .then(msg => alert(msg));
            } else {
                alert('Please enter a valid AC Voltage (50V - 350V)');
            }
        }

        setInterval(updateData, 1000);
        updateData();
    </script>
</body>
</html>
)rawliteral";

void WebDashboard::begin()
{
    server.on("/", HTTP_GET, [this]() {
        server.send_P(200, "text/html", INDEX_HTML);
    });

    server.on("/api/data", HTTP_GET, [this]() {
        if (!currentMeasurements || !currentEnergy || !currentRelays) {
            server.send(500, "application/json", "{}");
            return;
        }

        float totalNaira = 0;
        for (int c = 1; c <= NUM_SOCKETS; c++) totalNaira += currentEnergy[c].getNaira();

        String json = "{";
        json += "\"voltage\":" + String(currentMeasurements[0].voltage, 1) + ",";
        json += "\"powerIn\":" + String(currentMeasurements[0].realPower, 1) + ",";
        json += "\"totalNaira\":" + String(totalNaira, 2) + ",";
        json += "\"band\":" + String(static_cast<int>(tariff.getBand())) + ",";
        json += "\"bandName\":\"" + String(tariff.getBandName()) + "\",";
        json += "\"bandRate\":" + String(tariff.getRate(), 2) + ",";
        json += "\"sockets\":[";

        for (int i = 0; i < NUM_SOCKETS; i++) {
            int c = i + 1;
            json += "{";
            json += "\"id\":" + String(c) + ",";
            json += "\"current\":" + String(currentMeasurements[c].current, 2) + ",";
            json += "\"power\":" + String(currentMeasurements[c].realPower, 1) + ",";
            json += "\"kwh\":" + String(currentEnergy[c].getkWh(), 4) + ",";
            json += "\"naira\":" + String(currentEnergy[c].getNaira(), 2) + ",";
            json += "\"relay\":" + String(currentRelays[i].isOn() ? "true" : "false");
            json += "}";
            if (i < NUM_SOCKETS - 1) json += ",";
        }

        json += "]}";
        server.send(200, "application/json", json);
    });

    server.on("/api/relay", HTTP_POST, [this]() {
        if (server.hasArg("id") && server.hasArg("state")) {
            int id = server.arg("id").toInt();
            int state = server.arg("state").toInt();
            if (id >= 1 && id <= NUM_SOCKETS && currentRelays) {
                state ? currentRelays[id - 1].on() : currentRelays[id - 1].off();
            }
        }
        server.send(200, "text/plain", "OK");
    });

    server.on("/api/tariff", HTTP_POST, [this]() {
        if (server.hasArg("band")) {
            int b = server.arg("band").toInt();
            if (b >= 0 && b < NUM_BANDS) {
                tariff.setBand(static_cast<TariffBand>(b));
            }
        }
        server.send(200, "text/plain", "OK");
    });

    server.on("/api/reset", HTTP_POST, [this]() {
        if (currentEnergy) {
            for (int c = 0; c < NUM_CHANNELS; c++) currentEnergy[c].reset();
        }
        server.send(200, "text/plain", "OK");
    });

    server.on("/api/calibrate", HTTP_POST, [this]() {
        if (server.hasArg("v")) {
            float knownVoltage = server.arg("v").toFloat();
            if (knownVoltage > 50.0f && knownVoltage < 350.0f) {
                sensor.capture();
                float vOffset = dsp.mean(sensor.voltageBuffer);
                float vrmsADC = dsp.rms(sensor.voltageBuffer, vOffset);
                if (vrmsADC >= 1.0f) {
                    float newScale = knownVoltage / vrmsADC;
                    calibration.setVoltScale(newScale);
                    server.send(200, "text/plain", "Voltage Calibrated Successfully!");
                    return;
                }
            }
        }
        server.send(400, "text/plain", "Calibration Failed!");
    });

    server.begin();
    Serial.println("WebDashboard: Embedded Web Server started on port 80");
}

void WebDashboard::handleClient(Relay relay[NUM_SOCKETS], Energy energy[NUM_CHANNELS], Measurement measurements[NUM_CHANNELS])
{
    currentRelays = relay;
    currentEnergy = energy;
    currentMeasurements = measurements;
    server.handleClient();
}
