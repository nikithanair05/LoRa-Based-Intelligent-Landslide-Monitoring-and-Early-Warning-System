/*
  ====================================================================================
  GeoSentinel — Landslide Detection & Monitoring System
  Base Station ESP32 Receiver & Live Web Dashboard Server
  ====================================================================================

  Description:
    This firmware runs on the Base-Station ESP32. It receives real-time telemetry
    packets from the GeoSentinel remote sensor node over 433 MHz LoRa (SX1278),
    parses the multi-sensor payload, tracks communication signal quality (RSSI),
    and hosts a modern, responsive, zero-dependency live web dashboard accessible
    from any browser on laptop, desktop, tablet, or smartphone.

  Hardware Connections (Base-Station ESP32):
    - SX1278 LoRa Module:
        SCK   -> GPIO 18
        MISO  -> GPIO 19
        MOSI  -> GPIO 23
        NSS/CS-> GPIO 5
        RST   -> GPIO 14
        DIO0  -> GPIO 26
        VCC   -> 3.3V
        GND   -> GND
    - Optional Buzzer:
        Buzzer + -> GPIO 25 (Controlled via BUZZER_ENABLED flag)
        Buzzer - -> GND

  LoRa Packet Format Received:
    TEMP:<val>,HUM:<val>,SOIL:<val>,RAIN:<val>,AX:<val>,AY:<val>,AZ:<val>,GX:<val>,GY:<val>,GZ:<val>

  Libraries Required (Install via Arduino IDE Library Manager):
    - "LoRa" by Sandeep Mistry (v0.8.0 or newer)
    - Built-in ESP32 libraries: <WiFi.h>, <WebServer.h>, <SPI.h>

  ====================================================================================
*/

#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include <WiFi.h>
#include <WebServer.h>
#include "dashboard.h"

// ====================================================================
// 1. USER CONFIGURATION — WIFI SETTINGS
// ====================================================================
// Replace with your local Wi-Fi router credentials:
const char* WIFI_SSID     = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// Fallback Access Point (Used automatically if router Wi-Fi fails to connect)
const char* AP_SSID       = "GeoSentinel";
const char* AP_PASSWORD   = "GeoSentinel123";
const int   WIFI_TIMEOUT_SECS = 15;  // Wait up to 15 seconds for router connection

// ====================================================================
// 2. HARDWARE PIN DEFINITIONS
// ====================================================================
// SX1278 LoRa Pins (DO NOT CHANGE)
#define LORA_SCK       18
#define LORA_MISO      19
#define LORA_MOSI      23
#define LORA_SS        5
#define LORA_RST       14
#define LORA_DIO0      26
#define LORA_BAND      433E6  // 433 MHz

// Optional Alert Buzzer
#define BUZZER_PIN     25
#define BUZZER_ENABLED false  // Set to 'true' to enable physical buzzer on alerts

// ====================================================================
// 3. THRESHOLDS & TIMEOUT CONFIGURATION (CALIBRATION SECTION)
// ====================================================================
// Note: Soil and Rain readings are RAW ADC (0 - 4095).
// Thresholds below are placeholder baselines for prototype demonstration.
#define NODE_TIMEOUT_MS        15000   // 15 seconds without packet = Warning/Offline
#define SOIL_WET_THRESHOLD     3500    // Raw ADC threshold for saturated soil
#define RAIN_DETECT_THRESHOLD  500     // Raw ADC threshold for rainfall detection
#define ACCEL_TILT_THRESHOLD   22000   // Raw accelerometer magnitude threshold
#define GYRO_MOTION_THRESHOLD  1200    // Raw gyroscope rate threshold

// ====================================================================
// 4. DATA STRUCTURES & GLOBAL STATE
// ====================================================================
struct SensorData {
  // Environmental Data
  float temperature       = 0.0;
  float humidity          = 0.0;
  int   soil              = 0;
  int   rain              = 0;

  // Ground Movement (MPU6050 Raw Register Values)
  int   ax                = 0;
  int   ay                = 0;
  int   az                = 0;
  int   gx                = 0;
  int   gy                = 0;
  int   gz                = 0;

