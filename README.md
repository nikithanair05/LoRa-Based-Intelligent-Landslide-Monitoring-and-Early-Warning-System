# GeoSentinel — Landslide Detection & Monitoring System
### ESP32 Base Station & Real-Time IoT Web Dashboard

---

## 📌 Project Overview
**GeoSentinel** is an IoT-based landslide detection and early-warning prototype system. The remote sensor node acquires environmental data (Temperature, Humidity, Soil Moisture, Precipitation) and ground kinematic movement data (3-Axis Accelerometer and Gyroscope from MPU6050) and transmits it over **433 MHz LoRa (SX1278)**.

##  Project Demo
<p align="center">
  <a href="https://youtu.be/x0-8KgMGR_M">
    <img src="https://img.youtube.com/vi/x0-8KgMGR_M/maxresdefault.jpg" width="700" alt="Project Demo Video">
  </a>
</p>

The **Base-Station ESP32** receives the incoming LoRa packets, parses the telemetry payload, computes communication quality metrics, and serves a live, responsive web dashboard directly to connected devices over Wi-Fi without requiring external web servers or internet access.

```
+-------------------------------------------------------------+
|                     SENSOR NODE (ESP32)                     |
|  DHT22 (Temp/Hum) + Soil (ADC) + Rain (ADC) + MPU6050 (I2C) |
+-------------------------------------------------------------+
                               |
                               v (433 MHz LoRa)
+-------------------------------------------------------------+
|                   BASE STATION (ESP32)                      |
|  SX1278 LoRa Receiver + WebServer + Embedded HTML5 Engine   |
+-------------------------------------------------------------+
                               |
                               v (HTTP / WebSockets / AJAX)
+-------------------------------------------------------------+
|                     WEB DASHBOARD                           |
|  PC / Laptop / Android Phone / Tablet (Browser Interface)   |
+-------------------------------------------------------------+
```

---

## 🛠️ Hardware Pin Configuration

### Base-Station ESP32 Pinout
| Module / Component | ESP32 Pin | Details / Notes |
| :--- | :--- | :--- |
| **SX1278 SCK** | `GPIO 18` | SPI Clock |
| **SX1278 MISO** | `GPIO 19` | SPI MISO |
| **SX1278 MOSI** | `GPIO 23` | SPI MOSI |
| **SX1278 NSS / CS** | `GPIO 5` | Chip Select |
| **SX1278 RST** | `GPIO 14` | Reset Pin |
| **SX1278 DIO0** | `GPIO 26` | Packet Interrupt Pin |
| **SX1278 VCC** | `3.3V` | ⚠️ Connect to 3.3V, NOT 5V! |
| **SX1278 GND** | `GND` | Common Ground |
| **Optional Buzzer (+)** | `GPIO 25` | Optional (Disabled by default) |
| **Optional Buzzer (-)** | `GND` | Ground |

### Sensor Node Hardware Summary (For Reference)
- **DHT22**: DATA on `GPIO 4`, VCC `3.3V`
- **Soil Moisture**: AO on `GPIO 34` (Raw ADC 0–4095)
- **Rain Sensor**: AO on `GPIO 35` (Raw ADC 0–4095)
- **MPU6050**: SDA `GPIO 21`, SCL `GPIO 22` (I2C address `0x68`)
- **SX1278**: SCK `18`, MISO `19`, MOSI `23`, CS `5`, RST `14`, DIO0 `26` (433 MHz)

---

## 📦 Required Libraries

Install the following library via the **Arduino IDE Library Manager**:

1. **LoRa by Sandeep Mistry** (`LoRa` v0.8.0 or newer)
   - *How to install:* Open Arduino IDE -> Go to **Sketch** -> **Include Library** -> **Manage Libraries...** -> Search for `LoRa` by Sandeep Mistry -> Click **Install**.

*Note: `<WiFi.h>`, `<WebServer.h>`, and `<SPI.h>` are built directly into the official ESP32 Arduino core and do not need external installation.*

---

## 🚀 Setup & Upload Instructions

### 1. Arduino IDE Board Setup
1. In Arduino IDE, go to **Tools** -> **Board** -> **esp32** -> Select **ESP32 Dev Module** (or your specific ESP32 board).
2. Set **Upload Speed** to `921600` (or `115200`).
3. Set **Flash Frequency** to `80MHz`.
4. Set **Partition Scheme** to `Default 4MB with spiffs` (or `Huge APP`).
5. Select the appropriate **Port** for your connected ESP32.

### 2. Wi-Fi Configuration
Open `GeoSentinel_BaseStation.ino` and locate the Wi-Fi configuration block near line 45:
```cpp
const char* WIFI_SSID     = "YOUR_WIFI_NAME";      // Enter your Wi-Fi SSID
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";  // Enter your Wi-Fi Password
```
> **Automatic Fallback Access Point:**
> If the Base Station cannot connect to your home/lab Wi-Fi router within 15 seconds, it automatically spins up a local hotspot:
> - **SSID:** `GeoSentinel`
> - **Password:** `GeoSentinel123`
> - **IP Address:** `http://192.168.4.1`

