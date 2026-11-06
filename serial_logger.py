"""
GeoSentinel — USB Serial Telemetry Logger & Dataset Collector
==============================================================
Reads live LoRa telemetry streamed from the GeoSentinel ESP32 Base Station
or Sensor Node over USB Serial (115200 baud), parses all environmental and
kinematic sensor metrics, and logs them into a CSV file for landslide
prediction model training and analysis.

Usage:
    python serial_logger.py          # Auto-detects port or prompts
    python serial_logger.py COM6     # Connects directly to COM6
    python serial_logger.py COM3     # Connects directly to COM3

Prerequisites:
    pip install pyserial
"""

import sys
import os
import time
import math
import csv
import json
import re
from datetime import datetime
import serial
import serial.tools.list_ports

# ==============================================================================
# CONFIGURATION
# ==============================================================================
BAUD_RATE = 115200  # Matches Serial.begin(115200) on ESP32 firmware
CSV_FILENAME = "geosentinel_dataset.csv"
HTML_FILENAME = "standalone_dashboard_preview.html"
HEADER_FILENAME = os.path.join("GeoSentinel_BaseStation", "dashboard.h")

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

packet_seq_counter = 0

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

def list_ports():
    """Returns list of active serial ports."""
    return list(serial.tools.list_ports.comports())

def select_port(cli_arg=None):
    """Determines target COM port via CLI argument, auto-detection, or interactive selection."""
    if cli_arg:
        return cli_arg.upper()

    ports = list_ports()
    if not ports:
        print("[!] No active COM ports found. Ensure your ESP32 is plugged in.")
        return None

    print("\nAvailable Serial Ports:")
    for idx, p in enumerate(ports):
        print(f"  [{idx + 1}] {p.device} -- {p.description}")

    # If only 1 port exists, use it
    if len(ports) == 1:
        print(f"[OK] Auto-selected: {ports[0].device}")
        return ports[0].device

    # Prioritize COM3 (standard ESP32 Base Station) or first CP210x bridge
    default_p = ports[0].device
    for p in ports:
        if p.device.upper() == "COM3":
            default_p = p.device
            break

    print(f"\n[OK] Automatically selecting active device: {default_p}")
    return default_p


