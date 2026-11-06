"""
GeoSentinel — USB Serial Bridge & Local Dashboard Server
=========================================================
Connects to the GeoSentinel Receiver (Base Station ESP32) over USB Serial,
streams real-time telemetry into the interactive Web Dashboard, and logs
all incoming sensor packets to CSV for landslide dataset training.

Features:
  - Serves the live web dashboard on http://localhost:8000
  - Provides REST JSON endpoint (/api/data) compatible with all browsers
  - Automatically logs every packet to 'geosentinel_dataset.csv'
  - Automatically detects connected Receiver COM port (COM3, COM6, etc.)

Usage:
  python serial_bridge.py          # Auto-selects or prompts for COM port
  python serial_bridge.py COM6     # Connects directly to COM6
  python serial_bridge.py COM3     # Connects directly to COM3
"""

import sys
import os
import time
import math
import json
import csv
import re
import threading
import webbrowser
from http.server import HTTPServer, BaseHTTPRequestHandler
from datetime import datetime

try:
    import serial
    import serial.tools.list_ports
except ImportError:
    print("[!] pyserial is required. Install it using: pip install pyserial")
    sys.exit(1)

# ==============================================================================
# CONFIGURATION
# ==============================================================================
HTTP_PORT = 8000
BAUD_RATE = 115200
CSV_FILENAME = "geosentinel_dataset.csv"
HTML_FILENAME = "standalone_dashboard_preview.html"

CSV_HEADERS = [
    "timestamp",
    "packet_id",
    "temperature_c",
    "temperature_f",
    "humidity_pct",
    "dew_point_c",
    "soil_raw_adc",
    "soil_moisture_pct",
    "soil_condition",
    "rain_raw_adc",
    "rain_intensity_pct",
    "rain_condition",
    "accel_x_raw",
    "accel_y_raw",
    "accel_z_raw",
    "accel_x_g",
    "accel_y_g",
    "accel_z_g",
    "accel_magnitude_g",
    "gyro_x_raw",
    "gyro_y_raw",
    "gyro_z_raw",
    "gyro_x_dps",
    "gyro_y_dps",
    "gyro_z_dps",
    "gyro_magnitude_dps",
    "tilt_pitch_deg",
    "tilt_roll_deg",
    "rssi_dbm",
    "signal_quality",
    "packet_length_bytes",
    "landslide_risk_level",
    "hazard_status",
    "raw_packet"
]

# Shared Global State (Thread-safe)
telemetry_lock = threading.Lock()
telemetry_state = {
    "hasData": False,
    "packetCount": 0,
    "temperature": 0.0,
    "temperature_f": 32.0,
    "humidity": 0.0,
    "dew_point": 0.0,
    "soil": 0,
    "soil_pct": 0.0,
    "soil_raw": 4095,
    "soil_cond": "DRY",
    "rain": 0,
    "rain_pct": 0.0,
    "rain_raw": 4095,
    "rain_cond": "NO_RAIN",
    "ax": 0,
    "ay": 0,
    "az": 16384,
    "ax_g": 0.0,
    "ay_g": 0.0,
    "az_g": 1.0,
    "ax_raw": 0,
    "ay_raw": 0,
    "az_raw": 16384,
    "accel_mag": 1.0,
    "gx": 0,
    "gy": 0,
    "gz": 0,
    "gx_dps": 0.0,
    "gy_dps": 0.0,
    "gz_dps": 0.0,
    "gx_raw": 0,
    "gy_raw": 0,
    "gz_raw": 0,
    "gyro_mag": 0.0,
    "tx": 0.0,
    "ty": 0.0,
    "rssi": -85,
    "signal_quality": "GOOD",
    "packetLength": 78,
    "lastUpdate": 0,
    "nodeOnline": False,
    "source": "Receiver (USB LoRa Base Station)",
    "riskLevel": "MONITORING",
    "hazardStatus": "NORMAL_STABLE"
}

recent_packets_buffer = []
csv_packet_counter = 0

connected_port_name = "None"
is_serial_connected = False


def list_ports():
    return list(serial.tools.list_ports.comports())


def select_port(cli_arg=None):
    ports = list_ports()
    if cli_arg:
        port_name = cli_arg.upper()
        if any(p.device.upper() == port_name for p in ports):
            return port_name
        print(f"[!] Note: Specified port {port_name} will be used.")
        return port_name

    if not ports:
        print("[!] No active COM ports found. Ensure your Receiver ESP32 is plugged into USB.")
        return None

    print("\nAvailable USB COM Ports:")
    default_p = ports[0].device
    for idx, p in enumerate(ports):
        is_com3 = (p.device.upper() == "COM3")
        tag = "  --> [RECOMMENDED RECEIVER / BASE STATION]" if is_com3 else ""
        print(f"  [{idx + 1}] {p.device} -- {p.description}{tag}")
        if is_com3:
            default_p = p.device

    print(f"[OK] Selected Receiver Port: {default_p}")
    return default_p


def calc_dew_point(t, h):
    if t is None or h is None or h <= 0:
        return ""
    try:
        a = 17.27
        b = 237.7
        alpha = ((a * t) / (b + t)) + math.log(h / 100.0)
        return round((b * alpha) / (a - alpha), 2)
    except Exception:
        return ""