### 3. Uploading Code to Base Station
1. Connect your ESP32 Base Station board to your computer via micro-USB / Type-C cable.
2. Click the **Upload** button (Arrow icon in Arduino IDE).
3. If the IDE displays `Connecting........_____.....`, press and hold the **BOOT** button on the ESP32 for 2 seconds until the upload begins.
4. Once completed, open the **Serial Monitor** at **115200 baud**.

---

---

## 🌐 How to Access & Connect the Live Dashboard

The GeoSentinel dashboard supports **two flexible connection modes**:
1. **Direct USB Serial Mode (Web Serial API):** Connect either the **Receiver (Base Station)** or **Transmitter (Sensor Node)** directly to your laptop via USB cable and stream real-time data into Chrome / Edge / Opera without needing Wi-Fi!
2. **Wi-Fi Web Server Mode:** ESP32 serves the dashboard over local Wi-Fi or Hotspot mode.

---

### 🔌 Option A: Direct USB Serial Mode (Recommended for Testing & Laptops)

You can plug either the **Receiver ESP32** OR the **Transmitter ESP32** (or both simultaneously in Dual Mode!) into your laptop via USB:

1. **Close Arduino Serial Monitor:**
   Ensure the Serial Monitor in the Arduino IDE is closed so the COM port is free.