def parse_geosentinel_packet(line):
    """
    Parses incoming telemetry from GeoSentinel with full multi-sensor details.
    """
    data = {}
    clean = line.strip()

    # Case 1: Key-Value tokens (KEY:VALUE)
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

        # Soil
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

        data["SOIL_RAW"] = soil_raw if soil_raw is not None else 4095
        data["SOIL_PCT"] = round(soil_pct, 1) if soil_pct is not None else 0.0
        data["SOIL"] = data["SOIL_RAW"]
        data["SOIL_COND"] = raw_dict.get("SC", ("DRY" if data["SOIL_PCT"] < 25 else ("MODERATE" if data["SOIL_PCT"] < 65 else ("WET" if data["SOIL_PCT"] < 85 else "SATURATED"))))

        # Rain
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

        data["RAIN_RAW"] = rain_raw if rain_raw is not None else 4095
        data["RAIN_PCT"] = round(rain_pct, 1) if rain_pct is not None else 0.0
        data["RAIN"] = data["RAIN_RAW"]
        data["RAIN_COND"] = raw_dict.get("RC", ("NO_RAIN" if data["RAIN_PCT"] < 10 else ("LIGHT_RAIN" if data["RAIN_PCT"] < 40 else ("MODERATE_RAIN" if data["RAIN_PCT"] < 75 else "HEAVY_RAIN"))))

        # Accelerometer
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
        data["AX_RAW"] = int(ax_g * 16384)
        data["AY_RAW"] = int(ay_g * 16384)
        data["AZ_RAW"] = int(az_g * 16384)
        data["AX"] = data["AX_RAW"]
        data["AY"] = data["AY_RAW"]
        data["AZ"] = data["AZ_RAW"]

        # Gyroscope
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
        data["GX_RAW"] = int(gx_dps * 131)
        data["GY_RAW"] = int(gy_dps * 131)
        data["GZ_RAW"] = int(gz_dps * 131)
        data["GX"] = data["GX_RAW"]
        data["GY"] = data["GY_RAW"]
        data["GZ"] = data["GZ_RAW"]

        # Tilt
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

    # Case 2: Pure CSV numbers
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
                data["RAIN_PCT"] = max(0.0, min(100.0, round(((4095 - r_raw) / 3095.0) * 100.0, 1))) if r_raw > 100 else r_raw
                data["SOIL"] = s_raw
                data["RAIN"] = r_raw
                data["SOIL_COND"] = "DRY" if data["SOIL_PCT"] < 25 else ("MODERATE" if data["SOIL_PCT"] < 65 else ("WET" if data["SOIL_PCT"] < 85 else "SATURATED"))
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
                data["AX_RAW"] = int(ax_g * 16384)
                data["AY_RAW"] = int(ay_g * 16384)
                data["AZ_RAW"] = int(az_g * 16384)
                data["AX"] = data["AX_RAW"]
                data["AY"] = data["AY_RAW"]
                data["AZ"] = data["AZ_RAW"]

                gx_val = nums[7]
                gy_val = nums[8]
                gz_val = nums[9]
                gx_dps = gx_val if abs(gx_val) <= 30.0 else gx_val / 131.0
                gy_dps = gy_val if abs(gy_val) <= 30.0 else gy_val / 131.0
                gz_dps = gz_val if abs(gz_val) <= 30.0 else gz_val / 131.0
                data["GX_DPS"] = round(gx_dps, 2)
                data["GY_DPS"] = round(gy_dps, 2)
                data["GZ_DPS"] = round(gz_dps, 2)
                data["GX_RAW"] = int(gx_dps * 131)
                data["GY_RAW"] = int(gy_dps * 131)
                data["GZ_RAW"] = int(gz_dps * 131)
                data["GX"] = data["GX_RAW"]
                data["GY"] = data["GY_RAW"]
                data["GZ"] = data["GZ_RAW"]

                data["TX"] = round(math.atan2(ay_g, az_g) * 180.0 / math.pi, 2) if (ay_g != 0 or az_g != 0) else 0.0
                data["TY"] = round(math.atan2(-ax_g, math.sqrt(ay_g*ay_g + az_g*az_g)) * 180.0 / math.pi, 2) if (ay_g != 0 or az_g != 0 or ax_g != 0) else 0.0
        except ValueError:
            pass

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

trans_batch = {}

def assemble_batch(b):
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


def parse_serial_stream(raw_line):
    """
    Parses either single-line receiver packets or multi-line transmitter prints.
    """
    global trans_batch
    clean = raw_line.strip()
    if not clean:
        return None

    # Single-line packet format (Base Station receiver):
    if clean.count(',') >= 3 or ("TEMP:" in clean and "HUM:" in clean):
        pkt = parse_geosentinel_packet(clean)
        if pkt:
            pkt["raw_line"] = clean
            return pkt

    # Multi-line format (Sensor Node transmitter):
    if ":" in clean:
        parts = clean.split(":", 1)
        k = parts[0].strip().upper().replace(" ", "_")
        v = parts[1].strip()
        m = re.search(r"[-+]?\d*\.?\d+", v)
        num = float(m.group(0)) if m else None

        if "TEMP" in k and num is not None:
            trans_batch["TEMP"] = num
        elif "HUM" in k and num is not None:
            trans_batch["HUM"] = num
        elif ("SOIL_ADC" in k or k == "SOIL") and num is not None:
            trans_batch["SOIL_RAW"] = int(num)
        elif "SOIL_MOISTURE" in k and num is not None:
            trans_batch["SOIL_PCT"] = num
        elif ("RAIN_ADC" in k or k == "RAIN") and num is not None:
            trans_batch["RAIN_RAW"] = int(num)
        elif "RAIN_RESPONSE" in k and num is not None:
            trans_batch["RAIN_PCT"] = num
        elif "TILT_X" in k and num is not None:
            trans_batch["TX"] = num
        elif "TILT_Y" in k and num is not None:
            trans_batch["TY"] = num
        elif "ACCELERATION_X" in k and num is not None:
            trans_batch["AX_G"] = num
        elif "ACCELERATION_Y" in k and num is not None:
            trans_batch["AY_G"] = num
        elif "ACCELERATION_Z" in k and num is not None:
            trans_batch["AZ_G"] = num
        elif "GYROSCOPE_X" in k and num is not None:
            trans_batch["GX_DPS"] = num
        elif "GYROSCOPE_Y" in k and num is not None:
            trans_batch["GY_DPS"] = num
        elif "GYROSCOPE_Z" in k and num is not None:
            trans_batch["GZ_DPS"] = num
        elif "LORA_TRANSMISSION" in k:
            if len(trans_batch) >= 4:
                assembled = assemble_batch(trans_batch)
                trans_batch = {}
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
    """Initializes the CSV output file with headers if it does not already exist."""
    global packet_seq_counter
    packet_seq_counter = get_last_packet_seq()
    file_exists = os.path.isfile(CSV_FILENAME)
    try:
        with open(CSV_FILENAME, mode="a", newline="", encoding="utf-8") as f:
            writer = csv.writer(f)
            if not file_exists or os.path.getsize(CSV_FILENAME) == 0:
                writer.writerow(CSV_HEADERS)
                print(f"[OK] Created dataset file: {CSV_FILENAME}")
            else:
                print(f"[OK] Existing dataset found: {CSV_FILENAME} (Current Packet #{packet_seq_counter})")
    except PermissionError:
        print(f"\n[!] WARNING: '{CSV_FILENAME}' is locked by another program (e.g. Microsoft Excel).")
        print("    Please close Excel so the dataset file can be updated.\n")