def parse_packet(line):
    """
    Parses incoming receiver telemetry line.
    Supports:
      Format A: TEMP:31.5,HUM:77.7,SOIL:4095,RAIN:128,AX:-1284,AY:264,AZ:16768,GX:394,GY:72,GZ:-69
      Format B: GS1,TV:1,T:26.80,H:57.10,SRAW:1922,SM:59.9,SC:MODERATE,RRAW:3821,RP:0.0,RC:NO_RAIN,MV:1,TX:1.81,TY:4.78,AXG:-0.086,AYG:0.032,AZG:1.027,GXD:3.09,GYD:0.65,GZD:-0.28
    """
    data = {}
    clean = line.strip()
    if not clean:
        return None

    # Key:Value tokens
    if ":" in clean:
        tokens = [t.strip() for t in clean.split(',') if ':' in t]
        raw_dict = {}
        for tok in tokens:
            parts = tok.split(':', 1)
            if len(parts) == 2:
                raw_dict[parts[0].strip().upper()] = parts[1].strip()

        # Temperature
        for k in ["TEMP", "T", "TEMPERATURE", "TEMP_C"]:
            if k in raw_dict:
                try: data["TEMP"] = float(raw_dict[k]); break
                except ValueError: pass

        # Humidity
        for k in ["HUM", "H", "HUMIDITY", "RH"]:
            if k in raw_dict:
                try: data["HUM"] = float(raw_dict[k]); break
                except ValueError: pass

        # Soil Moisture (% calibrated & condition)
        soil_pct = None
        soil_raw = None
        if "SM" in raw_dict:
            try: soil_pct = float(raw_dict["SM"])
            except ValueError: pass
        if "SRAW" in raw_dict:
            try: soil_raw = int(float(raw_dict["SRAW"]))
            except ValueError: pass
        elif "SOIL" in raw_dict:
            try:
                v = float(raw_dict["SOIL"])
                if v <= 100.0: soil_pct = v
                else: soil_raw = int(v)
            except ValueError: pass

        if soil_pct is None and soil_raw is not None:
            soil_pct = max(0.0, min(100.0, round(((4095 - soil_raw) / 3095.0) * 100.0, 1)))

        data["SOIL_PCT"] = round(soil_pct, 1) if soil_pct is not None else 0.0
        data["SOIL"] = data["SOIL_PCT"]
        data["SOIL_RAW"] = soil_raw if soil_raw is not None else 4095
        data["SOIL_COND"] = raw_dict.get("SC", ("DRY" if data["SOIL_PCT"] < 25 else ("MODERATE" if data["SOIL_PCT"] < 65 else ("WET" if data["SOIL_PCT"] < 85 else "SATURATED"))))

        # Rain Sensor (% calibrated & condition)
        rain_pct = None
        rain_raw = None
        if "RP" in raw_dict:
            try: rain_pct = float(raw_dict["RP"])
            except ValueError: pass
        if "RRAW" in raw_dict:
            try: rain_raw = int(float(raw_dict["RRAW"]))
            except ValueError: pass
        elif "RAIN" in raw_dict:
            try:
                v = float(raw_dict["RAIN"])
                if v <= 100.0: rain_pct = v
                else: rain_raw = int(v)
            except ValueError: pass

        if rain_pct is None and rain_raw is not None:
            rain_pct = max(0.0, min(100.0, round(((4095 - rain_raw) / 3095.0) * 100.0, 1)))

        data["RAIN_PCT"] = round(rain_pct, 1) if rain_pct is not None else 0.0
        data["RAIN"] = data["RAIN_PCT"]
        data["RAIN_RAW"] = rain_raw if rain_raw is not None else 4095
        data["RAIN_COND"] = raw_dict.get("RC", ("NO_RAIN" if data["RAIN_PCT"] < 10 else ("LIGHT_RAIN" if data["RAIN_PCT"] < 40 else ("MODERATE_RAIN" if data["RAIN_PCT"] < 75 else "HEAVY_RAIN"))))

        # Accelerometer (calibrated physical g-force & raw registers)
        ax_g, ay_g, az_g = 0.0, 0.0, 1.0
        if "AXG" in raw_dict:
            try: ax_g = float(raw_dict["AXG"])
            except ValueError: pass
        elif "AX" in raw_dict:
            try:
                v = float(raw_dict["AX"])
                ax_g = v if abs(v) <= 4.0 else v / 16384.0
            except ValueError: pass

        if "AYG" in raw_dict:
            try: ay_g = float(raw_dict["AYG"])
            except ValueError: pass
        elif "AY" in raw_dict:
            try:
                v = float(raw_dict["AY"])
                ay_g = v if abs(v) <= 4.0 else v / 16384.0
            except ValueError: pass

        if "AZG" in raw_dict:
            try: az_g = float(raw_dict["AZG"])
            except ValueError: pass
        elif "AZ" in raw_dict:
            try:
                v = float(raw_dict["AZ"])
                az_g = v if abs(v) <= 4.0 else v / 16384.0
            except ValueError: pass

        data["AX_G"] = round(ax_g, 3)
        data["AY_G"] = round(ay_g, 3)
        data["AZ_G"] = round(az_g, 3)
        data["AX"] = data["AX_G"]
        data["AY"] = data["AY_G"]
        data["AZ"] = data["AZ_G"]
        data["AX_RAW"] = int(ax_g * 16384)
        data["AY_RAW"] = int(ay_g * 16384)
        data["AZ_RAW"] = int(az_g * 16384)

        # Gyroscope (calibrated physical °/s & raw registers)
        gx_dps, gy_dps, gz_dps = 0.0, 0.0, 0.0
        if "GXD" in raw_dict:
            try: gx_dps = float(raw_dict["GXD"])
            except ValueError: pass
        elif "GX" in raw_dict:
            try:
                v = float(raw_dict["GX"])
                gx_dps = v if abs(v) <= 30.0 else v / 131.0
            except ValueError: pass

        if "GYD" in raw_dict:
            try: gy_dps = float(raw_dict["GYD"])
            except ValueError: pass
        elif "GY" in raw_dict:
            try:
                v = float(raw_dict["GY"])
                gy_dps = v if abs(v) <= 30.0 else v / 131.0
            except ValueError: pass

        if "GZD" in raw_dict:
            try: gz_dps = float(raw_dict["GZD"])
            except ValueError: pass
        elif "GZ" in raw_dict:
            try:
                v = float(raw_dict["GZ"])
                gz_dps = v if abs(v) <= 30.0 else v / 131.0
            except ValueError: pass

        data["GX_DPS"] = round(gx_dps, 2)
        data["GY_DPS"] = round(gy_dps, 2)
        data["GZ_DPS"] = round(gz_dps, 2)
        data["GX"] = data["GX_DPS"]
        data["GY"] = data["GY_DPS"]
        data["GZ"] = data["GZ_DPS"]
        data["GX_RAW"] = int(gx_dps * 131)
        data["GY_RAW"] = int(gy_dps * 131)
        data["GZ_RAW"] = int(gz_dps * 131)

        # Tilt angles in degrees
        if "TX" in raw_dict:
            try: data["TX"] = round(float(raw_dict["TX"]), 2)
            except ValueError: pass
        elif ay_g != 0 or az_g != 0:
            data["TX"] = round(math.atan2(ay_g, az_g) * 180.0 / math.pi, 2)

        if "TY" in raw_dict:
            try: data["TY"] = round(float(raw_dict["TY"]), 2)
            except ValueError: pass
        elif ay_g != 0 or az_g != 0 or ax_g != 0:
            data["TY"] = round(math.atan2(-ax_g, math.sqrt(ay_g*ay_g + az_g*az_g)) * 180.0 / math.pi, 2)

        if "RSSI" in raw_dict:
            try: data["RSSI"] = int(float(raw_dict["RSSI"]))
            except ValueError: pass

    # Pure CSV numbers format
    elif clean.count(",") >= 3:
        parts = [p.strip() for p in clean.split(",")]
        try:
            nums = [float(p) for p in parts]
            if len(nums) >= 10:
                data["TEMP"] = nums[0]
                data["HUM"] = nums[1]
                s_raw = int(nums[2])
                r_raw = int(nums[3])
                data["SOIL_RAW"] = s_raw
                data["RAIN_RAW"] = r_raw
                data["SOIL_PCT"] = max(0.0, min(100.0, round(((4095 - s_raw) / 3095.0) * 100.0, 1))) if s_raw > 100 else s_raw
                data["SOIL"] = data["SOIL_PCT"]
                data["SOIL_COND"] = "DRY" if data["SOIL_PCT"] < 25 else ("MODERATE" if data["SOIL_PCT"] < 65 else ("WET" if data["SOIL_PCT"] < 85 else "SATURATED"))
                data["RAIN_PCT"] = max(0.0, min(100.0, round(((4095 - r_raw) / 3095.0) * 100.0, 1))) if r_raw > 100 else r_raw
                data["RAIN"] = data["RAIN_PCT"]
                data["RAIN_COND"] = "NO_RAIN" if data["RAIN_PCT"] < 10 else ("LIGHT_RAIN" if data["RAIN_PCT"] < 40 else ("MODERATE_RAIN" if data["RAIN_PCT"] < 75 else "HEAVY_RAIN"))

                ax_val = nums[4]
                ay_val = nums[5]
                az_val = nums[6]
                ax_g = ax_val if abs(ax_val) <= 4.0 else ax_val / 16384.0
                ay_g = ay_val if abs(ay_val) <= 4.0 else ay_val / 16384.0
                az_g = az_val if abs(az_val) <= 4.0 else az_val / 16384.0
                data["AX_G"] = round(ax_g, 3)
                data["AY_G"] = round(ay_g, 3)
                data["AZ_G"] = round(az_g, 3)
                data["AX"] = data["AX_G"]
                data["AY"] = data["AY_G"]
                data["AZ"] = data["AZ_G"]
                data["AX_RAW"] = int(ax_g * 16384)
                data["AY_RAW"] = int(ay_g * 16384)
                data["AZ_RAW"] = int(az_g * 16384)

                gx_val = nums[7]
                gy_val = nums[8]
                gz_val = nums[9]
                gx_dps = gx_val if abs(gx_val) <= 30.0 else gx_val / 131.0
                gy_dps = gy_val if abs(gy_val) <= 30.0 else gy_val / 131.0
                gz_dps = gz_val if abs(gz_val) <= 30.0 else gz_val / 131.0
                data["GX_DPS"] = round(gx_dps, 2)
                data["GY_DPS"] = round(gy_dps, 2)
                data["GZ_DPS"] = round(gz_dps, 2)
                data["GX"] = data["GX_DPS"]
                data["GY"] = data["GY_DPS"]
                data["GZ"] = data["GZ_DPS"]
                data["GX_RAW"] = int(gx_dps * 131)
                data["GY_RAW"] = int(gy_dps * 131)
                data["GZ_RAW"] = int(gz_dps * 131)

                data["TX"] = round(math.atan2(ay_g, az_g) * 180.0 / math.pi, 2) if (ay_g != 0 or az_g != 0) else 0.0
                data["TY"] = round(math.atan2(-ax_g, math.sqrt(ay_g*ay_g + az_g*az_g)) * 180.0 / math.pi, 2) if (ay_g != 0 or az_g != 0 or ax_g != 0) else 0.0
        except ValueError:
            pass

    # Computed fields
    # Ensure packet actually contains valid sensor measurements, not just banner/debug lines
    has_sensor_data = any(k in data for k in ["TEMP", "HUM", "SOIL_RAW", "SOIL_PCT", "AX_G", "AX_RAW", "RAIN_RAW", "RAIN_PCT"])
    if not has_sensor_data or len(data) < 2:
        return None

    # Computed fields
    if "TEMP" in data:
        data["TEMP_F"] = round(data["TEMP"] * 9.0 / 5.0 + 32.0, 2)
    if "TEMP" in data and "HUM" in data:
        data["DEW_POINT"] = calc_dew_point(data["TEMP"], data["HUM"])

    ax_g = data.get("AX_G", 0.0)
    ay_g = data.get("AY_G", 0.0)
    az_g = data.get("AZ_G", 1.0)
    data["ACCEL_MAG"] = round(math.sqrt(ax_g*ax_g + ay_g*ay_g + az_g*az_g), 3)

    gx_dps = data.get("GX_DPS", 0.0)
    gy_dps = data.get("GY_DPS", 0.0)
    gz_dps = data.get("GZ_DPS", 0.0)
    data["GYRO_MAG"] = round(math.sqrt(gx_dps*gx_dps + gy_dps*gy_dps + gz_dps*gz_dps), 2)

    # Risk Level & Hazard Status
    s_p = data.get("SOIL_PCT", 0.0)
    r_p = data.get("RAIN_PCT", 0.0)
    a_m = data.get("ACCEL_MAG", 1.0)
    g_m = data.get("GYRO_MAG", 0.0)
    t_x = abs(data.get("TX", 0.0))
    t_y = abs(data.get("TY", 0.0))

    is_motion = (a_m > 1.45 or a_m < 0.55) or (g_m > 18.0) or (t_x > 15.0 or t_y > 15.0)
    is_high_sat = (s_p > 80.0 and r_p > 45.0)

    if is_motion and is_high_sat:
        data["RISK_LEVEL"] = "CRITICAL ALERT"
        data["HAZARD_STATUS"] = "HIGH_LANDSLIDE_HAZARD"
    elif is_motion:
        data["RISK_LEVEL"] = "WARNING"
        data["HAZARD_STATUS"] = "GROUND_SHIFT_DETECTED"
    elif is_high_sat or (s_p > 75.0 and r_p > 40.0):
        data["RISK_LEVEL"] = "ELEVATED"
        data["HAZARD_STATUS"] = "SATURATION_WARNING"
    elif s_p > 60.0 or r_p > 25.0:
        data["RISK_LEVEL"] = "ADVISORY"
        data["HAZARD_STATUS"] = "MOISTURE_ADVISORY"
    else:
        data["RISK_LEVEL"] = "SAFE"
        data["HAZARD_STATUS"] = "NORMAL_STABLE"

    return data