  // LoRa Telemetry Diagnostics
  int   rssi              = -115;
  int   packetLength      = 0;
  unsigned long packetCount = 0;
  unsigned long lastPacketTime = 0;
  bool  hasData           = false;
  String rawPacket        = "";
};

SensorData telemetry;
WebServer server(80);

// Non-blocking Buzzer State Variables
unsigned long buzzerLastToggle = 0;
bool buzzerState = false;
int buzzerBeepCount = 0;

// ====================================================================
// 5. FUNCTION DECLARATIONS
// ====================================================================
void setupWiFi();
void setupLoRa();
void checkLoRa();
bool parsePacket(const String& payload, int rssi, int len);
void handleBuzzer();
void triggerBuzzerAlert(int beeps);
String calculateRiskAssessment();
String generateAlertsJson();

// Web Server Route Handlers
void handleRoot();
void handleApiData();
void handleApiStatus();
void handleNotFound();

// ====================================================================
// 6. SETUP ROUTINE
// ====================================================================
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("=================================================");
  Serial.println("   GeoSentinel — Base Station Starting Up...    ");
  Serial.println("   Landslide Detection & Monitoring System       ");
  Serial.println("=================================================");

  // 1. Initialize Buzzer pin (if enabled)
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // 2. Initialize LoRa SX1278 Receiver
  setupLoRa();

  // 3. Connect to Wi-Fi or initialize Soft-AP fallback
  setupWiFi();

  // 4. Setup Web Server Endpoints
  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/data", HTTP_GET, handleApiData);
  server.on("/api/status", HTTP_GET, handleApiStatus);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("[HTTP] Web server started successfully on port 80.");
  Serial.println("=================================================");
  Serial.println("Listening for incoming 433 MHz LoRa packets...\n");
}

// ====================================================================
// 7. MAIN LOOP (NON-BLOCKING)
// ====================================================================
void loop() {
  // 1. Handle incoming HTTP client requests (Web Dashboard)
  server.handleClient();

  // 2. Poll LoRa receiver for incoming radio packets
  checkLoRa();

  // 3. Manage non-blocking buzzer tones
  handleBuzzer();
}

// ====================================================================
// 8. LORA INITIALIZATION & RECEIVE HANDLER
// ====================================================================
void setupLoRa() {
  Serial.println("[LoRa] Initializing SX1278 transceiver on 433 MHz...");

  // Assign SPI and control pins
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

  if (!LoRa.begin(LORA_BAND)) {
    Serial.println("[LoRa ERROR] SX1278 initialization failed! Check wiring & 3.3V power.");
    while (1) {
      delay(1000); // Halt if LoRa hardware is missing
    }
  }

  // Optional: Set LoRa sync word to match transmitter (Default: 0x12)
  LoRa.setSyncWord(0x12);
  LoRa.receive(); // Put LoRa into continuous receive mode

  Serial.println("[LoRa] SX1278 initialized and listening on 433.00 MHz.");
}

void checkLoRa() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String payload = "";
    while (LoRa.available()) {
      payload += (char)LoRa.read();
    }

    int rssi = LoRa.packetRssi();

    // Print raw packet to Serial Monitor
    Serial.println("=================================");
    Serial.println("DATA RECEIVED");
    Serial.println("=================================");
    Serial.println(payload);
    Serial.print("RSSI: ");
    Serial.print(rssi);
    Serial.println(" dBm");
    Serial.print("Packet length: ");
    Serial.println(packetSize);
    Serial.println("=================================");

    // Parse and store the values safely
    if (parsePacket(payload, rssi, packetSize)) {
      // Print nicely formatted parsed values for debugging
      Serial.println("Parsed values:");
      Serial.print("Temperature: "); Serial.println(telemetry.temperature);
      Serial.print("Humidity:    "); Serial.println(telemetry.humidity);
      Serial.print("Soil:        "); Serial.println(telemetry.soil);
      Serial.print("Rain:        "); Serial.println(telemetry.rain);
      Serial.print("AX:          "); Serial.println(telemetry.ax);
      Serial.print("AY:          "); Serial.println(telemetry.ay);
      Serial.print("AZ:          "); Serial.println(telemetry.az);
      Serial.print("GX:          "); Serial.println(telemetry.gx);
      Serial.print("GY:          "); Serial.println(telemetry.gy);
      Serial.print("GZ:          "); Serial.println(telemetry.gz);
      Serial.println();
    } else {
      Serial.println("[LoRa WARN] Malformed packet received. Ignored.\n");
    }
  }
}