pending_csv_rows = []

def flush_pending_csv():
    """Attempts to write any buffered packets to disk."""
    global pending_csv_rows
    if not pending_csv_rows:
        return True
    try:
        with open(CSV_FILENAME, mode="a", newline="", encoding="utf-8") as f:
            writer = csv.writer(f)
            while pending_csv_rows:
                writer.writerow(pending_csv_rows[0])
                pending_csv_rows.pop(0)
        return True
    except PermissionError:
        print(f"\n[!] WARNING: Cannot save to '{CSV_FILENAME}' — file is locked by Microsoft Excel or another program.")
        print(f"    -> {len(pending_csv_rows)} packet(s) buffered in memory. Close Excel to save without losing data!\n")
        return False
    except Exception as e:
        print(f"[!] Error writing to CSV: {e}")
        return False


def append_to_csv(timestamp, data, raw_line, current_rssi):
    """Appends a validated reading row with all 34 sensor columns to the CSV dataset."""
    global packet_seq_counter
    packet_seq_counter += 1

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
        packet_seq_counter,
        data.get("TEMP", ""),
        data.get("TEMP_F", ""),
        data.get("HUM", ""),
        data.get("DEW_POINT", ""),
        data.get("SOIL_RAW", ""),
        data.get("SOIL_PCT", ""),
        data.get("SOIL_COND", ""),
        data.get("RAIN_RAW", ""),
        data.get("RAIN_PCT", ""),
        data.get("RAIN_COND", ""),
        data.get("AX_RAW", ""),
        data.get("AY_RAW", ""),
        data.get("AZ_RAW", ""),
        data.get("AX_G", ""),
        data.get("AY_G", ""),
        data.get("AZ_G", ""),
        data.get("ACCEL_MAG", ""),
        data.get("GX_RAW", ""),
        data.get("GY_RAW", ""),
        data.get("GZ_RAW", ""),
        data.get("GX_DPS", ""),
        data.get("GY_DPS", ""),
        data.get("GZ_DPS", ""),
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
    pending_csv_rows.append(row)
    flush_pending_csv()


def sync_html_files():
    """Syncs the latest rows of geosentinel_dataset.csv into standalone_dashboard_preview.html and dashboard.h."""
    if not os.path.isfile(CSV_FILENAME):
        return False
    try:
        with open(CSV_FILENAME, "r", encoding="utf-8", errors="ignore") as f:
            reader = csv.DictReader(f)
            records = [dict(r) for r in reader]
        if not records:
            return False

        json_str = json.dumps(records)
        replacement = f"const INITIAL_HISTORICAL_DATASET = {json_str};"

        updated_any = False
        for filepath in [HTML_FILENAME, HEADER_FILENAME]:
            if os.path.exists(filepath):
                with open(filepath, "r", encoding="utf-8") as f:
                    content = f.read()
                new_content, n = re.subn(
                    r'const INITIAL_HISTORICAL_DATASET = \[.*?\];',
                    replacement,
                    content,
                    count=1,
                    flags=re.DOTALL
                )
                if n > 0:
                    with open(filepath, "w", encoding="utf-8", newline="") as f:
                        f.write(new_content)
                    updated_any = True
        if updated_any:
            print(f"[OK] Synchronized {len(records)} packets into '{HTML_FILENAME}' and '{HEADER_FILENAME}'.")
        return True
    except Exception as e:
        print(f"[!] Warning syncing HTML dataset: {e}")
        return False


