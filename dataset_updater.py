#!/usr/bin/env python3
"""
================================================================================
GeoSentinel — Automated Telemetry Packet Dataset Updater
================================================================================
Purpose:
  Continuously and automatically updates 'geosentinel_dataset.csv' with new
  incoming telemetry packets.

Operating Modes:
  1. LIVE HARDWARE MODE (Default when ESP32 is detected):
     Automatically connects to the active ESP32 COM port (COM3, COM5, etc.),
     reads incoming sensor streams or LoRa packets, parses all 34 parameters,
     and appends each packet seamlessly to the dataset.

  2. AUTONOMOUS SIMULATION MODE (Fallback if no hardware is plugged in):
     Generates realistic physical geotechnical telemetry packets (atmospheric,
     hydrological soil saturation, rainfall intensity, and 3D kinematics)
     simulating real-world baseline and landslide hazard conditions.

Usage:
  python dataset_updater.py                     # Auto-detects port or runs simulation
  python dataset_updater.py --interval 3.0       # Update packet every 3 seconds
  python dataset_updater.py --count 50          # Append 50 packets and exit
  python dataset_updater.py --port COM3         # Force COM3 port
  python dataset_updater.py --simulate          # Force simulation mode
  python dataset_updater.py --status            # View dataset statistics
  python dataset_updater.py --reset             # Reset dataset to initial packet #1
================================================================================
"""

import os
import sys
import time
import math
import csv
import json
import re
import random
import argparse
from datetime import datetime

try:
    import serial
    import serial.tools.list_ports
    SERIAL_AVAILABLE = True
except ImportError:
    SERIAL_AVAILABLE = False

# ==============================================================================
# CONFIGURATION & CONSTANTS
# ==============================================================================
CSV_FILENAME = "geosentinel_dataset.csv"
HTML_FILENAME = "standalone_dashboard_preview.html"
HEADER_FILENAME = os.path.join("GeoSentinel_BaseStation", "dashboard.h")
BAUD_RATE = 115200

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

INITIAL_ROW = [
    "13-10-2026 11:36",
    "1",
    "31.6",
    "88.88",
    "77.6",
    "27.19",
    "4095",
    "0",
    "DRY",
    "132",
    "100",
    "HEAVY_RAIN",
    "-1196",
    "168",
    "16884",
    "-0.073",
    "0.01",
    "1.031",
    "1.034",
    "406",
    "122",
    "-46",
    "3.1",
    "0.93",
    "-0.35",
    "3.26",
    "0.56",
    "4.05",
    "",
    "UNKNOWN",
    "161",
    "ADVISORY",
    "MOISTURE_ADVISORY",
    "GS1,TV:1,T:31.60,H:77.60,SRAW:4095,SM:0.0,SC:DRY,RRAW:132,RP:100.0,RC:HEAVY_RAIN,MV:1,TX:0.56,TY:4.05,AXG:-0.073,AYG:0.010,AZG:1.031,GXD:3.10,GYD:0.93,GZD:-0.35"
]

# Memory buffer for writes when file is locked
pending_rows_buffer = []


# ==============================================================================
# DATASET FILE MANAGEMENT
# ==============================================================================
def get_dataset_stats():
    """Returns row count and latest packet info from the CSV."""
    if not os.path.isfile(CSV_FILENAME):
        return {"exists": False, "count": 0, "last_id": 0, "last_row": None}
    try:
        with open(CSV_FILENAME, mode="r", encoding="utf-8", errors="ignore") as f:
            reader = list(csv.reader(f))
            rows = [r for r in reader if r]
            if len(rows) <= 1:
                return {"exists": True, "count": 0, "last_id": 0, "last_row": None}
            last = rows[-1]
            last_id = int(last[1].strip()) if (len(last) > 1 and last[1].strip().isdigit()) else (len(rows) - 1)
            return {"exists": True, "count": len(rows) - 1, "last_id": last_id, "last_row": last}
    except Exception as e:
        return {"exists": True, "count": 0, "last_id": 0, "error": str(e)}