// ====================================================================
// 9. ROBUST PACKET PARSER
// Expected format:
// TEMP:34.7,HUM:69.2,SOIL:4095,RAIN:140,AX:-1696,AY:-172,AZ:16832,GX:389,GY:141,GZ:34
// ====================================================================
bool parsePacket(const String& payload, int rssi, int len) {
  if (payload.length() == 0) return false;

  telemetry.rawPacket = payload;
  telemetry.rssi = rssi;
  telemetry.packetLength = len;
  telemetry.lastPacketTime = millis();
  telemetry.packetCount++;
  telemetry.hasData = true;

  int startIndex = 0;
  int commaIndex = 0;
  int tokensFound = 0;

  while (startIndex < payload.length()) {
    commaIndex = payload.indexOf(',', startIndex);
    String token;

    if (commaIndex == -1) {
      token = payload.substring(startIndex);
      startIndex = payload.length();
    } else {
      token = payload.substring(startIndex, commaIndex);
      startIndex = commaIndex + 1;
    }

    token.trim();
    int colonIndex = token.indexOf(':');
    if (colonIndex != -1) {
      String key = token.substring(0, colonIndex);
      String val = token.substring(colonIndex + 1);
      key.trim();
      val.trim();

      if (key.equalsIgnoreCase("TEMP")) {
        telemetry.temperature = val.toFloat();
        tokensFound++;
      } else if (key.equalsIgnoreCase("HUM")) {
        telemetry.humidity = val.toFloat();
        tokensFound++;
      } else if (key.equalsIgnoreCase("SOIL")) {
        telemetry.soil = val.toInt();
        tokensFound++;
      } else if (key.equalsIgnoreCase("RAIN")) {
        telemetry.rain = val.toInt();
        tokensFound++;
      } else if (key.equalsIgnoreCase("AX")) {
        telemetry.ax = val.toInt();
        tokensFound++;
      } else if (key.equalsIgnoreCase("AY")) {
        telemetry.ay = val.toInt();
        tokensFound++;
      } else if (key.equalsIgnoreCase("AZ")) {
        telemetry.az = val.toInt();
        tokensFound++;
      } else if (key.equalsIgnoreCase("GX")) {
        telemetry.gx = val.toInt();
        tokensFound++;
      } else if (key.equalsIgnoreCase("GY")) {
        telemetry.gy = val.toInt();
        tokensFound++;
      } else if (key.equalsIgnoreCase("GZ")) {
        telemetry.gz = val.toInt();
        tokensFound++;
      }
    }
  }

  return (tokensFound >= 4); // Valid if at least primary fields parsed
}

// ====================================================================
// 10. MODULAR RISK CALCULATION & ALERTS
// ====================================================================
String calculateRiskAssessment() {
  // IMPORTANT: The sensors have not yet been scientifically calibrated
  // for a finalized landslide prediction model. Therefore, the system
  // operates in "MONITORING" status to maintain engineering integrity.
  return "MONITORING";
}

// ====================================================================
// 11. WEB SERVER ROUTE HANDLERS
// ====================================================================

// Serve Dashboard HTML from PROGMEM
void handleRoot() {
  server.send(200, "text/html", DASHBOARD_HTML);
}