def main():
    print("=" * 70)
    print("   GeoSentinel -- Serial Telemetry Reader & Dataset Collector   ")
    print("=" * 70)

    cli_port = sys.argv[1] if len(sys.argv) > 1 else None
    target_port = select_port(cli_port)
    if not target_port:
        return

    print(f"\n[INFO] Connecting to {target_port} at {BAUD_RATE} baud...")
    print("[!] Reminder: Close the Arduino IDE Serial Monitor or Browser Web Serial")
    print("    before connecting to prevent 'Access Denied / Port Busy' errors.")

    init_csv()

    ser = None
    try:
        # Prevent hardware DTR/RTS auto-reset on ESP32
        ser = serial.Serial()
        ser.port = target_port
        ser.baudrate = BAUD_RATE
        ser.timeout = 1.0
        ser.dtr = False
        ser.rts = False
        ser.open()

        time.sleep(1.0)
        print(f"[OK] Successfully opened {target_port}!")
        print("\nStreaming Live Telemetry... (Press Ctrl+C to stop logging)\n")
        print("-" * 75)
        print(f"{'Time':<10} | {'Temp':<7} | {'Hum':<7} | {'Soil':<6} | {'Rain':<6} | {'Accel (X,Y,Z)':<18} | {'RSSI'}")
        print("-" * 75)

        last_known_rssi = None
        packet_count = 0

        while True:
            raw_line = ser.readline().decode('utf-8', errors='ignore').strip()
            if not raw_line:
                continue

            # Catch RSSI if printed in receiver logs
            rssi_match = re.search(r"RSSI:\s*([-\d]+)", raw_line, re.IGNORECASE)
            if rssi_match:
                last_known_rssi = int(rssi_match.group(1))

            # Parse telemetry packet (supports both single-line receiver packets and multi-line transmitter streams)
            telemetry = parse_serial_stream(raw_line)
            if telemetry:
                packet_count += 1
                now_str = datetime.now().strftime("%d-%m-%Y %H:%M:%S")
                time_short = datetime.now().strftime("%H:%M:%S")

                temp_val = f"{telemetry.get('TEMP', 0.0):.1f} C" if "TEMP" in telemetry else "N/A"
                hum_val = f"{telemetry.get('HUM', 0.0):.1f} %" if "HUM" in telemetry else "N/A"
                soil_val = telemetry.get("SOIL", "N/A")
                rain_val = telemetry.get("RAIN", "N/A")

                ax = telemetry.get("AX", 0)
                ay = telemetry.get("AY", 0)
                az = telemetry.get("AZ", 0)
                accel_str = f"{ax},{ay},{az}" if ("AX" in telemetry) else "N/A"

                rssi_str = f"{telemetry.get('RSSI', last_known_rssi)} dBm" if (last_known_rssi or "RSSI" in telemetry) else "--"

                # Print formatted status row
                print(f"{time_short:<10} | {temp_val:<7} | {hum_val:<7} | {soil_val:<6} | {rain_val:<6} | {accel_str:<18} | {rssi_str}")

                # Save to training dataset CSV
                raw_to_log = telemetry.get("raw_line", raw_line)
                append_to_csv(now_str, telemetry, raw_to_log, last_known_rssi)

    except serial.SerialException as e:
        print(f"\n[ERROR] Serial Error on {target_port}: {e}")
        print("\nTroubleshooting:")
        print("  1. Close the Arduino IDE Serial Monitor (Tools -> Serial Monitor).")
        print("  2. If using the Chrome/Edge dashboard, disconnect the USB port first.")
        print(f"  3. Check that {target_port} is the correct port.")
    except KeyboardInterrupt:
        print(f"\n\n[OK] Logging stopped by user. Saved {packet_count} packets to '{CSV_FILENAME}'.")
        if packet_count > 0:
            sync_html_files()
    finally:
        if ser and ser.is_open:
            ser.close()
            print("[OK] Serial port closed cleanly.")


if __name__ == "__main__":
    main()