trans_bridge_batch = {}

def assemble_bridge_batch(b):
    data = dict(b)
    temp = data.get("TEMP", 25.0)
    hum = data.get("HUM", 50.0)
    data["TEMP"] = temp
    data["TEMP_F"] = round(temp * 9.0 / 5.0 + 32.0, 2)
    data["HUM"] = hum
    data["DEW_POINT"] = calc_dew_point(temp, hum)

    s_raw = data.get("SOIL_RAW", 4095)
    s_pct = data.get("SOIL_PCT", max(0.0, min(100.0, round(((4095 - s_raw) / 3095.0) * 100.0, 1))))
    data["SOIL_RAW"] = s_raw
    data["SOIL_PCT"] = s_pct
    data["SOIL"] = s_raw
    data["SOIL_COND"] = "DRY" if s_pct < 25 else ("MODERATE" if s_pct < 65 else ("WET" if s_pct < 85 else "SATURATED"))

    r_raw = data.get("RAIN_RAW", 4095)
    r_pct = data.get("RAIN_PCT", max(0.0, min(100.0, round(((4095 - r_raw) / 3095.0) * 100.0, 1))))
    data["RAIN_RAW"] = r_raw
    data["RAIN_PCT"] = r_pct
    data["RAIN"] = r_raw
    data["RAIN_COND"] = "NO_RAIN" if r_pct < 10 else ("LIGHT_RAIN" if r_pct < 40 else ("MODERATE_RAIN" if r_pct < 75 else "HEAVY_RAIN"))

    ax_g = data.get("AX_G", 0.0)
    ay_g = data.get("AY_G", 0.0)
    az_g = data.get("AZ_G", 1.0)
    data["AX_G"] = round(ax_g, 3)
    data["AY_G"] = round(ay_g, 3)
    data["AZ_G"] = round(az_g, 3)
    data["AX_RAW"] = int(ax_g * 16384)
    data["AY_RAW"] = int(ay_g * 16384)
    data["AZ_RAW"] = int(az_g * 16384)
    data["AX"] = data["AX_RAW"]
    data["AY"] = data["AY_RAW"]
    data["AZ"] = data["AZ_RAW"]
    data["ACCEL_MAG"] = round(math.sqrt(ax_g*ax_g + ay_g*ay_g + az_g*az_g), 3)

    gx_dps = data.get("GX_DPS", 0.0)
    gy_dps = data.get("GY_DPS", 0.0)
    gz_dps = data.get("GZ_DPS", 0.0)
    data["GX_DPS"] = round(gx_dps, 2)
    data["GY_DPS"] = round(gy_dps, 2)
    data["GZ_DPS"] = round(gz_dps, 2)
    data["GX_RAW"] = int(gx_dps * 131)
    data["GY_RAW"] = int(gy_dps * 131)
    data["GZ_RAW"] = int(gz_dps * 131)
    data["GX"] = data["GX_RAW"]
    data["GY"] = data["GY_RAW"]
    data["GZ"] = data["GZ_RAW"]
    data["GYRO_MAG"] = round(math.sqrt(gx_dps*gx_dps + gy_dps*gy_dps + gz_dps*gz_dps), 2)

    if "TX" not in data:
        data["TX"] = round(math.atan2(ay_g, az_g) * 180.0 / math.pi, 2) if (ay_g != 0 or az_g != 0) else 0.0
    if "TY" not in data:
        data["TY"] = round(math.atan2(-ax_g, math.sqrt(ay_g*ay_g + az_g*az_g)) * 180.0 / math.pi, 2) if (ay_g != 0 or az_g != 0 or ax_g != 0) else 0.0

    is_motion = (data["ACCEL_MAG"] > 1.45 or data["ACCEL_MAG"] < 0.55) or (data["GYRO_MAG"] > 18.0) or (abs(data["TX"]) > 15.0 or abs(data["TY"]) > 15.0)
    is_high_sat = (s_pct > 80.0 and r_pct > 45.0)

    if is_motion and is_high_sat:
        data["RISK_LEVEL"] = "CRITICAL ALERT"
        data["HAZARD_STATUS"] = "HIGH_LANDSLIDE_HAZARD"
    elif is_motion:
        data["RISK_LEVEL"] = "WARNING"
        data["HAZARD_STATUS"] = "GROUND_SHIFT_DETECTED"
    elif is_high_sat or (s_pct > 75.0 and r_pct > 40.0):
        data["RISK_LEVEL"] = "ELEVATED"
        data["HAZARD_STATUS"] = "SATURATION_WARNING"
    elif s_pct > 60.0 or r_pct > 25.0:
        data["RISK_LEVEL"] = "ADVISORY"
        data["HAZARD_STATUS"] = "MOISTURE_ADVISORY"
    else:
        data["RISK_LEVEL"] = "SAFE"
        data["HAZARD_STATUS"] = "NORMAL_STABLE"

    data["raw_line"] = f"GS1,TV:1,T:{temp:.2f},H:{hum:.2f},SRAW:{s_raw},SM:{s_pct:.1f},SC:{data['SOIL_COND']},RRAW:{r_raw},RP:{r_pct:.1f},RC:{data['RAIN_COND']},MV:1,TX:{data['TX']:.2f},TY:{data['TY']:.2f},AXG:{ax_g:.3f},AYG:{ay_g:.3f},AZG:{az_g:.3f},GXD:{gx_dps:.2f},GYD:{gy_dps:.2f},GZD:{gz_dps:.2f}"
    return data