// REST API: Live Sensor Data (JSON)
void handleApiData() {
  unsigned long now = millis();
  unsigned long timeSinceLastPacket = telemetry.hasData ? (now - telemetry.lastPacketTime) : 0;
  bool nodeOnline = telemetry.hasData && (timeSinceLastPacket <= NODE_TIMEOUT_MS);

  // Build JSON response
  String json = "{";
  json += "\"hasData\":" + String(telemetry.hasData ? "true" : "false") + ",";
  json += "\"temperature\":" + String(telemetry.temperature, 1) + ",";
  json += "\"humidity\":" + String(telemetry.humidity, 1) + ",";
  json += "\"soil\":" + String(telemetry.soil) + ",";
  json += "\"rain\":" + String(telemetry.rain) + ",";
  json += "\"ax\":" + String(telemetry.ax) + ",";
  json += "\"ay\":" + String(telemetry.ay) + ",";
  json += "\"az\":" + String(telemetry.az) + ",";
  json += "\"gx\":" + String(telemetry.gx) + ",";
  json += "\"gy\":" + String(telemetry.gy) + ",";
  json += "\"gz\":" + String(telemetry.gz) + ",";
  json += "\"rssi\":" + String(telemetry.rssi) + ",";
  json += "\"packetLength\":" + String(telemetry.packetLength) + ",";
  json += "\"packetCount\":" + String(telemetry.packetCount) + ",";
  json += "\"lastUpdate\":" + String(telemetry.lastPacketTime) + ",";
  json += "\"timeSinceLastPacket\":" + String(timeSinceLastPacket) + ",";
  json += "\"nodeOnline\":" + String(nodeOnline ? "true" : "false") + ",";
  json += "\"riskLevel\":\"" + calculateRiskAssessment() + "\"";
  json += "}";

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", json);
}

// REST API: System Diagnostic Status
void handleApiStatus() {
  String json = "{";
  json += "\"uptimeSec\":" + String(millis() / 1000) + ",";
  json += "\"freeHeap\":" + String(ESP.getFreeHeap()) + ",";
  json += "\"wifiMode\":\"" + String(WiFi.getMode() == WIFI_STA ? "Station" : "AccessPoint") + "\",";
  json += "\"ip\":\"" + (WiFi.getMode() == WIFI_STA ? WiFi.localIP().toString() : WiFi.softAPIP().toString()) + "\",";
  json += "\"totalPackets\":" + String(telemetry.packetCount);
  json += "}";

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", json);
}

void handleNotFound() {
  server.send(404, "text/plain", "GeoSentinel 404: Not Found");
}

// ====================================================================
// 12. WIFI & ACCESS POINT MANAGEMENT
// ====================================================================
void setupWiFi() {
  WiFi.mode(WIFI_STA);
  Serial.print("[WiFi] Connecting to network: ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && 
         millis() - startAttemptTime < (WIFI_TIMEOUT_SECS * 1000)) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("[WiFi] Connected successfully!");
    Serial.print("[WiFi] IP address: http://");
    Serial.println(WiFi.localIP());
    Serial.println("Open this IP address in your browser to view the dashboard.");
  } else {
    Serial.println("[WiFi WARN] Could not connect to router. Starting Fallback Access Point...");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASSWORD);

    Serial.println("[WiFi AP] Access Point started.");
    Serial.print("[WiFi AP] SSID: ");
    Serial.println(AP_SSID);
    Serial.print("[WiFi AP] Password: ");
    Serial.println(AP_PASSWORD);
    Serial.print("[WiFi AP] Open browser at: http://");
    Serial.println(WiFi.softAPIP());
  }
}

// ====================================================================
// 13. OPTIONAL NON-BLOCKING BUZZER CONTROLLER
// ====================================================================
void handleBuzzer() {
  #if BUZZER_ENABLED
    if (buzzerBeepCount > 0) {
      unsigned long currentMillis = millis();
      if (currentMillis - buzzerLastToggle >= 120) { // 120ms beep cadence
        buzzerLastToggle = currentMillis;
        buzzerState = !buzzerState;
        digitalWrite(BUZZER_PIN, buzzerState ? HIGH : LOW);
        if (!buzzerState) {
          buzzerBeepCount--;
        }
      }
    } else {
      digitalWrite(BUZZER_PIN, LOW);
    }
  #endif
}

void triggerBuzzerAlert(int beeps) {
  #if BUZZER_ENABLED
    buzzerBeepCount = beeps;
    buzzerLastToggle = millis();
  #endif
}