def init_dataset(reset=False):
    """Initializes or resets geosentinel_dataset.csv with proper headers and initial row."""
    file_exists = os.path.isfile(CSV_FILENAME)
    if reset or not file_exists or os.path.getsize(CSV_FILENAME) == 0:
        try:
            with open(CSV_FILENAME, mode="w", newline="", encoding="utf-8") as f:
                writer = csv.writer(f)
                writer.writerow(CSV_HEADERS)
                writer.writerow(INITIAL_ROW)
            print(f"[OK] Initialized new dataset: '{CSV_FILENAME}' (Header + Initial Packet #1).")
            return 1
        except PermissionError:
            print(f"[!] ERROR: '{CSV_FILENAME}' is locked by another application (e.g. Excel). Please close it.")
            return 0
    else:
        stats = get_dataset_stats()
        print(f"[OK] Existing dataset found: '{CSV_FILENAME}' ({stats['count']} packets logged, Current Seq #{stats['last_id']}).")
        return stats["last_id"]


def flush_pending_buffer():
    """Flushes buffered rows to disk, handling file lock gracefully."""
    global pending_rows_buffer
    if not pending_rows_buffer:
        return True
    try:
        with open(CSV_FILENAME, mode="a", newline="", encoding="utf-8") as f:
            writer = csv.writer(f)
            while pending_rows_buffer:
                writer.writerow(pending_rows_buffer[0])
                pending_rows_buffer.pop(0)
        return True
    except PermissionError:
        return False
    except Exception as e:
        print(f"[!] CSV write error: {e}")
        return False


def append_packet_to_dataset(timestamp_str, packet_id, data, raw_packet_str, rssi=""):
    """Appends a single validated 34-column row to geosentinel_dataset.csv."""
    rssi_val = data.get("RSSI", rssi if rssi is not None else "")
    if rssi_val != "" and rssi_val is not None:
        try:
            r_num = int(rssi_val)
            sig_qual = "EXCELLENT" if r_num >= -75 else ("GOOD" if r_num >= -90 else ("FAIR" if r_num >= -105 else "WEAK"))
        except Exception:
            sig_qual = "UNKNOWN"
    else:
        sig_qual = "UNKNOWN"

    row = [
        timestamp_str,
        packet_id,
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
        len(raw_packet_str.encode("utf-8")),
        data.get("RISK_LEVEL", "SAFE"),
        data.get("HAZARD_STATUS", "NORMAL_STABLE"),
        raw_packet_str
    ]

    pending_rows_buffer.append(row)
    flushed = flush_pending_buffer()
    if not flushed and len(pending_rows_buffer) % 5 == 0:
        print(f"[!] Notice: '{CSV_FILENAME}' is locked by Excel. {len(pending_rows_buffer)} packet(s) buffered safely in memory.")
    return flushed


def sync_html_files():
    """Syncs the latest rows of geosentinel_dataset.csv into standalone_dashboard_preview.html and dashboard.h."""
    if not os.path.isfile(CSV_FILENAME):
        return False
    try:
        with open(CSV_FILENAME, "r", encoding="utf-8", errors="ignore") as f:
            reader = csv.DictReader(f)
            records = [dict(row) for row in reader]
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


# ==============================================================================
# PHYSICAL GEOTECHNICAL CALCULATOR & RISK CLASSIFIER
# ==============================================================================
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