def parse_bridge_stream(raw_line):
    global trans_bridge_batch
    clean = raw_line.strip()
    if not clean:
        return None

    if clean.count(',') >= 3 or ("TEMP:" in clean and "HUM:" in clean):
        pkt = parse_packet(clean)
        if pkt:
            pkt["raw_line"] = clean
            return pkt

    if ":" in clean:
        parts = clean.split(":", 1)
        k = parts[0].strip().upper().replace(" ", "_")
        v = parts[1].strip()
        m = re.search(r"[-+]?\d*\.?\d+", v)
        num = float(m.group(0)) if m else None

        if "TEMP" in k and num is not None:
            trans_bridge_batch["TEMP"] = num
        elif "HUM" in k and num is not None:
            trans_bridge_batch["HUM"] = num
        elif ("SOIL_ADC" in k or k == "SOIL") and num is not None:
            trans_bridge_batch["SOIL_RAW"] = int(num)
        elif "SOIL_MOISTURE" in k and num is not None:
            trans_bridge_batch["SOIL_PCT"] = num
        elif ("RAIN_ADC" in k or k == "RAIN") and num is not None:
            trans_bridge_batch["RAIN_RAW"] = int(num)
        elif "RAIN_RESPONSE" in k and num is not None:
            trans_bridge_batch["RAIN_PCT"] = num
        elif "TILT_X" in k and num is not None:
            trans_bridge_batch["TX"] = num
        elif "TILT_Y" in k and num is not None:
            trans_bridge_batch["TY"] = num
        elif "ACCELERATION_X" in k and num is not None:
            trans_bridge_batch["AX_G"] = num
        elif "ACCELERATION_Y" in k and num is not None:
            trans_bridge_batch["AY_G"] = num
        elif "ACCELERATION_Z" in k and num is not None:
            trans_bridge_batch["AZ_G"] = num
        elif "GYROSCOPE_X" in k and num is not None:
            trans_bridge_batch["GX_DPS"] = num
        elif "GYROSCOPE_Y" in k and num is not None:
            trans_bridge_batch["GY_DPS"] = num
        elif "GYROSCOPE_Z" in k and num is not None:
            trans_bridge_batch["GZ_DPS"] = num
        elif "LORA_TRANSMISSION" in k:
            if len(trans_bridge_batch) >= 4:
                assembled = assemble_bridge_batch(trans_bridge_batch)
                trans_bridge_batch = {}
                return assembled

    return None