2. **Open the Dashboard in your Browser:**
   - Double-click [`standalone_dashboard_preview.html`](file:///c:/Users/loq/Documents/NEHA/Project/Landslide/standalone_dashboard_preview.html) in Google Chrome, Microsoft Edge, or Opera.
3. **Select Mode & Connect:**
   - In the top toolbar, select your device mode:
     - `🔄 Auto-Detect (RX or TX)`: Automatically parses whichever node is plugged in.
     - `📻 Receiver / Base Station`: Parses LoRa radio packets received over the air with RSSI signal metrics.
     - `🌱 Transmitter / Sensor Node`: Reads live sensor telemetry (DHT22, Soil, Rain, MPU6050) directly from the sensor node.
     - `⚡ Dual USB Mode`: Connect both Receiver and Transmitter on two separate USB ports simultaneously!
   - Ensure the baud rate is set to **115200 Baud**.
   - Click **"Connect USB Port"** and choose your ESP32 from the browser prompt.
4. **Live Inspection & Terminal:**
   - Sensor gauges, rolling graphs, seismograph, and the built-in **Live USB Serial Terminal** will immediately begin streaming live packets!

---

### 📶 Option B: Wi-Fi Web Server Mode (Wireless Access)

#### 1. Finding the ESP32 IP Address
1. Open the **Serial Monitor** (**115200 baud**).
2. Press the **EN / RST** button on the ESP32 Base Station.
3. The Serial Monitor will show:
   ```text
   [WiFi] Connected successfully!
   [WiFi] IP address: http://192.168.1.105
   Open this IP address in your browser to view the dashboard.
   ```

#### 2. Opening on Laptop / Desktop
1. Ensure your laptop is connected to the same Wi-Fi network as the ESP32.
2. Open Chrome, Edge, Safari, or Firefox and type `http://192.168.1.105`.

#### 3. Opening on Android / iOS Phone
1. Connect your phone to the same Wi-Fi network (or connect directly to the `GeoSentinel` Hotspot `GeoSentinel123` at `http://192.168.4.1`).
2. Open your mobile browser. The dashboard adapts cleanly to mobile screens.

---

## 📊 Telemetry Packet Parsing & Display

### Incoming Packet Format
```text
TEMP:34.7,HUM:69.2,SOIL:4095,RAIN:140,AX:-1696,AY:-172,AZ:16832,GX:389,GY:141,GZ:34
```

### Display Mapping
| Telemetry Field | Display Header | Unit / Details |
| :--- | :--- | :--- |
| `TEMP` | **Temperature** | `34.7 °C` (Calibrated DHT22) |
| `HUM` | **Humidity** | `69.2 %` (Calibrated DHT22) |
| `SOIL` | **Soil Moisture** | `4095` (*RAW ADC 0–4095, Uncalibrated*) |
| `RAIN` | **Rain Sensor** | `140` (*RAW ADC 0–4095, Uncalibrated*) |
| `AX, AY, AZ` | **Accelerometer** | `-1696, -172, 16832` (*RAW 16-bit registers*) |
| `GX, GY, GZ` | **Gyroscope** | `389, 141, 34` (*RAW 16-bit registers*) |
| `RSSI` | **Signal Strength** | `-115 dBm` (*Categorized as Weak/Good/Excellent*) |

---

## 🧠 Code Architecture Walkthrough

1. **`dashboard.h` (PROGMEM HTML/CSS/JS Engine)**:
   - Stores the entire dashboard client code inside the ESP32's Flash program memory (`PROGMEM`), leaving the SRAM free for network buffers and LoRa operations.
   - Includes a custom, self-contained HTML5 Canvas chart renderer that requires **zero external internet CDNs**. This ensures charts render instantly even on offline Wi-Fi networks.

2. **`parsePacket()`**:
   - Safely parses incoming comma-separated, colon-delimited string tokens (`KEY:VALUE`).
   - Resistant to missing fields or corrupted packets; does not crash or trigger kernel panics.

3. **`handleApiData()` (`/api/data`)**:
   - Returns a structured JSON object containing all live sensor variables, calculated elapsed time since last packet, packet counter, RSSI, and online status.
   - Polled by the frontend JavaScript every 1000ms.

4. **Non-blocking Loop**:
   - `loop()` calls `server.handleClient()` and `checkLoRa()` on every cycle with zero `delay()` calls, ensuring the web server is always responsive while continuously monitoring radio packets.

---

## 🔧 Troubleshooting Guide

### 1. Wi-Fi Not Connecting
- **Symptom:** Serial Monitor prints `.....` and falls back to AP mode.
- **Solution:**
  1. Verify `WIFI_SSID` and `WIFI_PASSWORD` are spelled correctly (Wi-Fi names are case-sensitive).
  2. ESP32 only supports **2.4 GHz Wi-Fi** bands. Ensure your router's 2.4 GHz channel is active.
  3. You can still connect directly to the ESP32's hotspot `GeoSentinel` (password `GeoSentinel123`) and open `http://192.168.4.1`.

### 2. Dashboard Not Opening in Browser
- **Symptom:** Browser gives "Connection Timed Out" or "Site Can't Be Reached".
- **Solution:**
  1. Make sure your phone/laptop is on the **same Wi-Fi network** as the ESP32.
  2. Ensure you type `http://` (not `https://`), e.g., `http://192.168.1.105`.
  3. Verify that your router is not configured with "AP Isolation / Client Isolation" enabled.

### 3. LoRa Packets Not Received
- **Symptom:** Serial Monitor stays quiet and dashboard shows "WAITING DATA".
- **Solution:**
  1. Verify the LoRa wiring on both nodes: `SCK -> 18`, `MISO -> 19`, `MOSI -> 23`, `NSS -> 5`, `RST -> 14`, `DIO0 -> 26`.
  2. Ensure SX1278 is powered with stable **3.3V** power.
  3. Confirm both transmitter and receiver use **433E6** (433 MHz).
  4. Ensure antennas are securely attached to the SX1278 modules.

### 4. Weak RSSI (-115 dBm to -125 dBm)
- **Explanation:** RSSI is solely a measure of radio frequency signal strength and is normal over longer distances or indoors with obstacles.
- **Solution:** Add external antennas, reduce physical obstructions, or adjust LoRa spreading factor on the transmitter if necessary.

### 5. ESP32 Constant Resets / Brownout Detector
- **Symptom:** Serial Monitor repeatedly prints `Brownout detector was triggered`.
- **Solution:** LoRa transmissions and Wi-Fi bursts draw peak currents. Use a dedicated 5V 2A USB power supply or high-quality USB data cable.

---

## 📈 Automated Telemetry Dataset & Packet Updating

GeoSentinel automatically records every live sensor telemetry packet into [`geosentinel_dataset.csv`](file:///c:/Users/loq/Documents/NEHA/Project/Landslide%20-%20Copy/geosentinel_dataset.csv) across all 34 sensor channels.

### 🔄 Automatic Packet Updating Options:
1. **Automated Dataset Engine ([`dataset_updater.py`](file:///c:/Users/loq/Documents/NEHA/Project/Landslide%20-%20Copy/dataset_updater.py)):**
   - Automatically connects to live hardware (`COM3` / `COM5`) if plugged in.
   - Falls back seamlessly to physical geotechnical simulation if no hardware is connected:
   ```bash
   # Automatically update dataset from live ESP32 hardware or simulation
   python dataset_updater.py

   # Check current dataset statistics and packet count
   python dataset_updater.py --status

   # Update at custom interval (e.g., every 2 seconds)
   python dataset_updater.py --interval 2.0
   ```

2. **Serial Logger ([`serial_logger.py`](file:///c:/Users/loq/Documents/NEHA/Project/Landslide%20-%20Copy/serial_logger.py)):**
   ```bash
   python serial_logger.py
   ```

3. **Live Web Bridge ([`serial_bridge.py`](file:///c:/Users/loq/Documents/NEHA/Project/Landslide%20-%20Copy/serial_bridge.py)):**
   ```bash
   python serial_bridge.py
   ```

---

## ⚠️ Engineering Disclaimer
*GeoSentinel is an academic & engineering prototype system. Sensor readings for soil moisture, precipitation, and ground movement represent raw physical ADC and register values. In-situ geological calibration and geotechnical verification are required before utilizing the system for life-critical early warning.*