def evaluate_geotechnical_packet(data):
    """Computes derived units, vector magnitudes, tilts, and hazard status."""
    temp = data.get("TEMP", 25.0)
    hum = data.get("HUM", 50.0)
    data["TEMP_F"] = round(temp * 9.0 / 5.0 + 32.0, 2)
    data["DEW_POINT"] = calc_dew_point(temp, hum)

    soil_pct = data.get("SOIL_PCT", 0.0)
    soil_raw = data.get("SOIL_RAW", int(4095 - (soil_pct / 100.0) * 3095))
    data["SOIL_RAW"] = soil_raw
    data["SOIL_COND"] = "DRY" if soil_pct < 25 else ("MODERATE" if soil_pct < 65 else ("WET" if soil_pct < 85 else "SATURATED"))

    rain_pct = data.get("RAIN_PCT", 0.0)
    rain_raw = data.get("RAIN_RAW", int(4095 - (rain_pct / 100.0) * 3963))
    data["RAIN_RAW"] = rain_raw
    data["RAIN_COND"] = "NO_RAIN" if rain_pct < 10 else ("LIGHT_RAIN" if rain_pct < 45 else "HEAVY_RAIN")

    ax_g = data.get("AX_G", 0.0)
    ay_g = data.get("AY_G", 0.0)
    az_g = data.get("AZ_G", 1.0)
    data["AX_RAW"] = data.get("AX_RAW", int(ax_g * 16384))
    data["AY_RAW"] = data.get("AY_RAW", int(ay_g * 16384))
    data["AZ_RAW"] = data.get("AZ_RAW", int(az_g * 16384))
    data["ACCEL_MAG"] = round(math.sqrt(ax_g*ax_g + ay_g*ay_g + az_g*az_g), 3)

    gx_dps = data.get("GX_DPS", 0.0)
    gy_dps = data.get("GY_DPS", 0.0)
    gz_dps = data.get("GZ_DPS", 0.0)
    data["GX_RAW"] = data.get("GX_RAW", int(gx_dps * 131))
    data["GY_RAW"] = data.get("GY_RAW", int(gy_dps * 131))
    data["GZ_RAW"] = data.get("GZ_RAW", int(gz_dps * 131))
    data["GYRO_MAG"] = round(math.sqrt(gx_dps*gx_dps + gy_dps*gy_dps + gz_dps*gz_dps), 2)

    if "TX" not in data or data["TX"] == "":
        data["TX"] = round(math.atan2(ay_g, az_g) * 180.0 / math.pi, 2) if (ay_g != 0 or az_g != 0) else 0.0
    if "TY" not in data or data["TY"] == "":
        data["TY"] = round(math.atan2(-ax_g, math.sqrt(ay_g*ay_g + az_g*az_g)) * 180.0 / math.pi, 2) if (ay_g != 0 or az_g != 0 or ax_g != 0) else 0.0

    is_motion = (data["ACCEL_MAG"] > 1.45 or data["ACCEL_MAG"] < 0.55) or (data["GYRO_MAG"] > 18.0) or (abs(data["TX"]) > 15.0 or abs(data["TY"]) > 15.0)
    is_high_sat = (soil_pct > 80.0 and rain_pct > 45.0)

    if is_motion and is_high_sat:
        data["RISK_LEVEL"] = "CRITICAL ALERT"
        data["HAZARD_STATUS"] = "HIGH_LANDSLIDE_HAZARD"
    elif is_motion:
        data["RISK_LEVEL"] = "WARNING"
        data["HAZARD_STATUS"] = "GROUND_SHIFT_DETECTED"
    elif is_high_sat or (soil_pct > 75.0 and rain_pct > 40.0):
        data["RISK_LEVEL"] = "ELEVATED"
        data["HAZARD_STATUS"] = "SATURATION_WARNING"
    elif soil_pct > 60.0 or rain_pct > 25.0:
        data["RISK_LEVEL"] = "ADVISORY"
        data["HAZARD_STATUS"] = "MOISTURE_ADVISORY"
    else:
        data["RISK_LEVEL"] = "SAFE"
        data["HAZARD_STATUS"] = "NORMAL_STABLE"

    data["raw_line"] = (
        f"GS1,TV:1,T:{temp:.2f},H:{hum:.2f},SRAW:{data['SOIL_RAW']},SM:{soil_pct:.1f},SC:{data['SOIL_COND']},"
        f"RRAW:{data['RAIN_RAW']},RP:{rain_pct:.1f},RC:{data['RAIN_COND']},MV:1,TX:{data['TX']:.2f},TY:{data['TY']:.2f},"
        f"AXG:{ax_g:.3f},AYG:{ay_g:.3f},AZG:{az_g:.3f},GXD:{gx_dps:.2f},GYD:{gy_dps:.2f},GZD:{gz_dps:.2f}"
    )
    return data


# ==============================================================================
# PARSER FOR LIVE HARDWARE SERIAL STREAM
# ==============================================================================
rx_trans_batch = {}