def get_last_packet_seq():
    """Reads existing CSV to resume sequence numbers seamlessly."""
    if not os.path.isfile(CSV_FILENAME):
        return 0
    try:
        with open(CSV_FILENAME, mode="r", encoding="utf-8", errors="ignore") as f:
            reader = csv.reader(f)
            rows = [r for r in reader if r]
            if len(rows) > 1 and len(rows[-1]) > 1 and rows[-1][1].strip().isdigit():
                return int(rows[-1][1].strip())
            return max(0, len(rows) - 1)
    except Exception:
        return 0


def init_csv():
    global csv_packet_counter
    csv_packet_counter = get_last_packet_seq()
    with telemetry_lock:
        telemetry_state["packetCount"] = csv_packet_counter
    file_exists = os.path.isfile(CSV_FILENAME)
    try:
        with open(CSV_FILENAME, mode="a", newline="", encoding="utf-8") as f:
            writer = csv.writer(f)
            if not file_exists or os.path.getsize(CSV_FILENAME) == 0:
                writer.writerow(CSV_HEADERS)
                print(f"[OK] Created dataset file: {CSV_FILENAME}")
            else:
                print(f"[OK] Existing dataset found: {CSV_FILENAME} (Current Packet #{csv_packet_counter})")
    except PermissionError:
        print(f"\n[!] WARNING: '{CSV_FILENAME}' is locked by another program (e.g. Microsoft Excel).")
        print("    Please close Excel so the dataset file can be updated.\n")


pending_bridge_csv_rows = []

def flush_pending_csv():
    """Attempts to write buffered rows to disk."""
    global pending_bridge_csv_rows
    if not pending_bridge_csv_rows:
        return True
    try:
        with open(CSV_FILENAME, mode="a", newline="", encoding="utf-8") as f:
            writer = csv.writer(f)
            while pending_bridge_csv_rows:
                writer.writerow(pending_bridge_csv_rows[0])
                pending_bridge_csv_rows.pop(0)
        return True
    except PermissionError:
        print(f"\n[!] WARNING: Cannot save to '{CSV_FILENAME}' — file is locked by Microsoft Excel or another program.")
        print(f"    -> {len(pending_bridge_csv_rows)} packet(s) buffered in memory. Close Excel to save without losing data!\n")
        return False
    except Exception as e:
        print(f"[!] CSV write error: {e}")
        return False


def append_to_csv(timestamp, data, raw_line, current_rssi):
    global csv_packet_counter
    csv_packet_counter += 1

    rssi_val = data.get("RSSI", current_rssi if current_rssi is not None else "")
    if rssi_val != "" and rssi_val is not None:
        try:
            r_num = int(rssi_val)
            sig_qual = "EXCELLENT" if r_num >= -75 else ("GOOD" if r_num >= -90 else ("FAIR" if r_num >= -105 else "WEAK"))
        except Exception:
            sig_qual = "UNKNOWN"
    else:
        sig_qual = "UNKNOWN"

    row = [
        timestamp,
        csv_packet_counter,
        data.get("TEMP", ""),
        data.get("TEMP_F", ""),
        data.get("HUM", ""),
        data.get("DEW_POINT", ""),
        data.get("SOIL_RAW", ""),
        data.get("SOIL_PCT", data.get("SOIL", "")),
        data.get("SOIL_COND", ""),
        data.get("RAIN_RAW", ""),
        data.get("RAIN_PCT", data.get("RAIN", "")),
        data.get("RAIN_COND", ""),
        data.get("AX_RAW", ""),
        data.get("AY_RAW", ""),
        data.get("AZ_RAW", ""),
        data.get("AX_G", data.get("AX", "")),
        data.get("AY_G", data.get("AY", "")),
        data.get("AZ_G", data.get("AZ", "")),
        data.get("ACCEL_MAG", ""),
        data.get("GX_RAW", ""),
        data.get("GY_RAW", ""),
        data.get("GZ_RAW", ""),
        data.get("GX_DPS", data.get("GX", "")),
        data.get("GY_DPS", data.get("GY", "")),
        data.get("GZ_DPS", data.get("GZ", "")),
        data.get("GYRO_MAG", ""),
        data.get("TX", ""),
        data.get("TY", ""),
        rssi_val,
        sig_qual,
        len(raw_line.encode("utf-8")),
        data.get("RISK_LEVEL", "SAFE"),
        data.get("HAZARD_STATUS", "NORMAL_STABLE"),
        raw_line
    ]
    pending_bridge_csv_rows.append(row)
    flush_pending_csv()