def parse_serial_line(raw_line):
    """Parses both single-line receiver packets and multi-line sensor streams."""
    global rx_trans_batch
    clean = raw_line.strip()
    if not clean:
        return None

    # Format 1: Single line packet (GS1,... or TEMP:... or comma-separated)
    if clean.startswith("GS1") or (clean.count(',') >= 4 and ":" in clean):
        tokens = [t.strip() for t in clean.split(',') if ':' in t]
        raw_dict = {}
        for tok in tokens:
            parts = tok.split(':', 1)
            if len(parts) == 2:
                raw_dict[parts[0].strip().upper()] = parts[1].strip()

        data = {}
        for k in ["TEMP", "T", "TEMPERATURE"]:
            if k in raw_dict:
                try: data["TEMP"] = float(raw_dict[k]); break
                except ValueError: pass

        for k in ["HUM", "H", "HUMIDITY"]:
            if k in raw_dict:
                try: data["HUM"] = float(raw_dict[k]); break
                except ValueError: pass

        if "SM" in raw_dict:
            try: data["SOIL_PCT"] = float(raw_dict["SM"])
            except ValueError: pass
        if "SRAW" in raw_dict:
            try: data["SOIL_RAW"] = int(float(raw_dict["SRAW"]))
            except ValueError: pass
        elif "SOIL" in raw_dict:
            try:
                v = float(raw_dict["SOIL"])
                if v <= 100: data["SOIL_PCT"] = v
                else: data["SOIL_RAW"] = int(v); data["SOIL_PCT"] = max(0.0, min(100.0, round(((4095 - v) / 3095.0) * 100.0, 1)))
            except ValueError: pass

        if "RP" in raw_dict:
            try: data["RAIN_PCT"] = float(raw_dict["RP"])
            except ValueError: pass
        if "RRAW" in raw_dict:
            try: data["RAIN_RAW"] = int(float(raw_dict["RRAW"]))
            except ValueError: pass
        elif "RAIN" in raw_dict:
            try:
                v = float(raw_dict["RAIN"])
                if v <= 100: data["RAIN_PCT"] = v
                else: data["RAIN_RAW"] = int(v); data["RAIN_PCT"] = max(0.0, min(100.0, round(((4095 - v) / 3095.0) * 100.0, 1)))
            except ValueError: pass

        for k, target in [("AXG", "AX_G"), ("AYG", "AY_G"), ("AZG", "AZ_G"), ("GXD", "GX_DPS"), ("GYD", "GY_DPS"), ("GZD", "GZDPS")]:
            if k in raw_dict:
                try: data[target] = float(raw_dict[k])
                except ValueError: pass

        if "TX" in raw_dict:
            try: data["TX"] = float(raw_dict["TX"])
            except ValueError: pass
        if "TY" in raw_dict:
            try: data["TY"] = float(raw_dict["TY"])
            except ValueError: pass

        return evaluate_geotechnical_packet(data)

    # Format 2: Multi-line sensor node transmitter printouts
    if ":" in clean:
        parts = clean.split(":", 1)
        k = parts[0].strip().upper().replace(" ", "_")
        v = parts[1].strip()
        m = re.search(r"[-+]?\d*\.?\d+", v)
        num = float(m.group(0)) if m else None

        if "TEMP" in k and num is not None: rx_trans_batch["TEMP"] = num
        elif "HUM" in k and num is not None: rx_trans_batch["HUM"] = num
        elif ("SOIL_ADC" in k or k == "SOIL") and num is not None: rx_trans_batch["SOIL_RAW"] = int(num)
        elif "SOIL_MOISTURE" in k and num is not None: rx_trans_batch["SOIL_PCT"] = num
        elif ("RAIN_ADC" in k or k == "RAIN") and num is not None: rx_trans_batch["RAIN_RAW"] = int(num)
        elif "RAIN_RESPONSE" in k and num is not None: rx_trans_batch["RAIN_PCT"] = num
        elif "TILT_X" in k and num is not None: rx_trans_batch["TX"] = num
        elif "TILT_Y" in k and num is not None: rx_trans_batch["TY"] = num
        elif "ACCELERATION_X" in k and num is not None: rx_trans_batch["AX_G"] = num
        elif "ACCELERATION_Y" in k and num is not None: rx_trans_batch["AY_G"] = num
        elif "ACCELERATION_Z" in k and num is not None: rx_trans_batch["AZ_G"] = num
        elif "GYROSCOPE_X" in k and num is not None: rx_trans_batch["GX_DPS"] = num
        elif "GYROSCOPE_Y" in k and num is not None: rx_trans_batch["GY_DPS"] = num
        elif "GYROSCOPE_Z" in k and num is not None: rx_trans_batch["GZ_DPS"] = num
        elif "LORA_TRANSMISSION" in k:
            if len(rx_trans_batch) >= 4:
                assembled = evaluate_geotechnical_packet(dict(rx_trans_batch))
                rx_trans_batch = {}
                return assembled

    return None