def serial_reader_thread(port_name):
    global is_serial_connected, connected_port_name
    print(f"\n[SERIAL] Connecting to Receiver on {port_name} at {BAUD_RATE} baud...")

    init_csv()
    last_rssi = -80
    last_pkt_len = 78

    while True:
        ser = None
        try:
            ser = serial.Serial()
            ser.port = port_name
            ser.baudrate = BAUD_RATE
            ser.timeout = 1.0
            ser.dtr = False
            ser.rts = False
            ser.open()

            is_serial_connected = True
            connected_port_name = port_name
            print(f"[SERIAL OK] Connected to {port_name}! Listening for LoRa telemetry...\n")

            while True:
                raw = ser.readline().decode('utf-8', errors='ignore').strip()
                if not raw:
                    continue

                print(f"[SERIAL] {raw}")

                # Catch RSSI if printed in receiver logs
                rssi_match = re.search(r"RSSI:\s*([-\d]+)", raw, re.IGNORECASE)
                if rssi_match:
                    last_rssi = int(rssi_match.group(1))

                # Catch Packet length
                len_match = re.search(r"packet length:\s*(\d+)", raw, re.IGNORECASE)
                if len_match:
                    last_pkt_len = int(len_match.group(1))

                parsed = parse_bridge_stream(raw)
                if parsed:
                    now_str = datetime.now().strftime("%d-%m-%Y %H:%M:%S")
                    raw_to_log = parsed.get("raw_line", raw)
                    append_to_csv(now_str, parsed, raw_to_log, last_rssi)

                    soil_pct = parsed.get("SOIL_PCT", 0.0)
                    rain_pct = parsed.get("RAIN_PCT", 0.0)
                    ax_g = abs(parsed.get("AX_G", 0.0))
                    ay_g = abs(parsed.get("AY_G", 0.0))
                    az_g = abs(parsed.get("AZ_G", 1.0))
                    acc_mag = (ax_g*ax_g + ay_g*ay_g + az_g*az_g)**0.5
                    gyro_mag = (parsed.get("GX_DPS", 0)**2 + parsed.get("GY_DPS", 0)**2 + parsed.get("GZ_DPS", 0)**2)**0.5

                    risk = "MONITORING - SAFE"
                    if (soil_pct > 80.0 and rain_pct > 50.0) and (acc_mag > 1.4 or acc_mag < 0.6 or gyro_mag > 18):
                        risk = "CRITICAL ALERT"
                    elif (acc_mag > 1.4 or acc_mag < 0.6 or gyro_mag > 18):
                        risk = "WARNING"
                    elif (soil_pct > 75.0 and rain_pct > 40.0):
                        risk = "ELEVATED"
                    else:
                        risk = "MONITORING - SAFE"

                    with telemetry_lock:
                        telemetry_state["hasData"] = True
                        telemetry_state["temperature"] = parsed.get("TEMP", telemetry_state["temperature"])
                        telemetry_state["temperature_f"] = parsed.get("TEMP_F", round(telemetry_state["temperature"] * 9.0 / 5.0 + 32.0, 2))
                        telemetry_state["humidity"] = parsed.get("HUM", telemetry_state["humidity"])
                        telemetry_state["dew_point"] = parsed.get("DEW_POINT", calc_dew_point(telemetry_state["temperature"], telemetry_state["humidity"]))
                        telemetry_state["soil"] = parsed.get("SOIL_PCT", 0.0)
                        telemetry_state["soil_pct"] = parsed.get("SOIL_PCT", 0.0)
                        telemetry_state["soil_raw"] = parsed.get("SOIL_RAW", 4095)
                        telemetry_state["soil_cond"] = parsed.get("SOIL_COND", "DRY")
                        telemetry_state["rain"] = parsed.get("RAIN_PCT", 0.0)
                        telemetry_state["rain_pct"] = parsed.get("RAIN_PCT", 0.0)
                        telemetry_state["rain_raw"] = parsed.get("RAIN_RAW", 4095)
                        telemetry_state["rain_cond"] = parsed.get("RAIN_COND", "NO_RAIN")
                        telemetry_state["ax"] = parsed.get("AX_G", 0.0)
                        telemetry_state["ay"] = parsed.get("AY_G", 0.0)
                        telemetry_state["az"] = parsed.get("AZ_G", 1.0)
                        telemetry_state["ax_g"] = parsed.get("AX_G", 0.0)
                        telemetry_state["ay_g"] = parsed.get("AY_G", 0.0)
                        telemetry_state["az_g"] = parsed.get("AZ_G", 1.0)
                        telemetry_state["ax_raw"] = parsed.get("AX_RAW", 0)
                        telemetry_state["ay_raw"] = parsed.get("AY_RAW", 0)
                        telemetry_state["az_raw"] = parsed.get("AZ_RAW", 16384)
                        telemetry_state["accel_mag"] = parsed.get("ACCEL_MAG", 1.0)
                        telemetry_state["gx"] = parsed.get("GX_DPS", 0.0)
                        telemetry_state["gy"] = parsed.get("GY_DPS", 0.0)
                        telemetry_state["gz"] = parsed.get("GZ_DPS", 0.0)
                        telemetry_state["gx_dps"] = parsed.get("GX_DPS", 0.0)
                        telemetry_state["gy_dps"] = parsed.get("GY_DPS", 0.0)
                        telemetry_state["gz_dps"] = parsed.get("GZ_DPS", 0.0)
                        telemetry_state["gx_raw"] = parsed.get("GX_RAW", 0)
                        telemetry_state["gy_raw"] = parsed.get("GY_RAW", 0)
                        telemetry_state["gz_raw"] = parsed.get("GZ_RAW", 0)
                        telemetry_state["gyro_mag"] = parsed.get("GYRO_MAG", 0.0)
                        if "TX" in parsed: telemetry_state["tx"] = parsed["TX"]
                        if "TY" in parsed: telemetry_state["ty"] = parsed["TY"]
                        telemetry_state["rssi"] = parsed.get("RSSI", last_rssi)
                        telemetry_state["signal_quality"] = "EXCELLENT" if telemetry_state["rssi"] >= -75 else ("GOOD" if telemetry_state["rssi"] >= -90 else ("FAIR" if telemetry_state["rssi"] >= -105 else "WEAK"))
                        telemetry_state["packetLength"] = last_pkt_len
                        telemetry_state["packetCount"] += 1
                        telemetry_state["lastUpdate"] = int(time.time() * 1000)
                        telemetry_state["nodeOnline"] = True
                        telemetry_state["port"] = port_name
                        telemetry_state["riskLevel"] = parsed.get("RISK_LEVEL", risk)
                        telemetry_state["hazardStatus"] = parsed.get("HAZARD_STATUS", "NORMAL_STABLE")

                        pkt_info = {
                            "packetId": telemetry_state["packetCount"],
                            "time": datetime.now().strftime("%H:%M:%S"),
                            "timestamp": now_str,
                            "source": "Receiver (LoRa SX1278)",
                            "rssi": telemetry_state["rssi"],
                            "signalQuality": telemetry_state["signal_quality"],
                            "tempC": telemetry_state["temperature"],
                            "tempF": telemetry_state["temperature_f"],
                            "hum": telemetry_state["humidity"],
                            "dewPoint": telemetry_state["dew_point"],
                            "soilRaw": telemetry_state["soil_raw"],
                            "soilPct": telemetry_state["soil_pct"],
                            "soilCond": telemetry_state["soil_cond"],
                            "rainRaw": telemetry_state["rain_raw"],
                            "rainPct": telemetry_state["rain_pct"],
                            "rainCond": telemetry_state["rain_cond"],
                            "axG": telemetry_state["ax_g"],
                            "ayG": telemetry_state["ay_g"],
                            "azG": telemetry_state["az_g"],
                            "accelMag": telemetry_state["accel_mag"],
                            "gxDps": telemetry_state["gx_dps"],
                            "gyDps": telemetry_state["gy_dps"],
                            "gzDps": telemetry_state["gz_dps"],
                            "gyroMag": telemetry_state["gyro_mag"],
                            "tiltX": telemetry_state["tx"],
                            "tiltY": telemetry_state["ty"],
                            "riskLevel": telemetry_state["riskLevel"],
                            "hazardStatus": telemetry_state["hazardStatus"],
                            "rawPacket": raw
                        }
                        recent_packets_buffer.append(pkt_info)
                        if len(recent_packets_buffer) > 200:
                            recent_packets_buffer.pop(0)

                        telemetry_state["recentPackets"] = recent_packets_buffer[-25:]

                    t_str = datetime.now().strftime("%H:%M:%S")
                    print(f"[{t_str}] [LoRa RX] Temp: {parsed.get('TEMP')}°C | Hum: {parsed.get('HUM')}% | Soil: {parsed.get('SOIL_PCT')}% ({parsed.get('SOIL_COND')}) | Rain: {parsed.get('RAIN_PCT')}% ({parsed.get('RAIN_COND')}) | Accel: {parsed.get('AX_G')}g,{parsed.get('AY_G')}g,{parsed.get('AZ_G')}g | Gyro: {parsed.get('GX_DPS')}°/s | RSSI: {last_rssi} dBm")

        except serial.SerialException as e:
            is_serial_connected = False
            print(f"\n[!] Serial Error on {port_name}: {e}")
            print("[!] Reminder: If the Arduino IDE Serial Monitor is open, CLOSE IT to release the COM port.")
            print("[!] Retrying in 3 seconds...\n")
            time.sleep(3)
        finally:
            if ser and ser.is_open:
                ser.close()


class DashboardHTTPHandler(BaseHTTPRequestHandler):
    def do_OPTIONS(self):
        self.send_response(200)
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Access-Control-Allow-Methods", "GET, OPTIONS")
        self.send_header("Access-Control-Allow-Headers", "Content-Type")
        self.end_headers()

    def do_GET(self):
        # 1. API: Live Data JSON
        if self.path == "/api/data" or self.path.startswith("/api/data?"):
            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.send_header("Access-Control-Allow-Origin", "*")
            self.send_header("Cache-Control", "no-cache, no-store, must-revalidate")
            self.end_headers()

            with telemetry_lock:
                copy_state = dict(telemetry_state)
                copy_state["port"] = connected_port_name
                # Compute elapsed time
                if copy_state["lastUpdate"] > 0:
                    elapsed_ms = int(time.time() * 1000) - copy_state["lastUpdate"]
                    copy_state["timeSinceLastPacket"] = elapsed_ms
                    copy_state["nodeOnline"] = (elapsed_ms < 15000)
                else:
                    copy_state["timeSinceLastPacket"] = 999999
                    copy_state["nodeOnline"] = False

            self.wfile.write(json.dumps(copy_state).encode('utf-8'))
            return

        # 2. API: Packets Stream (Recent Analyzed Packets)
        elif self.path == "/api/packets" or self.path.startswith("/api/packets?"):
            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.send_header("Access-Control-Allow-Origin", "*")
            self.send_header("Cache-Control", "no-cache")
            self.end_headers()
            self.wfile.write(json.dumps(recent_packets_buffer).encode('utf-8'))
            return

        # 3. API: Dataset Download (CSV)
        elif self.path == "/api/dataset" or self.path.startswith("/api/dataset?") or self.path == "/api/export-csv":
            if os.path.exists(CSV_FILENAME):
                with open(CSV_FILENAME, "rb") as f:
                    csv_data = f.read()
                self.send_response(200)
                self.send_header("Content-Type", "text/csv; charset=utf-8")
                self.send_header("Content-Disposition", 'attachment; filename="geosentinel_dataset.csv"')
                self.send_header("Access-Control-Allow-Origin", "*")
                self.end_headers()
                self.wfile.write(csv_data)
                return
            else:
                self.send_response(404)
                self.send_header("Content-Type", "text/plain")
                self.end_headers()
                self.wfile.write(b"Dataset CSV file not found")
                return

        # 3b. API: Dataset JSON
        elif self.path == "/api/dataset-json" or self.path.startswith("/api/dataset-json?"):
            if os.path.exists(CSV_FILENAME):
                with open(CSV_FILENAME, "r", encoding="utf-8", errors="ignore") as f:
                    rows = list(csv.DictReader(f))
                self.send_response(200)
                self.send_header("Content-Type", "application/json")
                self.send_header("Access-Control-Allow-Origin", "*")
                self.send_header("Cache-Control", "no-cache")
                self.end_headers()
                self.wfile.write(json.dumps(rows).encode('utf-8'))
                return
            else:
                self.send_response(404)
                self.send_header("Content-Type", "text/plain")
                self.end_headers()
                self.wfile.write(b"[]")
                return

        # 4. API: Status
        elif self.path == "/api/status":
            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.send_header("Access-Control-Allow-Origin", "*")
            self.end_headers()
            status_obj = {
                "serialConnected": is_serial_connected,
                "port": connected_port_name,
                "baudRate": BAUD_RATE,
                "serverTime": datetime.now().isoformat(),
                "totalPackets": telemetry_state["packetCount"]
            }
            self.wfile.write(json.dumps(status_obj).encode('utf-8'))
            return

        # 3. Serve Dashboard HTML
        elif self.path == "/" or self.path.startswith("/index"):
            if os.path.exists(HTML_FILENAME):
                with open(HTML_FILENAME, "rb") as f:
                    content = f.read()
                self.send_response(200)
                self.send_header("Content-Type", "text/html; charset=utf-8")
                self.send_header("Cache-Control", "no-cache")
                self.end_headers()
                self.wfile.write(content)
                return

        # 4. Fallback 404
        self.send_response(404)
        self.send_header("Content-Type", "text/plain")
        self.end_headers()
        self.wfile.write(b"404 Not Found")



    def log_message(self, format, *args):
        # Suppress routine GET logging for /api/data to keep terminal clean
        if "/api/data" not in format % args:
            super().log_message(format, *args)


def main():
    print("=" * 75)
    print("   GeoSentinel — Receiver USB Telemetry Bridge & Live Dashboard   ")
    print("   Landslide Detection & Early-Warning Monitoring System          ")
    print("=" * 75)

    cli_port = sys.argv[1] if len(sys.argv) > 1 else None
    target_port = select_port(cli_port)
    if not target_port:
        print("[!] No port chosen. Exiting.")
        return

    # Start Serial background thread
    t = threading.Thread(target=serial_reader_thread, args=(target_port,), daemon=True)
    t.start()

    # Start HTTP Web Server with resilient host/port fallback (handles Windows socket permissions)
    httpd = None
    active_port = HTTP_PORT
    active_host = "127.0.0.1"

    candidates = [
        ("127.0.0.1", 8000),
        ("0.0.0.0", 8000),
        ("127.0.0.1", 8080),
        ("0.0.0.0", 8080),
        ("127.0.0.1", 8088),
        ("0.0.0.0", 8088),
        ("127.0.0.1", 5000),
    ]

    for host, port in candidates:
        try:
            httpd = HTTPServer((host, port), DashboardHTTPHandler)
            active_host = host
            active_port = port
            break
        except Exception:
            continue

    if not httpd:
        print("[!] Could not bind HTTP server to any available port. Please check your firewall/network permissions.")
        return

    try:
        url = f"http://localhost:{active_port}"
        print("=" * 75)
        print(f"[HTTP] Dashboard Server running at: {url}")
        print("       (Opening default browser automatically...)")
        print("=" * 75)

        # Open browser in a separate thread after 1 second
        def open_browser():
            time.sleep(1.0)
            webbrowser.open(url)

        threading.Thread(target=open_browser, daemon=True).start()

        httpd.serve_forever()

    except KeyboardInterrupt:
        print("\n[OK] Stopping server. Bye!")
    except Exception as e:
        print(f"[!] Server error: {e}")


if __name__ == "__main__":
    main()