# ==============================================================================
# AUTONOMOUS TELEMETRY SIMULATOR ENGINE
# ==============================================================================
class LandslideSimulationEngine:
    """Generates continuous physical telemetry following realistic geotechnical hazard cycles."""
    def __init__(self):
        self.step = 0
        self.phase = "NORMAL"
        self.temp_base = 30.5
        self.hum_base = 82.0
        self.soil_pct = 5.0
        self.rain_pct = 0.0
        self.tilt_x = 0.2
        self.tilt_y = 4.1

    def next_packet(self):
        self.step += 1
        cycle = self.step % 80

        if cycle < 20:
            # Phase 1: Baseline Stable
            self.phase = "NORMAL"
            self.rain_pct = max(0.0, self.rain_pct - 2.0)
            self.soil_pct = max(0.0, self.soil_pct - 1.0)
            ax_g = -0.07 + random.uniform(-0.015, 0.015)
            ay_g = 0.01 + random.uniform(-0.01, 0.01)
            az_g = 1.028 + random.uniform(-0.01, 0.01)
            gx = random.uniform(2.7, 3.3)
            gy = random.uniform(0.5, 1.1)
            gz = random.uniform(-0.4, 0.2)
        elif cycle < 40:
            # Phase 2: Moderate Rain, Soil Saturation Starts
            self.phase = "RAIN_START"
            self.rain_pct = min(100.0, self.rain_pct + 4.5)
            self.soil_pct = min(65.0, self.soil_pct + 2.8)
            ax_g = -0.07 + random.uniform(-0.02, 0.02)
            ay_g = 0.01 + random.uniform(-0.02, 0.02)
            az_g = 1.025 + random.uniform(-0.02, 0.02)
            gx = random.uniform(2.8, 3.4)
            gy = random.uniform(0.6, 1.2)
            gz = random.uniform(-0.5, 0.3)
        elif cycle < 55:
            # Phase 3: Heavy Precipitation & High Saturation (Elevated / Warning)
            self.phase = "SATURATED"
            self.rain_pct = min(100.0, 90.0 + random.uniform(-5, 10))
            self.soil_pct = min(92.0, self.soil_pct + 1.8)
            self.tilt_y += 0.25
            ax_g = -0.15 + random.uniform(-0.04, 0.04)
            ay_g = 0.08 + random.uniform(-0.03, 0.03)
            az_g = 1.01 + random.uniform(-0.03, 0.03)
            gx = random.uniform(3.5, 5.2)
            gy = random.uniform(1.2, 2.5)
            gz = random.uniform(-1.0, 0.8)
        elif cycle < 65:
            # Phase 4: Dynamic Slope Movement (Critical Hazard Alert)
            self.phase = "DISPLACEMENT"
            self.rain_pct = 100.0
            self.soil_pct = min(98.0, 85.0 + random.uniform(0, 10))
            self.tilt_y += random.uniform(1.5, 3.0)
            self.tilt_x += random.uniform(-1.0, 1.5)
            # Dynamic kinematic displacement spike
            ax_g = random.choice([-0.85, 0.95, -1.2, 1.1]) + random.uniform(-0.1, 0.1)
            ay_g = random.choice([-0.7, 0.8, -0.9, 0.6]) + random.uniform(-0.1, 0.1)
            az_g = random.uniform(0.4, 1.6)
            gx = random.uniform(15.0, 32.0)
            gy = random.uniform(8.0, 22.0)
            gz = random.uniform(-12.0, 16.0)
        else:
            # Phase 5: Event subsiding, ground settling
            self.phase = "SETTLING"
            self.rain_pct = max(0.0, self.rain_pct - 6.0)
            self.soil_pct = max(20.0, self.soil_pct - 3.0)
            ax_g = -0.09 + random.uniform(-0.02, 0.02)
            ay_g = 0.02 + random.uniform(-0.01, 0.01)
            az_g = 1.03 + random.uniform(-0.01, 0.01)
            gx = random.uniform(3.0, 3.6)
            gy = random.uniform(0.6, 1.1)
            gz = random.uniform(-0.4, 0.2)

        data = {
            "TEMP": round(self.temp_base + random.uniform(-0.4, 0.4), 1),
            "HUM": round(self.hum_base + random.uniform(-1.0, 1.0), 1),
            "SOIL_PCT": round(self.soil_pct, 1),
            "RAIN_PCT": round(self.rain_pct, 1),
            "AX_G": round(ax_g, 3),
            "AY_G": round(ay_g, 3),
            "AZ_G": round(az_g, 3),
            "GX_DPS": round(gx, 2),
            "GY_DPS": round(gy, 2),
            "GZ_DPS": round(gz, 2),
            "TX": round(self.tilt_x, 2),
            "TY": round(self.tilt_y, 2),
            "RSSI": random.choice([-78, -80, -82, -85, -88])
        }
        return evaluate_geotechnical_packet(data)


# ==============================================================================
# MAIN EXECUTION CONTROLLER
# ==============================================================================
def find_esp32_port():
    """Detects connected ESP32 ports, prioritizing COM3."""
    if not SERIAL_AVAILABLE:
        return None
    ports = list(serial.tools.list_ports.comports())
    if not ports:
        return None
    # Prefer COM3 (default Base Station)
    for p in ports:
        if p.device.upper() == "COM3":
            return p.device
    # Prefer CP210x / CH340 / USB UART bridges
    for p in ports:
        desc = (p.description or "").lower()
        if "cp210" in desc or "ch340" in desc or "uart" in desc or "serial" in desc:
            return p.device
    return ports[0].device


def print_banner(mode_name, last_id):
    print("=" * 82)
    print("       GeoSentinel — Automated Telemetry Packet Dataset Updater Engine       ")
    print("=" * 82)
    print(f" Target Dataset File : {os.path.abspath(CSV_FILENAME)}")
    print(f" Active Mode         : {mode_name}")
    print(f" Starting Packet ID  : #{last_id + 1}")
    print(" Press Ctrl+C at any time to safely halt logging.\n")
    print("-" * 82)
    print(f"{'Seq':<6} | {'Timestamp':<19} | {'Temp':<6} | {'Soil%':<6} | {'Rain%':<6} | {'|A| (g)':<7} | {'Tilt P/R':<10} | {'Hazard Status'}")
    print("-" * 82)


def main():
    parser = argparse.ArgumentParser(description="GeoSentinel Automated Dataset Packet Updater")
    parser.add_argument("--interval", type=float, default=3.0, help="Packet update interval in seconds (default: 3.0s)")
    parser.add_argument("--count", type=int, default=0, help="Number of packets to update (0 = continuous)")
    parser.add_argument("--port", type=str, default=None, help="Force specific COM port (e.g. COM3)")
    parser.add_argument("--simulate", action="store_true", help="Force autonomous geotechnical simulation mode")
    parser.add_argument("--reset", action="store_true", help="Reset dataset to initial packet #1")
    parser.add_argument("--status", action="store_true", help="Show current dataset status and exit")
    parser.add_argument("--sync", action="store_true", help="Sync geosentinel_dataset.csv into UI dashboard files and exit")

    args = parser.parse_args()

    if args.sync:
        print("\n[*] Synchronizing geosentinel_dataset.csv into UI dashboard files...")
        sync_html_files()
        print()
        return

    if args.status:
        st = get_dataset_stats()
        print("\nGeoSentinel Dataset Status:")
        print(f"  File: {os.path.abspath(CSV_FILENAME)}")
        print(f"  Exists: {st['exists']}")
        print(f"  Total Packets: {st['count']}")
        print(f"  Latest Packet ID: #{st['last_id']}")
        if st.get("last_row"):
            print(f"  Latest Timestamp: {st['last_row'][0]}")
            print(f"  Latest Risk: {st['last_row'][31] if len(st['last_row']) > 31 else 'N/A'}")
        print()
        return

    # 1. Initialize dataset
    current_packet_id = init_dataset(reset=args.reset)

    # 2. Determine mode
    chosen_port = None
    if not args.simulate and SERIAL_AVAILABLE:
        chosen_port = args.port or find_esp32_port()

    # 3. Start Updater Loop
    packets_updated = 0

    if chosen_port:
        mode_desc = f"LIVE HARDWARE SERIAL ({chosen_port} @ {BAUD_RATE} baud)"
        print_banner(mode_desc, current_packet_id)

        ser = None
        try:
            ser = serial.Serial()
            ser.port = chosen_port
            ser.baudrate = BAUD_RATE
            ser.timeout = 1.5
            ser.dtr = False
            ser.rts = False
            ser.open()
            print(f"[OK] Connected to hardware port {chosen_port}. Listening for live packets...\n")

            last_known_rssi = -80
            while True:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                if not line:
                    continue

                rssi_match = re.search(r"RSSI:\s*([-\d]+)", line, re.IGNORECASE)
                if rssi_match:
                    last_known_rssi = int(rssi_match.group(1))

                parsed = parse_serial_line(line)
                if parsed:
                    current_packet_id += 1
                    packets_updated += 1
                    now_str = datetime.now().strftime("%d-%m-%Y %H:%M:%S")
                    raw_str = parsed.get("raw_line", line)

                    append_packet_to_dataset(now_str, current_packet_id, parsed, raw_str, last_known_rssi)

                    temp_str = f"{parsed.get('TEMP', 0.0):.1f}°C"
                    soil_str = f"{parsed.get('SOIL_PCT', 0.0):.0f}%"
                    rain_str = f"{parsed.get('RAIN_PCT', 0.0):.0f}%"
                    acc_str = f"{parsed.get('ACCEL_MAG', 1.0):.2f}g"
                    tilt_str = f"{parsed.get('TX', 0):.1f}/{parsed.get('TY', 0):.1f}°"
                    risk_str = parsed.get('HAZARD_STATUS', 'NORMAL_STABLE')

                    print(f"#{current_packet_id:<5} | {now_str} | {temp_str:<6} | {soil_str:<6} | {rain_str:<6} | {acc_str:<7} | {tilt_str:<10} | {risk_str}")

                    if args.count > 0 and packets_updated >= args.count:
                        print(f"\n[OK] Reached target count of {args.count} packets. Finished.")
                        sync_html_files()
                        break

        except serial.SerialException as e:
            print(f"\n[!] Serial communication error on {chosen_port}: {e}")
            print("    Switching seamlessly to Autonomous Telemetry Simulator Mode...\n")
            chosen_port = None
        except KeyboardInterrupt:
            print(f"\n[OK] Stopped by user. {packets_updated} packet(s) updated in '{CSV_FILENAME}'.")
            if packets_updated > 0:
                sync_html_files()
            return
        finally:
            if ser and ser.is_open:
                ser.close()

    if not chosen_port:
        mode_desc = f"AUTONOMOUS SIMULATION (Interval: {args.interval}s)"
        print_banner(mode_desc, current_packet_id)
        sim = LandslideSimulationEngine()

        try:
            while True:
                current_packet_id += 1
                packets_updated += 1
                now_str = datetime.now().strftime("%d-%m-%Y %H:%M:%S")

                pkt = sim.next_packet()
                raw_str = pkt.get("raw_line", "")

                append_packet_to_dataset(now_str, current_packet_id, pkt, raw_str, pkt.get("RSSI", -80))

                temp_str = f"{pkt.get('TEMP', 0.0):.1f}°C"
                soil_str = f"{pkt.get('SOIL_PCT', 0.0):.0f}%"
                rain_str = f"{pkt.get('RAIN_PCT', 0.0):.0f}%"
                acc_str = f"{pkt.get('ACCEL_MAG', 1.0):.2f}g"
                tilt_str = f"{pkt.get('TX', 0):.1f}/{pkt.get('TY', 0):.1f}°"
                risk_str = pkt.get('HAZARD_STATUS', 'NORMAL_STABLE')

                print(f"#{current_packet_id:<5} | {now_str} | {temp_str:<6} | {soil_str:<6} | {rain_str:<6} | {acc_str:<7} | {tilt_str:<10} | {risk_str}")

                if args.count > 0 and packets_updated >= args.count:
                    print(f"\n[OK] Reached target count of {args.count} packets. Finished.")
                    sync_html_files()
                    break

                time.sleep(args.interval)

        except KeyboardInterrupt:
            print(f"\n[OK] Stopped by user. {packets_updated} packet(s) updated in '{CSV_FILENAME}'.")
            if packets_updated > 0:
                sync_html_files()


if __name__ == "__main__":
    main()
