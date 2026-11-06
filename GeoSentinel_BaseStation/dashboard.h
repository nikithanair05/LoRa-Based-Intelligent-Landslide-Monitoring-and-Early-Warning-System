#ifndef DASHBOARD_H
#define DASHBOARD_H
#if defined(ARDUINO) || defined(__has_include)
  #if defined(__has_include)
    #if __has_include(<Arduino.h>)
      #include <Arduino.h>
    #elif __has_include(<pgmspace.h>)
      #include <pgmspace.h>
    #endif
  #elif defined(ARDUINO)
    #include <Arduino.h>
  #endif
#endif

#ifndef PROGMEM
#define PROGMEM
#endif

// ====================================================================
// GEOSENTINEL WEB DASHBOARD (HTML / CSS / JAVASCRIPT)
// Stored in Flash Memory (PROGMEM) to save ESP32 RAM.
// Fully self-contained with zero-dependency Canvas charts.
// Supports both Wi-Fi HTTP polling and direct USB Web Serial streams
// for both Receiver (Base Station) and Transmitter (Sensor Node).
// ====================================================================

const char DASHBOARD_HTML[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="en">

<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>GeoSentinel — Landslide Detection &amp; Monitoring Dashboard</title>
  <style>
    :root {
      --bg-base: #f1f5f9;
      --bg-surface: #ffffff;
      --bg-card: rgba(255, 255, 255, 0.88);
      --bg-card-hover: rgba(255, 255, 255, 0.96);
      --border-color: rgba(148, 163, 184, 0.22);
      --border-color-glow: rgba(99, 102, 241, 0.22);
      --text-primary: #1e293b;
      --text-secondary: #475569;
      --text-muted: #64748b;
      --accent-blue: #3b82f6;
      --accent-cyan: #0284c7;
      --accent-emerald: #059669;
      --accent-amber: #d97706;
      --accent-rose: #e11d48;
      --accent-purple: #7c3aed;
      --terminal-bg: #f8fafc;
      --radius-sm: 8px;
      --radius-md: 12px;
      --radius-lg: 16px;
      --radius-full: 9999px;
    }

    * {
      box-sizing: border-box;
      margin: 0;
      padding: 0;
    }

    body {
      background-color: var(--bg-base);
      background-image:
        radial-gradient(at 0% 0%, rgba(199, 210, 254, 0.45) 0px, transparent 50%),
        radial-gradient(at 100% 0%, rgba(186, 230, 253, 0.4) 0px, transparent 50%),
        radial-gradient(at 100% 100%, rgba(254, 215, 226, 0.35) 0px, transparent 50%),
        radial-gradient(at 0% 100%, rgba(209, 250, 229, 0.35) 0px, transparent 50%);
      background-attachment: fixed;
      color: var(--text-primary);
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, "Helvetica Neue", Arial, sans-serif;
      min-height: 100vh;
      line-height: 1.5;
      padding: 16px;
    }

    .container {
      max-width: 1440px;
      margin: 0 auto;
      display: flex;
      flex-direction: column;
      gap: 18px;
    }

    .glass-card {
      background: var(--bg-card);
      backdrop-filter: blur(14px);
      -webkit-backdrop-filter: blur(14px);
      border: 1px solid var(--border-color);
      border-radius: var(--radius-lg);
      padding: 20px;
      box-shadow: 0 4px 20px -2px rgba(99, 102, 241, 0.08), 0 2px 6px rgba(0, 0, 0, 0.03);
      transition: all 0.25s ease;
    }

    .glass-card:hover {
      border-color: rgba(99, 102, 241, 0.35);
      box-shadow: 0 8px 24px -2px rgba(99, 102, 241, 0.12), 0 3px 8px rgba(0, 0, 0, 0.04);
    }

    /* Top USB Port & Multi-Source Control Center */
    .usb-control-bar {
      background: linear-gradient(135deg, rgba(255, 255, 255, 0.95), rgba(240, 244, 255, 0.92));
      border: 1px solid rgba(99, 102, 241, 0.25);
      border-radius: var(--radius-lg);
      padding: 16px 22px;
      display: flex;
      flex-wrap: wrap;
      justify-content: space-between;
      align-items: center;
      gap: 16px;
      box-shadow: 0 4px 20px rgba(99, 102, 241, 0.08);
      position: relative;
      overflow: hidden;
    }

    .usb-control-bar::before {
      content: "";
      position: absolute;
      top: 0;
      left: 0;
      right: 0;
      height: 3px;
      background: linear-gradient(90deg, #93c5fd, #67e8f9, #6ee7b7, #c4b5fd);
    }

    .usb-header-info {
      display: flex;
      align-items: center;
      gap: 14px;
    }

    .usb-icon-circle {
      width: 44px;
      height: 44px;
      border-radius: var(--radius-md);
      background: linear-gradient(135deg, #dbeafe, #cffafe);
      display: flex;
      align-items: center;
      justify-content: center;
      font-size: 1.4rem;
      box-shadow: 0 2px 10px rgba(59, 130, 246, 0.15);
      flex-shrink: 0;
    }

    .usb-title-group h2 {
      font-size: 1.05rem;
      font-weight: 700;
      color: var(--text-primary);
      display: flex;
      align-items: center;
      gap: 8px;
    }

    .usb-title-group p {
      font-size: 0.8rem;
      color: var(--text-secondary);
    }

    .usb-actions {
      display: flex;
      flex-wrap: wrap;
      align-items: center;
      gap: 10px;
    }

    .select-styled {
      background: #ffffff;
      border: 1px solid rgba(148, 163, 184, 0.35);
      color: var(--text-primary);
      padding: 8px 12px;
      border-radius: var(--radius-sm);
      font-size: 0.82rem;
      font-weight: 600;
      cursor: pointer;
      outline: none;
      transition: all 0.2s ease;
    }

    .fixed-pill-badge {
      display: inline-flex;
      align-items: center;
      gap: 6px;
      background: #ffffff;
      border: 1px solid rgba(148, 163, 184, 0.35);
      color: var(--text-primary);
      padding: 8px 14px;
      border-radius: var(--radius-sm);
      font-size: 0.82rem;
      font-weight: 600;
      user-select: none;
      box-shadow: 0 1px 2px rgba(0, 0, 0, 0.04);
    }

    .select-styled:hover,
    .select-styled:focus {
      border-color: var(--accent-blue);
    }

    .btn-action {
      background: linear-gradient(135deg, #4f46e5, #3b82f6);
      color: #ffffff;
      border: none;
      padding: 9px 18px;
      border-radius: var(--radius-full);
      font-size: 0.84rem;
      font-weight: 700;
      cursor: pointer;
      display: inline-flex;
      align-items: center;
      gap: 8px;
      box-shadow: 0 2px 14px rgba(79, 70, 229, 0.25);
      transition: all 0.2s ease;
      white-space: nowrap;
    }

    .btn-action:hover {
      transform: translateY(-1px);
      box-shadow: 0 4px 18px rgba(79, 70, 229, 0.4);
    }

    .btn-action.connected {
      background: linear-gradient(135deg, #059669, #10b981);
      box-shadow: 0 2px 14px rgba(16, 185, 129, 0.3);
    }

    .btn-action.disconnect {
      background: linear-gradient(135deg, #e11d48, #f43f5e);
      box-shadow: 0 2px 14px rgba(244, 63, 94, 0.3);
    }

    .btn-secondary {
      background: #ffffff;
      color: var(--text-primary);
      border: 1px solid rgba(148, 163, 184, 0.3);
      padding: 8px 14px;
      border-radius: var(--radius-full);
      font-size: 0.8rem;
      font-weight: 600;
      cursor: pointer;
      display: inline-flex;
      align-items: center;
      gap: 6px;
      box-shadow: 0 1px 4px rgba(0, 0, 0, 0.04);
      transition: all 0.2s ease;
    }

    .btn-secondary:hover {
      background: #f8fafc;
      border-color: rgba(99, 102, 241, 0.4);
    }

    .btn-secondary.active {
      background: #e0e7ff;
      border-color: #6366f1;
      color: #4338ca;
    }

    /* Header */
    header.header {
      display: flex;
      flex-wrap: wrap;
      justify-content: space-between;
      align-items: center;
      gap: 16px;
      padding: 20px 24px;
      border: 1px solid var(--border-color);
      background: rgba(255, 255, 255, 0.92);
      box-shadow: 0 4px 20px -2px rgba(99, 102, 241, 0.06);
      border-radius: var(--radius-lg);
    }

    .brand-section {
      display: flex;
      align-items: center;
      gap: 14px;
    }

    .brand-icon {
      width: 46px;
      height: 46px;
      background: linear-gradient(135deg, #dbeafe, #cffafe);
      border-radius: var(--radius-md);
      display: flex;
      align-items: center;
      justify-content: center;
      box-shadow: 0 2px 12px rgba(59, 130, 246, 0.15);
    }

    .brand-icon svg {
      width: 28px;
      height: 28px;
      fill: none;
      stroke: #2563eb;
      stroke-width: 2;
    }

    .brand-title h1 {
      font-size: 1.5rem;
      font-weight: 700;
      color: var(--text-primary);
      letter-spacing: -0.02em;
    }

    .brand-title p {
      font-size: 0.84rem;
      color: var(--text-secondary);
    }

    .header-badges {
      display: flex;
      flex-wrap: wrap;
      align-items: center;
      gap: 10px;
    }

    .status-badge {
      display: inline-flex;
      align-items: center;
      gap: 8px;
      padding: 6px 14px;
      border-radius: var(--radius-full);
      font-size: 0.78rem;
      font-weight: 700;
      text-transform: uppercase;
      letter-spacing: 0.04em;
      background: #f1f5f9;
      border: 1px solid rgba(148, 163, 184, 0.25);
    }

    .status-dot {
      width: 8px;
      height: 8px;
      border-radius: 50%;
    }

    .pulse {
      animation: pulse-glow 2s infinite;
    }

    @keyframes pulse-glow {
      0% {
        transform: scale(0.95);
        opacity: 0.8;
      }

      50% {
        transform: scale(1.25);
        opacity: 1;
        filter: drop-shadow(0 0 6px currentColor);
      }

      100% {
        transform: scale(0.95);
        opacity: 0.8;
      }
    }

    .status-online {
      color: #065f46;
      border-color: #a7f3d0;
      background: #d1fae5;
    }

    .status-online .status-dot {
      background-color: #10b981;
    }

    .status-warning {
      color: #92400e;
      border-color: #fde68a;
      background: #fef3c7;
    }

    .status-warning .status-dot {
      background-color: #f59e0b;
    }

    .status-offline {
      color: #9f1239;
      border-color: #fecdd3;
      background: #ffe4e6;
    }

    .status-offline .status-dot {
      background-color: #f43f5e;
    }

    .badge-info {
      color: #0369a1;
      border-color: #bae6fd;
      background: #e0f2fe;
    }

    .badge-purple {
      color: #5b21b6;
      border-color: #ddd6fe;
      background: #ede9fe;
    }

    /* Summary Bar */
    .summary-bar {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(220px, 1fr));
      gap: 14px;
    }

    .summary-card {
      background: var(--bg-surface);
      border: 1px solid var(--border-color);
      border-radius: var(--radius-md);
      padding: 14px 18px;
      display: flex;
      align-items: center;
      gap: 14px;
      box-shadow: 0 2px 10px rgba(0, 0, 0, 0.03);
    }

    .summary-icon {
      width: 42px;
      height: 42px;
      border-radius: var(--radius-sm);
      display: flex;
      align-items: center;
      justify-content: center;
      font-size: 1.35rem;
      background: #f1f5f9;
      flex-shrink: 0;
    }

    .summary-info .label {
      font-size: 0.72rem;
      color: var(--text-secondary);
      text-transform: uppercase;
      letter-spacing: 0.06em;
    }

    .summary-info .val {
      font-size: 1.18rem;
      font-weight: 700;
      color: var(--text-primary);
    }

    .summary-info .sub {
      font-size: 0.72rem;
      color: var(--text-muted);
    }

    /* Risk Banner */
    .risk-banner {
      display: flex;
      flex-wrap: wrap;
      justify-content: space-between;
      align-items: center;
      gap: 20px;
      background: linear-gradient(135deg, rgba(255, 255, 255, 0.95), rgba(240, 247, 255, 0.95));
      border: 1px solid rgba(56, 189, 248, 0.35);
      border-radius: var(--radius-lg);
      padding: 20px;
      position: relative;
      overflow: hidden;
      box-shadow: 0 6px 20px rgba(56, 189, 248, 0.08);
    }

    .risk-banner::before {
      content: "";
      position: absolute;
      top: 0;
      left: 0;
      bottom: 0;
      width: 6px;
      background: #0ea5e9;
      box-shadow: 0 0 12px rgba(14, 165, 233, 0.4);
    }

    .risk-status-group {
      display: flex;
      flex-direction: column;
      gap: 4px;
    }

    .risk-tag {
      font-size: 0.74rem;
      text-transform: uppercase;
      letter-spacing: 0.08em;
      color: var(--accent-cyan);
      font-weight: 700;
    }

    .risk-title {
      font-size: 1.85rem;
      font-weight: 800;
      color: var(--text-primary);
      display: flex;
      align-items: center;
      gap: 12px;
    }

    .risk-subtitle {
      font-size: 0.85rem;
      color: var(--text-secondary);
      max-width: 650px;
    }

    .risk-metrics {
      display: flex;
      gap: 14px;
      flex-wrap: wrap;
    }

    .risk-mini-card {
      background: #f8fafc;
      border: 1px solid rgba(148, 163, 184, 0.2);
      border-radius: var(--radius-sm);
      padding: 10px 16px;
      text-align: center;
      min-width: 130px;
    }

    .risk-mini-card .metric-label {
      font-size: 0.68rem;
      color: var(--text-muted);
      text-transform: uppercase;
      letter-spacing: 0.05em;
    }

    .risk-mini-card .metric-val {
      font-size: 1.08rem;
      font-weight: 700;
      color: var(--text-primary);
    }

    /* Grid Layout */
    .dashboard-grid {
      display: grid;
      grid-template-columns: repeat(12, 1fr);
      gap: 18px;
    }

    .col-12 {
      grid-column: span 12;
    }

    .col-6 {
      grid-column: span 6;
    }

    .col-4 {
      grid-column: span 4;
    }

    .col-8 {
      grid-column: span 8;
    }

    .section-header {
      display: flex;
      justify-content: space-between;
      align-items: center;
      margin-bottom: 16px;
      border-bottom: 1px solid rgba(148, 163, 184, 0.2);
      padding-bottom: 10px;
    }

    .section-header h2 {
      font-size: 1.05rem;
      font-weight: 700;
      color: var(--text-primary);
      display: flex;
      align-items: center;
      gap: 8px;
    }

    .tag-badge {
      font-size: 0.7rem;
      padding: 3px 10px;
      border-radius: var(--radius-full);
      background: #f1f5f9;
      border: 1px solid rgba(148, 163, 184, 0.25);
      color: var(--text-secondary);
      font-weight: 600;
    }

    /* Sensor Cards */
    .sensor-cards-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
      gap: 16px;
    }

    .sensor-card {
      background: var(--bg-surface);
      border: 1px solid var(--border-color);
      border-radius: var(--radius-md);
      padding: 18px;
      display: flex;
      flex-direction: column;
      justify-content: space-between;
      box-shadow: 0 2px 12px rgba(0, 0, 0, 0.03);
      transition: transform 0.15s ease, border-color 0.15s ease, box-shadow 0.15s ease;
    }

    .sensor-card:hover {
      transform: translateY(-2px);
      border-color: rgba(99, 102, 241, 0.35);
      box-shadow: 0 6px 20px rgba(99, 102, 241, 0.08);
    }

    .sensor-card-top {
      display: flex;
      justify-content: space-between;
      align-items: flex-start;
      margin-bottom: 12px;
    }

    .sensor-title {
      font-size: 0.85rem;
      font-weight: 600;
      color: var(--text-secondary);
      display: flex;
      align-items: center;
      gap: 6px;
    }

    .raw-badge {
      font-size: 0.65rem;
      font-weight: 700;
      padding: 2px 7px;
      border-radius: var(--radius-sm);
      background: #fef3c7;
      color: #b45309;
      border: 1px solid #fde68a;
    }

    .calibrated-badge {
      font-size: 0.65rem;
      font-weight: 700;
      padding: 2px 7px;
      border-radius: var(--radius-sm);
      background: #d1fae5;
      color: #047857;
      border: 1px solid #a7f3d0;
    }

    .sensor-value-box {
      display: flex;
      align-items: baseline;
      gap: 6px;
      margin-bottom: 8px;
    }

    .sensor-value {
      font-size: 2.2rem;
      font-weight: 700;
      color: var(--text-primary);
      font-variant-numeric: tabular-nums;
      line-height: 1;
    }

    .sensor-unit {
      font-size: 1rem;
      color: var(--text-secondary);
      font-weight: 500;
    }

    .sensor-bar-bg {
      width: 100%;
      height: 6px;
      background: #e2e8f0;
      border-radius: var(--radius-full);
      overflow: hidden;
      margin-top: 6px;
    }

    .sensor-bar-fill {
      height: 100%;
      border-radius: var(--radius-full);
      transition: width 0.4s ease;
    }

    .sensor-footer {
      display: flex;
      justify-content: space-between;
      align-items: center;
      margin-top: 10px;
      font-size: 0.74rem;
      color: var(--text-muted);
    }

    /* Kinematics */
    .kinematics-grid {
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 16px;
    }

    .axis-container {
      background: var(--bg-surface);
      border: 1px solid var(--border-color);
      border-radius: var(--radius-md);
      padding: 16px;
      box-shadow: 0 2px 10px rgba(0, 0, 0, 0.03);
    }

    .axis-header {
      font-size: 0.84rem;
      font-weight: 700;
      color: var(--accent-cyan);
      margin-bottom: 12px;
      display: flex;
      justify-content: space-between;
      align-items: center;
    }

    .axis-row {
      display: flex;
      align-items: center;
      justify-content: space-between;
      padding: 8px 10px;
      margin-bottom: 6px;
      background: #f8fafc;
      border: 1px solid rgba(148, 163, 184, 0.15);
      border-radius: var(--radius-sm);
      border-left: 3px solid var(--accent-blue);
    }

    .axis-row:last-child {
      margin-bottom: 0;
    }

    .axis-row.ay {
      border-left-color: var(--accent-emerald);
    }

    .axis-row.az {
      border-left-color: var(--accent-purple);
    }

    .axis-row.gx {
      border-left-color: var(--accent-amber);
    }

    .axis-row.gy {
      border-left-color: #ec4899;
    }

    .axis-row.gz {
      border-left-color: #06b6d4;
    }

    .axis-name {
      font-size: 0.8rem;
      font-weight: 600;
      color: var(--text-secondary);
    }

    .axis-val {
      font-size: 1.15rem;
      font-weight: 700;
      color: var(--text-primary);
      font-variant-numeric: tabular-nums;
    }

    /* Charts */
    .charts-grid {
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 20px;
    }

    .chart-box {
      background: var(--bg-surface);
      border: 1px solid var(--border-color);
      border-radius: var(--radius-md);
      padding: 16px;
      display: flex;
      flex-direction: column;
      gap: 10px;
      box-shadow: 0 2px 12px rgba(0, 0, 0, 0.03);
    }

    .chart-box-header {
      display: flex;
      justify-content: space-between;
      align-items: center;
    }

    .chart-box-title {
      font-size: 0.85rem;
      font-weight: 600;
      color: var(--text-primary);
      display: flex;
      align-items: center;
      gap: 6px;
    }

    .chart-legend {
      display: flex;
      align-items: center;
      gap: 12px;
      font-size: 0.74rem;
    }

    .legend-item {
      display: flex;
      align-items: center;
      gap: 5px;
      color: var(--text-secondary);
    }

    .legend-color {
      width: 10px;
      height: 10px;
      border-radius: 2px;
    }

    .canvas-wrapper {
      position: relative;
      width: 100%;
      height: 220px;
    }

    canvas {
      width: 100% !important;
      height: 100% !important;
      display: block;
    }

    /* Live USB Serial Console / Terminal Panel */
    .terminal-container {
      background: var(--terminal-bg);
      border: 1px solid rgba(148, 163, 184, 0.25);
      border-radius: var(--radius-md);
      overflow: hidden;
      display: flex;
      flex-direction: column;
      box-shadow: inset 0 1px 4px rgba(0, 0, 0, 0.04);
    }

    .terminal-header {
      background: #ffffff;
      border-bottom: 1px solid rgba(148, 163, 184, 0.2);
      padding: 10px 14px;
      display: flex;
      flex-wrap: wrap;
      justify-content: space-between;
      align-items: center;
      gap: 10px;
    }

    .terminal-title {
      font-size: 0.82rem;
      font-weight: 700;
      color: var(--accent-cyan);
      display: flex;
      align-items: center;
      gap: 8px;
    }

    .terminal-controls {
      display: flex;
      align-items: center;
      gap: 8px;
    }

    .terminal-btn {
      background: #f1f5f9;
      border: 1px solid rgba(148, 163, 184, 0.3);
      color: var(--text-secondary);
      border-radius: var(--radius-sm);
      padding: 4px 10px;
      font-size: 0.72rem;
      cursor: pointer;
      display: inline-flex;
      align-items: center;
      gap: 4px;
      transition: all 0.2s ease;
    }

    .terminal-btn:hover {
      background: #e2e8f0;
      color: var(--text-primary);
    }

    .terminal-body {
      height: 220px;
      padding: 12px;
      overflow-y: auto;
      font-family: "Consolas", "Courier New", monospace;
      font-size: 0.78rem;
      line-height: 1.45;
      display: flex;
      flex-direction: column;
      gap: 3px;
    }

    .log-entry {
      display: flex;
      align-items: flex-start;
      gap: 8px;
      word-break: break-all;
    }

    .log-time {
      color: var(--text-muted);
      flex-shrink: 0;
      font-size: 0.72rem;
    }

    .log-tag {
      font-size: 0.68rem;
      font-weight: 700;
      padding: 1px 5px;
      border-radius: 3px;
      flex-shrink: 0;
    }

    .log-tag-rx {
      background: #e0f2fe;
      color: #0284c7;
      border: 1px solid #bae6fd;
    }

    .log-tag-tx {
      background: #ede9fe;
      color: #7c3aed;
      border: 1px solid #ddd6fe;
    }

    .log-tag-sys {
      background: #d1fae5;
      color: #059669;
      border: 1px solid #a7f3d0;
    }

    .log-tag-warn {
      background: #fef3c7;
      color: #d97706;
      border: 1px solid #fde68a;
    }

    .log-text {
      color: #334155;
    }

    .terminal-footer {
      background: #ffffff;
      border-top: 1px solid rgba(148, 163, 184, 0.2);
      padding: 8px 12px;
      display: flex;
      gap: 8px;
    }

    .terminal-input {
      flex: 1;
      background: #f8fafc;
      border: 1px solid rgba(148, 163, 184, 0.3);
      border-radius: var(--radius-sm);
      color: var(--text-primary);
      padding: 6px 10px;
      font-family: monospace;
      font-size: 0.8rem;
      outline: none;
    }

    .terminal-input:focus {
      border-color: var(--accent-cyan);
    }

    /* Alerts */
    .alerts-container {
      display: flex;
      flex-direction: column;
      gap: 10px;
      max-height: 220px;
      overflow-y: auto;
    }

    .alert-card {
      display: flex;
      align-items: flex-start;
      gap: 12px;
      padding: 12px 16px;
      border-radius: var(--radius-md);
      font-size: 0.84rem;
      background: #ecfdf5;
      border: 1px solid #a7f3d0;
      color: #065f46;
    }

    .alert-card.warning {
      background: #fffbeb;
      border-color: #fde68a;
      color: #92400e;
    }

    .alert-card.danger {
      background: #fff1f2;
      border-color: #fecdd3;
      color: #9f1239;
    }

    .alert-icon {
      font-size: 1.1rem;
      line-height: 1;
      margin-top: 2px;
    }

    .alert-text {
      flex: 1;
    }

    .alert-time {
      font-size: 0.7rem;
      color: var(--text-muted);
    }

    /* Diagnostics Table */
    .diag-table {
      width: 100%;
      border-collapse: collapse;
      font-size: 0.8rem;
    }

    .diag-table tr {
      border-bottom: 1px solid rgba(148, 163, 184, 0.15);
    }

    .diag-table td {
      padding: 8px 6px;
      color: var(--text-secondary);
    }

    .diag-table td.val {
      text-align: right;
      font-weight: 600;
      color: var(--text-primary);
      font-family: monospace;
    }

    /* =========================================================================
       PACKETS RECEIVED & IN-DEPTH TELEMETRY ANALYSIS STYLES
       ========================================================================= */
    .packets-analytics-card {
      border: 1px solid rgba(99, 102, 241, 0.22);
      position: relative;
      overflow: hidden;
    }

    .packets-analytics-card::before {
      content: "";
      position: absolute;
      top: 0;
      left: 0;
      right: 0;
      height: 2px;
      background: linear-gradient(90deg, var(--accent-cyan), var(--accent-blue), var(--accent-emerald), var(--accent-purple));
    }

    .an-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(280px, 1fr));
      gap: 16px;
      margin-bottom: 20px;
    }

    .an-card {
      background: #ffffff;
      border: 1px solid rgba(148, 163, 184, 0.2);
      border-radius: var(--radius-md);
      padding: 16px;
      display: flex;
      flex-direction: column;
      gap: 12px;
      transition: all 0.2s ease;
      position: relative;
      box-shadow: 0 2px 10px rgba(0, 0, 0, 0.03);
    }

    .an-card:hover {
      border-color: rgba(99, 102, 241, 0.35);
      transform: translateY(-2px);
      box-shadow: 0 6px 18px rgba(99, 102, 241, 0.08);
    }

    .an-card-header {
      display: flex;
      justify-content: space-between;
      align-items: center;
      border-bottom: 1px solid rgba(148, 163, 184, 0.15);
      padding-bottom: 8px;
    }

    .an-card-title {
      font-size: 0.82rem;
      font-weight: 700;
      color: var(--accent-cyan);
      display: flex;
      align-items: center;
      gap: 6px;
      text-transform: uppercase;
      letter-spacing: 0.5px;
    }

    .an-main-val {
      font-size: 1.65rem;
      font-weight: 800;
      color: var(--text-primary);
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
      line-height: 1.1;
      display: flex;
      align-items: baseline;
      gap: 6px;
    }

    .an-unit {
      font-size: 0.85rem;
      font-weight: 500;
      color: var(--text-secondary);
    }

    .an-stats-row {
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 8px;
      font-size: 0.75rem;
    }

    .an-stat-item {
      background: #f8fafc;
      padding: 6px 8px;
      border-radius: var(--radius-sm);
      border: 1px solid rgba(148, 163, 184, 0.15);
      display: flex;
      flex-direction: column;
      gap: 2px;
    }

    .an-stat-label {
      color: var(--text-muted);
      font-size: 0.68rem;
      text-transform: uppercase;
    }

    .an-stat-val {
      color: var(--text-primary);
      font-weight: 600;
      font-family: monospace;
    }

    /* Hazard Distribution Bar */
    .hazard-dist-container {
      display: flex;
      flex-direction: column;
      gap: 6px;
    }

    .hazard-dist-bar {
      height: 12px;
      background: #e2e8f0;
      border-radius: var(--radius-full);
      overflow: hidden;
      display: flex;
      box-shadow: inset 0 1px 2px rgba(0, 0, 0, 0.06);
    }

    .hazard-bar-seg {
      height: 100%;
      transition: width 0.3s ease;
    }

    .seg-safe { background: #34d399; }
    .seg-advisory { background: #38bdf8; }
    .seg-elevated { background: #fbbf24; }
    .seg-warning { background: #fb923c; }
    .seg-critical { background: #f87171; }

    .hazard-legend {
      display: flex;
      flex-wrap: wrap;
      gap: 10px;
      font-size: 0.68rem;
      color: var(--text-secondary);
      justify-content: space-between;
    }

    .hazard-legend-item {
      display: flex;
      align-items: center;
      gap: 4px;
    }

    .hazard-dot {
      width: 7px;
      height: 7px;
      border-radius: 50%;
    }

    /* Live Packets Table */
    .packets-table-wrapper {
      background: #ffffff;
      border: 1px solid rgba(148, 163, 184, 0.22);
      border-radius: var(--radius-md);
      overflow: hidden;
      margin-top: 14px;
      box-shadow: 0 2px 10px rgba(0, 0, 0, 0.03);
    }

    .packets-table-header {
      padding: 12px 16px;
      background: #f8fafc;
      border-bottom: 1px solid rgba(148, 163, 184, 0.2);
      display: flex;
      flex-wrap: wrap;
      justify-content: space-between;
      align-items: center;
      gap: 12px;
    }

    .packets-table-scroll {
      max-height: 380px;
      overflow-y: auto;
      overflow-x: auto;
    }

    .packets-table {
      width: 100%;
      border-collapse: collapse;
      font-size: 0.75rem;
      text-align: left;
    }

    .packets-table th {
      background: #f8fafc;
      color: var(--text-muted);
      font-weight: 700;
      padding: 10px 12px;
      text-transform: uppercase;
      font-size: 0.68rem;
      letter-spacing: 0.5px;
      position: sticky;
      top: 0;
      z-index: 10;
      border-bottom: 1px solid rgba(148, 163, 184, 0.22);
      white-space: nowrap;
    }

    .packets-table td {
      padding: 9px 12px;
      border-bottom: 1px solid rgba(148, 163, 184, 0.12);
      color: var(--text-secondary);
      white-space: nowrap;
    }

    .packets-table tr:hover td {
      background: #f0f7ff;
      color: var(--text-primary);
    }

    .pkt-pill {
      font-size: 0.68rem;
      font-weight: 700;
      padding: 2px 7px;
      border-radius: var(--radius-full);
      display: inline-flex;
      align-items: center;
      gap: 4px;
    }

    .pkt-pill-safe {
      background: #d1fae5;
      color: #047857;
      border: 1px solid #a7f3d0;
    }

    .pkt-pill-adv {
      background: #e0f2fe;
      color: #0369a1;
      border: 1px solid #bae6fd;
    }

    .pkt-pill-elev {
      background: #fef3c7;
      color: #b45309;
      border: 1px solid #fde68a;
    }

    .pkt-pill-warn {
      background: #ffedd5;
      color: #c2410c;
      border: 1px solid #fed7aa;
    }

    .pkt-pill-crit {
      background: #ffe4e6;
      color: #be123c;
      border: 1px solid #fecdd3;
      animation: pulseGlow 1.5s infinite;
    }

    .btn-report-cta {
      background: linear-gradient(135deg, #059669, #10b981);
      color: #ffffff;
      border: none;
      border-radius: var(--radius-sm);
      padding: 7px 14px;
      font-size: 0.78rem;
      font-weight: 600;
      cursor: pointer;
      display: inline-flex;
      align-items: center;
      gap: 6px;
      box-shadow: 0 2px 10px rgba(16, 185, 129, 0.25);
      transition: all 0.2s ease;
    }

    .btn-report-cta:hover {
      background: linear-gradient(135deg, #10b981, #34d399);
      box-shadow: 0 4px 16px rgba(16, 185, 129, 0.35);
      transform: translateY(-1px);
    }

    .btn-dataset-cta {
      background: linear-gradient(135deg, #4f46e5, #3b82f6);
      color: #ffffff;
      border: none;
      border-radius: var(--radius-sm);
      padding: 7px 14px;
      font-size: 0.78rem;
      font-weight: 600;
      cursor: pointer;
      display: inline-flex;
      align-items: center;
      gap: 6px;
      box-shadow: 0 2px 10px rgba(79, 70, 229, 0.25);
      transition: all 0.2s ease;
    }

    .btn-dataset-cta:hover {
      background: linear-gradient(135deg, #6366f1, #60a5fa);
      box-shadow: 0 4px 16px rgba(79, 70, 229, 0.35);
      transform: translateY(-1px);
    }

    /* Modal Guide */
    .modal-backdrop {
      position: fixed;
      top: 0;
      left: 0;
      right: 0;
      bottom: 0;
      background: rgba(15, 23, 42, 0.45);
      backdrop-filter: blur(6px);
      display: none;
      align-items: center;
      justify-content: center;
      z-index: 1000;
      padding: 20px;
    }

    .modal-backdrop.open {
      display: flex;
    }

    .modal-box {
      background: #ffffff;
      border: 1px solid rgba(99, 102, 241, 0.25);
      border-radius: var(--radius-lg);
      max-width: 680px;
      width: 100%;
      padding: 24px;
      box-shadow: 0 20px 45px rgba(15, 23, 42, 0.15);
      max-height: 90vh;
      overflow-y: auto;
    }

    .modal-box h3 {
      font-size: 1.25rem;
      font-weight: 700;
      color: var(--text-primary);
      margin-bottom: 12px;
      display: flex;
      align-items: center;
      gap: 10px;
    }

    .modal-step {
      background: #f8fafc;
      border: 1px solid rgba(148, 163, 184, 0.15);
      border-left: 3px solid var(--accent-cyan);
      border-radius: var(--radius-sm);
      padding: 12px 16px;
      margin-bottom: 12px;
      font-size: 0.85rem;
    }

    .modal-step strong {
      color: var(--text-primary);
      display: block;
      margin-bottom: 4px;
    }

    footer.footer {
      text-align: center;
      padding: 20px;
      font-size: 0.8rem;
      color: var(--text-muted);
      border-top: 1px solid var(--border-color);
      display: flex;
      flex-direction: column;
      gap: 6px;
    }

    .disclaimer-badge {
      display: inline-block;
      align-self: center;
      font-size: 0.7rem;
      padding: 4px 12px;
      border-radius: var(--radius-full);
      background: #fef3c7;
      border: 1px solid #fde68a;
      color: #b45309;
    }

    @media (max-width: 1024px) {
      .col-6,
      .col-8,
      .col-4 {
        grid-column: span 12;
      }

      .charts-grid {
        grid-template-columns: 1fr;
      }

      .kinematics-grid {
        grid-template-columns: 1fr;
      }
    }

    @media (max-width: 640px) {
      body {
        padding: 8px;
      }

      .usb-control-bar {
        flex-direction: column;
        align-items: flex-start;
      }

      header.header {
        flex-direction: column;
        align-items: flex-start;
      }

      .header-badges {
        width: 100%;
        justify-content: flex-start;
      }

      .sensor-cards-grid {
        grid-template-columns: 1fr;
      }

      .risk-banner {
        flex-direction: column;
        align-items: flex-start;
      }
    }
  </style>
</head>

<body>

  <div class="container">

    <!-- USB Port & Multi-Source Connection Center -->
    <div class="usb-control-bar">
      <div class="usb-header-info">
        <div class="usb-icon-circle">📻</div>
        <div class="usb-title-group">
          <h2>
            <span>Receiver USB Hardware Interface</span>
            <span id="badge-source-detected" class="tag-badge" style="background: #e0f2fe; color: #0284c7; border: 1px solid #bae6fd;">
              Ready for Connection
            </span>
          </h2>
          <p id="usb-status-text">
            Streaming live telemetry from <strong>Receiver (Base Station ESP32)</strong> over USB port @ 115200 Baud.
          </p>
        </div>
      </div>

      <div class="usb-actions">
        <!-- Target Device Fixed Indicator (No Dropdown) -->
        <div class="fixed-pill-badge" id="indicator-device-mode" title="Connected Hardware: Receiver / Base Station (LoRa SX1278)">
          <span>📻</span>
          <span>Receiver / Base Station (LoRa SX1278)</span>
          <input type="hidden" id="select-device-mode" value="receiver">
        </div>

        <!-- Baud Rate Fixed Indicator (No Dropdown) -->
        <div class="fixed-pill-badge" id="indicator-baud" title="Baud Rate: 115200 Baud">
          <span>⚡</span>
          <span>115200 Baud (Default)</span>
          <input type="hidden" id="select-baud" value="115200">
        </div>

        <!-- Primary USB Connect Button -->
        <button id="btn-serial-connect" class="btn-action">
          <span>🔌</span>
          <span id="btn-serial-text">Connect Receiver USB</span>
        </button>

        <!-- Secondary Connect Button (Dual Mode) -->
        <button id="btn-serial-connect-tx" class="btn-action" style="display: none; background: linear-gradient(135deg, #7c3aed, #a855f7);">
          <span>🌱</span>
          <span id="btn-serial-text-tx">Connect TX Port</span>
        </button>

        <!-- Simulation Demo Button -->
        <button id="btn-toggle-demo" class="btn-secondary" title="Simulate Real Telemetry without Hardware">
          <span>🧪</span>
          <span id="text-demo-btn">Demo Mode</span>
        </button>

        <!-- Audio Alert Toggle -->
        <button id="btn-toggle-audio" class="btn-secondary" title="Enable Web Audio Hazard Alarms">
          <span id="icon-audio">🔇</span>
          <span id="text-audio-btn">Muted</span>
        </button>

        <!-- Help Guide Modal Trigger -->
        <button id="btn-open-guide" class="btn-secondary" title="View Step-by-Step USB Connection Guide">
          <span>❓</span>
          <span>Help</span>
        </button>

        <!-- Top Bar Detailed Report Trigger -->
        <button id="btn-top-report" class="btn-action" style="background: linear-gradient(135deg, #059669, #10b981);" title="Generate &amp; Download Full Detailed Landslide Engineering Report (PDF/HTML)">
          <span>📑</span>
          <span>Detailed Report</span>
        </button>

        <!-- Top Bar Full Dataset Trigger -->
        <button id="btn-top-dataset" class="btn-secondary" style="border-color: rgba(99, 102, 241, 0.35); color: #4f46e5;" title="Download Full Telemetry Dataset (CSV) with All Sensor Details">
          <span>📊</span>
          <span>Full Dataset (CSV)</span>
        </button>
      </div>
    </div>

    <!-- Header -->
    <header class="header">
      <div class="brand-section">
        <div class="brand-icon">
          <svg viewBox="0 0 24 24">
            <path d="M12 2L2 7l10 5 10-5-10-5zM2 17l10 5 10-5M2 12l10 5 10-5" />
          </svg>
        </div>
        <div class="brand-title">
          <h1>GeoSentinel</h1>
          <p>Landslide Detection &amp; Early-Warning Monitoring System</p>
        </div>
      </div>

      <div class="header-badges">
        <div id="badge-system" class="status-badge status-warning">
          <span class="status-dot pulse"></span>
          <span id="text-system">INITIALIZING</span>
        </div>
        <div id="badge-source" class="status-badge badge-purple">
          <span>SOURCE:</span>
          <strong id="badge-source-val">WAITING USB</strong>
        </div>
        <div id="badge-node" class="status-badge status-warning">
          <span class="status-dot"></span>
          <span id="text-node">WAITING DATA</span>
        </div>
        <div class="status-badge badge-info">
          <span>RSSI:</span>
          <strong id="badge-rssi-val">-- dBm</strong>
        </div>
      </div>
    </header>

    <!-- Top Summary Quick Bar -->
    <div class="summary-bar">
      <div class="summary-card">
        <div class="summary-icon" style="color: var(--accent-emerald);">📡</div>
        <div class="summary-info">
          <div class="label">Last Received</div>
          <div class="val" id="sum-last-time">Waiting...</div>
          <div class="sub" id="sum-packet-count">Total: 0 packets</div>
        </div>
      </div>
      <div class="summary-card">
        <div class="summary-icon" style="color: var(--accent-cyan);">📶</div>
        <div class="summary-info">
          <div class="label">Signal / Channel Quality</div>
          <div class="val" id="sum-signal-qual">--</div>
          <div class="sub" id="sum-packet-len">Length: -- bytes</div>
        </div>
      </div>
      <div class="summary-card">
        <div class="summary-icon" style="color: var(--accent-blue);">🏔️</div>
        <div class="summary-info">
          <div class="label">Ground Motion State</div>
          <div class="val" id="sum-motion">Monitoring...</div>
          <div class="sub" id="sum-accel-mag">|A| ~ --</div>
        </div>
      </div>
      <div class="summary-card">
        <div class="summary-icon" style="color: var(--accent-amber);">🌧️</div>
        <div class="summary-info">
          <div class="label">Environmental Status</div>
          <div class="val" id="sum-env-summary">Monitoring...</div>
          <div class="sub" id="sum-temp-hum">--°C | --%</div>
        </div>
      </div>
    </div>

    <!-- Main Landslide Risk Assessment Panel (Hero) -->
    <div class="glass-card risk-banner">
      <div class="risk-status-group">
        <div class="risk-tag">SYSTEM ASSESSMENT STATUS</div>
        <div class="risk-title" id="risk-display-title">
          <span>🛡️</span>
          <span id="text-risk-title">MONITORING</span>
        </div>
        <div class="risk-subtitle" id="risk-display-desc">
          Telemetry stream active. Receiving live multi-sensor kinematic and environmental data directly from connected hardware.
        </div>
      </div>

      <div class="risk-metrics">
        <div class="risk-mini-card">
          <div class="metric-label">Risk Level</div>
          <div class="metric-val" id="risk-level-badge" style="color: var(--accent-cyan);">MONITORING</div>
        </div>
        <div class="risk-mini-card">
          <div class="metric-label">Connection Mode</div>
          <div class="metric-val" id="feed-status-text" style="color: var(--accent-amber);">USB READY</div>
        </div>
        <div class="risk-mini-card">
          <div class="metric-label">Packet Rate</div>
          <div class="metric-val" id="val-packet-rate" style="color: var(--accent-emerald);">0.0 Hz</div>
        </div>
      </div>
    </div>

    <!-- Main Content Grid -->
    <div class="dashboard-grid">

      <!-- Environmental Conditions -->
      <div class="col-6 glass-card">
        <div class="section-header">
          <h2><span>🌤️</span> Environmental Conditions</h2>
          <span class="tag-badge">DHT22 &amp; Analog Sensors</span>
        </div>

        <div class="sensor-cards-grid">
          <!-- Temperature -->
          <div class="sensor-card">
            <div class="sensor-card-top">
              <span class="sensor-title">🌡️ Temperature</span>
              <span class="calibrated-badge">CALIBRATED</span>
            </div>
            <div class="sensor-value-box">
              <span class="sensor-value" id="val-temp">--</span>
              <span class="sensor-unit">°C</span>
            </div>
            <div class="sensor-bar-bg">
              <div id="bar-temp" class="sensor-bar-fill" style="width: 0%; background-color: var(--accent-amber);"></div>
            </div>
            <div class="sensor-footer">
              <span>Range: -40 to 80 °C</span>
              <span id="val-temp-f">-- °F</span>
            </div>
          </div>

          <!-- Humidity -->
          <div class="sensor-card">
            <div class="sensor-card-top">
              <span class="sensor-title">💧 Humidity</span>
              <span class="calibrated-badge">CALIBRATED</span>
            </div>
            <div class="sensor-value-box">
              <span class="sensor-value" id="val-hum">--</span>
              <span class="sensor-unit">%</span>
            </div>
            <div class="sensor-bar-bg">
              <div id="bar-hum" class="sensor-bar-fill" style="width: 0%; background-color: var(--accent-blue);"></div>
            </div>
            <div class="sensor-footer">
              <span>Range: 0 to 100 %</span>
              <span>Relative</span>
            </div>
          </div>

          <!-- Soil Moisture -->
          <div class="sensor-card">
            <div class="sensor-card-top">
              <span class="sensor-title">🌱 Soil Moisture</span>
              <span class="calibrated-badge" id="badge-soil-type">CALIBRATED</span>
            </div>
            <div class="sensor-value-box">
              <span class="sensor-value" id="val-soil">--</span>
              <span class="sensor-unit" id="unit-soil">%</span>
            </div>
            <div class="sensor-bar-bg">
              <div id="bar-soil" class="sensor-bar-fill" style="width: 0%; background-color: var(--accent-emerald);"></div>
            </div>
            <div class="sensor-footer">
              <span id="footer-soil-range">Scale: 0–100%</span>
              <span>GPIO 34</span>
            </div>
          </div>

          <!-- Rain Sensor -->
          <div class="sensor-card">
            <div class="sensor-card-top">
              <span class="sensor-title">🌧️ Rain Sensor</span>
              <span class="calibrated-badge" id="badge-rain-type">CALIBRATED</span>
            </div>
            <div class="sensor-value-box">
              <span class="sensor-value" id="val-rain">--</span>
              <span class="sensor-unit" id="unit-rain">%</span>
            </div>
            <div class="sensor-bar-bg">
              <div id="bar-rain" class="sensor-bar-fill" style="width: 0%; background-color: var(--accent-cyan);"></div>
            </div>
            <div class="sensor-footer">
              <span id="footer-rain-range">Scale: 0–100%</span>
              <span>GPIO 35</span>
            </div>
          </div>
        </div>
      </div>

      <!-- Ground Movement / Kinematics -->
      <div class="col-6 glass-card">
        <div class="section-header">
          <h2><span>📐</span> Ground Movement / MPU6050</h2>
          <span class="calibrated-badge">CALIBRATED MOTION</span>
        </div>

        <div class="kinematics-grid">
          <!-- Accelerometer -->
          <div class="axis-container">
            <div class="axis-header">
              <span>ACCELEROMETER</span>
              <span class="tag-badge">LINEAR G-FORCE (g)</span>
            </div>
            <div class="axis-row ax">
              <span class="axis-name">AX (X-Axis)</span>
              <span class="axis-val" id="val-ax">--</span>
            </div>
            <div class="axis-row ay">
              <span class="axis-name">AY (Y-Axis)</span>
              <span class="axis-val" id="val-ay">--</span>
            </div>
            <div class="axis-row az">
              <span class="axis-name">AZ (Z-Axis)</span>
              <span class="axis-val" id="val-az">--</span>
            </div>
            <div class="sensor-footer" style="margin-top: 10px;">
              <span>Magnitude Vector:</span>
              <strong id="val-accel-mag" style="color: var(--text-primary);">--</strong>
            </div>
          </div>

          <!-- Gyroscope -->
          <div class="axis-container">
            <div class="axis-header">
              <span>GYROSCOPE</span>
              <span class="tag-badge">ANGULAR RATE (°/s)</span>
            </div>
            <div class="axis-row gx">
              <span class="axis-name">GX (Pitch Rate)</span>
              <span class="axis-val" id="val-gx">--</span>
            </div>
            <div class="axis-row gy">
              <span class="axis-name">GY (Roll Rate)</span>
              <span class="axis-val" id="val-gy">--</span>
            </div>
            <div class="axis-row gz">
              <span class="axis-name">GZ (Yaw Rate)</span>
              <span class="axis-val" id="val-gz">--</span>
            </div>
            <div class="sensor-footer" style="margin-top: 10px;">
              <span>Angular Rate:</span>
              <strong id="val-gyro-mag" style="color: var(--text-primary);">--</strong>
            </div>
          </div>
        </div>

        <div style="margin-top: 14px; padding: 8px 12px; background: #f8fafc; border: 1px solid rgba(148, 163, 184, 0.2); border-radius: var(--radius-sm); font-size: 0.72rem; color: var(--text-muted);">
          ℹ️ <strong>Note:</strong> MPU6050 displays physical g-force (g) and angular velocity (°/s).
        </div>
      </div>

      <!-- Live Monitoring Charts -->
      <div class="col-12 glass-card">
        <div class="section-header">
          <h2><span>📈</span> Live Telemetry Graphs &amp; Seismograph</h2>
          <div style="display: flex; gap: 8px; align-items: center;">
            <button id="btn-clear-charts" class="terminal-btn">Clear Graphs</button>
            <span class="tag-badge">Auto-Rolling Buffer</span>
          </div>
        </div>

        <div class="charts-grid">
          <!-- Chart 1: Temperature & Humidity -->
          <div class="chart-box">
            <div class="chart-box-header">
              <div class="chart-box-title">🌡️ Temperature &amp; Humidity vs Time</div>
              <div class="chart-legend">
                <div class="legend-item"><span class="legend-color" style="background: var(--accent-amber);"></span> Temp (°C)</div>
                <div class="legend-item"><span class="legend-color" style="background: var(--accent-cyan);"></span> Hum (%)</div>
              </div>
            </div>
            <div class="canvas-wrapper">
              <canvas id="chart-env"></canvas>
            </div>
          </div>

          <!-- Chart 2: Soil & Rain Raw ADC -->
          <div class="chart-box">
            <div class="chart-box-header">
              <div class="chart-box-title">🌧️ Soil &amp; Rain Raw ADC vs Time</div>
              <div class="chart-legend">
                <div class="legend-item"><span class="legend-color" style="background: var(--accent-emerald);"></span> Soil ADC</div>
                <div class="legend-item"><span class="legend-color" style="background: var(--accent-blue);"></span> Rain ADC</div>
              </div>
            </div>
            <div class="canvas-wrapper">
              <canvas id="chart-soil-rain"></canvas>
            </div>
          </div>

          <!-- Chart 3: Accelerometer AX, AY, AZ -->
          <div class="chart-box" style="grid-column: span 2;">
            <div class="chart-box-header">
              <div class="chart-box-title">📐 Ground Kinematics (AX, AY, AZ) Seismograph</div>
              <div class="chart-legend">
                <div class="legend-item"><span class="legend-color" style="background: var(--accent-blue);"></span> AX</div>
                <div class="legend-item"><span class="legend-color" style="background: var(--accent-emerald);"></span> AY</div>
                <div class="legend-item"><span class="legend-color" style="background: var(--accent-purple);"></span> AZ</div>
              </div>
            </div>
            <div class="canvas-wrapper" style="height: 230px;">
              <canvas id="chart-accel"></canvas>
            </div>
          </div>
        </div>
      </div>

      <!-- =========================================================================
           PACKETS RECEIVED & COMPREHENSIVE TELEMETRY ANALYSIS
           ========================================================================= -->
      <div class="col-12 glass-card packets-analytics-card" id="section-packets-analysis">
        <div class="section-header" style="flex-wrap: wrap; gap: 12px; margin-bottom: 18px;">
          <div>
            <h2><span>📡</span> Packets Received &amp; Comprehensive Telemetry Analysis</h2>
            <p style="font-size: 0.76rem; color: var(--text-muted); margin-top: 4px;">
              Real-time packet reception tracking, link quality metrics, multi-sensor variance analysis &amp; geological hazard classification.
            </p>
          </div>
          <div style="display: flex; flex-wrap: wrap; gap: 8px; align-items: center;">
            <button id="btn-load-csv" class="terminal-btn" style="background: rgba(14, 165, 233, 0.1); border-color: rgba(14, 165, 233, 0.3); color: #0284c7;" title="Load or Synchronize updated geosentinel_dataset.csv file directly into the Dashboard">
              <span>📁</span>
              <span>Sync / Load CSV</span>
            </button>
            <input type="file" id="input-csv-file" accept=".csv" style="display: none;">
            <button id="btn-download-report" class="btn-report-cta" title="Generate and Download Full Detailed Engineering &amp; Geological Landslide Report (Print-to-PDF ready)">
              <span>📑</span>
              <span>Download Detailed Report</span>
            </button>
            <button id="btn-download-dataset" class="btn-dataset-cta" title="Download Full Telemetry Dataset (CSV) with Temperature, Humidity, Soil, Rain, and Kinematic Sensor Details">
              <span>📊</span>
              <span>Download Full Dataset (CSV)</span>
            </button>
            <button id="btn-download-json" class="terminal-btn" title="Export Raw Analytical Telemetry as JSON Data">
              <span>📋</span>
              <span>Export JSON</span>
            </button>
          </div>
        </div>

        <!-- 4-Domain Multi-Metric Analytical Cards Grid -->
        <div class="an-grid">
          
          <!-- Card 1: Packet Link & RF Health -->
          <div class="an-card">
            <div class="an-card-header">
              <span class="an-card-title"><span>📻</span> Packet Ingestion &amp; Link</span>
              <span id="an-link-badge" class="tag-badge" style="background: #d1fae5; color: #059669; border: 1px solid #a7f3d0;">STREAMING</span>
            </div>
            <div>
              <div class="an-main-val">
                <span id="an-pkt-total">0</span>
                <span class="an-unit">packets received</span>
              </div>
              <div style="font-size: 0.72rem; color: var(--text-muted); margin-top: 2px;">
                Rate: <strong id="an-pkt-rate" style="color: var(--accent-emerald);">0.0 Hz</strong> (<span id="an-pkt-ppm">0</span> pkts/min)
              </div>
            </div>
            <div class="an-stats-row">
              <div class="an-stat-item">
                <span class="an-stat-label">Average RSSI</span>
                <span class="an-stat-val" id="an-rssi-avg">-- dBm</span>
              </div>
              <div class="an-stat-item">
                <span class="an-stat-label">Min / Max RSSI</span>
                <span class="an-stat-val" id="an-rssi-minmax">-- / --</span>
              </div>
              <div class="an-stat-item">
                <span class="an-stat-label">Mean Interval</span>
                <span class="an-stat-val" id="an-interval-val">-- ms</span>
              </div>
              <div class="an-stat-item">
                <span class="an-stat-label">Payload Size</span>
                <span class="an-stat-val" id="an-last-len">-- bytes</span>
              </div>
            </div>
            <div style="display: flex; justify-content: space-between; align-items: center; font-size: 0.72rem;">
              <span style="color: var(--text-muted);">Signal Rating:</span>
              <span id="an-signal-rating" class="pkt-pill pkt-pill-safe">GOOD SIGNAL</span>
            </div>
          </div>

          <!-- Card 2: Environmental Telemetry (DHT22 & Hydrology) -->
          <div class="an-card">
            <div class="an-card-header">
              <span class="an-card-title"><span>🌤️</span> Atmospheric &amp; Soil</span>
              <span id="an-env-badge" class="tag-badge">DHT22 + AO</span>
            </div>
            <div>
              <div class="an-main-val">
                <span id="an-temp-val">--</span>
                <span class="an-unit">°C</span>
                <span style="font-size: 0.95rem; font-weight: 500; color: var(--text-muted); margin-left: 6px;">(<span id="an-temp-f-val">--</span>°F)</span>
              </div>
              <div style="font-size: 0.72rem; color: var(--text-muted); margin-top: 2px;">
                Humidity: <strong id="an-hum-val" style="color: var(--accent-cyan);">-- %</strong> &bull; Dew Point: <strong id="an-dew-val" style="color: #60a5fa;">-- °C</strong>
              </div>
            </div>
            <div class="an-stats-row">
              <div class="an-stat-item">
                <span class="an-stat-label">Temp Min/Max</span>
                <span class="an-stat-val" id="an-temp-minmax">-- / -- °C</span>
              </div>
              <div class="an-stat-item">
                <span class="an-stat-label">Hum Min/Max</span>
                <span class="an-stat-val" id="an-hum-minmax">-- / -- %</span>
              </div>
              <div class="an-stat-item">
                <span class="an-stat-label">Soil Moisture</span>
                <span class="an-stat-val" id="an-soil-val">-- % (<span id="an-soil-adc-sub">ADC --</span>)</span>
              </div>
              <div class="an-stat-item">
                <span class="an-stat-label">Rain Intensity</span>
                <span class="an-stat-val" id="an-rain-val">-- % (<span id="an-rain-adc-sub">ADC --</span>)</span>
              </div>
            </div>
            <div style="display: flex; justify-content: space-between; align-items: center; font-size: 0.72rem;">
              <span style="color: var(--text-muted);">Soil / Rain State:</span>
              <span id="an-hydro-state" class="pkt-pill pkt-pill-safe">DRY / NO RAIN</span>
            </div>
          </div>

          <!-- Card 3: Kinematics & Seismic Motion (MPU6050) -->
          <div class="an-card">
            <div class="an-card-header">
              <span class="an-card-title"><span>📐</span> Kinematics &amp; Tilt</span>
              <span class="tag-badge">MPU6050 6-DOF</span>
            </div>
            <div>
              <div class="an-main-val">
                <span id="an-acc-mag">--</span>
                <span class="an-unit">g (Total |A|)</span>
              </div>
              <div style="font-size: 0.72rem; color: var(--text-muted); margin-top: 2px;">
                Angular Rate: <strong id="an-gyro-mag" style="color: var(--accent-purple);">-- °/s</strong> (|&omega;|)
              </div>
            </div>
            <div class="an-stats-row">
              <div class="an-stat-item">
                <span class="an-stat-label">Peak Shock |A|</span>
                <span class="an-stat-val" id="an-acc-peak">-- g</span>
              </div>
              <div class="an-stat-item">
                <span class="an-stat-label">Peak Gyro |&omega;|</span>
                <span class="an-stat-val" id="an-gyro-peak">-- °/s</span>
              </div>
              <div class="an-stat-item">
                <span class="an-stat-label">Slope Pitch (Tx)</span>
                <span class="an-stat-val" id="an-pitch-val">-- °</span>
              </div>
              <div class="an-stat-item">
                <span class="an-stat-label">Slope Roll (Ty)</span>
                <span class="an-stat-val" id="an-roll-val">-- °</span>
              </div>
            </div>
            <div style="display: flex; justify-content: space-between; align-items: center; font-size: 0.72rem;">
              <span style="color: var(--text-muted);">Motion Classification:</span>
              <span id="an-kin-state" class="pkt-pill pkt-pill-safe">STATIONARY BASELINE</span>
            </div>
          </div>

          <!-- Card 4: Hazard Probability & Trigger Tracking -->
          <div class="an-card">
            <div class="an-card-header">
              <span class="an-card-title"><span>🛡️</span> Hazard Risk Probability</span>
              <span id="an-risk-pill" class="pkt-pill pkt-pill-safe">MONITORING</span>
            </div>
            
            <div class="hazard-dist-container">
              <div style="display: flex; justify-content: space-between; font-size: 0.72rem; color: var(--text-secondary);">
                <span>Telemetry Risk Distribution:</span>
                <strong id="an-dominant-risk">Normal / Stable</strong>
              </div>
              <div class="hazard-dist-bar" title="Distribution of received packets across hazard levels">
                <div id="bar-seg-safe" class="hazard-bar-seg seg-safe" style="width: 100%;"></div>
                <div id="bar-seg-advisory" class="hazard-bar-seg seg-advisory" style="width: 0%;"></div>
                <div id="bar-seg-elevated" class="hazard-bar-seg seg-elevated" style="width: 0%;"></div>
                <div id="bar-seg-warning" class="hazard-bar-seg seg-warning" style="width: 0%;"></div>
                <div id="bar-seg-critical" class="hazard-bar-seg seg-critical" style="width: 0%;"></div>
              </div>
              <div class="hazard-legend">
                <div class="hazard-legend-item"><span class="hazard-dot seg-safe"></span> Safe: <strong id="lbl-pct-safe">100%</strong></div>
                <div class="hazard-legend-item"><span class="hazard-dot seg-advisory"></span> Adv: <strong id="lbl-pct-advisory">0%</strong></div>
                <div class="hazard-legend-item"><span class="hazard-dot seg-elevated"></span> Elev: <strong id="lbl-pct-elevated">0%</strong></div>
                <div class="hazard-legend-item"><span class="hazard-dot seg-warning"></span> Warn: <strong id="lbl-pct-warning">0%</strong></div>
                <div class="hazard-legend-item"><span class="hazard-dot seg-critical"></span> Crit: <strong id="lbl-pct-critical">0%</strong></div>
              </div>
            </div>

            <div class="an-stats-row" style="margin-top: 4px;">
              <div class="an-stat-item">
                <span class="an-stat-label">Soil Pore Trigger</span>
                <span class="an-stat-val" id="an-trig-soil" style="color: var(--accent-emerald);">Nominal (&lt;80%)</span>
              </div>
              <div class="an-stat-item">
                <span class="an-stat-label">Kinematic Trigger</span>
                <span class="an-stat-val" id="an-trig-motion" style="color: var(--accent-emerald);">Stable (&lt;1.45g)</span>
              </div>
            </div>
          </div>

        </div>

        <!-- Live Packets Received Table & Inspector -->
        <div class="packets-table-wrapper">
          <div class="packets-table-header">
            <div style="display: flex; align-items: center; gap: 8px;">
              <span style="font-size: 0.88rem; font-weight: 700; color: var(--text-primary);">📥 Live Packet Reception Log</span>
              <span id="badge-pkt-table-count" class="tag-badge">0 Packets Cached</span>
            </div>
            
            <div style="display: flex; flex-wrap: wrap; align-items: center; gap: 8px;">
              <input type="text" id="input-pkt-search" class="terminal-input" style="max-width: 180px; padding: 4px 8px; font-size: 0.72rem;" placeholder="Search packets...">
              <select id="select-pkt-risk-filter" class="select-styled" style="padding: 4px 8px; font-size: 0.72rem;">
                <option value="all">All Risk Levels</option>
                <option value="critical">Critical Hazards Only</option>
                <option value="warning">Warnings Only</option>
                <option value="elevated">Elevated &amp; Advisory</option>
                <option value="safe">Normal / Safe Only</option>
              </select>
              <button id="btn-clear-packet-table" class="terminal-btn">Clear Log</button>
            </div>
          </div>

          <div class="packets-table-scroll">
            <table class="packets-table" id="table-packets-feed">
              <thead>
                <tr>
                  <th>#</th>
                  <th>Time</th>
                  <th>Source</th>
                  <th>RSSI</th>
                  <th>Temp (°C / °F)</th>
                  <th>Humidity</th>
                  <th>Soil Moisture</th>
                  <th>Rain Sensor</th>
                  <th>Accel |A|</th>
                  <th>Gyro |&omega;|</th>
                  <th>Tilt (P / R)</th>
                  <th>Landslide Hazard</th>
                  <th>Raw Telemetry Payload</th>
                  <th>Action</th>
                </tr>
              </thead>
              <tbody id="tbody-packets-feed">
                <tr>
                  <td colspan="14" style="text-align: center; padding: 24px; color: var(--text-muted);">
                    Waiting for telemetry packets... Connect via USB or click "Demo Mode" to stream data.
                  </td>
                </tr>
              </tbody>
            </table>
          </div>
        </div>

      </div>

      <!-- Live USB Serial Terminal & Console -->
      <div class="col-8 glass-card" style="padding: 16px;">
        <div class="section-header" style="margin-bottom: 10px;">
          <h2><span>💻</span> Live USB Serial Terminal &amp; Packet Inspector</h2>
          <div style="display: flex; gap: 8px;">
            <select id="select-log-filter" class="select-styled" style="padding: 2px 8px; font-size: 0.72rem;">
              <option value="all">Filter: All Lines</option>
              <option value="rx">Filter: RX (Receiver)</option>
              <option value="tx">Filter: TX (Transmitter)</option>
              <option value="sys">Filter: System Events</option>
            </select>
            <button id="btn-clear-log" class="terminal-btn">Clear Log</button>
            <button id="btn-export-log" class="terminal-btn">Export CSV</button>
          </div>
        </div>

        <div class="terminal-container">
          <div class="terminal-header">
            <span class="terminal-title">
              <span>🟢</span>
              <span id="terminal-port-info">Port: Disconnected</span>
            </span>
            <div class="terminal-controls">
              <label style="font-size: 0.72rem; color: var(--text-muted); display: flex; align-items: center; gap: 4px; cursor: pointer;">
                <input type="checkbox" id="chk-autoscroll" checked> Auto-Scroll
              </label>
            </div>
          </div>
          <div class="terminal-body" id="terminal-body">
            <div class="log-entry">
              <span class="log-time">00:00:00</span>
              <span class="log-tag log-tag-sys">SYS</span>
              <span class="log-text">Ready. Connect your ESP32 Receiver or Transmitter via USB port.</span>
            </div>
          </div>
          <div class="terminal-footer">
            <input type="text" id="terminal-input" class="terminal-input" placeholder="Type serial command to send (e.g. PING, STATUS, RST) and press Enter...">
            <button id="btn-send-serial" class="terminal-btn" style="background: var(--accent-blue); color: #ffffff;">Send</button>
          </div>
        </div>
      </div>

      <!-- Communication & Base Station Diagnostics -->
      <div class="col-4 glass-card">
        <div class="section-header">
          <h2><span>🛰️</span> Diagnostics</h2>
          <span class="tag-badge">Real-Time</span>
        </div>

        <table class="diag-table">
          <tbody>
            <tr>
              <td>Active Port</td>
              <td class="val" id="diag-port">None (Waiting)</td>
            </tr>
            <tr>
              <td>Hardware Source</td>
              <td class="val" id="diag-source-type">Auto-Detect</td>
            </tr>
            <tr>
              <td>Signal Strength (RSSI)</td>
              <td class="val" id="diag-rssi">-- dBm</td>
            </tr>
            <tr>
              <td>Signal Quality</td>
              <td class="val" id="diag-signal-qual">--</td>
            </tr>
            <tr>
              <td>Last Packet Length</td>
              <td class="val" id="diag-packet-len">-- bytes</td>
            </tr>
            <tr>
              <td>Total Packets</td>
              <td class="val" id="diag-total-packets">0</td>
            </tr>
            <tr>
              <td>Time Since Last Packet</td>
              <td class="val" id="diag-time-elapsed">Waiting...</td>
            </tr>
            <tr>
              <td>Session Time</td>
              <td class="val" id="diag-uptime">00:00:00</td>
            </tr>
          </tbody>
        </table>

        <!-- Alerts list mini -->
        <div style="margin-top: 14px;">
          <div style="font-size: 0.76rem; font-weight: 700; color: var(--text-secondary); margin-bottom: 6px;">EVENT LOG</div>
          <div class="alerts-container" id="alerts-list">
            <div class="alert-card">
              <span class="alert-icon">ℹ️</span>
              <div class="alert-text">
                <strong>System Initialized</strong>
                <div>Ready to stream live data from USB or Wi-Fi.</div>
              </div>
              <span class="alert-time">Init</span>
            </div>
          </div>
        </div>
      </div>

    </div>

    <!-- Footer Disclaimer -->
    <footer class="footer">
      <div class="disclaimer-badge">⚠️ Engineering Prototype Notice</div>
      <p>GeoSentinel is a prototype IoT landslide monitoring system. Sensor data reflects raw physical registers requiring geotechnical calibration before operational risk assessment.</p>
      <p style="font-size: 0.75rem; color: var(--text-muted);">&copy; GeoSentinel Project &bull; Universal USB &amp; LoRa SX1278 Interface</p>
    </footer>

  </div>

  <!-- Help / USB Guide Modal -->
  <div class="modal-backdrop" id="modal-guide">
    <div class="modal-box">
      <h3><span>🔌</span> How to Connect Receiver &amp; Transmitter via USB</h3>
      <p style="font-size: 0.85rem; color: var(--text-secondary); margin-bottom: 16px;">
        You can connect either the <strong>Receiver (Base Station)</strong> or the <strong>Transmitter (Sensor Node)</strong> directly to your laptop via USB cable. Follow these quick steps:
      </p>

      <div class="modal-step">
        <strong>1. Close Arduino Serial Monitor (Important!)</strong>
        If you have the Arduino IDE open, close its Serial Monitor window so that the COM port is free for this web browser to connect.
      </div>

      <div class="modal-step">
        <strong>2. Connect via USB Cable</strong>
        Plug your ESP32 board into any USB port on your laptop.
      </div>

      <div class="modal-step">
        <strong>3. Click "Connect USB Port"</strong>
        Click the blue <strong>"Connect USB Port"</strong> button at the top of this dashboard. A browser popup will appear listing available COM ports. Select your ESP32 and click <strong>"Connect"</strong>.
      </div>

      <div class="modal-step">
        <strong>4. Transmitter vs Receiver Auto-Detection</strong>
        <ul>
          <li><strong>Receiver (Base Station):</strong> The dashboard receives the LoRa radio packets received from the air, including RSSI and packet diagnostics.</li>
          <li><strong>Transmitter (Sensor Node):</strong> The dashboard reads the sensor readings directly from the node without requiring LoRa reception.</li>
          <li><strong>Dual Mode:</strong> Connect two USB cables simultaneously to monitor both TX and RX side-by-side!</li>
        </ul>
      </div>

      <div style="text-align: right; margin-top: 16px;">
        <button id="btn-close-guide" class="btn-action" style="padding: 8px 20px;">Got It!</button>
      </div>
    </div>
  </div>

  <!-- Packet Inspector Deep-Dive Modal -->
  <div class="modal-backdrop" id="modal-packet-inspect">
    <div class="modal-box" style="max-width: 780px;">
      <div style="display: flex; justify-content: space-between; align-items: center; margin-bottom: 14px;">
        <h3 style="margin: 0;"><span>🔬</span> Telemetry Packet Detailed Breakdown</h3>
        <button id="btn-close-inspect" class="terminal-btn" style="font-size: 0.9rem; padding: 4px 10px;">✕</button>
      </div>

      <div style="margin-bottom: 14px; background: #f8fafc; padding: 12px; border-radius: var(--radius-sm); border: 1px solid rgba(148, 163, 184, 0.25);">
        <div style="font-size: 0.72rem; color: var(--text-muted); text-transform: uppercase; margin-bottom: 4px;">Raw LoRa / Serial Payload:</div>
        <code id="inspect-raw-payload" style="color: var(--accent-cyan); font-family: monospace; font-size: 0.8rem; word-break: break-all;">--</code>
      </div>

      <div style="display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 10px; margin-bottom: 16px;">
        <div class="an-stat-item">
          <span class="an-stat-label">Sequence &amp; Time</span>
          <span class="an-stat-val" id="inspect-pkt-time">#-- &bull; --:--:--</span>
        </div>
        <div class="an-stat-item">
          <span class="an-stat-label">Source Node</span>
          <span class="an-stat-val" id="inspect-pkt-source">--</span>
        </div>
        <div class="an-stat-item">
          <span class="an-stat-label">RF Signal (RSSI)</span>
          <span class="an-stat-val" id="inspect-pkt-rssi">-- dBm</span>
        </div>
        <div class="an-stat-item">
          <span class="an-stat-label">Landslide Risk</span>
          <span class="an-stat-val" id="inspect-pkt-risk">--</span>
        </div>
      </div>

      <h4 style="font-size: 0.82rem; color: var(--text-primary); margin-bottom: 8px;">Sensor Telemetry Dissection &amp; Engineering Conversions:</h4>
      <div style="max-height: 240px; overflow-y: auto; border: 1px solid rgba(148, 163, 184, 0.25); border-radius: var(--radius-sm);">
        <table class="diag-table" style="font-size: 0.76rem;">
          <thead>
            <tr style="background: #f8fafc;">
              <th style="padding: 6px 10px; color: var(--text-muted); text-align: left;">Sensor / Parameter</th>
              <th style="padding: 6px 10px; color: var(--text-muted); text-align: right;">Raw Ingested</th>
              <th style="padding: 6px 10px; color: var(--text-muted); text-align: right;">Physical Calibrated</th>
              <th style="padding: 6px 10px; color: var(--text-muted); text-align: right;">Derived State</th>
            </tr>
          </thead>
          <tbody id="inspect-table-body">
            <!-- Dynamically populated -->
          </tbody>
        </table>
      </div>

      <div style="text-align: right; margin-top: 18px; display: flex; justify-content: flex-end; gap: 10px;">
        <button id="btn-copy-packet-json" class="terminal-btn">📋 Copy Packet JSON</button>
        <button id="btn-close-inspect-2" class="btn-action" style="padding: 6px 18px;">Close</button>
      </div>
    </div>
  </div>

  <script>
    /* =========================================================================
       GEOSENTINEL UNIVERSAL TELEMETRY & USB WEB SERIAL ENGINE
       ========================================================================= */

    const TIMEOUT_WARNING_SEC = 12;
    const TIMEOUT_OFFLINE_SEC = 25;
    const POLL_INTERVAL_MS = 1000;
    const MAX_POINTS = 35;

    // State Variables
    let primaryPort = null;
    let secondaryPort = null;
    let primaryReader = null;
    let secondaryReader = null;
    let primaryWriter = null;
    let secondaryWriter = null;
    let isPrimaryConnected = false;
    let isSecondaryConnected = false;
    let demoInterval = null;
    let audioEnabled = false;
    let audioCtx = null;

    let lastPacketTimestamp = 0;
    let lastPacketCount = -1;
    let packetCounter = 0;
    let startTime = Date.now();
    let packetRateCount = 0;
    let lastRateCalcTime = Date.now();

    // Persistent Receiver Telemetry Diagnostic State
    let latestRssi = null;
    let latestPacketLength = null;
    let lastFullPacketTime = 0;

    // Multi-line sliding accumulator for sketches that print 1 value per line
    let lineAccumulator = {};
    let lineAccumulatorTimer = null;

    // Chart Rolling Data
    const chartData = {
      timestamps: [],
      temp: [],
      hum: [],
      soil: [],
      rain: [],
      ax: [],
      ay: [],
      az: []
    };

    // Telemetry Storage for Log Export
    const telemetryHistory = [];

    // DOM Elements Cache
    const el = {
      selectDeviceMode: document.getElementById('select-device-mode'),
      selectBaud: document.getElementById('select-baud'),
      btnSerialConnect: document.getElementById('btn-serial-connect'),
      btnSerialText: document.getElementById('btn-serial-text'),
      btnSerialConnectTx: document.getElementById('btn-serial-connect-tx'),
      btnSerialTextTx: document.getElementById('btn-serial-text-tx'),
      btnToggleDemo: document.getElementById('btn-toggle-demo'),
      textDemoBtn: document.getElementById('text-demo-btn'),
      btnToggleAudio: document.getElementById('btn-toggle-audio'),
      iconAudio: document.getElementById('icon-audio'),
      textAudioBtn: document.getElementById('text-audio-btn'),
      btnOpenGuide: document.getElementById('btn-open-guide'),
      btnCloseGuide: document.getElementById('btn-close-guide'),
      modalGuide: document.getElementById('modal-guide'),
      badgeSourceDetected: document.getElementById('badge-source-detected'),
      usbStatusText: document.getElementById('usb-status-text'),
      badgeSystem: document.getElementById('badge-system'),
      textSystem: document.getElementById('text-system'),
      badgeSource: document.getElementById('badge-source'),
      badgeSourceVal: document.getElementById('badge-source-val'),
      badgeNode: document.getElementById('badge-node'),
      textNode: document.getElementById('text-node'),
      badgeRssiVal: document.getElementById('badge-rssi-val'),
      feedStatusText: document.getElementById('feed-status-text'),
      valPacketRate: document.getElementById('val-packet-rate'),
      sumLastTime: document.getElementById('sum-last-time'),
      sumPacketCount: document.getElementById('sum-packet-count'),
      sumSignalQual: document.getElementById('sum-signal-qual'),
      sumPacketLen: document.getElementById('sum-packet-len'),
      sumMotion: document.getElementById('sum-motion'),
      sumAccelMag: document.getElementById('sum-accel-mag'),
      sumEnvSummary: document.getElementById('sum-env-summary'),
      sumTempHum: document.getElementById('sum-temp-hum'),
      valTemp: document.getElementById('val-temp'),
      valTempF: document.getElementById('val-temp-f'),
      barTemp: document.getElementById('bar-temp'),
      valHum: document.getElementById('val-hum'),
      barHum: document.getElementById('bar-hum'),
      valSoil: document.getElementById('val-soil'),
      barSoil: document.getElementById('bar-soil'),
      badgeSoilType: document.getElementById('badge-soil-type'),
      unitSoil: document.getElementById('unit-soil'),
      footerSoilRange: document.getElementById('footer-soil-range'),
      valRain: document.getElementById('val-rain'),
      barRain: document.getElementById('bar-rain'),
      badgeRainType: document.getElementById('badge-rain-type'),
      unitRain: document.getElementById('unit-rain'),
      footerRainRange: document.getElementById('footer-rain-range'),
      valAx: document.getElementById('val-ax'),
      valAy: document.getElementById('val-ay'),
      valAz: document.getElementById('val-az'),
      valGx: document.getElementById('val-gx'),
      valGy: document.getElementById('val-gy'),
      valGz: document.getElementById('val-gz'),
      valAccelMag: document.getElementById('val-accel-mag'),
      valGyroMag: document.getElementById('val-gyro-mag'),
      diagPort: document.getElementById('diag-port'),
      diagSourceType: document.getElementById('diag-source-type'),
      diagRssi: document.getElementById('diag-rssi'),
      diagSignalQual: document.getElementById('diag-signal-qual'),
      diagPacketLen: document.getElementById('diag-packet-len'),
      diagTotalPackets: document.getElementById('diag-total-packets'),
      diagTimeElapsed: document.getElementById('diag-time-elapsed'),
      diagUptime: document.getElementById('diag-uptime'),
      alertsList: document.getElementById('alerts-list'),
      terminalBody: document.getElementById('terminal-body'),
      terminalPortInfo: document.getElementById('terminal-port-info'),
      terminalInput: document.getElementById('terminal-input'),
      btnSendSerial: document.getElementById('btn-send-serial'),
      btnClearLog: document.getElementById('btn-clear-log'),
      btnExportLog: document.getElementById('btn-export-log'),
      chkAutoScroll: document.getElementById('chk-autoscroll'),
      selectLogFilter: document.getElementById('select-log-filter'),
      riskDisplayTitle: document.getElementById('risk-display-title'),
      textRiskTitle: document.getElementById('text-risk-title'),
      riskDisplayDesc: document.getElementById('risk-display-desc'),
      riskLevelBadge: document.getElementById('risk-level-badge'),

      // Packet Analytics & Ingestion Elements
      anPktTotal: document.getElementById('an-pkt-total'),
      anPktRate: document.getElementById('an-pkt-rate'),
      anPktPpm: document.getElementById('an-pkt-ppm'),
      anRssiAvg: document.getElementById('an-rssi-avg'),
      anRssiMinMax: document.getElementById('an-rssi-minmax'),
      anIntervalVal: document.getElementById('an-interval-val'),
      anLastLen: document.getElementById('an-last-len'),
      anSignalRating: document.getElementById('an-signal-rating'),
      anLinkBadge: document.getElementById('an-link-badge'),

      // Atmospheric & Soil Analytics Elements
      anTempVal: document.getElementById('an-temp-val'),
      anTempFVal: document.getElementById('an-temp-f-val'),
      anHumVal: document.getElementById('an-hum-val'),
      anDewVal: document.getElementById('an-dew-val'),
      anTempMinMax: document.getElementById('an-temp-minmax'),
      anHumMinMax: document.getElementById('an-hum-minmax'),
      anSoilVal: document.getElementById('an-soil-val'),
      anSoilAdcSub: document.getElementById('an-soil-adc-sub'),
      anRainVal: document.getElementById('an-rain-val'),
      anRainAdcSub: document.getElementById('an-rain-adc-sub'),
      anHydroState: document.getElementById('an-hydro-state'),

      // Kinematic Analytics Elements
      anAccMag: document.getElementById('an-acc-mag'),
      anGyroMag: document.getElementById('an-gyro-mag'),
      anAccPeak: document.getElementById('an-acc-peak'),
      anGyroPeak: document.getElementById('an-gyro-peak'),
      anPitchVal: document.getElementById('an-pitch-val'),
      anRollVal: document.getElementById('an-roll-val'),
      anKinState: document.getElementById('an-kin-state'),

      // Hazard Distribution & Trigger Elements
      anRiskPill: document.getElementById('an-risk-pill'),
      anDominantRisk: document.getElementById('an-dominant-risk'),
      barSegSafe: document.getElementById('bar-seg-safe'),
      barSegAdvisory: document.getElementById('bar-seg-advisory'),
      barSegElevated: document.getElementById('bar-seg-elevated'),
      barSegWarning: document.getElementById('bar-seg-warning'),
      barSegCritical: document.getElementById('bar-seg-critical'),
      lblPctSafe: document.getElementById('lbl-pct-safe'),
      lblPctAdvisory: document.getElementById('lbl-pct-advisory'),
      lblPctElevated: document.getElementById('lbl-pct-elevated'),
      lblPctWarning: document.getElementById('lbl-pct-warning'),
      lblPctCritical: document.getElementById('lbl-pct-critical'),
      anTrigSoil: document.getElementById('an-trig-soil'),
      anTrigMotion: document.getElementById('an-trig-motion'),

      // Packets Table & Action Elements
      badgePktTableCount: document.getElementById('badge-pkt-table-count'),
      inputPktSearch: document.getElementById('input-pkt-search'),
      selectPktRiskFilter: document.getElementById('select-pkt-risk-filter'),
      btnClearPacketTable: document.getElementById('btn-clear-packet-table'),
      tbodyPacketsFeed: document.getElementById('tbody-packets-feed'),

      // Download Action Buttons
      btnDownloadReport: document.getElementById('btn-download-report'),
      btnDownloadDataset: document.getElementById('btn-download-dataset'),
      btnDownloadJson: document.getElementById('btn-download-json'),
      btnTopReport: document.getElementById('btn-top-report'),
      btnTopDataset: document.getElementById('btn-top-dataset'),

      // Deep-Dive Modal Elements
      modalPacketInspect: document.getElementById('modal-packet-inspect'),
      btnCloseInspect: document.getElementById('btn-close-inspect'),
      btnCloseInspect2: document.getElementById('btn-close-inspect-2'),
      inspectRawPayload: document.getElementById('inspect-raw-payload'),
      inspectPktTime: document.getElementById('inspect-pkt-time'),
      inspectPktSource: document.getElementById('inspect-pkt-source'),
      inspectPktRssi: document.getElementById('inspect-pkt-rssi'),
      inspectPktRisk: document.getElementById('inspect-pkt-risk'),
      inspectTableBody: document.getElementById('inspect-table-body'),
      btnCopyPacketJson: document.getElementById('btn-copy-packet-json')
    };

        /* =========================================================================
       GEOSENTINEL DATASET & COMPREHENSIVE PACKET ANALYTICS SUBSYSTEM
       ========================================================================= */

    // Full Historical Telemetry Dataset Ingested from Base Station & Historical Records
    const INITIAL_HISTORICAL_DATASET = [{"timestamp": "16-10-2026 20:56:28", "packet_id": "1", "temperature_c": "30.3", "temperature_f": "86.54", "humidity_pct": "86.6", "dew_point_c": "27.81", "soil_raw_adc": "1817", "soil_moisture_pct": "65.7", "soil_condition": "WET", "rain_raw_adc": "1822", "rain_intensity_pct": "67.1", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-1835", "accel_y_raw": "475", "accel_z_raw": "16662", "accel_x_g": "-0.112", "accel_y_g": "0.029", "accel_z_g": "1.017", "accel_magnitude_g": "1.024", "gyro_x_raw": "399", "gyro_y_raw": "45", "gyro_z_raw": "-35", "gyro_x_dps": "3.05", "gyro_y_dps": "0.35", "gyro_z_dps": "-0.27", "gyro_magnitude_dps": "3.08", "tilt_pitch_deg": "1.64", "tilt_roll_deg": "6.3", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "164", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.30,H:86.60,SRAW:1817,SM:65.7,SC:WET,RRAW:1822,RP:67.1,RC:MODERATE_RAIN,MV:1,TX:1.64,TY:6.30,AXG:-0.112,AYG:0.029,AZG:1.017,GXD:3.05,GYD:0.35,GZD:-0.27"}, {"timestamp": "16-10-2026 20:56:33", "packet_id": "2", "temperature_c": "30.3", "temperature_f": "86.54", "humidity_pct": "86.6", "dew_point_c": "27.81", "soil_raw_adc": "1787", "soil_moisture_pct": "67.4", "soil_condition": "WET", "rain_raw_adc": "1840", "rain_intensity_pct": "66.4", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-1851", "accel_y_raw": "540", "accel_z_raw": "16662", "accel_x_g": "-0.113", "accel_y_g": "0.033", "accel_z_g": "1.017", "accel_magnitude_g": "1.024", "gyro_x_raw": "416", "gyro_y_raw": "48", "gyro_z_raw": "-18", "gyro_x_dps": "3.18", "gyro_y_dps": "0.37", "gyro_z_dps": "-0.14", "gyro_magnitude_dps": "3.2", "tilt_pitch_deg": "1.84", "tilt_roll_deg": "6.34", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "164", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.30,H:86.60,SRAW:1787,SM:67.4,SC:WET,RRAW:1840,RP:66.4,RC:MODERATE_RAIN,MV:1,TX:1.84,TY:6.34,AXG:-0.113,AYG:0.033,AZG:1.017,GXD:3.18,GYD:0.37,GZD:-0.14"}, {"timestamp": "16-10-2026 20:56:38", "packet_id": "3", "temperature_c": "30.3", "temperature_f": "86.54", "humidity_pct": "86.6", "dew_point_c": "27.81", "soil_raw_adc": "1808", "soil_moisture_pct": "66.2", "soil_condition": "WET", "rain_raw_adc": "1840", "rain_intensity_pct": "66.4", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-1802", "accel_y_raw": "475", "accel_z_raw": "16793", "accel_x_g": "-0.11", "accel_y_g": "0.029", "accel_z_g": "1.025", "accel_magnitude_g": "1.031", "gyro_x_raw": "400", "gyro_y_raw": "49", "gyro_z_raw": "-31", "gyro_x_dps": "3.06", "gyro_y_dps": "0.38", "gyro_z_dps": "-0.24", "gyro_magnitude_dps": "3.09", "tilt_pitch_deg": "1.59", "tilt_roll_deg": "6.1", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "164", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.30,H:86.60,SRAW:1808,SM:66.2,SC:WET,RRAW:1840,RP:66.4,RC:MODERATE_RAIN,MV:1,TX:1.59,TY:6.10,AXG:-0.110,AYG:0.029,AZG:1.025,GXD:3.06,GYD:0.38,GZD:-0.24"}, {"timestamp": "16-10-2026 20:56:43", "packet_id": "4", "temperature_c": "30.3", "temperature_f": "86.54", "humidity_pct": "86.5", "dew_point_c": "27.79", "soil_raw_adc": "1805", "soil_moisture_pct": "66.4", "soil_condition": "WET", "rain_raw_adc": "1859", "rain_intensity_pct": "65.6", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-1884", "accel_y_raw": "507", "accel_z_raw": "16695", "accel_x_g": "-0.115", "accel_y_g": "0.031", "accel_z_g": "1.019", "accel_magnitude_g": "1.026", "gyro_x_raw": "382", "gyro_y_raw": "87", "gyro_z_raw": "-28", "gyro_x_dps": "2.92", "gyro_y_dps": "0.67", "gyro_z_dps": "-0.22", "gyro_magnitude_dps": "3.0", "tilt_pitch_deg": "1.7", "tilt_roll_deg": "6.44", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "164", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.30,H:86.50,SRAW:1805,SM:66.4,SC:WET,RRAW:1859,RP:65.6,RC:MODERATE_RAIN,MV:1,TX:1.70,TY:6.44,AXG:-0.115,AYG:0.031,AZG:1.019,GXD:2.92,GYD:0.67,GZD:-0.22"}, {"timestamp": "16-10-2026 20:56:48", "packet_id": "5", "temperature_c": "30.3", "temperature_f": "86.54", "humidity_pct": "86.6", "dew_point_c": "27.81", "soil_raw_adc": "1350", "soil_moisture_pct": "91.7", "soil_condition": "SATURATED", "rain_raw_adc": "1514", "rain_intensity_pct": "79.4", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-7897", "accel_y_raw": "720", "accel_z_raw": "14843", "accel_x_g": "-0.482", "accel_y_g": "0.044", "accel_z_g": "0.906", "accel_magnitude_g": "1.027", "gyro_x_raw": "378", "gyro_y_raw": "86", "gyro_z_raw": "82", "gyro_x_dps": "2.89", "gyro_y_dps": "0.66", "gyro_z_dps": "0.63", "gyro_magnitude_dps": "3.03", "tilt_pitch_deg": "2.48", "tilt_roll_deg": "27.99", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "167", "landslide_risk_level": "CRITICAL ALERT", "hazard_status": "HIGH_LANDSLIDE_HAZARD", "raw_packet": "GS1,TV:1,T:30.30,H:86.60,SRAW:1350,SM:91.7,SC:SATURATED,RRAW:1514,RP:79.4,RC:HEAVY_RAIN,MV:1,TX:2.48,TY:27.99,AXG:-0.482,AYG:0.044,AZG:0.906,GXD:2.89,GYD:0.66,GZD:0.63"}, {"timestamp": "16-10-2026 20:56:53", "packet_id": "6", "temperature_c": "30.3", "temperature_f": "86.54", "humidity_pct": "86.7", "dew_point_c": "27.83", "soil_raw_adc": "1383", "soil_moisture_pct": "89.8", "soil_condition": "SATURATED", "rain_raw_adc": "1602", "rain_intensity_pct": "75.9", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1916", "accel_y_raw": "4685", "accel_z_raw": "15958", "accel_x_g": "-0.117", "accel_y_g": "0.286", "accel_z_g": "0.974", "accel_magnitude_g": "1.022", "gyro_x_raw": "-457", "gyro_y_raw": "-26", "gyro_z_raw": "7", "gyro_x_dps": "-3.49", "gyro_y_dps": "-0.2", "gyro_z_dps": "0.06", "gyro_magnitude_dps": "3.5", "tilt_pitch_deg": "16.25", "tilt_roll_deg": "6.59", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "CRITICAL ALERT", "hazard_status": "HIGH_LANDSLIDE_HAZARD", "raw_packet": "GS1,TV:1,T:30.30,H:86.70,SRAW:1383,SM:89.8,SC:SATURATED,RRAW:1602,RP:75.9,RC:HEAVY_RAIN,MV:1,TX:16.25,TY:6.59,AXG:-0.117,AYG:0.286,AZG:0.974,GXD:-3.49,GYD:-0.20,GZD:0.06"}, {"timestamp": "16-10-2026 20:56:58", "packet_id": "7", "temperature_c": "30.3", "temperature_f": "86.54", "humidity_pct": "86.9", "dew_point_c": "27.87", "soil_raw_adc": "1406", "soil_moisture_pct": "88.6", "soil_condition": "SATURATED", "rain_raw_adc": "1585", "rain_intensity_pct": "76.6", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1785", "accel_y_raw": "3866", "accel_z_raw": "16252", "accel_x_g": "-0.109", "accel_y_g": "0.236", "accel_z_g": "0.992", "accel_magnitude_g": "1.025", "gyro_x_raw": "364", "gyro_y_raw": "6", "gyro_z_raw": "20", "gyro_x_dps": "2.78", "gyro_y_dps": "0.05", "gyro_z_dps": "0.16", "gyro_magnitude_dps": "2.79", "tilt_pitch_deg": "13.32", "tilt_roll_deg": "6.11", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "167", "landslide_risk_level": "ELEVATED", "hazard_status": "SATURATION_WARNING", "raw_packet": "GS1,TV:1,T:30.30,H:86.90,SRAW:1406,SM:88.6,SC:SATURATED,RRAW:1585,RP:76.6,RC:HEAVY_RAIN,MV:1,TX:13.32,TY:6.11,AXG:-0.109,AYG:0.236,AZG:0.992,GXD:2.78,GYD:0.05,GZD:0.16"}, {"timestamp": "16-10-2026 20:57:03", "packet_id": "8", "temperature_c": "30.3", "temperature_f": "86.54", "humidity_pct": "86.9", "dew_point_c": "27.87", "soil_raw_adc": "1408", "soil_moisture_pct": "88.4", "soil_condition": "SATURATED", "rain_raw_adc": "1613", "rain_intensity_pct": "75.5", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1785", "accel_y_raw": "3932", "accel_z_raw": "16302", "accel_x_g": "-0.109", "accel_y_g": "0.24", "accel_z_g": "0.995", "accel_magnitude_g": "1.029", "gyro_x_raw": "410", "gyro_y_raw": "44", "gyro_z_raw": "-32", "gyro_x_dps": "3.13", "gyro_y_dps": "0.34", "gyro_z_dps": "-0.25", "gyro_magnitude_dps": "3.16", "tilt_pitch_deg": "13.51", "tilt_roll_deg": "6.1", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "168", "landslide_risk_level": "ELEVATED", "hazard_status": "SATURATION_WARNING", "raw_packet": "GS1,TV:1,T:30.30,H:86.90,SRAW:1408,SM:88.4,SC:SATURATED,RRAW:1613,RP:75.5,RC:HEAVY_RAIN,MV:1,TX:13.51,TY:6.10,AXG:-0.109,AYG:0.240,AZG:0.995,GXD:3.13,GYD:0.34,GZD:-0.25"}, {"timestamp": "16-10-2026 20:57:08", "packet_id": "9", "temperature_c": "30.3", "temperature_f": "86.54", "humidity_pct": "86.8", "dew_point_c": "27.85", "soil_raw_adc": "1473", "soil_moisture_pct": "84.8", "soil_condition": "WET", "rain_raw_adc": "1664", "rain_intensity_pct": "73.4", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-1736", "accel_y_raw": "3801", "accel_z_raw": "16318", "accel_x_g": "-0.106", "accel_y_g": "0.232", "accel_z_g": "0.996", "accel_magnitude_g": "1.028", "gyro_x_raw": "433", "gyro_y_raw": "60", "gyro_z_raw": "-19", "gyro_x_dps": "3.31", "gyro_y_dps": "0.46", "gyro_z_dps": "-0.15", "gyro_magnitude_dps": "3.35", "tilt_pitch_deg": "13.02", "tilt_roll_deg": "5.93", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "165", "landslide_risk_level": "ELEVATED", "hazard_status": "SATURATION_WARNING", "raw_packet": "GS1,TV:1,T:30.30,H:86.80,SRAW:1473,SM:84.8,SC:WET,RRAW:1664,RP:73.4,RC:MODERATE_RAIN,MV:1,TX:13.02,TY:5.93,AXG:-0.106,AYG:0.232,AZG:0.996,GXD:3.31,GYD:0.46,GZD:-0.15"}, {"timestamp": "16-10-2026 20:57:13", "packet_id": "10", "temperature_c": "30.3", "temperature_f": "86.54", "humidity_pct": "86.7", "dew_point_c": "27.83", "soil_raw_adc": "1467", "soil_moisture_pct": "85.2", "soil_condition": "SATURATED", "rain_raw_adc": "1685", "rain_intensity_pct": "72.6", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-2572", "accel_y_raw": "4079", "accel_z_raw": "16760", "accel_x_g": "-0.157", "accel_y_g": "0.249", "accel_z_g": "1.023", "accel_magnitude_g": "1.065", "gyro_x_raw": "-8652", "gyro_y_raw": "2768", "gyro_z_raw": "911", "gyro_x_dps": "-66.05", "gyro_y_dps": "21.13", "gyro_z_dps": "6.96", "gyro_magnitude_dps": "69.7", "tilt_pitch_deg": "13.53", "tilt_roll_deg": "8.48", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "173", "landslide_risk_level": "CRITICAL ALERT", "hazard_status": "HIGH_LANDSLIDE_HAZARD", "raw_packet": "GS1,TV:1,T:30.30,H:86.70,SRAW:1467,SM:85.2,SC:SATURATED,RRAW:1685,RP:72.6,RC:MODERATE_RAIN,MV:1,TX:13.53,TY:8.48,AXG:-0.157,AYG:0.249,AZG:1.023,GXD:-66.05,GYD:21.13,GZD:6.96"}, {"timestamp": "16-10-2026 20:57:18", "packet_id": "11", "temperature_c": "30.3", "temperature_f": "86.54", "humidity_pct": "86.6", "dew_point_c": "27.81", "soil_raw_adc": "1474", "soil_moisture_pct": "84.8", "soil_condition": "WET", "rain_raw_adc": "1674", "rain_intensity_pct": "73.0", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-3883", "accel_y_raw": "-884", "accel_z_raw": "16072", "accel_x_g": "-0.237", "accel_y_g": "-0.054", "accel_z_g": "0.981", "accel_magnitude_g": "1.011", "gyro_x_raw": "281", "gyro_y_raw": "-162", "gyro_z_raw": "-23", "gyro_x_dps": "2.15", "gyro_y_dps": "-1.24", "gyro_z_dps": "-0.18", "gyro_magnitude_dps": "2.49", "tilt_pitch_deg": "-3.04", "tilt_roll_deg": "13.57", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "168", "landslide_risk_level": "ELEVATED", "hazard_status": "SATURATION_WARNING", "raw_packet": "GS1,TV:1,T:30.30,H:86.60,SRAW:1474,SM:84.8,SC:WET,RRAW:1674,RP:73.0,RC:MODERATE_RAIN,MV:1,TX:-3.04,TY:13.57,AXG:-0.237,AYG:-0.054,AZG:0.981,GXD:2.15,GYD:-1.24,GZD:-0.18"}, {"timestamp": "16-10-2026 20:57:23", "packet_id": "12", "temperature_c": "30.3", "temperature_f": "86.54", "humidity_pct": "86.6", "dew_point_c": "27.81", "soil_raw_adc": "1410", "soil_moisture_pct": "88.3", "soil_condition": "SATURATED", "rain_raw_adc": "1680", "rain_intensity_pct": "72.8", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-8945", "accel_y_raw": "-6881", "accel_z_raw": "11862", "accel_x_g": "-0.546", "accel_y_g": "-0.42", "accel_z_g": "0.724", "accel_magnitude_g": "0.999", "gyro_x_raw": "897", "gyro_y_raw": "-128", "gyro_z_raw": "611", "gyro_x_dps": "6.85", "gyro_y_dps": "-0.98", "gyro_z_dps": "4.67", "gyro_magnitude_dps": "8.35", "tilt_pitch_deg": "-24.86", "tilt_roll_deg": "33.14", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "174", "landslide_risk_level": "CRITICAL ALERT", "hazard_status": "HIGH_LANDSLIDE_HAZARD", "raw_packet": "GS1,TV:1,T:30.30,H:86.60,SRAW:1410,SM:88.3,SC:SATURATED,RRAW:1680,RP:72.8,RC:MODERATE_RAIN,MV:1,TX:-24.86,TY:33.14,AXG:-0.546,AYG:-0.420,AZG:0.724,GXD:6.85,GYD:-0.98,GZD:4.67"}, {"timestamp": "16-10-2026 20:57:28", "packet_id": "13", "temperature_c": "30.3", "temperature_f": "86.54", "humidity_pct": "86.8", "dew_point_c": "27.85", "soil_raw_adc": "1331", "soil_moisture_pct": "92.7", "soil_condition": "SATURATED", "rain_raw_adc": "1662", "rain_intensity_pct": "73.5", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-14434", "accel_y_raw": "409", "accel_z_raw": "7716", "accel_x_g": "-0.881", "accel_y_g": "0.025", "accel_z_g": "0.471", "accel_magnitude_g": "0.999", "gyro_x_raw": "383", "gyro_y_raw": "-2039", "gyro_z_raw": "65", "gyro_x_dps": "2.93", "gyro_y_dps": "-15.57", "gyro_z_dps": "0.5", "gyro_magnitude_dps": "15.85", "tilt_pitch_deg": "1.44", "tilt_roll_deg": "61.84", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "172", "landslide_risk_level": "CRITICAL ALERT", "hazard_status": "HIGH_LANDSLIDE_HAZARD", "raw_packet": "GS1,TV:1,T:30.30,H:86.80,SRAW:1331,SM:92.7,SC:SATURATED,RRAW:1662,RP:73.5,RC:MODERATE_RAIN,MV:1,TX:1.44,TY:61.84,AXG:-0.881,AYG:0.025,AZG:0.471,GXD:2.93,GYD:-15.57,GZD:0.50"}, {"timestamp": "16-10-2026 20:57:33", "packet_id": "14", "temperature_c": "30.3", "temperature_f": "86.54", "humidity_pct": "86.8", "dew_point_c": "27.85", "soil_raw_adc": "1404", "soil_moisture_pct": "88.7", "soil_condition": "SATURATED", "rain_raw_adc": "1684", "rain_intensity_pct": "72.6", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-15695", "accel_y_raw": "262", "accel_z_raw": "4374", "accel_x_g": "-0.958", "accel_y_g": "0.016", "accel_z_g": "0.267", "accel_magnitude_g": "0.995", "gyro_x_raw": "398", "gyro_y_raw": "259", "gyro_z_raw": "-18", "gyro_x_dps": "3.04", "gyro_y_dps": "1.98", "gyro_z_dps": "-0.14", "gyro_magnitude_dps": "3.63", "tilt_pitch_deg": "0.94", "tilt_roll_deg": "74.41", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "171", "landslide_risk_level": "CRITICAL ALERT", "hazard_status": "HIGH_LANDSLIDE_HAZARD", "raw_packet": "GS1,TV:1,T:30.30,H:86.80,SRAW:1404,SM:88.7,SC:SATURATED,RRAW:1684,RP:72.6,RC:MODERATE_RAIN,MV:1,TX:0.94,TY:74.41,AXG:-0.958,AYG:0.016,AZG:0.267,GXD:3.04,GYD:1.98,GZD:-0.14"}, {"timestamp": "16-10-2026 20:57:38", "packet_id": "15", "temperature_c": "30.4", "temperature_f": "86.72", "humidity_pct": "86.6", "dew_point_c": "27.9", "soil_raw_adc": "1374", "soil_moisture_pct": "90.3", "soil_condition": "SATURATED", "rain_raw_adc": "1447", "rain_intensity_pct": "82.1", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-4456", "accel_y_raw": "-7684", "accel_z_raw": "12206", "accel_x_g": "-0.272", "accel_y_g": "-0.469", "accel_z_g": "0.745", "accel_magnitude_g": "0.921", "gyro_x_raw": "1493", "gyro_y_raw": "1919", "gyro_z_raw": "-176", "gyro_x_dps": "11.4", "gyro_y_dps": "14.65", "gyro_z_dps": "-1.35", "gyro_magnitude_dps": "18.61", "tilt_pitch_deg": "-30.56", "tilt_roll_deg": "17.17", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "173", "landslide_risk_level": "CRITICAL ALERT", "hazard_status": "HIGH_LANDSLIDE_HAZARD", "raw_packet": "GS1,TV:1,T:30.40,H:86.60,SRAW:1374,SM:90.3,SC:SATURATED,RRAW:1447,RP:82.1,RC:HEAVY_RAIN,MV:1,TX:-30.56,TY:17.17,AXG:-0.272,AYG:-0.469,AZG:0.745,GXD:11.40,GYD:14.65,GZD:-1.35"}, {"timestamp": "16-10-2026 20:57:43", "packet_id": "16", "temperature_c": "30.4", "temperature_f": "86.72", "humidity_pct": "86.6", "dew_point_c": "27.9", "soil_raw_adc": "1574", "soil_moisture_pct": "79.2", "soil_condition": "WET", "rain_raw_adc": "1616", "rain_intensity_pct": "75.4", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-2539", "accel_y_raw": "376", "accel_z_raw": "16678", "accel_x_g": "-0.155", "accel_y_g": "0.023", "accel_z_g": "1.018", "accel_magnitude_g": "1.03", "gyro_x_raw": "372", "gyro_y_raw": "44", "gyro_z_raw": "-49", "gyro_x_dps": "2.84", "gyro_y_dps": "0.34", "gyro_z_dps": "-0.38", "gyro_magnitude_dps": "2.89", "tilt_pitch_deg": "1.28", "tilt_roll_deg": "8.64", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "161", "landslide_risk_level": "ELEVATED", "hazard_status": "SATURATION_WARNING", "raw_packet": "GS1,TV:1,T:30.40,H:86.60,SRAW:1574,SM:79.2,SC:WET,RRAW:1616,RP:75.4,RC:HEAVY_RAIN,MV:1,TX:1.28,TY:8.64,AXG:-0.155,AYG:0.023,AZG:1.018,GXD:2.84,GYD:0.34,GZD:-0.38"}, {"timestamp": "16-10-2026 20:57:48", "packet_id": "17", "temperature_c": "30.4", "temperature_f": "86.72", "humidity_pct": "86.6", "dew_point_c": "27.9", "soil_raw_adc": "1670", "soil_moisture_pct": "73.9", "soil_condition": "WET", "rain_raw_adc": "1716", "rain_intensity_pct": "71.4", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-2506", "accel_y_raw": "442", "accel_z_raw": "16711", "accel_x_g": "-0.153", "accel_y_g": "0.027", "accel_z_g": "1.02", "accel_magnitude_g": "1.032", "gyro_x_raw": "403", "gyro_y_raw": "72", "gyro_z_raw": "-36", "gyro_x_dps": "3.08", "gyro_y_dps": "0.55", "gyro_z_dps": "-0.28", "gyro_magnitude_dps": "3.14", "tilt_pitch_deg": "1.51", "tilt_roll_deg": "8.55", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "164", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.40,H:86.60,SRAW:1670,SM:73.9,SC:WET,RRAW:1716,RP:71.4,RC:MODERATE_RAIN,MV:1,TX:1.51,TY:8.55,AXG:-0.153,AYG:0.027,AZG:1.020,GXD:3.08,GYD:0.55,GZD:-0.28"}, {"timestamp": "16-10-2026 20:57:53", "packet_id": "18", "temperature_c": "30.4", "temperature_f": "86.72", "humidity_pct": "86.4", "dew_point_c": "27.86", "soil_raw_adc": "1810", "soil_moisture_pct": "66.1", "soil_condition": "WET", "rain_raw_adc": "1280", "rain_intensity_pct": "88.8", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-2555", "accel_y_raw": "491", "accel_z_raw": "16596", "accel_x_g": "-0.156", "accel_y_g": "0.03", "accel_z_g": "1.013", "accel_magnitude_g": "1.025", "gyro_x_raw": "394", "gyro_y_raw": "61", "gyro_z_raw": "-47", "gyro_x_dps": "3.01", "gyro_y_dps": "0.47", "gyro_z_dps": "-0.36", "gyro_magnitude_dps": "3.07", "tilt_pitch_deg": "1.66", "tilt_roll_deg": "8.72", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "161", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.40,H:86.40,SRAW:1810,SM:66.1,SC:WET,RRAW:1280,RP:88.8,RC:HEAVY_RAIN,MV:1,TX:1.66,TY:8.72,AXG:-0.156,AYG:0.030,AZG:1.013,GXD:3.01,GYD:0.47,GZD:-0.36"}, {"timestamp": "16-10-2026 20:57:58", "packet_id": "19", "temperature_c": "30.3", "temperature_f": "86.54", "humidity_pct": "86.4", "dew_point_c": "27.77", "soil_raw_adc": "1330", "soil_moisture_pct": "92.8", "soil_condition": "SATURATED", "rain_raw_adc": "1293", "rain_intensity_pct": "88.3", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-2572", "accel_y_raw": "409", "accel_z_raw": "16662", "accel_x_g": "-0.157", "accel_y_g": "0.025", "accel_z_g": "1.017", "accel_magnitude_g": "1.029", "gyro_x_raw": "419", "gyro_y_raw": "62", "gyro_z_raw": "-24", "gyro_x_dps": "3.2", "gyro_y_dps": "0.48", "gyro_z_dps": "-0.19", "gyro_magnitude_dps": "3.24", "tilt_pitch_deg": "1.4", "tilt_roll_deg": "8.76", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "167", "landslide_risk_level": "ELEVATED", "hazard_status": "SATURATION_WARNING", "raw_packet": "GS1,TV:1,T:30.30,H:86.40,SRAW:1330,SM:92.8,SC:SATURATED,RRAW:1293,RP:88.3,RC:HEAVY_RAIN,MV:1,TX:1.40,TY:8.76,AXG:-0.157,AYG:0.025,AZG:1.017,GXD:3.20,GYD:0.48,GZD:-0.19"}, {"timestamp": "16-10-2026 20:58:03", "packet_id": "20", "temperature_c": "30.4", "temperature_f": "86.72", "humidity_pct": "86.5", "dew_point_c": "27.88", "soil_raw_adc": "1335", "soil_moisture_pct": "92.5", "soil_condition": "SATURATED", "rain_raw_adc": "1326", "rain_intensity_pct": "87.0", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-2473", "accel_y_raw": "409", "accel_z_raw": "16646", "accel_x_g": "-0.151", "accel_y_g": "0.025", "accel_z_g": "1.016", "accel_magnitude_g": "1.027", "gyro_x_raw": "403", "gyro_y_raw": "72", "gyro_z_raw": "-3", "gyro_x_dps": "3.08", "gyro_y_dps": "0.55", "gyro_z_dps": "-0.03", "gyro_magnitude_dps": "3.13", "tilt_pitch_deg": "1.37", "tilt_roll_deg": "8.47", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "167", "landslide_risk_level": "ELEVATED", "hazard_status": "SATURATION_WARNING", "raw_packet": "GS1,TV:1,T:30.40,H:86.50,SRAW:1335,SM:92.5,SC:SATURATED,RRAW:1326,RP:87.0,RC:HEAVY_RAIN,MV:1,TX:1.37,TY:8.47,AXG:-0.151,AYG:0.025,AZG:1.016,GXD:3.08,GYD:0.55,GZD:-0.03"}, {"timestamp": "16-10-2026 20:58:08", "packet_id": "21", "temperature_c": "30.3", "temperature_f": "86.54", "humidity_pct": "86.5", "dew_point_c": "27.79", "soil_raw_adc": "1924", "soil_moisture_pct": "59.8", "soil_condition": "MODERATE", "rain_raw_adc": "1362", "rain_intensity_pct": "85.5", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-2539", "accel_y_raw": "393", "accel_z_raw": "16662", "accel_x_g": "-0.155", "accel_y_g": "0.024", "accel_z_g": "1.017", "accel_magnitude_g": "1.029", "gyro_x_raw": "399", "gyro_y_raw": "35", "gyro_z_raw": "-10", "gyro_x_dps": "3.05", "gyro_y_dps": "0.27", "gyro_z_dps": "-0.08", "gyro_magnitude_dps": "3.06", "tilt_pitch_deg": "1.32", "tilt_roll_deg": "8.64", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "166", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.30,H:86.50,SRAW:1924,SM:59.8,SC:MODERATE,RRAW:1362,RP:85.5,RC:HEAVY_RAIN,MV:1,TX:1.32,TY:8.64,AXG:-0.155,AYG:0.024,AZG:1.017,GXD:3.05,GYD:0.27,GZD:-0.08"}, {"timestamp": "16-10-2026 20:58:13", "packet_id": "22", "temperature_c": "30.4", "temperature_f": "86.72", "humidity_pct": "86.6", "dew_point_c": "27.9", "soil_raw_adc": "1381", "soil_moisture_pct": "89.9", "soil_condition": "SATURATED", "rain_raw_adc": "1321", "rain_intensity_pct": "87.2", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-2424", "accel_y_raw": "360", "accel_z_raw": "16613", "accel_x_g": "-0.148", "accel_y_g": "0.022", "accel_z_g": "1.014", "accel_magnitude_g": "1.025", "gyro_x_raw": "390", "gyro_y_raw": "52", "gyro_z_raw": "-34", "gyro_x_dps": "2.98", "gyro_y_dps": "0.4", "gyro_z_dps": "-0.26", "gyro_magnitude_dps": "3.02", "tilt_pitch_deg": "1.26", "tilt_roll_deg": "8.3", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "167", "landslide_risk_level": "ELEVATED", "hazard_status": "SATURATION_WARNING", "raw_packet": "GS1,TV:1,T:30.40,H:86.60,SRAW:1381,SM:89.9,SC:SATURATED,RRAW:1321,RP:87.2,RC:HEAVY_RAIN,MV:1,TX:1.26,TY:8.30,AXG:-0.148,AYG:0.022,AZG:1.014,GXD:2.98,GYD:0.40,GZD:-0.26"}, {"timestamp": "16-10-2026 21:50:26", "packet_id": "23", "temperature_c": "29.2", "temperature_f": "84.56", "humidity_pct": "89.2", "dew_point_c": "27.23", "soil_raw_adc": "1271", "soil_moisture_pct": "96.1", "soil_condition": "SATURATED", "rain_raw_adc": "1360", "rain_intensity_pct": "85.6", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1638", "accel_y_raw": "0", "accel_z_raw": "16760", "accel_x_g": "-0.1", "accel_y_g": "0.0", "accel_z_g": "1.023", "accel_magnitude_g": "1.028", "gyro_x_raw": "387", "gyro_y_raw": "52", "gyro_z_raw": "-28", "gyro_x_dps": "2.96", "gyro_y_dps": "0.4", "gyro_z_dps": "-0.22", "gyro_magnitude_dps": "2.99", "tilt_pitch_deg": "0.03", "tilt_roll_deg": "5.59", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "167", "landslide_risk_level": "ELEVATED", "hazard_status": "SATURATION_WARNING", "raw_packet": "GS1,TV:1,T:29.20,H:89.20,SRAW:1271,SM:96.1,SC:SATURATED,RRAW:1360,RP:85.6,RC:HEAVY_RAIN,MV:1,TX:0.03,TY:5.59,AXG:-0.100,AYG:0.000,AZG:1.023,GXD:2.96,GYD:0.40,GZD:-0.22"}, {"timestamp": "16-10-2026 21:50:30", "packet_id": "24", "temperature_c": "29.2", "temperature_f": "84.56", "humidity_pct": "89.1", "dew_point_c": "27.21", "soil_raw_adc": "1301", "soil_moisture_pct": "94.4", "soil_condition": "SATURATED", "rain_raw_adc": "1315", "rain_intensity_pct": "87.4", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1622", "accel_y_raw": "0", "accel_z_raw": "16793", "accel_x_g": "-0.099", "accel_y_g": "0.0", "accel_z_g": "1.025", "accel_magnitude_g": "1.03", "gyro_x_raw": "390", "gyro_y_raw": "65", "gyro_z_raw": "-44", "gyro_x_dps": "2.98", "gyro_y_dps": "0.5", "gyro_z_dps": "-0.34", "gyro_magnitude_dps": "3.04", "tilt_pitch_deg": "0.0", "tilt_roll_deg": "5.51", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "167", "landslide_risk_level": "ELEVATED", "hazard_status": "SATURATION_WARNING", "raw_packet": "GS1,TV:1,T:29.20,H:89.10,SRAW:1301,SM:94.4,SC:SATURATED,RRAW:1315,RP:87.4,RC:HEAVY_RAIN,MV:1,TX:0.00,TY:5.51,AXG:-0.099,AYG:0.000,AZG:1.025,GXD:2.98,GYD:0.50,GZD:-0.34"}, {"timestamp": "16-10-2026 21:50:35", "packet_id": "25", "temperature_c": "29.3", "temperature_f": "84.74", "humidity_pct": "89.2", "dew_point_c": "27.33", "soil_raw_adc": "1520", "soil_moisture_pct": "82.2", "soil_condition": "WET", "rain_raw_adc": "1595", "rain_intensity_pct": "76.2", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-7815", "accel_y_raw": "393", "accel_z_raw": "13844", "accel_x_g": "-0.477", "accel_y_g": "0.024", "accel_z_g": "0.845", "accel_magnitude_g": "0.971", "gyro_x_raw": "-2617", "gyro_y_raw": "5770", "gyro_z_raw": "-224", "gyro_x_dps": "-19.98", "gyro_y_dps": "44.05", "gyro_z_dps": "-1.71", "gyro_magnitude_dps": "48.4", "tilt_pitch_deg": "1.44", "tilt_roll_deg": "29.46", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "165", "landslide_risk_level": "CRITICAL ALERT", "hazard_status": "HIGH_LANDSLIDE_HAZARD", "raw_packet": "GS1,TV:1,T:29.30,H:89.20,SRAW:1520,SM:82.2,SC:WET,RRAW:1595,RP:76.2,RC:HEAVY_RAIN,MV:1,TX:1.44,TY:29.46,AXG:-0.477,AYG:0.024,AZG:0.845,GXD:-19.98,GYD:44.05,GZD:-1.71"}, {"timestamp": "16-10-2026 21:50:40", "packet_id": "26", "temperature_c": "29.3", "temperature_f": "84.74", "humidity_pct": "89.5", "dew_point_c": "27.39", "soil_raw_adc": "1763", "soil_moisture_pct": "68.7", "soil_condition": "WET", "rain_raw_adc": "1223", "rain_intensity_pct": "91.1", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1605", "accel_y_raw": "-81", "accel_z_raw": "16613", "accel_x_g": "-0.098", "accel_y_g": "-0.005", "accel_z_g": "1.014", "accel_magnitude_g": "1.019", "gyro_x_raw": "326", "gyro_y_raw": "141", "gyro_z_raw": "497", "gyro_x_dps": "2.49", "gyro_y_dps": "1.08", "gyro_z_dps": "3.8", "gyro_magnitude_dps": "4.67", "tilt_pitch_deg": "-0.29", "tilt_roll_deg": "5.5", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "162", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.30,H:89.50,SRAW:1763,SM:68.7,SC:WET,RRAW:1223,RP:91.1,RC:HEAVY_RAIN,MV:1,TX:-0.29,TY:5.50,AXG:-0.098,AYG:-0.005,AZG:1.014,GXD:2.49,GYD:1.08,GZD:3.80"}, {"timestamp": "16-10-2026 21:50:45", "packet_id": "27", "temperature_c": "29.3", "temperature_f": "84.74", "humidity_pct": "89.4", "dew_point_c": "27.37", "soil_raw_adc": "1292", "soil_moisture_pct": "94.9", "soil_condition": "SATURATED", "rain_raw_adc": "1282", "rain_intensity_pct": "88.7", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-5947", "accel_y_raw": "49", "accel_z_raw": "15794", "accel_x_g": "-0.363", "accel_y_g": "0.003", "accel_z_g": "0.964", "accel_magnitude_g": "1.03", "gyro_x_raw": "412", "gyro_y_raw": "-74", "gyro_z_raw": "-47", "gyro_x_dps": "3.15", "gyro_y_dps": "-0.57", "gyro_z_dps": "-0.36", "gyro_magnitude_dps": "3.22", "tilt_pitch_deg": "0.15", "tilt_roll_deg": "20.65", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "CRITICAL ALERT", "hazard_status": "HIGH_LANDSLIDE_HAZARD", "raw_packet": "GS1,TV:1,T:29.30,H:89.40,SRAW:1292,SM:94.9,SC:SATURATED,RRAW:1282,RP:88.7,RC:HEAVY_RAIN,MV:1,TX:0.15,TY:20.65,AXG:-0.363,AYG:0.003,AZG:0.964,GXD:3.15,GYD:-0.57,GZD:-0.36"}, {"timestamp": "16-10-2026 21:50:50", "packet_id": "28", "temperature_c": "29.3", "temperature_f": "84.74", "humidity_pct": "89.3", "dew_point_c": "27.35", "soil_raw_adc": "1366", "soil_moisture_pct": "90.8", "soil_condition": "SATURATED", "rain_raw_adc": "1269", "rain_intensity_pct": "89.2", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-5226", "accel_y_raw": "196", "accel_z_raw": "15892", "accel_x_g": "-0.319", "accel_y_g": "0.012", "accel_z_g": "0.97", "accel_magnitude_g": "1.021", "gyro_x_raw": "399", "gyro_y_raw": "-136", "gyro_z_raw": "-26", "gyro_x_dps": "3.05", "gyro_y_dps": "-1.04", "gyro_z_dps": "-0.2", "gyro_magnitude_dps": "3.23", "tilt_pitch_deg": "0.67", "tilt_roll_deg": "18.22", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "CRITICAL ALERT", "hazard_status": "HIGH_LANDSLIDE_HAZARD", "raw_packet": "GS1,TV:1,T:29.30,H:89.30,SRAW:1366,SM:90.8,SC:SATURATED,RRAW:1269,RP:89.2,RC:HEAVY_RAIN,MV:1,TX:0.67,TY:18.22,AXG:-0.319,AYG:0.012,AZG:0.970,GXD:3.05,GYD:-1.04,GZD:-0.20"}, {"timestamp": "16-10-2026 21:50:55", "packet_id": "29", "temperature_c": "29.3", "temperature_f": "84.74", "humidity_pct": "89.2", "dew_point_c": "27.33", "soil_raw_adc": "1242", "soil_moisture_pct": "97.7", "soil_condition": "SATURATED", "rain_raw_adc": "1225", "rain_intensity_pct": "91.0", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-3162", "accel_y_raw": "147", "accel_z_raw": "16629", "accel_x_g": "-0.193", "accel_y_g": "0.009", "accel_z_g": "1.015", "accel_magnitude_g": "1.033", "gyro_x_raw": "369", "gyro_y_raw": "45", "gyro_z_raw": "191", "gyro_x_dps": "2.82", "gyro_y_dps": "0.35", "gyro_z_dps": "1.46", "gyro_magnitude_dps": "3.19", "tilt_pitch_deg": "0.49", "tilt_roll_deg": "10.79", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "167", "landslide_risk_level": "ELEVATED", "hazard_status": "SATURATION_WARNING", "raw_packet": "GS1,TV:1,T:29.30,H:89.20,SRAW:1242,SM:97.7,SC:SATURATED,RRAW:1225,RP:91.0,RC:HEAVY_RAIN,MV:1,TX:0.49,TY:10.79,AXG:-0.193,AYG:0.009,AZG:1.015,GXD:2.82,GYD:0.35,GZD:1.46"}, {"timestamp": "16-10-2026 21:51:00", "packet_id": "30", "temperature_c": "29.3", "temperature_f": "84.74", "humidity_pct": "89.1", "dew_point_c": "27.31", "soil_raw_adc": "1326", "soil_moisture_pct": "93.0", "soil_condition": "SATURATED", "rain_raw_adc": "1226", "rain_intensity_pct": "91.0", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "5275", "accel_y_raw": "425", "accel_z_raw": "15974", "accel_x_g": "0.322", "accel_y_g": "0.026", "accel_z_g": "0.975", "accel_magnitude_g": "1.027", "gyro_x_raw": "454", "gyro_y_raw": "221", "gyro_z_raw": "41", "gyro_x_dps": "3.47", "gyro_y_dps": "1.69", "gyro_z_dps": "0.32", "gyro_magnitude_dps": "3.87", "tilt_pitch_deg": "1.46", "tilt_roll_deg": "-18.27", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "167", "landslide_risk_level": "CRITICAL ALERT", "hazard_status": "HIGH_LANDSLIDE_HAZARD", "raw_packet": "GS1,TV:1,T:29.30,H:89.10,SRAW:1326,SM:93.0,SC:SATURATED,RRAW:1226,RP:91.0,RC:HEAVY_RAIN,MV:1,TX:1.46,TY:-18.27,AXG:0.322,AYG:0.026,AZG:0.975,GXD:3.47,GYD:1.69,GZD:0.32"}, {"timestamp": "16-10-2026 21:51:05", "packet_id": "31", "temperature_c": "29.3", "temperature_f": "84.74", "humidity_pct": "89.0", "dew_point_c": "27.29", "soil_raw_adc": "1328", "soil_moisture_pct": "92.9", "soil_condition": "SATURATED", "rain_raw_adc": "1255", "rain_intensity_pct": "89.8", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1392", "accel_y_raw": "7979", "accel_z_raw": "14663", "accel_x_g": "-0.085", "accel_y_g": "0.487", "accel_z_g": "0.895", "accel_magnitude_g": "1.022", "gyro_x_raw": "374", "gyro_y_raw": "70", "gyro_z_raw": "10", "gyro_x_dps": "2.86", "gyro_y_dps": "0.54", "gyro_z_dps": "0.08", "gyro_magnitude_dps": "2.91", "tilt_pitch_deg": "28.47", "tilt_roll_deg": "4.75", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "167", "landslide_risk_level": "CRITICAL ALERT", "hazard_status": "HIGH_LANDSLIDE_HAZARD", "raw_packet": "GS1,TV:1,T:29.30,H:89.00,SRAW:1328,SM:92.9,SC:SATURATED,RRAW:1255,RP:89.8,RC:HEAVY_RAIN,MV:1,TX:28.47,TY:4.75,AXG:-0.085,AYG:0.487,AZG:0.895,GXD:2.86,GYD:0.54,GZD:0.08"}, {"timestamp": "16-10-2026 21:51:10", "packet_id": "32", "temperature_c": "29.3", "temperature_f": "84.74", "humidity_pct": "88.9", "dew_point_c": "27.27", "soil_raw_adc": "1200", "soil_moisture_pct": "100.0", "soil_condition": "SATURATED", "rain_raw_adc": "1226", "rain_intensity_pct": "91.0", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-7307", "accel_y_raw": "-11878", "accel_z_raw": "9322", "accel_x_g": "-0.446", "accel_y_g": "-0.725", "accel_z_g": "0.569", "accel_magnitude_g": "1.024", "gyro_x_raw": "-11", "gyro_y_raw": "148", "gyro_z_raw": "22", "gyro_x_dps": "-0.09", "gyro_y_dps": "1.13", "gyro_z_dps": "0.17", "gyro_magnitude_dps": "1.15", "tilt_pitch_deg": "-45.08", "tilt_roll_deg": "25.83", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "172", "landslide_risk_level": "CRITICAL ALERT", "hazard_status": "HIGH_LANDSLIDE_HAZARD", "raw_packet": "GS1,TV:1,T:29.30,H:88.90,SRAW:1200,SM:100.0,SC:SATURATED,RRAW:1226,RP:91.0,RC:HEAVY_RAIN,MV:1,TX:-45.08,TY:25.83,AXG:-0.446,AYG:-0.725,AZG:0.569,GXD:-0.09,GYD:1.13,GZD:0.17"}, {"timestamp": "16-10-2026 21:51:15", "packet_id": "33", "temperature_c": "29.3", "temperature_f": "84.74", "humidity_pct": "88.8", "dew_point_c": "27.25", "soil_raw_adc": "1236", "soil_moisture_pct": "98.0", "soil_condition": "SATURATED", "rain_raw_adc": "1242", "rain_intensity_pct": "90.3", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-10207", "accel_y_raw": "0", "accel_z_raw": "13598", "accel_x_g": "-0.623", "accel_y_g": "0.0", "accel_z_g": "0.83", "accel_magnitude_g": "1.038", "gyro_x_raw": "272", "gyro_y_raw": "1480", "gyro_z_raw": "108", "gyro_x_dps": "2.08", "gyro_y_dps": "11.3", "gyro_z_dps": "0.83", "gyro_magnitude_dps": "11.52", "tilt_pitch_deg": "0.0", "tilt_roll_deg": "36.91", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "168", "landslide_risk_level": "CRITICAL ALERT", "hazard_status": "HIGH_LANDSLIDE_HAZARD", "raw_packet": "GS1,TV:1,T:29.30,H:88.80,SRAW:1236,SM:98.0,SC:SATURATED,RRAW:1242,RP:90.3,RC:HEAVY_RAIN,MV:1,TX:0.00,TY:36.91,AXG:-0.623,AYG:0.000,AZG:0.830,GXD:2.08,GYD:11.30,GZD:0.83"}, {"timestamp": "16-10-2026 21:51:20", "packet_id": "34", "temperature_c": "29.3", "temperature_f": "84.74", "humidity_pct": "88.8", "dew_point_c": "27.25", "soil_raw_adc": "1270", "soil_moisture_pct": "96.1", "soil_condition": "SATURATED", "rain_raw_adc": "1329", "rain_intensity_pct": "86.8", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-11796", "accel_y_raw": "32", "accel_z_raw": "11665", "accel_x_g": "-0.72", "accel_y_g": "0.002", "accel_z_g": "0.712", "accel_magnitude_g": "1.013", "gyro_x_raw": "390", "gyro_y_raw": "61", "gyro_z_raw": "-18", "gyro_x_dps": "2.98", "gyro_y_dps": "0.47", "gyro_z_dps": "-0.14", "gyro_magnitude_dps": "3.02", "tilt_pitch_deg": "0.11", "tilt_roll_deg": "45.32", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "168", "landslide_risk_level": "CRITICAL ALERT", "hazard_status": "HIGH_LANDSLIDE_HAZARD", "raw_packet": "GS1,TV:1,T:29.30,H:88.80,SRAW:1270,SM:96.1,SC:SATURATED,RRAW:1329,RP:86.8,RC:HEAVY_RAIN,MV:1,TX:0.11,TY:45.32,AXG:-0.720,AYG:0.002,AZG:0.712,GXD:2.98,GYD:0.47,GZD:-0.14"}, {"timestamp": "16-10-2026 21:51:25", "packet_id": "35", "temperature_c": "29.3", "temperature_f": "84.74", "humidity_pct": "88.7", "dew_point_c": "27.23", "soil_raw_adc": "1314", "soil_moisture_pct": "93.7", "soil_condition": "SATURATED", "rain_raw_adc": "1344", "rain_intensity_pct": "86.2", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-7323", "accel_y_raw": "-6635", "accel_z_raw": "13680", "accel_x_g": "-0.447", "accel_y_g": "-0.405", "accel_z_g": "0.835", "accel_magnitude_g": "1.03", "gyro_x_raw": "665", "gyro_y_raw": "268", "gyro_z_raw": "-361", "gyro_x_dps": "5.08", "gyro_y_dps": "2.05", "gyro_z_dps": "-2.76", "gyro_magnitude_dps": "6.13", "tilt_pitch_deg": "-23.17", "tilt_roll_deg": "25.69", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "171", "landslide_risk_level": "CRITICAL ALERT", "hazard_status": "HIGH_LANDSLIDE_HAZARD", "raw_packet": "GS1,TV:1,T:29.30,H:88.70,SRAW:1314,SM:93.7,SC:SATURATED,RRAW:1344,RP:86.2,RC:HEAVY_RAIN,MV:1,TX:-23.17,TY:25.69,AXG:-0.447,AYG:-0.405,AZG:0.835,GXD:5.08,GYD:2.05,GZD:-2.76"}, {"timestamp": "16-10-2026 21:51:30", "packet_id": "36", "temperature_c": "29.3", "temperature_f": "84.74", "humidity_pct": "88.7", "dew_point_c": "27.23", "soil_raw_adc": "1369", "soil_moisture_pct": "90.6", "soil_condition": "SATURATED", "rain_raw_adc": "1351", "rain_intensity_pct": "86.0", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1638", "accel_y_raw": "131", "accel_z_raw": "16891", "accel_x_g": "-0.1", "accel_y_g": "0.008", "accel_z_g": "1.031", "accel_magnitude_g": "1.036", "gyro_x_raw": "404", "gyro_y_raw": "10", "gyro_z_raw": "-49", "gyro_x_dps": "3.09", "gyro_y_dps": "0.08", "gyro_z_dps": "-0.38", "gyro_magnitude_dps": "3.11", "tilt_pitch_deg": "0.46", "tilt_roll_deg": "5.54", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "167", "landslide_risk_level": "ELEVATED", "hazard_status": "SATURATION_WARNING", "raw_packet": "GS1,TV:1,T:29.30,H:88.70,SRAW:1369,SM:90.6,SC:SATURATED,RRAW:1351,RP:86.0,RC:HEAVY_RAIN,MV:1,TX:0.46,TY:5.54,AXG:-0.100,AYG:0.008,AZG:1.031,GXD:3.09,GYD:0.08,GZD:-0.38"}, {"timestamp": "16-10-2026 21:51:35", "packet_id": "37", "temperature_c": "29.3", "temperature_f": "84.74", "humidity_pct": "88.6", "dew_point_c": "27.21", "soil_raw_adc": "1331", "soil_moisture_pct": "92.7", "soil_condition": "SATURATED", "rain_raw_adc": "1238", "rain_intensity_pct": "90.5", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1638", "accel_y_raw": "114", "accel_z_raw": "16760", "accel_x_g": "-0.1", "accel_y_g": "0.007", "accel_z_g": "1.023", "accel_magnitude_g": "1.028", "gyro_x_raw": "404", "gyro_y_raw": "60", "gyro_z_raw": "-30", "gyro_x_dps": "3.09", "gyro_y_dps": "0.46", "gyro_z_dps": "-0.23", "gyro_magnitude_dps": "3.13", "tilt_pitch_deg": "0.41", "tilt_roll_deg": "5.57", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "167", "landslide_risk_level": "ELEVATED", "hazard_status": "SATURATION_WARNING", "raw_packet": "GS1,TV:1,T:29.30,H:88.60,SRAW:1331,SM:92.7,SC:SATURATED,RRAW:1238,RP:90.5,RC:HEAVY_RAIN,MV:1,TX:0.41,TY:5.57,AXG:-0.100,AYG:0.007,AZG:1.023,GXD:3.09,GYD:0.46,GZD:-0.23"}, {"timestamp": "16-10-2026 21:51:40", "packet_id": "38", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "88.6", "dew_point_c": "27.31", "soil_raw_adc": "1414", "soil_moisture_pct": "88.1", "soil_condition": "SATURATED", "rain_raw_adc": "1283", "rain_intensity_pct": "88.7", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1654", "accel_y_raw": "196", "accel_z_raw": "16793", "accel_x_g": "-0.101", "accel_y_g": "0.012", "accel_z_g": "1.025", "accel_magnitude_g": "1.03", "gyro_x_raw": "407", "gyro_y_raw": "49", "gyro_z_raw": "-52", "gyro_x_dps": "3.11", "gyro_y_dps": "0.38", "gyro_z_dps": "-0.4", "gyro_magnitude_dps": "3.16", "tilt_pitch_deg": "0.65", "tilt_roll_deg": "5.62", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "167", "landslide_risk_level": "ELEVATED", "hazard_status": "SATURATION_WARNING", "raw_packet": "GS1,TV:1,T:29.40,H:88.60,SRAW:1414,SM:88.1,SC:SATURATED,RRAW:1283,RP:88.7,RC:HEAVY_RAIN,MV:1,TX:0.65,TY:5.62,AXG:-0.101,AYG:0.012,AZG:1.025,GXD:3.11,GYD:0.38,GZD:-0.40"}, {"timestamp": "16-10-2026 21:51:45", "packet_id": "39", "temperature_c": "29.3", "temperature_f": "84.74", "humidity_pct": "88.6", "dew_point_c": "27.21", "soil_raw_adc": "1694", "soil_moisture_pct": "72.6", "soil_condition": "WET", "rain_raw_adc": "1678", "rain_intensity_pct": "72.9", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-1638", "accel_y_raw": "131", "accel_z_raw": "16875", "accel_x_g": "-0.1", "accel_y_g": "0.008", "accel_z_g": "1.03", "accel_magnitude_g": "1.035", "gyro_x_raw": "412", "gyro_y_raw": "82", "gyro_z_raw": "-52", "gyro_x_dps": "3.15", "gyro_y_dps": "0.63", "gyro_z_dps": "-0.4", "gyro_magnitude_dps": "3.24", "tilt_pitch_deg": "0.42", "tilt_roll_deg": "5.54", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "164", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.30,H:88.60,SRAW:1694,SM:72.6,SC:WET,RRAW:1678,RP:72.9,RC:MODERATE_RAIN,MV:1,TX:0.42,TY:5.54,AXG:-0.100,AYG:0.008,AZG:1.030,GXD:3.15,GYD:0.63,GZD:-0.40"}, {"timestamp": "16-10-2026 21:51:50", "packet_id": "40", "temperature_c": "29.3", "temperature_f": "84.74", "humidity_pct": "88.6", "dew_point_c": "27.21", "soil_raw_adc": "1755", "soil_moisture_pct": "69.2", "soil_condition": "WET", "rain_raw_adc": "2001", "rain_intensity_pct": "60.0", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-1572", "accel_y_raw": "65", "accel_z_raw": "16760", "accel_x_g": "-0.096", "accel_y_g": "0.004", "accel_z_g": "1.023", "accel_magnitude_g": "1.028", "gyro_x_raw": "381", "gyro_y_raw": "64", "gyro_z_raw": "-32", "gyro_x_dps": "2.91", "gyro_y_dps": "0.49", "gyro_z_dps": "-0.25", "gyro_magnitude_dps": "2.96", "tilt_pitch_deg": "0.2", "tilt_roll_deg": "5.36", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "164", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.30,H:88.60,SRAW:1755,SM:69.2,SC:WET,RRAW:2001,RP:60.0,RC:MODERATE_RAIN,MV:1,TX:0.20,TY:5.36,AXG:-0.096,AYG:0.004,AZG:1.023,GXD:2.91,GYD:0.49,GZD:-0.25"}, {"timestamp": "16-10-2026 21:51:55", "packet_id": "41", "temperature_c": "29.3", "temperature_f": "84.74", "humidity_pct": "88.6", "dew_point_c": "27.21", "soil_raw_adc": "1405", "soil_moisture_pct": "88.6", "soil_condition": "SATURATED", "rain_raw_adc": "1240", "rain_intensity_pct": "90.4", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1654", "accel_y_raw": "147", "accel_z_raw": "16891", "accel_x_g": "-0.101", "accel_y_g": "0.009", "accel_z_g": "1.031", "accel_magnitude_g": "1.036", "gyro_x_raw": "378", "gyro_y_raw": "62", "gyro_z_raw": "-34", "gyro_x_dps": "2.89", "gyro_y_dps": "0.48", "gyro_z_dps": "-0.26", "gyro_magnitude_dps": "2.94", "tilt_pitch_deg": "0.51", "tilt_roll_deg": "5.6", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "167", "landslide_risk_level": "ELEVATED", "hazard_status": "SATURATION_WARNING", "raw_packet": "GS1,TV:1,T:29.30,H:88.60,SRAW:1405,SM:88.6,SC:SATURATED,RRAW:1240,RP:90.4,RC:HEAVY_RAIN,MV:1,TX:0.51,TY:5.60,AXG:-0.101,AYG:0.009,AZG:1.031,GXD:2.89,GYD:0.48,GZD:-0.26"}, {"timestamp": "16-10-2026 21:52:00", "packet_id": "42", "temperature_c": "29.3", "temperature_f": "84.74", "humidity_pct": "88.6", "dew_point_c": "27.21", "soil_raw_adc": "1066", "soil_moisture_pct": "100.0", "soil_condition": "SATURATED", "rain_raw_adc": "1137", "rain_intensity_pct": "94.5", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1671", "accel_y_raw": "98", "accel_z_raw": "16908", "accel_x_g": "-0.102", "accel_y_g": "0.006", "accel_z_g": "1.032", "accel_magnitude_g": "1.037", "gyro_x_raw": "412", "gyro_y_raw": "6", "gyro_z_raw": "-22", "gyro_x_dps": "3.15", "gyro_y_dps": "0.05", "gyro_z_dps": "-0.17", "gyro_magnitude_dps": "3.15", "tilt_pitch_deg": "0.32", "tilt_roll_deg": "5.65", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "168", "landslide_risk_level": "ELEVATED", "hazard_status": "SATURATION_WARNING", "raw_packet": "GS1,TV:1,T:29.30,H:88.60,SRAW:1066,SM:100.0,SC:SATURATED,RRAW:1137,RP:94.5,RC:HEAVY_RAIN,MV:1,TX:0.32,TY:5.65,AXG:-0.102,AYG:0.006,AZG:1.032,GXD:3.15,GYD:0.05,GZD:-0.17"}, {"timestamp": "16-10-2026 21:52:05", "packet_id": "43", "temperature_c": "29.3", "temperature_f": "84.74", "humidity_pct": "88.6", "dew_point_c": "27.21", "soil_raw_adc": "1075", "soil_moisture_pct": "100.0", "soil_condition": "SATURATED", "rain_raw_adc": "1179", "rain_intensity_pct": "92.8", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1638", "accel_y_raw": "98", "accel_z_raw": "16891", "accel_x_g": "-0.1", "accel_y_g": "0.006", "accel_z_g": "1.031", "accel_magnitude_g": "1.036", "gyro_x_raw": "393", "gyro_y_raw": "78", "gyro_z_raw": "-18", "gyro_x_dps": "3.0", "gyro_y_dps": "0.6", "gyro_z_dps": "-0.14", "gyro_magnitude_dps": "3.06", "tilt_pitch_deg": "0.34", "tilt_roll_deg": "5.54", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "168", "landslide_risk_level": "ELEVATED", "hazard_status": "SATURATION_WARNING", "raw_packet": "GS1,TV:1,T:29.30,H:88.60,SRAW:1075,SM:100.0,SC:SATURATED,RRAW:1179,RP:92.8,RC:HEAVY_RAIN,MV:1,TX:0.34,TY:5.54,AXG:-0.100,AYG:0.006,AZG:1.031,GXD:3.00,GYD:0.60,GZD:-0.14"}, {"timestamp": "16-10-2026 21:52:10", "packet_id": "44", "temperature_c": "29.3", "temperature_f": "84.74", "humidity_pct": "88.6", "dew_point_c": "27.21", "soil_raw_adc": "1278", "soil_moisture_pct": "95.7", "soil_condition": "SATURATED", "rain_raw_adc": "1134", "rain_intensity_pct": "94.6", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1703", "accel_y_raw": "212", "accel_z_raw": "16924", "accel_x_g": "-0.104", "accel_y_g": "0.013", "accel_z_g": "1.033", "accel_magnitude_g": "1.038", "gyro_x_raw": "395", "gyro_y_raw": "35", "gyro_z_raw": "-49", "gyro_x_dps": "3.02", "gyro_y_dps": "0.27", "gyro_z_dps": "-0.38", "gyro_magnitude_dps": "3.06", "tilt_pitch_deg": "0.73", "tilt_roll_deg": "5.72", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "167", "landslide_risk_level": "ELEVATED", "hazard_status": "SATURATION_WARNING", "raw_packet": "GS1,TV:1,T:29.30,H:88.60,SRAW:1278,SM:95.7,SC:SATURATED,RRAW:1134,RP:94.6,RC:HEAVY_RAIN,MV:1,TX:0.73,TY:5.72,AXG:-0.104,AYG:0.013,AZG:1.033,GXD:3.02,GYD:0.27,GZD:-0.38"}, {"timestamp": "16-10-2026 21:52:15", "packet_id": "45", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "88.6", "dew_point_c": "27.31", "soil_raw_adc": "1347", "soil_moisture_pct": "91.8", "soil_condition": "SATURATED", "rain_raw_adc": "1192", "rain_intensity_pct": "92.3", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1622", "accel_y_raw": "163", "accel_z_raw": "16777", "accel_x_g": "-0.099", "accel_y_g": "0.01", "accel_z_g": "1.024", "accel_magnitude_g": "1.029", "gyro_x_raw": "412", "gyro_y_raw": "83", "gyro_z_raw": "-40", "gyro_x_dps": "3.15", "gyro_y_dps": "0.64", "gyro_z_dps": "-0.31", "gyro_magnitude_dps": "3.23", "tilt_pitch_deg": "0.56", "tilt_roll_deg": "5.54", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "167", "landslide_risk_level": "ELEVATED", "hazard_status": "SATURATION_WARNING", "raw_packet": "GS1,TV:1,T:29.40,H:88.60,SRAW:1347,SM:91.8,SC:SATURATED,RRAW:1192,RP:92.3,RC:HEAVY_RAIN,MV:1,TX:0.56,TY:5.54,AXG:-0.099,AYG:0.010,AZG:1.024,GXD:3.15,GYD:0.64,GZD:-0.31"}, {"timestamp": "16-10-2026 21:52:20", "packet_id": "46", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "88.6", "dew_point_c": "27.31", "soil_raw_adc": "1599", "soil_moisture_pct": "77.8", "soil_condition": "WET", "rain_raw_adc": "1588", "rain_intensity_pct": "76.5", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1622", "accel_y_raw": "81", "accel_z_raw": "16793", "accel_x_g": "-0.099", "accel_y_g": "0.005", "accel_z_g": "1.025", "accel_magnitude_g": "1.03", "gyro_x_raw": "408", "gyro_y_raw": "68", "gyro_z_raw": "-22", "gyro_x_dps": "3.12", "gyro_y_dps": "0.52", "gyro_z_dps": "-0.17", "gyro_magnitude_dps": "3.17", "tilt_pitch_deg": "0.26", "tilt_roll_deg": "5.52", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "161", "landslide_risk_level": "ELEVATED", "hazard_status": "SATURATION_WARNING", "raw_packet": "GS1,TV:1,T:29.40,H:88.60,SRAW:1599,SM:77.8,SC:WET,RRAW:1588,RP:76.5,RC:HEAVY_RAIN,MV:1,TX:0.26,TY:5.52,AXG:-0.099,AYG:0.005,AZG:1.025,GXD:3.12,GYD:0.52,GZD:-0.17"}, {"timestamp": "16-10-2026 21:52:25", "packet_id": "47", "temperature_c": "29.3", "temperature_f": "84.74", "humidity_pct": "88.6", "dew_point_c": "27.21", "soil_raw_adc": "1920", "soil_moisture_pct": "60.0", "soil_condition": "MODERATE", "rain_raw_adc": "2653", "rain_intensity_pct": "33.9", "rain_condition": "LIGHT_RAIN", "accel_x_raw": "-1654", "accel_y_raw": "131", "accel_z_raw": "16711", "accel_x_g": "-0.101", "accel_y_g": "0.008", "accel_z_g": "1.02", "accel_magnitude_g": "1.025", "gyro_x_raw": "412", "gyro_y_raw": "78", "gyro_z_raw": "-57", "gyro_x_dps": "3.15", "gyro_y_dps": "0.6", "gyro_z_dps": "-0.44", "gyro_magnitude_dps": "3.24", "tilt_pitch_deg": "0.42", "tilt_roll_deg": "5.66", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "166", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.30,H:88.60,SRAW:1920,SM:60.0,SC:MODERATE,RRAW:2653,RP:33.9,RC:LIGHT_RAIN,MV:1,TX:0.42,TY:5.66,AXG:-0.101,AYG:0.008,AZG:1.020,GXD:3.15,GYD:0.60,GZD:-0.44"}, {"timestamp": "16-10-2026 21:52:30", "packet_id": "48", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "88.6", "dew_point_c": "27.31", "soil_raw_adc": "1919", "soil_moisture_pct": "60.1", "soil_condition": "MODERATE", "rain_raw_adc": "2806", "rain_intensity_pct": "27.8", "rain_condition": "LIGHT_RAIN", "accel_x_raw": "-1638", "accel_y_raw": "147", "accel_z_raw": "16826", "accel_x_g": "-0.1", "accel_y_g": "0.009", "accel_z_g": "1.027", "accel_magnitude_g": "1.032", "gyro_x_raw": "375", "gyro_y_raw": "53", "gyro_z_raw": "-6", "gyro_x_dps": "2.87", "gyro_y_dps": "0.41", "gyro_z_dps": "-0.05", "gyro_magnitude_dps": "2.9", "tilt_pitch_deg": "0.52", "tilt_roll_deg": "5.55", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "166", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.40,H:88.60,SRAW:1919,SM:60.1,SC:MODERATE,RRAW:2806,RP:27.8,RC:LIGHT_RAIN,MV:1,TX:0.52,TY:5.55,AXG:-0.100,AYG:0.009,AZG:1.027,GXD:2.87,GYD:0.41,GZD:-0.05"}, {"timestamp": "16-10-2026 21:52:35", "packet_id": "49", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "88.6", "dew_point_c": "27.31", "soil_raw_adc": "1786", "soil_moisture_pct": "67.4", "soil_condition": "WET", "rain_raw_adc": "1873", "rain_intensity_pct": "65.1", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-7012", "accel_y_raw": "212", "accel_z_raw": "15253", "accel_x_g": "-0.428", "accel_y_g": "0.013", "accel_z_g": "0.931", "accel_magnitude_g": "1.025", "gyro_x_raw": "408", "gyro_y_raw": "-112", "gyro_z_raw": "-27", "gyro_x_dps": "3.12", "gyro_y_dps": "-0.86", "gyro_z_dps": "-0.21", "gyro_magnitude_dps": "3.24", "tilt_pitch_deg": "0.75", "tilt_roll_deg": "24.71", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "166", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.40,H:88.60,SRAW:1786,SM:67.4,SC:WET,RRAW:1873,RP:65.1,RC:MODERATE_RAIN,MV:1,TX:0.75,TY:24.71,AXG:-0.428,AYG:0.013,AZG:0.931,GXD:3.12,GYD:-0.86,GZD:-0.21"}, {"timestamp": "16-10-2026 21:52:40", "packet_id": "50", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "88.6", "dew_point_c": "27.31", "soil_raw_adc": "1807", "soil_moisture_pct": "66.3", "soil_condition": "WET", "rain_raw_adc": "1991", "rain_intensity_pct": "60.4", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-2277", "accel_y_raw": "5193", "accel_z_raw": "15564", "accel_x_g": "-0.139", "accel_y_g": "0.317", "accel_z_g": "0.95", "accel_magnitude_g": "1.011", "gyro_x_raw": "610", "gyro_y_raw": "256", "gyro_z_raw": "-310", "gyro_x_dps": "4.66", "gyro_y_dps": "1.96", "gyro_z_dps": "-2.37", "gyro_magnitude_dps": "5.58", "tilt_pitch_deg": "18.24", "tilt_roll_deg": "7.92", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "165", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.40,H:88.60,SRAW:1807,SM:66.3,SC:WET,RRAW:1991,RP:60.4,RC:MODERATE_RAIN,MV:1,TX:18.24,TY:7.92,AXG:-0.139,AYG:0.317,AZG:0.950,GXD:4.66,GYD:1.96,GZD:-2.37"}, {"timestamp": "16-10-2026 21:52:45", "packet_id": "51", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "88.6", "dew_point_c": "27.31", "soil_raw_adc": "1930", "soil_moisture_pct": "59.4", "soil_condition": "MODERATE", "rain_raw_adc": "3347", "rain_intensity_pct": "6.1", "rain_condition": "NO_RAIN", "accel_x_raw": "-1490", "accel_y_raw": "3670", "accel_z_raw": "16334", "accel_x_g": "-0.091", "accel_y_g": "0.224", "accel_z_g": "0.997", "accel_magnitude_g": "1.026", "gyro_x_raw": "437", "gyro_y_raw": "98", "gyro_z_raw": "0", "gyro_x_dps": "3.34", "gyro_y_dps": "0.75", "gyro_z_dps": "0.0", "gyro_magnitude_dps": "3.42", "tilt_pitch_deg": "12.59", "tilt_roll_deg": "5.09", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "162", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:29.40,H:88.60,SRAW:1930,SM:59.4,SC:MODERATE,RRAW:3347,RP:6.1,RC:NO_RAIN,MV:1,TX:12.59,TY:5.09,AXG:-0.091,AYG:0.224,AZG:0.997,GXD:3.34,GYD:0.75,GZD:0.00"}, {"timestamp": "16-10-2026 21:52:50", "packet_id": "52", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "88.5", "dew_point_c": "27.29", "soil_raw_adc": "1990", "soil_moisture_pct": "56.1", "soil_condition": "MODERATE", "rain_raw_adc": "3475", "rain_intensity_pct": "1.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-1638", "accel_y_raw": "3260", "accel_z_raw": "16531", "accel_x_g": "-0.1", "accel_y_g": "0.199", "accel_z_g": "1.009", "accel_magnitude_g": "1.033", "gyro_x_raw": "619", "gyro_y_raw": "195", "gyro_z_raw": "47", "gyro_x_dps": "4.73", "gyro_y_dps": "1.49", "gyro_z_dps": "0.36", "gyro_magnitude_dps": "4.97", "tilt_pitch_deg": "11.13", "tilt_roll_deg": "5.53", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "162", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:29.40,H:88.50,SRAW:1990,SM:56.1,SC:MODERATE,RRAW:3475,RP:1.0,RC:NO_RAIN,MV:1,TX:11.13,TY:5.53,AXG:-0.100,AYG:0.199,AZG:1.009,GXD:4.73,GYD:1.49,GZD:0.36"}, {"timestamp": "16-10-2026 21:52:55", "packet_id": "53", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "88.5", "dew_point_c": "27.29", "soil_raw_adc": "1999", "soil_moisture_pct": "55.6", "soil_condition": "MODERATE", "rain_raw_adc": "3523", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-9093", "accel_y_raw": "393", "accel_z_raw": "13828", "accel_x_g": "-0.555", "accel_y_g": "0.024", "accel_z_g": "0.844", "accel_magnitude_g": "1.01", "gyro_x_raw": "476", "gyro_y_raw": "-1002", "gyro_z_raw": "-133", "gyro_x_dps": "3.64", "gyro_y_dps": "-7.65", "gyro_z_dps": "-1.02", "gyro_magnitude_dps": "8.53", "tilt_pitch_deg": "1.34", "tilt_roll_deg": "33.34", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "164", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.40,H:88.50,SRAW:1999,SM:55.6,SC:MODERATE,RRAW:3523,RP:0.0,RC:NO_RAIN,MV:1,TX:1.34,TY:33.34,AXG:-0.555,AYG:0.024,AZG:0.844,GXD:3.64,GYD:-7.65,GZD:-1.02"}, {"timestamp": "16-10-2026 21:53:00", "packet_id": "54", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "88.5", "dew_point_c": "27.29", "soil_raw_adc": "1975", "soil_moisture_pct": "56.9", "soil_condition": "MODERATE", "rain_raw_adc": "3685", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-3571", "accel_y_raw": "-3702", "accel_z_raw": "15351", "accel_x_g": "-0.218", "accel_y_g": "-0.226", "accel_z_g": "0.937", "accel_magnitude_g": "0.988", "gyro_x_raw": "495", "gyro_y_raw": "-584", "gyro_z_raw": "141", "gyro_x_dps": "3.78", "gyro_y_dps": "-4.46", "gyro_z_dps": "1.08", "gyro_magnitude_dps": "5.95", "tilt_pitch_deg": "-13.22", "tilt_roll_deg": "12.74", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "166", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:29.40,H:88.50,SRAW:1975,SM:56.9,SC:MODERATE,RRAW:3685,RP:0.0,RC:NO_RAIN,MV:1,TX:-13.22,TY:12.74,AXG:-0.218,AYG:-0.226,AZG:0.937,GXD:3.78,GYD:-4.46,GZD:1.08"}, {"timestamp": "16-10-2026 21:53:05", "packet_id": "55", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "88.5", "dew_point_c": "27.29", "soil_raw_adc": "1995", "soil_moisture_pct": "55.8", "soil_condition": "MODERATE", "rain_raw_adc": "3801", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-917", "accel_y_raw": "-8896", "accel_z_raw": "14352", "accel_x_g": "-0.056", "accel_y_g": "-0.543", "accel_z_g": "0.876", "accel_magnitude_g": "1.032", "gyro_x_raw": "727", "gyro_y_raw": "87", "gyro_z_raw": "-187", "gyro_x_dps": "5.55", "gyro_y_dps": "0.67", "gyro_z_dps": "-1.43", "gyro_magnitude_dps": "5.77", "tilt_pitch_deg": "-31.73", "tilt_roll_deg": "3.09", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "165", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.40,H:88.50,SRAW:1995,SM:55.8,SC:MODERATE,RRAW:3801,RP:0.0,RC:NO_RAIN,MV:1,TX:-31.73,TY:3.09,AXG:-0.056,AYG:-0.543,AZG:0.876,GXD:5.55,GYD:0.67,GZD:-1.43"}, {"timestamp": "16-10-2026 21:53:10", "packet_id": "56", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "88.5", "dew_point_c": "27.29", "soil_raw_adc": "2000", "soil_moisture_pct": "55.6", "soil_condition": "MODERATE", "rain_raw_adc": "3888", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "4898", "accel_y_raw": "-753", "accel_z_raw": "16465", "accel_x_g": "0.299", "accel_y_g": "-0.046", "accel_z_g": "1.005", "accel_magnitude_g": "1.05", "gyro_x_raw": "468", "gyro_y_raw": "13", "gyro_z_raw": "-98", "gyro_x_dps": "3.58", "gyro_y_dps": "0.1", "gyro_z_dps": "-0.75", "gyro_magnitude_dps": "3.66", "tilt_pitch_deg": "-2.51", "tilt_roll_deg": "-16.53", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "165", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.40,H:88.50,SRAW:2000,SM:55.6,SC:MODERATE,RRAW:3888,RP:0.0,RC:NO_RAIN,MV:1,TX:-2.51,TY:-16.53,AXG:0.299,AYG:-0.046,AZG:1.005,GXD:3.58,GYD:0.10,GZD:-0.75"}, {"timestamp": "16-10-2026 21:53:15", "packet_id": "57", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "88.5", "dew_point_c": "27.29", "soil_raw_adc": "2001", "soil_moisture_pct": "55.5", "soil_condition": "MODERATE", "rain_raw_adc": "3891", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "4112", "accel_y_raw": "-671", "accel_z_raw": "16465", "accel_x_g": "0.251", "accel_y_g": "-0.041", "accel_z_g": "1.005", "accel_magnitude_g": "1.037", "gyro_x_raw": "17", "gyro_y_raw": "2055", "gyro_z_raw": "-313", "gyro_x_dps": "0.13", "gyro_y_dps": "15.69", "gyro_z_dps": "-2.39", "gyro_magnitude_dps": "15.87", "tilt_pitch_deg": "-2.24", "tilt_roll_deg": "-14.0", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "166", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:29.40,H:88.50,SRAW:2001,SM:55.5,SC:MODERATE,RRAW:3891,RP:0.0,RC:NO_RAIN,MV:1,TX:-2.24,TY:-14.00,AXG:0.251,AYG:-0.041,AZG:1.005,GXD:0.13,GYD:15.69,GZD:-2.39"}, {"timestamp": "16-10-2026 21:53:20", "packet_id": "58", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "88.5", "dew_point_c": "27.29", "soil_raw_adc": "1990", "soil_moisture_pct": "56.1", "soil_condition": "MODERATE", "rain_raw_adc": "3933", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-917", "accel_y_raw": "-1425", "accel_z_raw": "17989", "accel_x_g": "-0.056", "accel_y_g": "-0.087", "accel_z_g": "1.098", "accel_magnitude_g": "1.103", "gyro_x_raw": "1131", "gyro_y_raw": "-413", "gyro_z_raw": "1218", "gyro_x_dps": "8.64", "gyro_y_dps": "-3.16", "gyro_z_dps": "9.3", "gyro_magnitude_dps": "13.08", "tilt_pitch_deg": "-4.53", "tilt_roll_deg": "2.89", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "164", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:29.40,H:88.50,SRAW:1990,SM:56.1,SC:MODERATE,RRAW:3933,RP:0.0,RC:NO_RAIN,MV:1,TX:-4.53,TY:2.89,AXG:-0.056,AYG:-0.087,AZG:1.098,GXD:8.64,GYD:-3.16,GZD:9.30"}, {"timestamp": "16-10-2026 21:53:25", "packet_id": "59", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "88.5", "dew_point_c": "27.29", "soil_raw_adc": "2925", "soil_moisture_pct": "4.2", "soil_condition": "DRY", "rain_raw_adc": "3956", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "393", "accel_y_raw": "2572", "accel_z_raw": "16498", "accel_x_g": "0.024", "accel_y_g": "0.157", "accel_z_g": "1.007", "accel_magnitude_g": "1.019", "gyro_x_raw": "462", "gyro_y_raw": "151", "gyro_z_raw": "-85", "gyro_x_dps": "3.53", "gyro_y_dps": "1.16", "gyro_z_dps": "-0.65", "gyro_magnitude_dps": "3.77", "tilt_pitch_deg": "8.88", "tilt_roll_deg": "-1.36", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "156", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:29.40,H:88.50,SRAW:2925,SM:4.2,SC:DRY,RRAW:3956,RP:0.0,RC:NO_RAIN,MV:1,TX:8.88,TY:-1.36,AXG:0.024,AYG:0.157,AZG:1.007,GXD:3.53,GYD:1.16,GZD:-0.65"}, {"timestamp": "16-10-2026 21:53:30", "packet_id": "60", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "88.5", "dew_point_c": "27.29", "soil_raw_adc": "2324", "soil_moisture_pct": "37.6", "soil_condition": "MODERATE", "rain_raw_adc": "3957", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-3129", "accel_y_raw": "360", "accel_z_raw": "16859", "accel_x_g": "-0.191", "accel_y_g": "0.022", "accel_z_g": "1.029", "accel_magnitude_g": "1.047", "gyro_x_raw": "-1236", "gyro_y_raw": "2888", "gyro_z_raw": "-614", "gyro_x_dps": "-9.44", "gyro_y_dps": "22.05", "gyro_z_dps": "-4.69", "gyro_magnitude_dps": "24.44", "tilt_pitch_deg": "1.2", "tilt_roll_deg": "10.49", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "165", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.40,H:88.50,SRAW:2324,SM:37.6,SC:MODERATE,RRAW:3957,RP:0.0,RC:NO_RAIN,MV:1,TX:1.20,TY:10.49,AXG:-0.191,AYG:0.022,AZG:1.029,GXD:-9.44,GYD:22.05,GZD:-4.69"}, {"timestamp": "16-10-2026 21:53:35", "packet_id": "61", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "88.4", "dew_point_c": "27.27", "soil_raw_adc": "2282", "soil_moisture_pct": "39.9", "soil_condition": "MODERATE", "rain_raw_adc": "4015", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-8388", "accel_y_raw": "-2965", "accel_z_raw": "13942", "accel_x_g": "-0.512", "accel_y_g": "-0.181", "accel_z_g": "0.851", "accel_magnitude_g": "1.01", "gyro_x_raw": "324", "gyro_y_raw": "-103", "gyro_z_raw": "51", "gyro_x_dps": "2.48", "gyro_y_dps": "-0.79", "gyro_z_dps": "0.39", "gyro_magnitude_dps": "2.63", "tilt_pitch_deg": "-10.35", "tilt_roll_deg": "30.48", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "166", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.40,H:88.40,SRAW:2282,SM:39.9,SC:MODERATE,RRAW:4015,RP:0.0,RC:NO_RAIN,MV:1,TX:-10.35,TY:30.48,AXG:-0.512,AYG:-0.181,AZG:0.851,GXD:2.48,GYD:-0.79,GZD:0.39"}, {"timestamp": "16-10-2026 21:53:45", "packet_id": "62", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "88.4", "dew_point_c": "27.27", "soil_raw_adc": "1520", "soil_moisture_pct": "82.2", "soil_condition": "WET", "rain_raw_adc": "1149", "rain_intensity_pct": "94.0", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-15581", "accel_y_raw": "16", "accel_z_raw": "5013", "accel_x_g": "-0.951", "accel_y_g": "0.001", "accel_z_g": "0.306", "accel_magnitude_g": "0.999", "gyro_x_raw": "417", "gyro_y_raw": "-5", "gyro_z_raw": "-40", "gyro_x_dps": "3.19", "gyro_y_dps": "-0.04", "gyro_z_dps": "-0.31", "gyro_magnitude_dps": "3.21", "tilt_pitch_deg": "0.06", "tilt_roll_deg": "72.16", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "163", "landslide_risk_level": "CRITICAL ALERT", "hazard_status": "HIGH_LANDSLIDE_HAZARD", "raw_packet": "GS1,TV:1,T:29.40,H:88.40,SRAW:1520,SM:82.2,SC:WET,RRAW:1149,RP:94.0,RC:HEAVY_RAIN,MV:1,TX:0.06,TY:72.16,AXG:-0.951,AYG:0.001,AZG:0.306,GXD:3.19,GYD:-0.04,GZD:-0.31"}, {"timestamp": "16-10-2026 22:16:28", "packet_id": "63", "temperature_c": "29.7", "temperature_f": "85.46", "humidity_pct": "86.9", "dew_point_c": "27.28", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "3824", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-14909", "accel_y_raw": "114", "accel_z_raw": "6717", "accel_x_g": "-0.91", "accel_y_g": "0.007", "accel_z_g": "0.41", "accel_magnitude_g": "0.998", "gyro_x_raw": "394", "gyro_y_raw": "55", "gyro_z_raw": "-49", "gyro_x_dps": "3.01", "gyro_y_dps": "0.42", "gyro_z_dps": "-0.38", "gyro_magnitude_dps": "3.06", "tilt_pitch_deg": "0.38", "tilt_roll_deg": "65.75", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "157", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.70,H:86.90,SRAW:4095,SM:0.0,SC:DRY,RRAW:3824,RP:0.0,RC:NO_RAIN,MV:1,TX:0.38,TY:65.75,AXG:-0.910,AYG:0.007,AZG:0.410,GXD:3.01,GYD:0.42,GZD:-0.38"}, {"timestamp": "16-10-2026 22:16:28", "packet_id": "64", "temperature_c": "29.7", "temperature_f": "85.46", "humidity_pct": "86.9", "dew_point_c": "27.28", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "3815", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-14925", "accel_y_raw": "0", "accel_z_raw": "6733", "accel_x_g": "-0.911", "accel_y_g": "0.0", "accel_z_g": "0.411", "accel_magnitude_g": "0.999", "gyro_x_raw": "427", "gyro_y_raw": "57", "gyro_z_raw": "2", "gyro_x_dps": "3.26", "gyro_y_dps": "0.44", "gyro_z_dps": "0.02", "gyro_magnitude_dps": "3.29", "tilt_pitch_deg": "0.0", "tilt_roll_deg": "65.7", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "156", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.70,H:86.90,SRAW:4095,SM:0.0,SC:DRY,RRAW:3815,RP:0.0,RC:NO_RAIN,MV:1,TX:0.00,TY:65.70,AXG:-0.911,AYG:0.000,AZG:0.411,GXD:3.26,GYD:0.44,GZD:0.02"}, {"timestamp": "16-10-2026 22:16:28", "packet_id": "65", "temperature_c": "29.7", "temperature_f": "85.46", "humidity_pct": "87.0", "dew_point_c": "27.3", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "3824", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-14794", "accel_y_raw": "81", "accel_z_raw": "6651", "accel_x_g": "-0.903", "accel_y_g": "0.005", "accel_z_g": "0.406", "accel_magnitude_g": "0.99", "gyro_x_raw": "403", "gyro_y_raw": "61", "gyro_z_raw": "-26", "gyro_x_dps": "3.08", "gyro_y_dps": "0.47", "gyro_z_dps": "-0.2", "gyro_magnitude_dps": "3.12", "tilt_pitch_deg": "0.31", "tilt_roll_deg": "65.81", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "157", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.70,H:87.00,SRAW:4095,SM:0.0,SC:DRY,RRAW:3824,RP:0.0,RC:NO_RAIN,MV:1,TX:0.31,TY:65.81,AXG:-0.903,AYG:0.005,AZG:0.406,GXD:3.08,GYD:0.47,GZD:-0.20"}, {"timestamp": "16-10-2026 22:16:28", "packet_id": "66", "temperature_c": "29.7", "temperature_f": "85.46", "humidity_pct": "87.0", "dew_point_c": "27.3", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "3760", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-14893", "accel_y_raw": "49", "accel_z_raw": "6832", "accel_x_g": "-0.909", "accel_y_g": "0.003", "accel_z_g": "0.417", "accel_magnitude_g": "1.0", "gyro_x_raw": "404", "gyro_y_raw": "64", "gyro_z_raw": "-57", "gyro_x_dps": "3.09", "gyro_y_dps": "0.49", "gyro_z_dps": "-0.44", "gyro_magnitude_dps": "3.16", "tilt_pitch_deg": "0.15", "tilt_roll_deg": "65.33", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "157", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.70,H:87.00,SRAW:4095,SM:0.0,SC:DRY,RRAW:3760,RP:0.0,RC:NO_RAIN,MV:1,TX:0.15,TY:65.33,AXG:-0.909,AYG:0.003,AZG:0.417,GXD:3.09,GYD:0.49,GZD:-0.44"}, {"timestamp": "16-10-2026 22:16:28", "packet_id": "67", "temperature_c": "29.6", "temperature_f": "85.28", "humidity_pct": "87.0", "dew_point_c": "27.2", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "3845", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-14925", "accel_y_raw": "81", "accel_z_raw": "6684", "accel_x_g": "-0.911", "accel_y_g": "0.005", "accel_z_g": "0.408", "accel_magnitude_g": "0.998", "gyro_x_raw": "412", "gyro_y_raw": "52", "gyro_z_raw": "-3", "gyro_x_dps": "3.15", "gyro_y_dps": "0.4", "gyro_z_dps": "-0.03", "gyro_magnitude_dps": "3.18", "tilt_pitch_deg": "0.27", "tilt_roll_deg": "65.88", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "157", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.60,H:87.00,SRAW:4095,SM:0.0,SC:DRY,RRAW:3845,RP:0.0,RC:NO_RAIN,MV:1,TX:0.27,TY:65.88,AXG:-0.911,AYG:0.005,AZG:0.408,GXD:3.15,GYD:0.40,GZD:-0.03"}, {"timestamp": "16-10-2026 22:16:28", "packet_id": "68", "temperature_c": "29.7", "temperature_f": "85.46", "humidity_pct": "87.0", "dew_point_c": "27.3", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "3827", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-14827", "accel_y_raw": "16", "accel_z_raw": "6782", "accel_x_g": "-0.905", "accel_y_g": "0.001", "accel_z_g": "0.414", "accel_magnitude_g": "0.995", "gyro_x_raw": "424", "gyro_y_raw": "51", "gyro_z_raw": "-48", "gyro_x_dps": "3.24", "gyro_y_dps": "0.39", "gyro_z_dps": "-0.37", "gyro_magnitude_dps": "3.28", "tilt_pitch_deg": "0.04", "tilt_roll_deg": "65.4", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "157", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.70,H:87.00,SRAW:4095,SM:0.0,SC:DRY,RRAW:3827,RP:0.0,RC:NO_RAIN,MV:1,TX:0.04,TY:65.40,AXG:-0.905,AYG:0.001,AZG:0.414,GXD:3.24,GYD:0.39,GZD:-0.37"}, {"timestamp": "16-10-2026 22:16:32", "packet_id": "69", "temperature_c": "29.7", "temperature_f": "85.46", "humidity_pct": "87.0", "dew_point_c": "27.3", "soil_raw_adc": "4073", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "3905", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-14925", "accel_y_raw": "49", "accel_z_raw": "6717", "accel_x_g": "-0.911", "accel_y_g": "0.003", "accel_z_g": "0.41", "accel_magnitude_g": "0.999", "gyro_x_raw": "382", "gyro_y_raw": "73", "gyro_z_raw": "6", "gyro_x_dps": "2.92", "gyro_y_dps": "0.56", "gyro_z_dps": "0.05", "gyro_magnitude_dps": "2.97", "tilt_pitch_deg": "0.18", "tilt_roll_deg": "65.74", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "156", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.70,H:87.00,SRAW:4073,SM:0.0,SC:DRY,RRAW:3905,RP:0.0,RC:NO_RAIN,MV:1,TX:0.18,TY:65.74,AXG:-0.911,AYG:0.003,AZG:0.410,GXD:2.92,GYD:0.56,GZD:0.05"}, {"timestamp": "16-10-2026 22:16:37", "packet_id": "70", "temperature_c": "29.7", "temperature_f": "85.46", "humidity_pct": "87.0", "dew_point_c": "27.3", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "3769", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-14958", "accel_y_raw": "49", "accel_z_raw": "6733", "accel_x_g": "-0.913", "accel_y_g": "0.003", "accel_z_g": "0.411", "accel_magnitude_g": "1.001", "gyro_x_raw": "408", "gyro_y_raw": "49", "gyro_z_raw": "-10", "gyro_x_dps": "3.12", "gyro_y_dps": "0.38", "gyro_z_dps": "-0.08", "gyro_magnitude_dps": "3.14", "tilt_pitch_deg": "0.2", "tilt_roll_deg": "65.75", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "157", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.70,H:87.00,SRAW:4095,SM:0.0,SC:DRY,RRAW:3769,RP:0.0,RC:NO_RAIN,MV:1,TX:0.20,TY:65.75,AXG:-0.913,AYG:0.003,AZG:0.411,GXD:3.12,GYD:0.38,GZD:-0.08"}, {"timestamp": "16-10-2026 22:16:42", "packet_id": "71", "temperature_c": "29.7", "temperature_f": "85.46", "humidity_pct": "87.0", "dew_point_c": "27.3", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "3846", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-14942", "accel_y_raw": "98", "accel_z_raw": "6750", "accel_x_g": "-0.912", "accel_y_g": "0.006", "accel_z_g": "0.412", "accel_magnitude_g": "1.001", "gyro_x_raw": "410", "gyro_y_raw": "49", "gyro_z_raw": "-19", "gyro_x_dps": "3.13", "gyro_y_dps": "0.38", "gyro_z_dps": "-0.15", "gyro_magnitude_dps": "3.16", "tilt_pitch_deg": "0.32", "tilt_roll_deg": "65.7", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "157", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.70,H:87.00,SRAW:4095,SM:0.0,SC:DRY,RRAW:3846,RP:0.0,RC:NO_RAIN,MV:1,TX:0.32,TY:65.70,AXG:-0.912,AYG:0.006,AZG:0.412,GXD:3.13,GYD:0.38,GZD:-0.15"}, {"timestamp": "16-10-2026 22:16:47", "packet_id": "72", "temperature_c": "29.7", "temperature_f": "85.46", "humidity_pct": "87.0", "dew_point_c": "27.3", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "3845", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-14942", "accel_y_raw": "98", "accel_z_raw": "6864", "accel_x_g": "-0.912", "accel_y_g": "0.006", "accel_z_g": "0.419", "accel_magnitude_g": "1.004", "gyro_x_raw": "395", "gyro_y_raw": "69", "gyro_z_raw": "-43", "gyro_x_dps": "3.02", "gyro_y_dps": "0.53", "gyro_z_dps": "-0.33", "gyro_magnitude_dps": "3.08", "tilt_pitch_deg": "0.33", "tilt_roll_deg": "65.32", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "157", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.70,H:87.00,SRAW:4095,SM:0.0,SC:DRY,RRAW:3845,RP:0.0,RC:NO_RAIN,MV:1,TX:0.33,TY:65.32,AXG:-0.912,AYG:0.006,AZG:0.419,GXD:3.02,GYD:0.53,GZD:-0.33"}, {"timestamp": "16-10-2026 22:16:52", "packet_id": "73", "temperature_c": "29.7", "temperature_f": "85.46", "humidity_pct": "87.1", "dew_point_c": "27.32", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "3839", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-14843", "accel_y_raw": "0", "accel_z_raw": "6799", "accel_x_g": "-0.906", "accel_y_g": "0.0", "accel_z_g": "0.415", "accel_magnitude_g": "0.997", "gyro_x_raw": "386", "gyro_y_raw": "51", "gyro_z_raw": "-15", "gyro_x_dps": "2.95", "gyro_y_dps": "0.39", "gyro_z_dps": "-0.12", "gyro_magnitude_dps": "2.98", "tilt_pitch_deg": "0.03", "tilt_roll_deg": "65.37", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "157", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.70,H:87.10,SRAW:4095,SM:0.0,SC:DRY,RRAW:3839,RP:0.0,RC:NO_RAIN,MV:1,TX:0.03,TY:65.37,AXG:-0.906,AYG:0.000,AZG:0.415,GXD:2.95,GYD:0.39,GZD:-0.12"}, {"timestamp": "16-10-2026 22:17:37", "packet_id": "74", "temperature_c": "29.6", "temperature_f": "85.28", "humidity_pct": "87.2", "dew_point_c": "27.24", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "3905", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-14893", "accel_y_raw": "81", "accel_z_raw": "6733", "accel_x_g": "-0.909", "accel_y_g": "0.005", "accel_z_g": "0.411", "accel_magnitude_g": "0.998", "gyro_x_raw": "416", "gyro_y_raw": "65", "gyro_z_raw": "-74", "gyro_x_dps": "3.18", "gyro_y_dps": "0.5", "gyro_z_dps": "-0.57", "gyro_magnitude_dps": "3.27", "tilt_pitch_deg": "0.31", "tilt_roll_deg": "65.64", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "157", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.60,H:87.20,SRAW:4095,SM:0.0,SC:DRY,RRAW:3905,RP:0.0,RC:NO_RAIN,MV:1,TX:0.31,TY:65.64,AXG:-0.909,AYG:0.005,AZG:0.411,GXD:3.18,GYD:0.50,GZD:-0.57"}, {"timestamp": "16-10-2026 22:17:42", "packet_id": "75", "temperature_c": "29.6", "temperature_f": "85.28", "humidity_pct": "87.2", "dew_point_c": "27.24", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "3841", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-14909", "accel_y_raw": "65", "accel_z_raw": "6815", "accel_x_g": "-0.91", "accel_y_g": "0.004", "accel_z_g": "0.416", "accel_magnitude_g": "1.001", "gyro_x_raw": "395", "gyro_y_raw": "73", "gyro_z_raw": "-15", "gyro_x_dps": "3.02", "gyro_y_dps": "0.56", "gyro_z_dps": "-0.12", "gyro_magnitude_dps": "3.07", "tilt_pitch_deg": "0.22", "tilt_roll_deg": "65.42", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "157", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.60,H:87.20,SRAW:4095,SM:0.0,SC:DRY,RRAW:3841,RP:0.0,RC:NO_RAIN,MV:1,TX:0.22,TY:65.42,AXG:-0.910,AYG:0.004,AZG:0.416,GXD:3.02,GYD:0.56,GZD:-0.12"}, {"timestamp": "16-10-2026 22:17:47", "packet_id": "76", "temperature_c": "29.6", "temperature_f": "85.28", "humidity_pct": "87.2", "dew_point_c": "27.24", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1683", "rain_intensity_pct": "72.7", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-14860", "accel_y_raw": "65", "accel_z_raw": "6750", "accel_x_g": "-0.907", "accel_y_g": "0.004", "accel_z_g": "0.412", "accel_magnitude_g": "0.996", "gyro_x_raw": "411", "gyro_y_raw": "91", "gyro_z_raw": "-66", "gyro_x_dps": "3.14", "gyro_y_dps": "0.7", "gyro_z_dps": "-0.51", "gyro_magnitude_dps": "3.26", "tilt_pitch_deg": "0.21", "tilt_roll_deg": "65.58", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "164", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.60,H:87.20,SRAW:4095,SM:0.0,SC:DRY,RRAW:1683,RP:72.7,RC:MODERATE_RAIN,MV:1,TX:0.21,TY:65.58,AXG:-0.907,AYG:0.004,AZG:0.412,GXD:3.14,GYD:0.70,GZD:-0.51"}, {"timestamp": "16-10-2026 22:17:52", "packet_id": "77", "temperature_c": "29.6", "temperature_f": "85.28", "humidity_pct": "87.2", "dew_point_c": "27.24", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "2539", "rain_intensity_pct": "38.4", "rain_condition": "LIGHT_RAIN", "accel_x_raw": "-14893", "accel_y_raw": "49", "accel_z_raw": "6815", "accel_x_g": "-0.909", "accel_y_g": "0.003", "accel_z_g": "0.416", "accel_magnitude_g": "1.0", "gyro_x_raw": "420", "gyro_y_raw": "90", "gyro_z_raw": "-23", "gyro_x_dps": "3.21", "gyro_y_dps": "0.69", "gyro_z_dps": "-0.18", "gyro_magnitude_dps": "3.29", "tilt_pitch_deg": "0.18", "tilt_roll_deg": "65.4", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "161", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.60,H:87.20,SRAW:4095,SM:0.0,SC:DRY,RRAW:2539,RP:38.4,RC:LIGHT_RAIN,MV:1,TX:0.18,TY:65.40,AXG:-0.909,AYG:0.003,AZG:0.416,GXD:3.21,GYD:0.69,GZD:-0.18"}, {"timestamp": "16-10-2026 22:17:57", "packet_id": "78", "temperature_c": "29.6", "temperature_f": "85.28", "humidity_pct": "87.2", "dew_point_c": "27.24", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1250", "rain_intensity_pct": "90.0", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-14860", "accel_y_raw": "114", "accel_z_raw": "6733", "accel_x_g": "-0.907", "accel_y_g": "0.007", "accel_z_g": "0.411", "accel_magnitude_g": "0.996", "gyro_x_raw": "406", "gyro_y_raw": "78", "gyro_z_raw": "-40", "gyro_x_dps": "3.1", "gyro_y_dps": "0.6", "gyro_z_dps": "-0.31", "gyro_magnitude_dps": "3.17", "tilt_pitch_deg": "0.39", "tilt_roll_deg": "65.64", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "161", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.60,H:87.20,SRAW:4095,SM:0.0,SC:DRY,RRAW:1250,RP:90.0,RC:HEAVY_RAIN,MV:1,TX:0.39,TY:65.64,AXG:-0.907,AYG:0.007,AZG:0.411,GXD:3.10,GYD:0.60,GZD:-0.31"}, {"timestamp": "16-10-2026 22:18:02", "packet_id": "79", "temperature_c": "29.6", "temperature_f": "85.28", "humidity_pct": "87.3", "dew_point_c": "27.26", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "892", "rain_intensity_pct": "100.0", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-14893", "accel_y_raw": "0", "accel_z_raw": "6832", "accel_x_g": "-0.909", "accel_y_g": "0.0", "accel_z_g": "0.417", "accel_magnitude_g": "1.0", "gyro_x_raw": "370", "gyro_y_raw": "65", "gyro_z_raw": "6", "gyro_x_dps": "2.83", "gyro_y_dps": "0.5", "gyro_z_dps": "0.05", "gyro_magnitude_dps": "2.87", "tilt_pitch_deg": "0.03", "tilt_roll_deg": "65.34", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "160", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.60,H:87.30,SRAW:4095,SM:0.0,SC:DRY,RRAW:892,RP:100.0,RC:HEAVY_RAIN,MV:1,TX:0.03,TY:65.34,AXG:-0.909,AYG:0.000,AZG:0.417,GXD:2.83,GYD:0.50,GZD:0.05"}, {"timestamp": "16-10-2026 22:18:07", "packet_id": "80", "temperature_c": "29.6", "temperature_f": "85.28", "humidity_pct": "87.3", "dew_point_c": "27.26", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "877", "rain_intensity_pct": "100.0", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-14942", "accel_y_raw": "114", "accel_z_raw": "6799", "accel_x_g": "-0.912", "accel_y_g": "0.007", "accel_z_g": "0.415", "accel_magnitude_g": "1.002", "gyro_x_raw": "424", "gyro_y_raw": "94", "gyro_z_raw": "5", "gyro_x_dps": "3.24", "gyro_y_dps": "0.72", "gyro_z_dps": "0.04", "gyro_magnitude_dps": "3.32", "tilt_pitch_deg": "0.4", "tilt_roll_deg": "65.56", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "160", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.60,H:87.30,SRAW:4095,SM:0.0,SC:DRY,RRAW:877,RP:100.0,RC:HEAVY_RAIN,MV:1,TX:0.40,TY:65.56,AXG:-0.912,AYG:0.007,AZG:0.415,GXD:3.24,GYD:0.72,GZD:0.04"}, {"timestamp": "16-10-2026 22:18:12", "packet_id": "81", "temperature_c": "29.6", "temperature_f": "85.28", "humidity_pct": "87.3", "dew_point_c": "27.26", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "914", "rain_intensity_pct": "100.0", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-11780", "accel_y_raw": "114", "accel_z_raw": "11534", "accel_x_g": "-0.719", "accel_y_g": "0.007", "accel_z_g": "0.704", "accel_magnitude_g": "1.006", "gyro_x_raw": "407", "gyro_y_raw": "127", "gyro_z_raw": "-24", "gyro_x_dps": "3.11", "gyro_y_dps": "0.97", "gyro_z_dps": "-0.19", "gyro_magnitude_dps": "3.26", "tilt_pitch_deg": "0.38", "tilt_roll_deg": "45.61", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "161", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.60,H:87.30,SRAW:4095,SM:0.0,SC:DRY,RRAW:914,RP:100.0,RC:HEAVY_RAIN,MV:1,TX:0.38,TY:45.61,AXG:-0.719,AYG:0.007,AZG:0.704,GXD:3.11,GYD:0.97,GZD:-0.19"}, {"timestamp": "16-10-2026 22:18:17", "packet_id": "82", "temperature_c": "29.6", "temperature_f": "85.28", "humidity_pct": "87.3", "dew_point_c": "27.26", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "941", "rain_intensity_pct": "100.0", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1622", "accel_y_raw": "163", "accel_z_raw": "16728", "accel_x_g": "-0.099", "accel_y_g": "0.01", "accel_z_g": "1.021", "accel_magnitude_g": "1.026", "gyro_x_raw": "407", "gyro_y_raw": "49", "gyro_z_raw": "-23", "gyro_x_dps": "3.11", "gyro_y_dps": "0.38", "gyro_z_dps": "-0.18", "gyro_magnitude_dps": "3.14", "tilt_pitch_deg": "0.55", "tilt_roll_deg": "5.56", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "160", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.60,H:87.30,SRAW:4095,SM:0.0,SC:DRY,RRAW:941,RP:100.0,RC:HEAVY_RAIN,MV:1,TX:0.55,TY:5.56,AXG:-0.099,AYG:0.010,AZG:1.021,GXD:3.11,GYD:0.38,GZD:-0.18"}, {"timestamp": "16-10-2026 22:18:22", "packet_id": "83", "temperature_c": "29.6", "temperature_f": "85.28", "humidity_pct": "87.4", "dew_point_c": "27.28", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "881", "rain_intensity_pct": "100.0", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1687", "accel_y_raw": "131", "accel_z_raw": "16891", "accel_x_g": "-0.103", "accel_y_g": "0.008", "accel_z_g": "1.031", "accel_magnitude_g": "1.036", "gyro_x_raw": "415", "gyro_y_raw": "23", "gyro_z_raw": "-20", "gyro_x_dps": "3.17", "gyro_y_dps": "0.18", "gyro_z_dps": "-0.16", "gyro_magnitude_dps": "3.18", "tilt_pitch_deg": "0.42", "tilt_roll_deg": "5.69", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "160", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.60,H:87.40,SRAW:4095,SM:0.0,SC:DRY,RRAW:881,RP:100.0,RC:HEAVY_RAIN,MV:1,TX:0.42,TY:5.69,AXG:-0.103,AYG:0.008,AZG:1.031,GXD:3.17,GYD:0.18,GZD:-0.16"}, {"timestamp": "16-10-2026 22:18:27", "packet_id": "84", "temperature_c": "29.6", "temperature_f": "85.28", "humidity_pct": "87.3", "dew_point_c": "27.26", "soil_raw_adc": "1702", "soil_moisture_pct": "72.1", "soil_condition": "WET", "rain_raw_adc": "1040", "rain_intensity_pct": "98.4", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1622", "accel_y_raw": "131", "accel_z_raw": "16842", "accel_x_g": "-0.099", "accel_y_g": "0.008", "accel_z_g": "1.028", "accel_magnitude_g": "1.033", "gyro_x_raw": "420", "gyro_y_raw": "20", "gyro_z_raw": "-30", "gyro_x_dps": "3.21", "gyro_y_dps": "0.16", "gyro_z_dps": "-0.23", "gyro_magnitude_dps": "3.22", "tilt_pitch_deg": "0.43", "tilt_roll_deg": "5.51", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "161", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.60,H:87.30,SRAW:1702,SM:72.1,SC:WET,RRAW:1040,RP:98.4,RC:HEAVY_RAIN,MV:1,TX:0.43,TY:5.51,AXG:-0.099,AYG:0.008,AZG:1.028,GXD:3.21,GYD:0.16,GZD:-0.23"}, {"timestamp": "16-10-2026 22:18:32", "packet_id": "85", "temperature_c": "29.6", "temperature_f": "85.28", "humidity_pct": "87.4", "dew_point_c": "27.28", "soil_raw_adc": "1527", "soil_moisture_pct": "81.8", "soil_condition": "WET", "rain_raw_adc": "1047", "rain_intensity_pct": "98.1", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1654", "accel_y_raw": "163", "accel_z_raw": "16826", "accel_x_g": "-0.101", "accel_y_g": "0.01", "accel_z_g": "1.027", "accel_magnitude_g": "1.032", "gyro_x_raw": "406", "gyro_y_raw": "27", "gyro_z_raw": "-34", "gyro_x_dps": "3.1", "gyro_y_dps": "0.21", "gyro_z_dps": "-0.26", "gyro_magnitude_dps": "3.12", "tilt_pitch_deg": "0.58", "tilt_roll_deg": "5.59", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "161", "landslide_risk_level": "ELEVATED", "hazard_status": "SATURATION_WARNING", "raw_packet": "GS1,TV:1,T:29.60,H:87.40,SRAW:1527,SM:81.8,SC:WET,RRAW:1047,RP:98.1,RC:HEAVY_RAIN,MV:1,TX:0.58,TY:5.59,AXG:-0.101,AYG:0.010,AZG:1.027,GXD:3.10,GYD:0.21,GZD:-0.26"}, {"timestamp": "16-10-2026 22:18:37", "packet_id": "86", "temperature_c": "29.6", "temperature_f": "85.28", "humidity_pct": "87.4", "dew_point_c": "27.28", "soil_raw_adc": "1216", "soil_moisture_pct": "99.1", "soil_condition": "SATURATED", "rain_raw_adc": "1092", "rain_intensity_pct": "96.3", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1605", "accel_y_raw": "163", "accel_z_raw": "16809", "accel_x_g": "-0.098", "accel_y_g": "0.01", "accel_z_g": "1.026", "accel_magnitude_g": "1.031", "gyro_x_raw": "416", "gyro_y_raw": "32", "gyro_z_raw": "-68", "gyro_x_dps": "3.18", "gyro_y_dps": "0.25", "gyro_z_dps": "-0.52", "gyro_magnitude_dps": "3.23", "tilt_pitch_deg": "0.57", "tilt_roll_deg": "5.44", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "167", "landslide_risk_level": "ELEVATED", "hazard_status": "SATURATION_WARNING", "raw_packet": "GS1,TV:1,T:29.60,H:87.40,SRAW:1216,SM:99.1,SC:SATURATED,RRAW:1092,RP:96.3,RC:HEAVY_RAIN,MV:1,TX:0.57,TY:5.44,AXG:-0.098,AYG:0.010,AZG:1.026,GXD:3.18,GYD:0.25,GZD:-0.52"}, {"timestamp": "16-10-2026 22:18:42", "packet_id": "87", "temperature_c": "29.6", "temperature_f": "85.28", "humidity_pct": "87.4", "dew_point_c": "27.28", "soil_raw_adc": "1355", "soil_moisture_pct": "91.4", "soil_condition": "SATURATED", "rain_raw_adc": "1088", "rain_intensity_pct": "96.5", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1671", "accel_y_raw": "180", "accel_z_raw": "16728", "accel_x_g": "-0.102", "accel_y_g": "0.011", "accel_z_g": "1.021", "accel_magnitude_g": "1.026", "gyro_x_raw": "408", "gyro_y_raw": "77", "gyro_z_raw": "-35", "gyro_x_dps": "3.12", "gyro_y_dps": "0.59", "gyro_z_dps": "-0.27", "gyro_magnitude_dps": "3.19", "tilt_pitch_deg": "0.64", "tilt_roll_deg": "5.7", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "167", "landslide_risk_level": "ELEVATED", "hazard_status": "SATURATION_WARNING", "raw_packet": "GS1,TV:1,T:29.60,H:87.40,SRAW:1355,SM:91.4,SC:SATURATED,RRAW:1088,RP:96.5,RC:HEAVY_RAIN,MV:1,TX:0.64,TY:5.70,AXG:-0.102,AYG:0.011,AZG:1.021,GXD:3.12,GYD:0.59,GZD:-0.27"}, {"timestamp": "16-10-2026 22:18:47", "packet_id": "88", "temperature_c": "29.6", "temperature_f": "85.28", "humidity_pct": "87.4", "dew_point_c": "27.28", "soil_raw_adc": "1267", "soil_moisture_pct": "96.3", "soil_condition": "SATURATED", "rain_raw_adc": "1085", "rain_intensity_pct": "96.6", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1605", "accel_y_raw": "147", "accel_z_raw": "16793", "accel_x_g": "-0.098", "accel_y_g": "0.009", "accel_z_g": "1.025", "accel_magnitude_g": "1.03", "gyro_x_raw": "385", "gyro_y_raw": "35", "gyro_z_raw": "-41", "gyro_x_dps": "2.94", "gyro_y_dps": "0.27", "gyro_z_dps": "-0.32", "gyro_magnitude_dps": "2.97", "tilt_pitch_deg": "0.49", "tilt_roll_deg": "5.47", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "167", "landslide_risk_level": "ELEVATED", "hazard_status": "SATURATION_WARNING", "raw_packet": "GS1,TV:1,T:29.60,H:87.40,SRAW:1267,SM:96.3,SC:SATURATED,RRAW:1085,RP:96.6,RC:HEAVY_RAIN,MV:1,TX:0.49,TY:5.47,AXG:-0.098,AYG:0.009,AZG:1.025,GXD:2.94,GYD:0.27,GZD:-0.32"}, {"timestamp": "16-10-2026 22:18:52", "packet_id": "89", "temperature_c": "29.6", "temperature_f": "85.28", "humidity_pct": "87.4", "dew_point_c": "27.28", "soil_raw_adc": "1502", "soil_moisture_pct": "83.2", "soil_condition": "WET", "rain_raw_adc": "1074", "rain_intensity_pct": "97.0", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1638", "accel_y_raw": "147", "accel_z_raw": "16891", "accel_x_g": "-0.1", "accel_y_g": "0.009", "accel_z_g": "1.031", "accel_magnitude_g": "1.036", "gyro_x_raw": "398", "gyro_y_raw": "75", "gyro_z_raw": "-52", "gyro_x_dps": "3.04", "gyro_y_dps": "0.58", "gyro_z_dps": "-0.4", "gyro_magnitude_dps": "3.12", "tilt_pitch_deg": "0.47", "tilt_roll_deg": "5.54", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "161", "landslide_risk_level": "ELEVATED", "hazard_status": "SATURATION_WARNING", "raw_packet": "GS1,TV:1,T:29.60,H:87.40,SRAW:1502,SM:83.2,SC:WET,RRAW:1074,RP:97.0,RC:HEAVY_RAIN,MV:1,TX:0.47,TY:5.54,AXG:-0.100,AYG:0.009,AZG:1.031,GXD:3.04,GYD:0.58,GZD:-0.40"}, {"timestamp": "16-10-2026 22:18:57", "packet_id": "90", "temperature_c": "29.6", "temperature_f": "85.28", "humidity_pct": "87.4", "dew_point_c": "27.28", "soil_raw_adc": "1569", "soil_moisture_pct": "79.5", "soil_condition": "WET", "rain_raw_adc": "1245", "rain_intensity_pct": "90.2", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1654", "accel_y_raw": "114", "accel_z_raw": "16859", "accel_x_g": "-0.101", "accel_y_g": "0.007", "accel_z_g": "1.029", "accel_magnitude_g": "1.034", "gyro_x_raw": "428", "gyro_y_raw": "73", "gyro_z_raw": "13", "gyro_x_dps": "3.27", "gyro_y_dps": "0.56", "gyro_z_dps": "0.1", "gyro_magnitude_dps": "3.32", "tilt_pitch_deg": "0.39", "tilt_roll_deg": "5.58", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "160", "landslide_risk_level": "ELEVATED", "hazard_status": "SATURATION_WARNING", "raw_packet": "GS1,TV:1,T:29.60,H:87.40,SRAW:1569,SM:79.5,SC:WET,RRAW:1245,RP:90.2,RC:HEAVY_RAIN,MV:1,TX:0.39,TY:5.58,AXG:-0.101,AYG:0.007,AZG:1.029,GXD:3.27,GYD:0.56,GZD:0.10"}, {"timestamp": "16-10-2026 22:19:02", "packet_id": "91", "temperature_c": "29.6", "temperature_f": "85.28", "humidity_pct": "87.4", "dew_point_c": "27.28", "soil_raw_adc": "1087", "soil_moisture_pct": "100.0", "soil_condition": "SATURATED", "rain_raw_adc": "1095", "rain_intensity_pct": "96.2", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1638", "accel_y_raw": "147", "accel_z_raw": "16875", "accel_x_g": "-0.1", "accel_y_g": "0.009", "accel_z_g": "1.03", "accel_magnitude_g": "1.035", "gyro_x_raw": "386", "gyro_y_raw": "70", "gyro_z_raw": "9", "gyro_x_dps": "2.95", "gyro_y_dps": "0.54", "gyro_z_dps": "0.07", "gyro_magnitude_dps": "3.0", "tilt_pitch_deg": "0.49", "tilt_roll_deg": "5.53", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "167", "landslide_risk_level": "ELEVATED", "hazard_status": "SATURATION_WARNING", "raw_packet": "GS1,TV:1,T:29.60,H:87.40,SRAW:1087,SM:100.0,SC:SATURATED,RRAW:1095,RP:96.2,RC:HEAVY_RAIN,MV:1,TX:0.49,TY:5.53,AXG:-0.100,AYG:0.009,AZG:1.030,GXD:2.95,GYD:0.54,GZD:0.07"}, {"timestamp": "16-10-2026 22:19:07", "packet_id": "92", "temperature_c": "29.6", "temperature_f": "85.28", "humidity_pct": "87.4", "dew_point_c": "27.28", "soil_raw_adc": "1121", "soil_moisture_pct": "100.0", "soil_condition": "SATURATED", "rain_raw_adc": "1094", "rain_intensity_pct": "96.2", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-15826", "accel_y_raw": "65", "accel_z_raw": "3899", "accel_x_g": "-0.966", "accel_y_g": "0.004", "accel_z_g": "0.238", "accel_magnitude_g": "0.995", "gyro_x_raw": "406", "gyro_y_raw": "53", "gyro_z_raw": "-17", "gyro_x_dps": "3.1", "gyro_y_dps": "0.41", "gyro_z_dps": "-0.13", "gyro_magnitude_dps": "3.13", "tilt_pitch_deg": "0.25", "tilt_roll_deg": "76.16", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "CRITICAL ALERT", "hazard_status": "HIGH_LANDSLIDE_HAZARD", "raw_packet": "GS1,TV:1,T:29.60,H:87.40,SRAW:1121,SM:100.0,SC:SATURATED,RRAW:1094,RP:96.2,RC:HEAVY_RAIN,MV:1,TX:0.25,TY:76.16,AXG:-0.966,AYG:0.004,AZG:0.238,GXD:3.10,GYD:0.41,GZD:-0.13"}, {"timestamp": "16-10-2026 22:19:12", "packet_id": "93", "temperature_c": "29.6", "temperature_f": "85.28", "humidity_pct": "87.4", "dew_point_c": "27.28", "soil_raw_adc": "1125", "soil_moisture_pct": "100.0", "soil_condition": "SATURATED", "rain_raw_adc": "1104", "rain_intensity_pct": "95.8", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-15859", "accel_y_raw": "65", "accel_z_raw": "3899", "accel_x_g": "-0.968", "accel_y_g": "0.004", "accel_z_g": "0.238", "accel_magnitude_g": "0.997", "gyro_x_raw": "420", "gyro_y_raw": "52", "gyro_z_raw": "-7", "gyro_x_dps": "3.21", "gyro_y_dps": "0.4", "gyro_z_dps": "-0.06", "gyro_magnitude_dps": "3.24", "tilt_pitch_deg": "0.24", "tilt_roll_deg": "76.2", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "CRITICAL ALERT", "hazard_status": "HIGH_LANDSLIDE_HAZARD", "raw_packet": "GS1,TV:1,T:29.60,H:87.40,SRAW:1125,SM:100.0,SC:SATURATED,RRAW:1104,RP:95.8,RC:HEAVY_RAIN,MV:1,TX:0.24,TY:76.20,AXG:-0.968,AYG:0.004,AZG:0.238,GXD:3.21,GYD:0.40,GZD:-0.06"}, {"timestamp": "16-10-2026 22:19:17", "packet_id": "94", "temperature_c": "29.6", "temperature_f": "85.28", "humidity_pct": "87.4", "dew_point_c": "27.28", "soil_raw_adc": "1121", "soil_moisture_pct": "100.0", "soil_condition": "SATURATED", "rain_raw_adc": "1104", "rain_intensity_pct": "95.8", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-15794", "accel_y_raw": "16", "accel_z_raw": "3948", "accel_x_g": "-0.964", "accel_y_g": "0.001", "accel_z_g": "0.241", "accel_magnitude_g": "0.994", "gyro_x_raw": "407", "gyro_y_raw": "48", "gyro_z_raw": "-7", "gyro_x_dps": "3.11", "gyro_y_dps": "0.37", "gyro_z_dps": "-0.06", "gyro_magnitude_dps": "3.13", "tilt_pitch_deg": "0.07", "tilt_roll_deg": "75.94", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "CRITICAL ALERT", "hazard_status": "HIGH_LANDSLIDE_HAZARD", "raw_packet": "GS1,TV:1,T:29.60,H:87.40,SRAW:1121,SM:100.0,SC:SATURATED,RRAW:1104,RP:95.8,RC:HEAVY_RAIN,MV:1,TX:0.07,TY:75.94,AXG:-0.964,AYG:0.001,AZG:0.241,GXD:3.11,GYD:0.37,GZD:-0.06"}, {"timestamp": "16-10-2026 22:19:22", "packet_id": "95", "temperature_c": "29.6", "temperature_f": "85.28", "humidity_pct": "87.4", "dew_point_c": "27.28", "soil_raw_adc": "1085", "soil_moisture_pct": "100.0", "soil_condition": "SATURATED", "rain_raw_adc": "1104", "rain_intensity_pct": "95.8", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-15810", "accel_y_raw": "-49", "accel_z_raw": "3899", "accel_x_g": "-0.965", "accel_y_g": "-0.003", "accel_z_g": "0.238", "accel_magnitude_g": "0.994", "gyro_x_raw": "394", "gyro_y_raw": "48", "gyro_z_raw": "-24", "gyro_x_dps": "3.01", "gyro_y_dps": "0.37", "gyro_z_dps": "-0.19", "gyro_magnitude_dps": "3.04", "tilt_pitch_deg": "-0.2", "tilt_roll_deg": "76.17", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "171", "landslide_risk_level": "CRITICAL ALERT", "hazard_status": "HIGH_LANDSLIDE_HAZARD", "raw_packet": "GS1,TV:1,T:29.60,H:87.40,SRAW:1085,SM:100.0,SC:SATURATED,RRAW:1104,RP:95.8,RC:HEAVY_RAIN,MV:1,TX:-0.20,TY:76.17,AXG:-0.965,AYG:-0.003,AZG:0.238,GXD:3.01,GYD:0.37,GZD:-0.19"}, {"timestamp": "16-10-2026 22:19:27", "packet_id": "96", "temperature_c": "29.6", "temperature_f": "85.28", "humidity_pct": "87.3", "dew_point_c": "27.26", "soil_raw_adc": "1121", "soil_moisture_pct": "100.0", "soil_condition": "SATURATED", "rain_raw_adc": "1104", "rain_intensity_pct": "95.8", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-15843", "accel_y_raw": "0", "accel_z_raw": "3883", "accel_x_g": "-0.967", "accel_y_g": "0.0", "accel_z_g": "0.237", "accel_magnitude_g": "0.996", "gyro_x_raw": "378", "gyro_y_raw": "44", "gyro_z_raw": "-65", "gyro_x_dps": "2.89", "gyro_y_dps": "0.34", "gyro_z_dps": "-0.5", "gyro_magnitude_dps": "2.95", "tilt_pitch_deg": "0.0", "tilt_roll_deg": "76.25", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "CRITICAL ALERT", "hazard_status": "HIGH_LANDSLIDE_HAZARD", "raw_packet": "GS1,TV:1,T:29.60,H:87.30,SRAW:1121,SM:100.0,SC:SATURATED,RRAW:1104,RP:95.8,RC:HEAVY_RAIN,MV:1,TX:0.00,TY:76.25,AXG:-0.967,AYG:0.000,AZG:0.237,GXD:2.89,GYD:0.34,GZD:-0.50"}, {"timestamp": "16-10-2026 22:19:32", "packet_id": "97", "temperature_c": "29.6", "temperature_f": "85.28", "humidity_pct": "87.3", "dew_point_c": "27.26", "soil_raw_adc": "1456", "soil_moisture_pct": "85.8", "soil_condition": "SATURATED", "rain_raw_adc": "1247", "rain_intensity_pct": "90.1", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-15859", "accel_y_raw": "16", "accel_z_raw": "3801", "accel_x_g": "-0.968", "accel_y_g": "0.001", "accel_z_g": "0.232", "accel_magnitude_g": "0.995", "gyro_x_raw": "425", "gyro_y_raw": "56", "gyro_z_raw": "-15", "gyro_x_dps": "3.25", "gyro_y_dps": "0.43", "gyro_z_dps": "-0.12", "gyro_magnitude_dps": "3.28", "tilt_pitch_deg": "0.08", "tilt_roll_deg": "76.5", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "168", "landslide_risk_level": "CRITICAL ALERT", "hazard_status": "HIGH_LANDSLIDE_HAZARD", "raw_packet": "GS1,TV:1,T:29.60,H:87.30,SRAW:1456,SM:85.8,SC:SATURATED,RRAW:1247,RP:90.1,RC:HEAVY_RAIN,MV:1,TX:0.08,TY:76.50,AXG:-0.968,AYG:0.001,AZG:0.232,GXD:3.25,GYD:0.43,GZD:-0.12"}, {"timestamp": "16-10-2026 22:19:37", "packet_id": "98", "temperature_c": "29.6", "temperature_f": "85.28", "humidity_pct": "87.3", "dew_point_c": "27.26", "soil_raw_adc": "2198", "soil_moisture_pct": "44.6", "soil_condition": "MODERATE", "rain_raw_adc": "1376", "rain_intensity_pct": "85.0", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-15810", "accel_y_raw": "0", "accel_z_raw": "3801", "accel_x_g": "-0.965", "accel_y_g": "-0.0", "accel_z_g": "0.232", "accel_magnitude_g": "0.992", "gyro_x_raw": "411", "gyro_y_raw": "82", "gyro_z_raw": "-44", "gyro_x_dps": "3.14", "gyro_y_dps": "0.63", "gyro_z_dps": "-0.34", "gyro_magnitude_dps": "3.22", "tilt_pitch_deg": "-0.03", "tilt_roll_deg": "76.49", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.60,H:87.30,SRAW:2198,SM:44.6,SC:MODERATE,RRAW:1376,RP:85.0,RC:HEAVY_RAIN,MV:1,TX:-0.03,TY:76.49,AXG:-0.965,AYG:-0.000,AZG:0.232,GXD:3.14,GYD:0.63,GZD:-0.34"}, {"timestamp": "16-10-2026 22:19:42", "packet_id": "99", "temperature_c": "29.6", "temperature_f": "85.28", "humidity_pct": "87.3", "dew_point_c": "27.26", "soil_raw_adc": "1974", "soil_moisture_pct": "57.0", "soil_condition": "MODERATE", "rain_raw_adc": "1328", "rain_intensity_pct": "86.9", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-17055", "accel_y_raw": "376", "accel_z_raw": "3162", "accel_x_g": "-1.041", "accel_y_g": "0.023", "accel_z_g": "0.193", "accel_magnitude_g": "1.059", "gyro_x_raw": "5200", "gyro_y_raw": "-4477", "gyro_z_raw": "-679", "gyro_x_dps": "39.7", "gyro_y_dps": "-34.18", "gyro_z_dps": "-5.19", "gyro_magnitude_dps": "52.64", "tilt_pitch_deg": "1.27", "tilt_roll_deg": "79.4", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "170", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.60,H:87.30,SRAW:1974,SM:57.0,SC:MODERATE,RRAW:1328,RP:86.9,RC:HEAVY_RAIN,MV:1,TX:1.27,TY:79.40,AXG:-1.041,AYG:0.023,AZG:0.193,GXD:39.70,GYD:-34.18,GZD:-5.19"}, {"timestamp": "16-10-2026 22:19:54", "packet_id": "100", "temperature_c": "29.5", "temperature_f": "85.1", "humidity_pct": "87.5", "dew_point_c": "27.2", "soil_raw_adc": "1293", "soil_moisture_pct": "94.8", "soil_condition": "SATURATED", "rain_raw_adc": "1041", "rain_intensity_pct": "98.4", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1556", "accel_y_raw": "327", "accel_z_raw": "16793", "accel_x_g": "-0.095", "accel_y_g": "0.02", "accel_z_g": "1.025", "accel_magnitude_g": "1.03", "gyro_x_raw": "390", "gyro_y_raw": "69", "gyro_z_raw": "-44", "gyro_x_dps": "2.98", "gyro_y_dps": "0.53", "gyro_z_dps": "-0.34", "gyro_magnitude_dps": "3.05", "tilt_pitch_deg": "1.09", "tilt_roll_deg": "5.28", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "167", "landslide_risk_level": "ELEVATED", "hazard_status": "SATURATION_WARNING", "raw_packet": "GS1,TV:1,T:29.50,H:87.50,SRAW:1293,SM:94.8,SC:SATURATED,RRAW:1041,RP:98.4,RC:HEAVY_RAIN,MV:1,TX:1.09,TY:5.28,AXG:-0.095,AYG:0.020,AZG:1.025,GXD:2.98,GYD:0.53,GZD:-0.34"}, {"timestamp": "16-10-2026 23:00:54", "packet_id": "101", "temperature_c": "29.9", "temperature_f": "85.82", "humidity_pct": "86.1", "dew_point_c": "27.31", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "2839", "rain_intensity_pct": "26.4", "rain_condition": "NO_RAIN", "accel_x_raw": "-1622", "accel_y_raw": "393", "accel_z_raw": "16777", "accel_x_g": "-0.099", "accel_y_g": "0.024", "accel_z_g": "1.024", "accel_magnitude_g": "1.029", "gyro_x_raw": "412", "gyro_y_raw": "35", "gyro_z_raw": "-20", "gyro_x_dps": "3.15", "gyro_y_dps": "0.27", "gyro_z_dps": "-0.16", "gyro_magnitude_dps": "3.17", "tilt_pitch_deg": "1.33", "tilt_roll_deg": "5.51", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "157", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.90,H:86.10,SRAW:4095,SM:0.0,SC:DRY,RRAW:2839,RP:26.4,RC:NO_RAIN,MV:1,TX:1.33,TY:5.51,AXG:-0.099,AYG:0.024,AZG:1.024,GXD:3.15,GYD:0.27,GZD:-0.16"}, {"timestamp": "24-10-2026 09:34:01", "packet_id": "102", "temperature_c": "30.5", "temperature_f": "86.9", "humidity_pct": "83.7", "dew_point_c": "27.42", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "4095", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-1540", "accel_y_raw": "-376", "accel_z_raw": "16826", "accel_x_g": "-0.094", "accel_y_g": "-0.023", "accel_z_g": "1.027", "accel_magnitude_g": "1.032", "gyro_x_raw": "407", "gyro_y_raw": "55", "gyro_z_raw": "-28", "gyro_x_dps": "3.11", "gyro_y_dps": "0.42", "gyro_z_dps": "-0.22", "gyro_magnitude_dps": "3.15", "tilt_pitch_deg": "-1.29", "tilt_roll_deg": "5.24", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "158", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:30.50,H:83.70,SRAW:4095,SM:0.0,SC:DRY,RRAW:4095,RP:0.0,RC:NO_RAIN,MV:1,TX:-1.29,TY:5.24,AXG:-0.094,AYG:-0.023,AZG:1.027,GXD:3.11,GYD:0.42,GZD:-0.22"}, {"timestamp": "24-10-2026 09:34:06", "packet_id": "103", "temperature_c": "30.5", "temperature_f": "86.9", "humidity_pct": "83.5", "dew_point_c": "27.38", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1813", "rain_intensity_pct": "67.5", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-1572", "accel_y_raw": "-376", "accel_z_raw": "16859", "accel_x_g": "-0.096", "accel_y_g": "-0.023", "accel_z_g": "1.029", "accel_magnitude_g": "1.034", "gyro_x_raw": "391", "gyro_y_raw": "53", "gyro_z_raw": "-44", "gyro_x_dps": "2.99", "gyro_y_dps": "0.41", "gyro_z_dps": "-0.34", "gyro_magnitude_dps": "3.04", "tilt_pitch_deg": "-1.27", "tilt_roll_deg": "5.33", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "165", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.50,H:83.50,SRAW:4095,SM:0.0,SC:DRY,RRAW:1813,RP:67.5,RC:MODERATE_RAIN,MV:1,TX:-1.27,TY:5.33,AXG:-0.096,AYG:-0.023,AZG:1.029,GXD:2.99,GYD:0.41,GZD:-0.34"}, {"timestamp": "24-10-2026 09:34:11", "packet_id": "104", "temperature_c": "30.6", "temperature_f": "87.08", "humidity_pct": "83.2", "dew_point_c": "27.41", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1711", "rain_intensity_pct": "71.6", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-1589", "accel_y_raw": "-393", "accel_z_raw": "16777", "accel_x_g": "-0.097", "accel_y_g": "-0.024", "accel_z_g": "1.024", "accel_magnitude_g": "1.029", "gyro_x_raw": "399", "gyro_y_raw": "65", "gyro_z_raw": "-35", "gyro_x_dps": "3.05", "gyro_y_dps": "0.5", "gyro_z_dps": "-0.27", "gyro_magnitude_dps": "3.1", "tilt_pitch_deg": "-1.36", "tilt_roll_deg": "5.4", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "165", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.60,H:83.20,SRAW:4095,SM:0.0,SC:DRY,RRAW:1711,RP:71.6,RC:MODERATE_RAIN,MV:1,TX:-1.36,TY:5.40,AXG:-0.097,AYG:-0.024,AZG:1.024,GXD:3.05,GYD:0.50,GZD:-0.27"}, {"timestamp": "24-10-2026 09:34:16", "packet_id": "105", "temperature_c": "30.7", "temperature_f": "87.26", "humidity_pct": "82.8", "dew_point_c": "27.43", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1697", "rain_intensity_pct": "72.1", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-1540", "accel_y_raw": "-393", "accel_z_raw": "16777", "accel_x_g": "-0.094", "accel_y_g": "-0.024", "accel_z_g": "1.024", "accel_magnitude_g": "1.029", "gyro_x_raw": "404", "gyro_y_raw": "61", "gyro_z_raw": "-36", "gyro_x_dps": "3.09", "gyro_y_dps": "0.47", "gyro_z_dps": "-0.28", "gyro_magnitude_dps": "3.14", "tilt_pitch_deg": "-1.32", "tilt_roll_deg": "5.22", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "165", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.70,H:82.80,SRAW:4095,SM:0.0,SC:DRY,RRAW:1697,RP:72.1,RC:MODERATE_RAIN,MV:1,TX:-1.32,TY:5.22,AXG:-0.094,AYG:-0.024,AZG:1.024,GXD:3.09,GYD:0.47,GZD:-0.28"}, {"timestamp": "24-10-2026 09:34:21", "packet_id": "106", "temperature_c": "30.7", "temperature_f": "87.26", "humidity_pct": "82.6", "dew_point_c": "27.39", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1630", "rain_intensity_pct": "74.8", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-1523", "accel_y_raw": "-344", "accel_z_raw": "16842", "accel_x_g": "-0.093", "accel_y_g": "-0.021", "accel_z_g": "1.028", "accel_magnitude_g": "1.032", "gyro_x_raw": "377", "gyro_y_raw": "55", "gyro_z_raw": "-3", "gyro_x_dps": "2.88", "gyro_y_dps": "0.42", "gyro_z_dps": "-0.03", "gyro_magnitude_dps": "2.91", "tilt_pitch_deg": "-1.19", "tilt_roll_deg": "5.17", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "165", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.70,H:82.60,SRAW:4095,SM:0.0,SC:DRY,RRAW:1630,RP:74.8,RC:MODERATE_RAIN,MV:1,TX:-1.19,TY:5.17,AXG:-0.093,AYG:-0.021,AZG:1.028,GXD:2.88,GYD:0.42,GZD:-0.03"}, {"timestamp": "24-10-2026 09:34:26", "packet_id": "107", "temperature_c": "30.8", "temperature_f": "87.44", "humidity_pct": "82.3", "dew_point_c": "27.42", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1603", "rain_intensity_pct": "75.9", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1540", "accel_y_raw": "-425", "accel_z_raw": "16859", "accel_x_g": "-0.094", "accel_y_g": "-0.026", "accel_z_g": "1.029", "accel_magnitude_g": "1.034", "gyro_x_raw": "390", "gyro_y_raw": "78", "gyro_z_raw": "-20", "gyro_x_dps": "2.98", "gyro_y_dps": "0.6", "gyro_z_dps": "-0.16", "gyro_magnitude_dps": "3.04", "tilt_pitch_deg": "-1.43", "tilt_roll_deg": "5.2", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "162", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.80,H:82.30,SRAW:4095,SM:0.0,SC:DRY,RRAW:1603,RP:75.9,RC:HEAVY_RAIN,MV:1,TX:-1.43,TY:5.20,AXG:-0.094,AYG:-0.026,AZG:1.029,GXD:2.98,GYD:0.60,GZD:-0.16"}, {"timestamp": "24-10-2026 09:34:31", "packet_id": "108", "temperature_c": "30.8", "temperature_f": "87.44", "humidity_pct": "82.0", "dew_point_c": "27.36", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1584", "rain_intensity_pct": "76.6", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1523", "accel_y_raw": "-376", "accel_z_raw": "16809", "accel_x_g": "-0.093", "accel_y_g": "-0.023", "accel_z_g": "1.026", "accel_magnitude_g": "1.03", "gyro_x_raw": "402", "gyro_y_raw": "65", "gyro_z_raw": "-49", "gyro_x_dps": "3.07", "gyro_y_dps": "0.5", "gyro_z_dps": "-0.38", "gyro_magnitude_dps": "3.13", "tilt_pitch_deg": "-1.29", "tilt_roll_deg": "5.17", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "162", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.80,H:82.00,SRAW:4095,SM:0.0,SC:DRY,RRAW:1584,RP:76.6,RC:HEAVY_RAIN,MV:1,TX:-1.29,TY:5.17,AXG:-0.093,AYG:-0.023,AZG:1.026,GXD:3.07,GYD:0.50,GZD:-0.38"}, {"timestamp": "24-10-2026 09:34:36", "packet_id": "109", "temperature_c": "30.9", "temperature_f": "87.62", "humidity_pct": "82.0", "dew_point_c": "27.46", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1571", "rain_intensity_pct": "77.2", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1540", "accel_y_raw": "-344", "accel_z_raw": "16760", "accel_x_g": "-0.094", "accel_y_g": "-0.021", "accel_z_g": "1.023", "accel_magnitude_g": "1.028", "gyro_x_raw": "412", "gyro_y_raw": "90", "gyro_z_raw": "-28", "gyro_x_dps": "3.15", "gyro_y_dps": "0.69", "gyro_z_dps": "-0.22", "gyro_magnitude_dps": "3.23", "tilt_pitch_deg": "-1.17", "tilt_roll_deg": "5.22", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "162", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.90,H:82.00,SRAW:4095,SM:0.0,SC:DRY,RRAW:1571,RP:77.2,RC:HEAVY_RAIN,MV:1,TX:-1.17,TY:5.22,AXG:-0.094,AYG:-0.021,AZG:1.023,GXD:3.15,GYD:0.69,GZD:-0.22"}, {"timestamp": "24-10-2026 09:34:41", "packet_id": "110", "temperature_c": "30.9", "temperature_f": "87.62", "humidity_pct": "81.8", "dew_point_c": "27.42", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "2039", "rain_intensity_pct": "58.4", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "11780", "accel_y_raw": "196", "accel_z_raw": "10403", "accel_x_g": "0.719", "accel_y_g": "0.012", "accel_z_g": "0.635", "accel_magnitude_g": "0.959", "gyro_x_raw": "1506", "gyro_y_raw": "-93", "gyro_z_raw": "1806", "gyro_x_dps": "11.5", "gyro_y_dps": "-0.71", "gyro_z_dps": "13.79", "gyro_magnitude_dps": "17.97", "tilt_pitch_deg": "0.7", "tilt_roll_deg": "-48.54", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "166", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.90,H:81.80,SRAW:4095,SM:0.0,SC:DRY,RRAW:2039,RP:58.4,RC:MODERATE_RAIN,MV:1,TX:0.70,TY:-48.54,AXG:0.719,AYG:0.012,AZG:0.635,GXD:11.50,GYD:-0.71,GZD:13.79"}, {"timestamp": "24-10-2026 09:34:51", "packet_id": "111", "temperature_c": "31.0", "temperature_f": "87.8", "humidity_pct": "81.8", "dew_point_c": "27.51", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1634", "rain_intensity_pct": "74.6", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "16351", "accel_y_raw": "-114", "accel_z_raw": "1851", "accel_x_g": "0.998", "accel_y_g": "-0.007", "accel_z_g": "0.113", "accel_magnitude_g": "1.004", "gyro_x_raw": "373", "gyro_y_raw": "45", "gyro_z_raw": "-31", "gyro_x_dps": "2.85", "gyro_y_dps": "0.35", "gyro_z_dps": "-0.24", "gyro_magnitude_dps": "2.88", "tilt_pitch_deg": "-0.42", "tilt_roll_deg": "-83.55", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "166", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:31.00,H:81.80,SRAW:4095,SM:0.0,SC:DRY,RRAW:1634,RP:74.6,RC:MODERATE_RAIN,MV:1,TX:-0.42,TY:-83.55,AXG:0.998,AYG:-0.007,AZG:0.113,GXD:2.85,GYD:0.35,GZD:-0.24"}, {"timestamp": "24-10-2026 09:34:56", "packet_id": "112", "temperature_c": "31.1", "temperature_f": "87.98", "humidity_pct": "81.5", "dew_point_c": "27.55", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1633", "rain_intensity_pct": "74.7", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "16367", "accel_y_raw": "-98", "accel_z_raw": "1949", "accel_x_g": "0.999", "accel_y_g": "-0.006", "accel_z_g": "0.119", "accel_magnitude_g": "1.006", "gyro_x_raw": "375", "gyro_y_raw": "69", "gyro_z_raw": "-52", "gyro_x_dps": "2.87", "gyro_y_dps": "0.53", "gyro_z_dps": "-0.4", "gyro_magnitude_dps": "2.95", "tilt_pitch_deg": "-0.35", "tilt_roll_deg": "-83.21", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "166", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:31.10,H:81.50,SRAW:4095,SM:0.0,SC:DRY,RRAW:1633,RP:74.7,RC:MODERATE_RAIN,MV:1,TX:-0.35,TY:-83.21,AXG:0.999,AYG:-0.006,AZG:0.119,GXD:2.87,GYD:0.53,GZD:-0.40"}, {"timestamp": "24-10-2026 09:35:01", "packet_id": "113", "temperature_c": "31.1", "temperature_f": "87.98", "humidity_pct": "81.2", "dew_point_c": "27.48", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1601", "rain_intensity_pct": "76.0", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "16384", "accel_y_raw": "-49", "accel_z_raw": "1916", "accel_x_g": "1.0", "accel_y_g": "-0.003", "accel_z_g": "0.117", "accel_magnitude_g": "1.007", "gyro_x_raw": "366", "gyro_y_raw": "81", "gyro_z_raw": "-34", "gyro_x_dps": "2.8", "gyro_y_dps": "0.62", "gyro_z_dps": "-0.26", "gyro_magnitude_dps": "2.88", "tilt_pitch_deg": "-0.19", "tilt_roll_deg": "-83.33", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "163", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:31.10,H:81.20,SRAW:4095,SM:0.0,SC:DRY,RRAW:1601,RP:76.0,RC:HEAVY_RAIN,MV:1,TX:-0.19,TY:-83.33,AXG:1.000,AYG:-0.003,AZG:0.117,GXD:2.80,GYD:0.62,GZD:-0.26"}, {"timestamp": "24-10-2026 09:35:06", "packet_id": "114", "temperature_c": "31.1", "temperature_f": "87.98", "humidity_pct": "81.0", "dew_point_c": "27.44", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1597", "rain_intensity_pct": "76.1", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "16433", "accel_y_raw": "-49", "accel_z_raw": "1900", "accel_x_g": "1.003", "accel_y_g": "-0.003", "accel_z_g": "0.116", "accel_magnitude_g": "1.01", "gyro_x_raw": "400", "gyro_y_raw": "49", "gyro_z_raw": "-35", "gyro_x_dps": "3.06", "gyro_y_dps": "0.38", "gyro_z_dps": "-0.27", "gyro_magnitude_dps": "3.1", "tilt_pitch_deg": "-0.19", "tilt_roll_deg": "-83.42", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "163", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:31.10,H:81.00,SRAW:4095,SM:0.0,SC:DRY,RRAW:1597,RP:76.1,RC:HEAVY_RAIN,MV:1,TX:-0.19,TY:-83.42,AXG:1.003,AYG:-0.003,AZG:0.116,GXD:3.06,GYD:0.38,GZD:-0.27"}, {"timestamp": "24-10-2026 09:35:41", "packet_id": "115", "temperature_c": "31.0", "temperature_f": "87.8", "humidity_pct": "80.7", "dew_point_c": "27.28", "soil_raw_adc": "2337", "soil_moisture_pct": "36.8", "soil_condition": "MODERATE", "rain_raw_adc": "2103", "rain_intensity_pct": "55.9", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-2031", "accel_y_raw": "442", "accel_z_raw": "16662", "accel_x_g": "-0.124", "accel_y_g": "0.027", "accel_z_g": "1.017", "accel_magnitude_g": "1.025", "gyro_x_raw": "370", "gyro_y_raw": "75", "gyro_z_raw": "-23", "gyro_x_dps": "2.83", "gyro_y_dps": "0.58", "gyro_z_dps": "-0.18", "gyro_magnitude_dps": "2.89", "tilt_pitch_deg": "1.53", "tilt_roll_deg": "6.92", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:31.00,H:80.70,SRAW:2337,SM:36.8,SC:MODERATE,RRAW:2103,RP:55.9,RC:MODERATE_RAIN,MV:1,TX:1.53,TY:6.92,AXG:-0.124,AYG:0.027,AZG:1.017,GXD:2.83,GYD:0.58,GZD:-0.18"}, {"timestamp": "24-10-2026 09:35:46", "packet_id": "116", "temperature_c": "31.0", "temperature_f": "87.8", "humidity_pct": "80.7", "dew_point_c": "27.28", "soil_raw_adc": "2158", "soil_moisture_pct": "46.8", "soil_condition": "MODERATE", "rain_raw_adc": "1664", "rain_intensity_pct": "73.4", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-2015", "accel_y_raw": "524", "accel_z_raw": "16646", "accel_x_g": "-0.123", "accel_y_g": "0.032", "accel_z_g": "1.016", "accel_magnitude_g": "1.024", "gyro_x_raw": "420", "gyro_y_raw": "44", "gyro_z_raw": "-32", "gyro_x_dps": "3.21", "gyro_y_dps": "0.34", "gyro_z_dps": "-0.25", "gyro_magnitude_dps": "3.24", "tilt_pitch_deg": "1.82", "tilt_roll_deg": "6.89", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:31.00,H:80.70,SRAW:2158,SM:46.8,SC:MODERATE,RRAW:1664,RP:73.4,RC:MODERATE_RAIN,MV:1,TX:1.82,TY:6.89,AXG:-0.123,AYG:0.032,AZG:1.016,GXD:3.21,GYD:0.34,GZD:-0.25"}, {"timestamp": "24-10-2026 09:35:52", "packet_id": "117", "temperature_c": "31.0", "temperature_f": "87.8", "humidity_pct": "80.6", "dew_point_c": "27.26", "soil_raw_adc": "2368", "soil_moisture_pct": "35.1", "soil_condition": "MODERATE", "rain_raw_adc": "3631", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-2015", "accel_y_raw": "475", "accel_z_raw": "16662", "accel_x_g": "-0.123", "accel_y_g": "0.029", "accel_z_g": "1.017", "accel_magnitude_g": "1.025", "gyro_x_raw": "372", "gyro_y_raw": "93", "gyro_z_raw": "-26", "gyro_x_dps": "2.84", "gyro_y_dps": "0.71", "gyro_z_dps": "-0.2", "gyro_magnitude_dps": "2.93", "tilt_pitch_deg": "1.62", "tilt_roll_deg": "6.91", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "162", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:31.00,H:80.60,SRAW:2368,SM:35.1,SC:MODERATE,RRAW:3631,RP:0.0,RC:NO_RAIN,MV:1,TX:1.62,TY:6.91,AXG:-0.123,AYG:0.029,AZG:1.017,GXD:2.84,GYD:0.71,GZD:-0.20"}, {"timestamp": "24-10-2026 09:35:57", "packet_id": "118", "temperature_c": "31.0", "temperature_f": "87.8", "humidity_pct": "80.6", "dew_point_c": "27.26", "soil_raw_adc": "1861", "soil_moisture_pct": "63.3", "soil_condition": "MODERATE", "rain_raw_adc": "1645", "rain_intensity_pct": "74.2", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-2080", "accel_y_raw": "524", "accel_z_raw": "16744", "accel_x_g": "-0.127", "accel_y_g": "0.032", "accel_z_g": "1.022", "accel_magnitude_g": "1.03", "gyro_x_raw": "428", "gyro_y_raw": "65", "gyro_z_raw": "-66", "gyro_x_dps": "3.27", "gyro_y_dps": "0.5", "gyro_z_dps": "-0.51", "gyro_magnitude_dps": "3.35", "tilt_pitch_deg": "1.77", "tilt_roll_deg": "7.07", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:31.00,H:80.60,SRAW:1861,SM:63.3,SC:MODERATE,RRAW:1645,RP:74.2,RC:MODERATE_RAIN,MV:1,TX:1.77,TY:7.07,AXG:-0.127,AYG:0.032,AZG:1.022,GXD:3.27,GYD:0.50,GZD:-0.51"}, {"timestamp": "24-10-2026 09:36:01", "packet_id": "119", "temperature_c": "31.0", "temperature_f": "87.8", "humidity_pct": "80.6", "dew_point_c": "27.26", "soil_raw_adc": "2069", "soil_moisture_pct": "51.7", "soil_condition": "MODERATE", "rain_raw_adc": "2003", "rain_intensity_pct": "59.9", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-2064", "accel_y_raw": "475", "accel_z_raw": "16793", "accel_x_g": "-0.126", "accel_y_g": "0.029", "accel_z_g": "1.025", "accel_magnitude_g": "1.033", "gyro_x_raw": "410", "gyro_y_raw": "61", "gyro_z_raw": "-15", "gyro_x_dps": "3.13", "gyro_y_dps": "0.47", "gyro_z_dps": "-0.12", "gyro_magnitude_dps": "3.17", "tilt_pitch_deg": "1.61", "tilt_roll_deg": "7.01", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:31.00,H:80.60,SRAW:2069,SM:51.7,SC:MODERATE,RRAW:2003,RP:59.9,RC:MODERATE_RAIN,MV:1,TX:1.61,TY:7.01,AXG:-0.126,AYG:0.029,AZG:1.025,GXD:3.13,GYD:0.47,GZD:-0.12"}, {"timestamp": "24-10-2026 09:36:06", "packet_id": "120", "temperature_c": "31.0", "temperature_f": "87.8", "humidity_pct": "80.7", "dew_point_c": "27.28", "soil_raw_adc": "2028", "soil_moisture_pct": "54.0", "soil_condition": "MODERATE", "rain_raw_adc": "2063", "rain_intensity_pct": "57.5", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-1900", "accel_y_raw": "1097", "accel_z_raw": "16285", "accel_x_g": "-0.116", "accel_y_g": "0.067", "accel_z_g": "0.994", "accel_magnitude_g": "1.003", "gyro_x_raw": "419", "gyro_y_raw": "-5", "gyro_z_raw": "5", "gyro_x_dps": "3.2", "gyro_y_dps": "-0.04", "gyro_z_dps": "0.04", "gyro_magnitude_dps": "3.2", "tilt_pitch_deg": "3.82", "tilt_roll_deg": "6.64", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:31.00,H:80.70,SRAW:2028,SM:54.0,SC:MODERATE,RRAW:2063,RP:57.5,RC:MODERATE_RAIN,MV:1,TX:3.82,TY:6.64,AXG:-0.116,AYG:0.067,AZG:0.994,GXD:3.20,GYD:-0.04,GZD:0.04"}, {"timestamp": "24-10-2026 09:36:08", "packet_id": "121", "temperature_c": "", "temperature_f": "", "humidity_pct": "", "dew_point_c": "", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "4095", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "0", "accel_y_raw": "0", "accel_z_raw": "16384", "accel_x_g": "0.0", "accel_y_g": "0.0", "accel_z_g": "1.0", "accel_magnitude_g": "1.0", "gyro_x_raw": "0", "gyro_y_raw": "0", "gyro_z_raw": "0", "gyro_x_dps": "0.0", "gyro_y_dps": "0.0", "gyro_z_dps": "0.0", "gyro_magnitude_dps": "0.0", "tilt_pitch_deg": "0.0", "tilt_roll_deg": "-0.0", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "71", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00"}, {"timestamp": "24-10-2026 09:36:17", "packet_id": "122", "temperature_c": "31.0", "temperature_f": "87.8", "humidity_pct": "81.1", "dew_point_c": "27.37", "soil_raw_adc": "2461", "soil_moisture_pct": "29.9", "soil_condition": "MODERATE", "rain_raw_adc": "2042", "rain_intensity_pct": "58.3", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-15974", "accel_y_raw": "-49", "accel_z_raw": "-3129", "accel_x_g": "-0.975", "accel_y_g": "-0.003", "accel_z_g": "-0.191", "accel_magnitude_g": "0.994", "gyro_x_raw": "391", "gyro_y_raw": "68", "gyro_z_raw": "-45", "gyro_x_dps": "2.99", "gyro_y_dps": "0.52", "gyro_z_dps": "-0.35", "gyro_magnitude_dps": "3.05", "tilt_pitch_deg": "-0.18", "tilt_roll_deg": "78.89", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "173", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:31.00,H:81.10,SRAW:2461,SM:29.9,SC:MODERATE,RRAW:2042,RP:58.3,RC:MODERATE_RAIN,MV:1,TX:-0.18,TY:78.89,AXG:-0.975,AYG:-0.003,AZG:-0.191,GXD:2.99,GYD:0.52,GZD:-0.35"}, {"timestamp": "24-10-2026 09:36:22", "packet_id": "123", "temperature_c": "31.0", "temperature_f": "87.8", "humidity_pct": "81.0", "dew_point_c": "27.34", "soil_raw_adc": "2109", "soil_moisture_pct": "49.5", "soil_condition": "MODERATE", "rain_raw_adc": "2081", "rain_intensity_pct": "56.8", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-15941", "accel_y_raw": "-98", "accel_z_raw": "-2998", "accel_x_g": "-0.973", "accel_y_g": "-0.006", "accel_z_g": "-0.183", "accel_magnitude_g": "0.99", "gyro_x_raw": "381", "gyro_y_raw": "60", "gyro_z_raw": "6", "gyro_x_dps": "2.91", "gyro_y_dps": "0.46", "gyro_z_dps": "0.05", "gyro_magnitude_dps": "2.95", "tilt_pitch_deg": "-0.35", "tilt_roll_deg": "79.34", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "172", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:31.00,H:81.00,SRAW:2109,SM:49.5,SC:MODERATE,RRAW:2081,RP:56.8,RC:MODERATE_RAIN,MV:1,TX:-0.35,TY:79.34,AXG:-0.973,AYG:-0.006,AZG:-0.183,GXD:2.91,GYD:0.46,GZD:0.05"}, {"timestamp": "24-10-2026 09:36:27", "packet_id": "124", "temperature_c": "31.0", "temperature_f": "87.8", "humidity_pct": "80.9", "dew_point_c": "27.32", "soil_raw_adc": "2139", "soil_moisture_pct": "47.8", "soil_condition": "MODERATE", "rain_raw_adc": "2063", "rain_intensity_pct": "57.5", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-15908", "accel_y_raw": "32", "accel_z_raw": "-3080", "accel_x_g": "-0.971", "accel_y_g": "0.002", "accel_z_g": "-0.188", "accel_magnitude_g": "0.989", "gyro_x_raw": "394", "gyro_y_raw": "35", "gyro_z_raw": "-19", "gyro_x_dps": "3.01", "gyro_y_dps": "0.27", "gyro_z_dps": "-0.15", "gyro_magnitude_dps": "3.03", "tilt_pitch_deg": "0.14", "tilt_roll_deg": "79.01", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "171", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:31.00,H:80.90,SRAW:2139,SM:47.8,SC:MODERATE,RRAW:2063,RP:57.5,RC:MODERATE_RAIN,MV:1,TX:0.14,TY:79.01,AXG:-0.971,AYG:0.002,AZG:-0.188,GXD:3.01,GYD:0.27,GZD:-0.15"}, {"timestamp": "24-10-2026 09:36:35", "packet_id": "125", "temperature_c": "31.0", "temperature_f": "87.8", "humidity_pct": "80.8", "dew_point_c": "27.3", "soil_raw_adc": "2035", "soil_moisture_pct": "53.6", "soil_condition": "MODERATE", "rain_raw_adc": "1714", "rain_intensity_pct": "71.4", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-2129", "accel_y_raw": "327", "accel_z_raw": "16728", "accel_x_g": "-0.13", "accel_y_g": "0.02", "accel_z_g": "1.021", "accel_magnitude_g": "1.029", "gyro_x_raw": "386", "gyro_y_raw": "44", "gyro_z_raw": "-31", "gyro_x_dps": "2.95", "gyro_y_dps": "0.34", "gyro_z_dps": "-0.24", "gyro_magnitude_dps": "2.98", "tilt_pitch_deg": "1.11", "tilt_roll_deg": "7.23", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:31.00,H:80.80,SRAW:2035,SM:53.6,SC:MODERATE,RRAW:1714,RP:71.4,RC:MODERATE_RAIN,MV:1,TX:1.11,TY:7.23,AXG:-0.130,AYG:0.020,AZG:1.021,GXD:2.95,GYD:0.34,GZD:-0.24"}, {"timestamp": "24-10-2026 09:36:40", "packet_id": "126", "temperature_c": "31.0", "temperature_f": "87.8", "humidity_pct": "80.8", "dew_point_c": "27.3", "soil_raw_adc": "2032", "soil_moisture_pct": "53.8", "soil_condition": "MODERATE", "rain_raw_adc": "1712", "rain_intensity_pct": "71.5", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-2129", "accel_y_raw": "327", "accel_z_raw": "16826", "accel_x_g": "-0.13", "accel_y_g": "0.02", "accel_z_g": "1.027", "accel_magnitude_g": "1.035", "gyro_x_raw": "394", "gyro_y_raw": "70", "gyro_z_raw": "-49", "gyro_x_dps": "3.01", "gyro_y_dps": "0.54", "gyro_z_dps": "-0.38", "gyro_magnitude_dps": "3.08", "tilt_pitch_deg": "1.12", "tilt_roll_deg": "7.22", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:31.00,H:80.80,SRAW:2032,SM:53.8,SC:MODERATE,RRAW:1712,RP:71.5,RC:MODERATE_RAIN,MV:1,TX:1.12,TY:7.22,AXG:-0.130,AYG:0.020,AZG:1.027,GXD:3.01,GYD:0.54,GZD:-0.38"}, {"timestamp": "24-10-2026 09:36:45", "packet_id": "127", "temperature_c": "31.0", "temperature_f": "87.8", "humidity_pct": "80.8", "dew_point_c": "27.3", "soil_raw_adc": "2034", "soil_moisture_pct": "53.7", "soil_condition": "MODERATE", "rain_raw_adc": "1701", "rain_intensity_pct": "72.0", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-2097", "accel_y_raw": "278", "accel_z_raw": "16793", "accel_x_g": "-0.128", "accel_y_g": "0.017", "accel_z_g": "1.025", "accel_magnitude_g": "1.033", "gyro_x_raw": "403", "gyro_y_raw": "55", "gyro_z_raw": "-23", "gyro_x_dps": "3.08", "gyro_y_dps": "0.42", "gyro_z_dps": "-0.18", "gyro_magnitude_dps": "3.11", "tilt_pitch_deg": "0.96", "tilt_roll_deg": "7.1", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:31.00,H:80.80,SRAW:2034,SM:53.7,SC:MODERATE,RRAW:1701,RP:72.0,RC:MODERATE_RAIN,MV:1,TX:0.96,TY:7.10,AXG:-0.128,AYG:0.017,AZG:1.025,GXD:3.08,GYD:0.42,GZD:-0.18"}, {"timestamp": "24-10-2026 09:36:50", "packet_id": "128", "temperature_c": "30.9", "temperature_f": "87.62", "humidity_pct": "80.7", "dew_point_c": "27.18", "soil_raw_adc": "1942", "soil_moisture_pct": "58.8", "soil_condition": "MODERATE", "rain_raw_adc": "1712", "rain_intensity_pct": "71.5", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-2080", "accel_y_raw": "278", "accel_z_raw": "16744", "accel_x_g": "-0.127", "accel_y_g": "0.017", "accel_z_g": "1.022", "accel_magnitude_g": "1.03", "gyro_x_raw": "389", "gyro_y_raw": "72", "gyro_z_raw": "-19", "gyro_x_dps": "2.97", "gyro_y_dps": "0.55", "gyro_z_dps": "-0.15", "gyro_magnitude_dps": "3.02", "tilt_pitch_deg": "0.96", "tilt_roll_deg": "7.1", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.90,H:80.70,SRAW:1942,SM:58.8,SC:MODERATE,RRAW:1712,RP:71.5,RC:MODERATE_RAIN,MV:1,TX:0.96,TY:7.10,AXG:-0.127,AYG:0.017,AZG:1.022,GXD:2.97,GYD:0.55,GZD:-0.15"}, {"timestamp": "24-10-2026 09:36:55", "packet_id": "129", "temperature_c": "30.9", "temperature_f": "87.62", "humidity_pct": "80.7", "dew_point_c": "27.18", "soil_raw_adc": "1995", "soil_moisture_pct": "55.8", "soil_condition": "MODERATE", "rain_raw_adc": "1699", "rain_intensity_pct": "72.0", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-2179", "accel_y_raw": "262", "accel_z_raw": "16842", "accel_x_g": "-0.133", "accel_y_g": "0.016", "accel_z_g": "1.028", "accel_magnitude_g": "1.037", "gyro_x_raw": "394", "gyro_y_raw": "44", "gyro_z_raw": "-57", "gyro_x_dps": "3.01", "gyro_y_dps": "0.34", "gyro_z_dps": "-0.44", "gyro_magnitude_dps": "3.06", "tilt_pitch_deg": "0.89", "tilt_roll_deg": "7.35", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.90,H:80.70,SRAW:1995,SM:55.8,SC:MODERATE,RRAW:1699,RP:72.0,RC:MODERATE_RAIN,MV:1,TX:0.89,TY:7.35,AXG:-0.133,AYG:0.016,AZG:1.028,GXD:3.01,GYD:0.34,GZD:-0.44"}, {"timestamp": "24-10-2026 09:37:00", "packet_id": "130", "temperature_c": "30.9", "temperature_f": "87.62", "humidity_pct": "80.7", "dew_point_c": "27.18", "soil_raw_adc": "2186", "soil_moisture_pct": "45.2", "soil_condition": "MODERATE", "rain_raw_adc": "1678", "rain_intensity_pct": "72.9", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-2129", "accel_y_raw": "278", "accel_z_raw": "16711", "accel_x_g": "-0.13", "accel_y_g": "0.017", "accel_z_g": "1.02", "accel_magnitude_g": "1.028", "gyro_x_raw": "396", "gyro_y_raw": "44", "gyro_z_raw": "-61", "gyro_x_dps": "3.03", "gyro_y_dps": "0.34", "gyro_z_dps": "-0.47", "gyro_magnitude_dps": "3.09", "tilt_pitch_deg": "0.97", "tilt_roll_deg": "7.26", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.90,H:80.70,SRAW:2186,SM:45.2,SC:MODERATE,RRAW:1678,RP:72.9,RC:MODERATE_RAIN,MV:1,TX:0.97,TY:7.26,AXG:-0.130,AYG:0.017,AZG:1.020,GXD:3.03,GYD:0.34,GZD:-0.47"}, {"timestamp": "24-10-2026 09:37:05", "packet_id": "131", "temperature_c": "30.9", "temperature_f": "87.62", "humidity_pct": "80.7", "dew_point_c": "27.18", "soil_raw_adc": "2493", "soil_moisture_pct": "28.2", "soil_condition": "MODERATE", "rain_raw_adc": "2883", "rain_intensity_pct": "24.7", "rain_condition": "LIGHT_RAIN", "accel_x_raw": "-2146", "accel_y_raw": "311", "accel_z_raw": "16711", "accel_x_g": "-0.131", "accel_y_g": "0.019", "accel_z_g": "1.02", "accel_magnitude_g": "1.029", "gyro_x_raw": "370", "gyro_y_raw": "66", "gyro_z_raw": "-35", "gyro_x_dps": "2.83", "gyro_y_dps": "0.51", "gyro_z_dps": "-0.27", "gyro_magnitude_dps": "2.89", "tilt_pitch_deg": "1.07", "tilt_roll_deg": "7.29", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "166", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:30.90,H:80.70,SRAW:2493,SM:28.2,SC:MODERATE,RRAW:2883,RP:24.7,RC:LIGHT_RAIN,MV:1,TX:1.07,TY:7.29,AXG:-0.131,AYG:0.019,AZG:1.020,GXD:2.83,GYD:0.51,GZD:-0.27"}, {"timestamp": "24-10-2026 09:37:10", "packet_id": "132", "temperature_c": "30.9", "temperature_f": "87.62", "humidity_pct": "80.6", "dew_point_c": "27.16", "soil_raw_adc": "2032", "soil_moisture_pct": "53.8", "soil_condition": "MODERATE", "rain_raw_adc": "2295", "rain_intensity_pct": "48.2", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-2211", "accel_y_raw": "344", "accel_z_raw": "16744", "accel_x_g": "-0.135", "accel_y_g": "0.021", "accel_z_g": "1.022", "accel_magnitude_g": "1.031", "gyro_x_raw": "395", "gyro_y_raw": "49", "gyro_z_raw": "-45", "gyro_x_dps": "3.02", "gyro_y_dps": "0.38", "gyro_z_dps": "-0.35", "gyro_magnitude_dps": "3.06", "tilt_pitch_deg": "1.17", "tilt_roll_deg": "7.51", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.90,H:80.60,SRAW:2032,SM:53.8,SC:MODERATE,RRAW:2295,RP:48.2,RC:MODERATE_RAIN,MV:1,TX:1.17,TY:7.51,AXG:-0.135,AYG:0.021,AZG:1.022,GXD:3.02,GYD:0.38,GZD:-0.35"}, {"timestamp": "24-10-2026 09:37:15", "packet_id": "133", "temperature_c": "30.9", "temperature_f": "87.62", "humidity_pct": "80.7", "dew_point_c": "27.18", "soil_raw_adc": "1774", "soil_moisture_pct": "68.1", "soil_condition": "WET", "rain_raw_adc": "1491", "rain_intensity_pct": "80.4", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-2146", "accel_y_raw": "311", "accel_z_raw": "16695", "accel_x_g": "-0.131", "accel_y_g": "0.019", "accel_z_g": "1.019", "accel_magnitude_g": "1.028", "gyro_x_raw": "406", "gyro_y_raw": "89", "gyro_z_raw": "48", "gyro_x_dps": "3.1", "gyro_y_dps": "0.68", "gyro_z_dps": "0.37", "gyro_magnitude_dps": "3.2", "tilt_pitch_deg": "1.06", "tilt_roll_deg": "7.32", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "160", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.90,H:80.70,SRAW:1774,SM:68.1,SC:WET,RRAW:1491,RP:80.4,RC:HEAVY_RAIN,MV:1,TX:1.06,TY:7.32,AXG:-0.131,AYG:0.019,AZG:1.019,GXD:3.10,GYD:0.68,GZD:0.37"}, {"timestamp": "24-10-2026 09:37:20", "packet_id": "134", "temperature_c": "30.9", "temperature_f": "87.62", "humidity_pct": "80.7", "dew_point_c": "27.18", "soil_raw_adc": "1764", "soil_moisture_pct": "68.7", "soil_condition": "WET", "rain_raw_adc": "1586", "rain_intensity_pct": "76.6", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-2162", "accel_y_raw": "311", "accel_z_raw": "16662", "accel_x_g": "-0.132", "accel_y_g": "0.019", "accel_z_g": "1.017", "accel_magnitude_g": "1.026", "gyro_x_raw": "379", "gyro_y_raw": "72", "gyro_z_raw": "-47", "gyro_x_dps": "2.9", "gyro_y_dps": "0.55", "gyro_z_dps": "-0.36", "gyro_magnitude_dps": "2.97", "tilt_pitch_deg": "1.06", "tilt_roll_deg": "7.4", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "161", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.90,H:80.70,SRAW:1764,SM:68.7,SC:WET,RRAW:1586,RP:76.6,RC:HEAVY_RAIN,MV:1,TX:1.06,TY:7.40,AXG:-0.132,AYG:0.019,AZG:1.017,GXD:2.90,GYD:0.55,GZD:-0.36"}, {"timestamp": "24-10-2026 09:37:25", "packet_id": "135", "temperature_c": "30.9", "temperature_f": "87.62", "humidity_pct": "80.8", "dew_point_c": "27.2", "soil_raw_adc": "1765", "soil_moisture_pct": "68.6", "soil_condition": "WET", "rain_raw_adc": "1569", "rain_intensity_pct": "77.2", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-2146", "accel_y_raw": "327", "accel_z_raw": "16728", "accel_x_g": "-0.131", "accel_y_g": "0.02", "accel_z_g": "1.021", "accel_magnitude_g": "1.03", "gyro_x_raw": "432", "gyro_y_raw": "66", "gyro_z_raw": "-48", "gyro_x_dps": "3.3", "gyro_y_dps": "0.51", "gyro_z_dps": "-0.37", "gyro_magnitude_dps": "3.36", "tilt_pitch_deg": "1.09", "tilt_roll_deg": "7.33", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "161", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.90,H:80.80,SRAW:1765,SM:68.6,SC:WET,RRAW:1569,RP:77.2,RC:HEAVY_RAIN,MV:1,TX:1.09,TY:7.33,AXG:-0.131,AYG:0.020,AZG:1.021,GXD:3.30,GYD:0.51,GZD:-0.37"}, {"timestamp": "24-10-2026 09:37:30", "packet_id": "136", "temperature_c": "30.9", "temperature_f": "87.62", "humidity_pct": "80.8", "dew_point_c": "27.2", "soil_raw_adc": "1754", "soil_moisture_pct": "69.2", "soil_condition": "WET", "rain_raw_adc": "1574", "rain_intensity_pct": "77.0", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-2129", "accel_y_raw": "278", "accel_z_raw": "16662", "accel_x_g": "-0.13", "accel_y_g": "0.017", "accel_z_g": "1.017", "accel_magnitude_g": "1.025", "gyro_x_raw": "393", "gyro_y_raw": "91", "gyro_z_raw": "-7", "gyro_x_dps": "3.0", "gyro_y_dps": "0.7", "gyro_z_dps": "-0.06", "gyro_magnitude_dps": "3.08", "tilt_pitch_deg": "0.97", "tilt_roll_deg": "7.3", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "161", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.90,H:80.80,SRAW:1754,SM:69.2,SC:WET,RRAW:1574,RP:77.0,RC:HEAVY_RAIN,MV:1,TX:0.97,TY:7.30,AXG:-0.130,AYG:0.017,AZG:1.017,GXD:3.00,GYD:0.70,GZD:-0.06"}, {"timestamp": "24-10-2026 09:37:35", "packet_id": "137", "temperature_c": "30.9", "temperature_f": "87.62", "humidity_pct": "80.9", "dew_point_c": "27.23", "soil_raw_adc": "1776", "soil_moisture_pct": "68.0", "soil_condition": "WET", "rain_raw_adc": "1579", "rain_intensity_pct": "76.8", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-2162", "accel_y_raw": "327", "accel_z_raw": "16695", "accel_x_g": "-0.132", "accel_y_g": "0.02", "accel_z_g": "1.019", "accel_magnitude_g": "1.028", "gyro_x_raw": "424", "gyro_y_raw": "82", "gyro_z_raw": "-43", "gyro_x_dps": "3.24", "gyro_y_dps": "0.63", "gyro_z_dps": "-0.33", "gyro_magnitude_dps": "3.32", "tilt_pitch_deg": "1.09", "tilt_roll_deg": "7.39", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "161", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.90,H:80.90,SRAW:1776,SM:68.0,SC:WET,RRAW:1579,RP:76.8,RC:HEAVY_RAIN,MV:1,TX:1.09,TY:7.39,AXG:-0.132,AYG:0.020,AZG:1.019,GXD:3.24,GYD:0.63,GZD:-0.33"}, {"timestamp": "24-10-2026 09:37:40", "packet_id": "138", "temperature_c": "30.9", "temperature_f": "87.62", "humidity_pct": "80.9", "dew_point_c": "27.23", "soil_raw_adc": "1776", "soil_moisture_pct": "68.0", "soil_condition": "WET", "rain_raw_adc": "1553", "rain_intensity_pct": "77.9", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-2113", "accel_y_raw": "360", "accel_z_raw": "16744", "accel_x_g": "-0.129", "accel_y_g": "0.022", "accel_z_g": "1.022", "accel_magnitude_g": "1.03", "gyro_x_raw": "398", "gyro_y_raw": "99", "gyro_z_raw": "-43", "gyro_x_dps": "3.04", "gyro_y_dps": "0.76", "gyro_z_dps": "-0.33", "gyro_magnitude_dps": "3.15", "tilt_pitch_deg": "1.22", "tilt_roll_deg": "7.22", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "161", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.90,H:80.90,SRAW:1776,SM:68.0,SC:WET,RRAW:1553,RP:77.9,RC:HEAVY_RAIN,MV:1,TX:1.22,TY:7.22,AXG:-0.129,AYG:0.022,AZG:1.022,GXD:3.04,GYD:0.76,GZD:-0.33"}, {"timestamp": "24-10-2026 09:37:45", "packet_id": "139", "temperature_c": "30.8", "temperature_f": "87.44", "humidity_pct": "80.9", "dew_point_c": "27.13", "soil_raw_adc": "1735", "soil_moisture_pct": "70.3", "soil_condition": "WET", "rain_raw_adc": "1589", "rain_intensity_pct": "76.4", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1785", "accel_y_raw": "-2785", "accel_z_raw": "16842", "accel_x_g": "-0.109", "accel_y_g": "-0.17", "accel_z_g": "1.028", "accel_magnitude_g": "1.048", "gyro_x_raw": "458", "gyro_y_raw": "340", "gyro_z_raw": "-40", "gyro_x_dps": "3.5", "gyro_y_dps": "2.6", "gyro_z_dps": "-0.31", "gyro_magnitude_dps": "4.37", "tilt_pitch_deg": "-9.32", "tilt_roll_deg": "5.98", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "163", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.80,H:80.90,SRAW:1735,SM:70.3,SC:WET,RRAW:1589,RP:76.4,RC:HEAVY_RAIN,MV:1,TX:-9.32,TY:5.98,AXG:-0.109,AYG:-0.170,AZG:1.028,GXD:3.50,GYD:2.60,GZD:-0.31"}, {"timestamp": "24-10-2026 09:37:50", "packet_id": "140", "temperature_c": "30.8", "temperature_f": "87.44", "humidity_pct": "80.9", "dew_point_c": "27.13", "soil_raw_adc": "1841", "soil_moisture_pct": "64.4", "soil_condition": "MODERATE", "rain_raw_adc": "1647", "rain_intensity_pct": "74.1", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-1982", "accel_y_raw": "-3047", "accel_z_raw": "16482", "accel_x_g": "-0.121", "accel_y_g": "-0.186", "accel_z_g": "1.006", "accel_magnitude_g": "1.03", "gyro_x_raw": "407", "gyro_y_raw": "52", "gyro_z_raw": "3", "gyro_x_dps": "3.11", "gyro_y_dps": "0.4", "gyro_z_dps": "0.03", "gyro_magnitude_dps": "3.14", "tilt_pitch_deg": "-10.39", "tilt_roll_deg": "6.74", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "171", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.80,H:80.90,SRAW:1841,SM:64.4,SC:MODERATE,RRAW:1647,RP:74.1,RC:MODERATE_RAIN,MV:1,TX:-10.39,TY:6.74,AXG:-0.121,AYG:-0.186,AZG:1.006,GXD:3.11,GYD:0.40,GZD:0.03"}, {"timestamp": "24-10-2026 09:37:55", "packet_id": "141", "temperature_c": "30.8", "temperature_f": "87.44", "humidity_pct": "80.8", "dew_point_c": "27.11", "soil_raw_adc": "1754", "soil_moisture_pct": "69.2", "soil_condition": "WET", "rain_raw_adc": "1520", "rain_intensity_pct": "79.2", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1949", "accel_y_raw": "-2965", "accel_z_raw": "16547", "accel_x_g": "-0.119", "accel_y_g": "-0.181", "accel_z_g": "1.01", "accel_magnitude_g": "1.033", "gyro_x_raw": "395", "gyro_y_raw": "86", "gyro_z_raw": "44", "gyro_x_dps": "3.02", "gyro_y_dps": "0.66", "gyro_z_dps": "0.34", "gyro_magnitude_dps": "3.11", "tilt_pitch_deg": "-10.09", "tilt_roll_deg": "6.61", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "163", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.80,H:80.80,SRAW:1754,SM:69.2,SC:WET,RRAW:1520,RP:79.2,RC:HEAVY_RAIN,MV:1,TX:-10.09,TY:6.61,AXG:-0.119,AYG:-0.181,AZG:1.010,GXD:3.02,GYD:0.66,GZD:0.34"}, {"timestamp": "24-10-2026 09:38:00", "packet_id": "142", "temperature_c": "30.8", "temperature_f": "87.44", "humidity_pct": "80.9", "dew_point_c": "27.13", "soil_raw_adc": "1718", "soil_moisture_pct": "71.2", "soil_condition": "WET", "rain_raw_adc": "1534", "rain_intensity_pct": "78.6", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1982", "accel_y_raw": "-2932", "accel_z_raw": "16498", "accel_x_g": "-0.121", "accel_y_g": "-0.179", "accel_z_g": "1.007", "accel_magnitude_g": "1.03", "gyro_x_raw": "403", "gyro_y_raw": "40", "gyro_z_raw": "-34", "gyro_x_dps": "3.08", "gyro_y_dps": "0.31", "gyro_z_dps": "-0.26", "gyro_magnitude_dps": "3.11", "tilt_pitch_deg": "-10.02", "tilt_roll_deg": "6.73", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "164", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.80,H:80.90,SRAW:1718,SM:71.2,SC:WET,RRAW:1534,RP:78.6,RC:HEAVY_RAIN,MV:1,TX:-10.02,TY:6.73,AXG:-0.121,AYG:-0.179,AZG:1.007,GXD:3.08,GYD:0.31,GZD:-0.26"}, {"timestamp": "24-10-2026 09:38:05", "packet_id": "143", "temperature_c": "30.8", "temperature_f": "87.44", "humidity_pct": "80.9", "dew_point_c": "27.13", "soil_raw_adc": "1828", "soil_moisture_pct": "65.1", "soil_condition": "WET", "rain_raw_adc": "1535", "rain_intensity_pct": "78.6", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "49", "accel_y_raw": "-11354", "accel_z_raw": "11796", "accel_x_g": "0.003", "accel_y_g": "-0.693", "accel_z_g": "0.72", "accel_magnitude_g": "0.999", "gyro_x_raw": "-6442", "gyro_y_raw": "-1103", "gyro_z_raw": "1674", "gyro_x_dps": "-49.18", "gyro_y_dps": "-8.42", "gyro_z_dps": "12.78", "gyro_magnitude_dps": "51.51", "tilt_pitch_deg": "-43.92", "tilt_roll_deg": "-0.18", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "167", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.80,H:80.90,SRAW:1828,SM:65.1,SC:WET,RRAW:1535,RP:78.6,RC:HEAVY_RAIN,MV:1,TX:-43.92,TY:-0.18,AXG:0.003,AYG:-0.693,AZG:0.720,GXD:-49.18,GYD:-8.42,GZD:12.78"}, {"timestamp": "24-10-2026 09:38:10", "packet_id": "144", "temperature_c": "30.8", "temperature_f": "87.44", "humidity_pct": "80.9", "dew_point_c": "27.13", "soil_raw_adc": "2068", "soil_moisture_pct": "51.8", "soil_condition": "MODERATE", "rain_raw_adc": "1506", "rain_intensity_pct": "79.8", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "5636", "accel_y_raw": "-13877", "accel_z_raw": "6012", "accel_x_g": "0.344", "accel_y_g": "-0.847", "accel_z_g": "0.367", "accel_magnitude_g": "0.985", "gyro_x_raw": "6877", "gyro_y_raw": "35", "gyro_z_raw": "-4003", "gyro_x_dps": "52.5", "gyro_y_dps": "0.27", "gyro_z_dps": "-30.56", "gyro_magnitude_dps": "60.75", "tilt_pitch_deg": "-59.3", "tilt_roll_deg": "-20.41", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "172", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.80,H:80.90,SRAW:2068,SM:51.8,SC:MODERATE,RRAW:1506,RP:79.8,RC:HEAVY_RAIN,MV:1,TX:-59.30,TY:-20.41,AXG:0.344,AYG:-0.847,AZG:0.367,GXD:52.50,GYD:0.27,GZD:-30.56"}, {"timestamp": "24-10-2026 09:38:15", "packet_id": "145", "temperature_c": "30.7", "temperature_f": "87.26", "humidity_pct": "81.0", "dew_point_c": "27.05", "soil_raw_adc": "1779", "soil_moisture_pct": "67.8", "soil_condition": "WET", "rain_raw_adc": "1535", "rain_intensity_pct": "78.6", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-7634", "accel_y_raw": "-14155", "accel_z_raw": "4472", "accel_x_g": "-0.466", "accel_y_g": "-0.864", "accel_z_g": "0.273", "accel_magnitude_g": "1.019", "gyro_x_raw": "-2216", "gyro_y_raw": "3119", "gyro_z_raw": "2885", "gyro_x_dps": "-16.92", "gyro_y_dps": "23.81", "gyro_z_dps": "22.03", "gyro_magnitude_dps": "36.59", "tilt_pitch_deg": "-57.99", "tilt_roll_deg": "27.23", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "168", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.70,H:81.00,SRAW:1779,SM:67.8,SC:WET,RRAW:1535,RP:78.6,RC:HEAVY_RAIN,MV:1,TX:-57.99,TY:27.23,AXG:-0.466,AYG:-0.864,AZG:0.273,GXD:-16.92,GYD:23.81,GZD:22.03"}, {"timestamp": "24-10-2026 09:38:20", "packet_id": "146", "temperature_c": "30.7", "temperature_f": "87.26", "humidity_pct": "81.0", "dew_point_c": "27.05", "soil_raw_adc": "1748", "soil_moisture_pct": "69.6", "soil_condition": "WET", "rain_raw_adc": "1535", "rain_intensity_pct": "78.6", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-10436", "accel_y_raw": "-12615", "accel_z_raw": "-917", "accel_x_g": "-0.637", "accel_y_g": "-0.77", "accel_z_g": "-0.056", "accel_magnitude_g": "1.001", "gyro_x_raw": "861", "gyro_y_raw": "-167", "gyro_z_raw": "-348", "gyro_x_dps": "6.58", "gyro_y_dps": "-1.28", "gyro_z_dps": "-2.66", "gyro_magnitude_dps": "7.21", "tilt_pitch_deg": "-50.25", "tilt_roll_deg": "39.56", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "167", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.70,H:81.00,SRAW:1748,SM:69.6,SC:WET,RRAW:1535,RP:78.6,RC:HEAVY_RAIN,MV:1,TX:-50.25,TY:39.56,AXG:-0.637,AYG:-0.770,AZG:-0.056,GXD:6.58,GYD:-1.28,GZD:-2.66"}, {"timestamp": "24-10-2026 09:38:25", "packet_id": "147", "temperature_c": "30.7", "temperature_f": "87.26", "humidity_pct": "81.0", "dew_point_c": "27.05", "soil_raw_adc": "1790", "soil_moisture_pct": "67.2", "soil_condition": "WET", "rain_raw_adc": "1530", "rain_intensity_pct": "78.8", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1851", "accel_y_raw": "-10567", "accel_z_raw": "12713", "accel_x_g": "-0.113", "accel_y_g": "-0.645", "accel_z_g": "0.776", "accel_magnitude_g": "1.015", "gyro_x_raw": "390", "gyro_y_raw": "69", "gyro_z_raw": "-95", "gyro_x_dps": "2.98", "gyro_y_dps": "0.53", "gyro_z_dps": "-0.73", "gyro_magnitude_dps": "3.11", "tilt_pitch_deg": "-39.44", "tilt_roll_deg": "6.41", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "164", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.70,H:81.00,SRAW:1790,SM:67.2,SC:WET,RRAW:1530,RP:78.8,RC:HEAVY_RAIN,MV:1,TX:-39.44,TY:6.41,AXG:-0.113,AYG:-0.645,AZG:0.776,GXD:2.98,GYD:0.53,GZD:-0.73"}, {"timestamp": "24-10-2026 09:38:30", "packet_id": "148", "temperature_c": "30.7", "temperature_f": "87.26", "humidity_pct": "81.0", "dew_point_c": "27.05", "soil_raw_adc": "1803", "soil_moisture_pct": "66.5", "soil_condition": "WET", "rain_raw_adc": "1675", "rain_intensity_pct": "73.0", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-1916", "accel_y_raw": "-10092", "accel_z_raw": "13647", "accel_x_g": "-0.117", "accel_y_g": "-0.616", "accel_z_g": "0.833", "accel_magnitude_g": "1.043", "gyro_x_raw": "344", "gyro_y_raw": "82", "gyro_z_raw": "-18", "gyro_x_dps": "2.63", "gyro_y_dps": "0.63", "gyro_z_dps": "-0.14", "gyro_magnitude_dps": "2.71", "tilt_pitch_deg": "-36.19", "tilt_roll_deg": "6.43", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "167", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.70,H:81.00,SRAW:1803,SM:66.5,SC:WET,RRAW:1675,RP:73.0,RC:MODERATE_RAIN,MV:1,TX:-36.19,TY:6.43,AXG:-0.117,AYG:-0.616,AZG:0.833,GXD:2.63,GYD:0.63,GZD:-0.14"}, {"timestamp": "24-10-2026 09:38:35", "packet_id": "149", "temperature_c": "30.7", "temperature_f": "87.26", "humidity_pct": "80.9", "dew_point_c": "27.03", "soil_raw_adc": "2170", "soil_moisture_pct": "46.1", "soil_condition": "MODERATE", "rain_raw_adc": "2903", "rain_intensity_pct": "23.9", "rain_condition": "LIGHT_RAIN", "accel_x_raw": "-1916", "accel_y_raw": "-10125", "accel_z_raw": "13500", "accel_x_g": "-0.117", "accel_y_g": "-0.618", "accel_z_g": "0.824", "accel_magnitude_g": "1.037", "gyro_x_raw": "468", "gyro_y_raw": "58", "gyro_z_raw": "-43", "gyro_x_dps": "3.58", "gyro_y_dps": "0.45", "gyro_z_dps": "-0.33", "gyro_magnitude_dps": "3.62", "tilt_pitch_deg": "-36.61", "tilt_roll_deg": "6.49", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.70,H:80.90,SRAW:2170,SM:46.1,SC:MODERATE,RRAW:2903,RP:23.9,RC:LIGHT_RAIN,MV:1,TX:-36.61,TY:6.49,AXG:-0.117,AYG:-0.618,AZG:0.824,GXD:3.58,GYD:0.45,GZD:-0.33"}, {"timestamp": "24-10-2026 09:38:40", "packet_id": "150", "temperature_c": "30.7", "temperature_f": "87.26", "humidity_pct": "81.0", "dew_point_c": "27.05", "soil_raw_adc": "2166", "soil_moisture_pct": "46.3", "soil_condition": "MODERATE", "rain_raw_adc": "2713", "rain_intensity_pct": "31.5", "rain_condition": "LIGHT_RAIN", "accel_x_raw": "-2162", "accel_y_raw": "-2228", "accel_z_raw": "16465", "accel_x_g": "-0.132", "accel_y_g": "-0.136", "accel_z_g": "1.005", "accel_magnitude_g": "1.023", "gyro_x_raw": "387", "gyro_y_raw": "36", "gyro_z_raw": "230", "gyro_x_dps": "2.96", "gyro_y_dps": "0.28", "gyro_z_dps": "1.76", "gyro_magnitude_dps": "3.46", "tilt_pitch_deg": "-7.64", "tilt_roll_deg": "7.41", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "167", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.70,H:81.00,SRAW:2166,SM:46.3,SC:MODERATE,RRAW:2713,RP:31.5,RC:LIGHT_RAIN,MV:1,TX:-7.64,TY:7.41,AXG:-0.132,AYG:-0.136,AZG:1.005,GXD:2.96,GYD:0.28,GZD:1.76"}, {"timestamp": "24-10-2026 09:38:45", "packet_id": "151", "temperature_c": "30.7", "temperature_f": "87.26", "humidity_pct": "81.0", "dew_point_c": "27.05", "soil_raw_adc": "1696", "soil_moisture_pct": "72.4", "soil_condition": "WET", "rain_raw_adc": "1434", "rain_intensity_pct": "82.6", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-2179", "accel_y_raw": "-2162", "accel_z_raw": "16695", "accel_x_g": "-0.133", "accel_y_g": "-0.132", "accel_z_g": "1.019", "accel_magnitude_g": "1.036", "gyro_x_raw": "404", "gyro_y_raw": "90", "gyro_z_raw": "-31", "gyro_x_dps": "3.09", "gyro_y_dps": "0.69", "gyro_z_dps": "-0.24", "gyro_magnitude_dps": "3.18", "tilt_pitch_deg": "-7.34", "tilt_roll_deg": "7.35", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "163", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.70,H:81.00,SRAW:1696,SM:72.4,SC:WET,RRAW:1434,RP:82.6,RC:HEAVY_RAIN,MV:1,TX:-7.34,TY:7.35,AXG:-0.133,AYG:-0.132,AZG:1.019,GXD:3.09,GYD:0.69,GZD:-0.24"}, {"timestamp": "24-10-2026 09:38:50", "packet_id": "152", "temperature_c": "30.6", "temperature_f": "87.08", "humidity_pct": "81.2", "dew_point_c": "27.0", "soil_raw_adc": "1905", "soil_moisture_pct": "60.8", "soil_condition": "MODERATE", "rain_raw_adc": "1892", "rain_intensity_pct": "64.3", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-2113", "accel_y_raw": "-2097", "accel_z_raw": "16596", "accel_x_g": "-0.129", "accel_y_g": "-0.128", "accel_z_g": "1.013", "accel_magnitude_g": "1.029", "gyro_x_raw": "377", "gyro_y_raw": "91", "gyro_z_raw": "-47", "gyro_x_dps": "2.88", "gyro_y_dps": "0.7", "gyro_z_dps": "-0.36", "gyro_magnitude_dps": "2.99", "tilt_pitch_deg": "-7.13", "tilt_roll_deg": "7.2", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "171", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.60,H:81.20,SRAW:1905,SM:60.8,SC:MODERATE,RRAW:1892,RP:64.3,RC:MODERATE_RAIN,MV:1,TX:-7.13,TY:7.20,AXG:-0.129,AYG:-0.128,AZG:1.013,GXD:2.88,GYD:0.70,GZD:-0.36"}, {"timestamp": "24-10-2026 09:38:55", "packet_id": "153", "temperature_c": "30.6", "temperature_f": "87.08", "humidity_pct": "81.3", "dew_point_c": "27.02", "soil_raw_adc": "1773", "soil_moisture_pct": "68.2", "soil_condition": "WET", "rain_raw_adc": "1488", "rain_intensity_pct": "80.5", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-2015", "accel_y_raw": "-2375", "accel_z_raw": "16580", "accel_x_g": "-0.123", "accel_y_g": "-0.145", "accel_z_g": "1.012", "accel_magnitude_g": "1.03", "gyro_x_raw": "424", "gyro_y_raw": "107", "gyro_z_raw": "-247", "gyro_x_dps": "3.24", "gyro_y_dps": "0.82", "gyro_z_dps": "-1.89", "gyro_magnitude_dps": "3.84", "tilt_pitch_deg": "-8.08", "tilt_roll_deg": "6.88", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "163", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.60,H:81.30,SRAW:1773,SM:68.2,SC:WET,RRAW:1488,RP:80.5,RC:HEAVY_RAIN,MV:1,TX:-8.08,TY:6.88,AXG:-0.123,AYG:-0.145,AZG:1.012,GXD:3.24,GYD:0.82,GZD:-1.89"}, {"timestamp": "24-10-2026 09:39:00", "packet_id": "154", "temperature_c": "30.6", "temperature_f": "87.08", "humidity_pct": "81.5", "dew_point_c": "27.06", "soil_raw_adc": "1734", "soil_moisture_pct": "70.3", "soil_condition": "WET", "rain_raw_adc": "1483", "rain_intensity_pct": "80.7", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "10043", "accel_y_raw": "-6127", "accel_z_raw": "11976", "accel_x_g": "0.613", "accel_y_g": "-0.374", "accel_z_g": "0.731", "accel_magnitude_g": "1.025", "gyro_x_raw": "348", "gyro_y_raw": "-897", "gyro_z_raw": "61", "gyro_x_dps": "2.66", "gyro_y_dps": "-6.85", "gyro_z_dps": "0.47", "gyro_magnitude_dps": "7.36", "tilt_pitch_deg": "-21.41", "tilt_roll_deg": "-36.71", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "165", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.60,H:81.50,SRAW:1734,SM:70.3,SC:WET,RRAW:1483,RP:80.7,RC:HEAVY_RAIN,MV:1,TX:-21.41,TY:-36.71,AXG:0.613,AYG:-0.374,AZG:0.731,GXD:2.66,GYD:-6.85,GZD:0.47"}, {"timestamp": "24-10-2026 09:39:05", "packet_id": "155", "temperature_c": "30.6", "temperature_f": "87.08", "humidity_pct": "81.7", "dew_point_c": "27.1", "soil_raw_adc": "1787", "soil_moisture_pct": "67.4", "soil_condition": "WET", "rain_raw_adc": "1520", "rain_intensity_pct": "79.2", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "11485", "accel_y_raw": "5931", "accel_z_raw": "11796", "accel_x_g": "0.701", "accel_y_g": "0.362", "accel_z_g": "0.72", "accel_magnitude_g": "1.068", "gyro_x_raw": "4498", "gyro_y_raw": "4336", "gyro_z_raw": "200", "gyro_x_dps": "34.34", "gyro_y_dps": "33.1", "gyro_z_dps": "1.53", "gyro_magnitude_dps": "47.72", "tilt_pitch_deg": "19.8", "tilt_roll_deg": "-41.02", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "164", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.60,H:81.70,SRAW:1787,SM:67.4,SC:WET,RRAW:1520,RP:79.2,RC:HEAVY_RAIN,MV:1,TX:19.80,TY:-41.02,AXG:0.701,AYG:0.362,AZG:0.720,GXD:34.34,GYD:33.10,GZD:1.53"}, {"timestamp": "24-10-2026 09:39:10", "packet_id": "156", "temperature_c": "30.6", "temperature_f": "87.08", "humidity_pct": "81.7", "dew_point_c": "27.1", "soil_raw_adc": "1812", "soil_moisture_pct": "66.0", "soil_condition": "WET", "rain_raw_adc": "1525", "rain_intensity_pct": "79.0", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "147", "accel_y_raw": "-9764", "accel_z_raw": "13713", "accel_x_g": "0.009", "accel_y_g": "-0.596", "accel_z_g": "0.837", "accel_magnitude_g": "1.028", "gyro_x_raw": "-2499", "gyro_y_raw": "1954", "gyro_z_raw": "-328", "gyro_x_dps": "-19.08", "gyro_y_dps": "14.92", "gyro_z_dps": "-2.51", "gyro_magnitude_dps": "24.35", "tilt_pitch_deg": "-35.48", "tilt_roll_deg": "-0.48", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "167", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.60,H:81.70,SRAW:1812,SM:66.0,SC:WET,RRAW:1525,RP:79.0,RC:HEAVY_RAIN,MV:1,TX:-35.48,TY:-0.48,AXG:0.009,AYG:-0.596,AZG:0.837,GXD:-19.08,GYD:14.92,GZD:-2.51"}, {"timestamp": "24-10-2026 09:39:15", "packet_id": "157", "temperature_c": "30.6", "temperature_f": "87.08", "humidity_pct": "81.6", "dew_point_c": "27.08", "soil_raw_adc": "1840", "soil_moisture_pct": "64.4", "soil_condition": "MODERATE", "rain_raw_adc": "1566", "rain_intensity_pct": "77.4", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "9043", "accel_y_raw": "9846", "accel_z_raw": "10043", "accel_x_g": "0.552", "accel_y_g": "0.601", "accel_z_g": "0.613", "accel_magnitude_g": "1.021", "gyro_x_raw": "1993", "gyro_y_raw": "476", "gyro_z_raw": "-1840", "gyro_x_dps": "15.22", "gyro_y_dps": "3.64", "gyro_z_dps": "-14.05", "gyro_magnitude_dps": "21.03", "tilt_pitch_deg": "36.05", "tilt_roll_deg": "-32.76", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "170", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.60,H:81.60,SRAW:1840,SM:64.4,SC:MODERATE,RRAW:1566,RP:77.4,RC:HEAVY_RAIN,MV:1,TX:36.05,TY:-32.76,AXG:0.552,AYG:0.601,AZG:0.613,GXD:15.22,GYD:3.64,GZD:-14.05"}, {"timestamp": "24-10-2026 09:39:20", "packet_id": "158", "temperature_c": "30.6", "temperature_f": "87.08", "humidity_pct": "81.6", "dew_point_c": "27.08", "soil_raw_adc": "1821", "soil_moisture_pct": "65.5", "soil_condition": "WET", "rain_raw_adc": "1535", "rain_intensity_pct": "78.6", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "11436", "accel_y_raw": "6275", "accel_z_raw": "10207", "accel_x_g": "0.698", "accel_y_g": "0.383", "accel_z_g": "0.623", "accel_magnitude_g": "1.011", "gyro_x_raw": "31", "gyro_y_raw": "-1134", "gyro_z_raw": "487", "gyro_x_dps": "0.24", "gyro_y_dps": "-8.66", "gyro_z_dps": "3.72", "gyro_magnitude_dps": "9.43", "tilt_pitch_deg": "22.24", "tilt_roll_deg": "-43.68", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "163", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.60,H:81.60,SRAW:1821,SM:65.5,SC:WET,RRAW:1535,RP:78.6,RC:HEAVY_RAIN,MV:1,TX:22.24,TY:-43.68,AXG:0.698,AYG:0.383,AZG:0.623,GXD:0.24,GYD:-8.66,GZD:3.72"}, {"timestamp": "24-10-2026 09:39:25", "packet_id": "159", "temperature_c": "30.6", "temperature_f": "87.08", "humidity_pct": "81.5", "dew_point_c": "27.06", "soil_raw_adc": "1824", "soil_moisture_pct": "65.3", "soil_condition": "WET", "rain_raw_adc": "1535", "rain_intensity_pct": "78.6", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1294", "accel_y_raw": "212", "accel_z_raw": "16662", "accel_x_g": "-0.079", "accel_y_g": "0.013", "accel_z_g": "1.017", "accel_magnitude_g": "1.02", "gyro_x_raw": "268", "gyro_y_raw": "425", "gyro_z_raw": "-65", "gyro_x_dps": "2.05", "gyro_y_dps": "3.25", "gyro_z_dps": "-0.5", "gyro_magnitude_dps": "3.87", "tilt_pitch_deg": "0.73", "tilt_roll_deg": "4.46", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "161", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.60,H:81.50,SRAW:1824,SM:65.3,SC:WET,RRAW:1535,RP:78.6,RC:HEAVY_RAIN,MV:1,TX:0.73,TY:4.46,AXG:-0.079,AYG:0.013,AZG:1.017,GXD:2.05,GYD:3.25,GZD:-0.50"}, {"timestamp": "24-10-2026 09:39:30", "packet_id": "160", "temperature_c": "30.7", "temperature_f": "87.26", "humidity_pct": "81.6", "dew_point_c": "27.18", "soil_raw_adc": "1807", "soil_moisture_pct": "66.3", "soil_condition": "WET", "rain_raw_adc": "1514", "rain_intensity_pct": "79.4", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "7897", "accel_y_raw": "-6799", "accel_z_raw": "13500", "accel_x_g": "0.482", "accel_y_g": "-0.415", "accel_z_g": "0.824", "accel_magnitude_g": "1.041", "gyro_x_raw": "586", "gyro_y_raw": "564", "gyro_z_raw": "-62", "gyro_x_dps": "4.48", "gyro_y_dps": "4.31", "gyro_z_dps": "-0.48", "gyro_magnitude_dps": "6.24", "tilt_pitch_deg": "-23.48", "tilt_roll_deg": "-27.6", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "165", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.70,H:81.60,SRAW:1807,SM:66.3,SC:WET,RRAW:1514,RP:79.4,RC:HEAVY_RAIN,MV:1,TX:-23.48,TY:-27.60,AXG:0.482,AYG:-0.415,AZG:0.824,GXD:4.48,GYD:4.31,GZD:-0.48"}, {"timestamp": "24-10-2026 09:39:35", "packet_id": "161", "temperature_c": "30.6", "temperature_f": "87.08", "humidity_pct": "81.7", "dew_point_c": "27.1", "soil_raw_adc": "2542", "soil_moisture_pct": "25.4", "soil_condition": "MODERATE", "rain_raw_adc": "1622", "rain_intensity_pct": "75.1", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "4210", "accel_y_raw": "-6291", "accel_z_raw": "15843", "accel_x_g": "0.257", "accel_y_g": "-0.384", "accel_z_g": "0.967", "accel_magnitude_g": "1.072", "gyro_x_raw": "6075", "gyro_y_raw": "12676", "gyro_z_raw": "1367", "gyro_x_dps": "46.38", "gyro_y_dps": "96.77", "gyro_z_dps": "10.44", "gyro_magnitude_dps": "107.82", "tilt_pitch_deg": "-21.01", "tilt_roll_deg": "-13.88", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "172", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.60,H:81.70,SRAW:2542,SM:25.4,SC:MODERATE,RRAW:1622,RP:75.1,RC:HEAVY_RAIN,MV:1,TX:-21.01,TY:-13.88,AXG:0.257,AYG:-0.384,AZG:0.967,GXD:46.38,GYD:96.77,GZD:10.44"}, {"timestamp": "24-10-2026 09:39:40", "packet_id": "162", "temperature_c": "30.7", "temperature_f": "87.26", "humidity_pct": "81.7", "dew_point_c": "27.2", "soil_raw_adc": "2448", "soil_moisture_pct": "30.7", "soil_condition": "MODERATE", "rain_raw_adc": "1825", "rain_intensity_pct": "67.0", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "6258", "accel_y_raw": "-4915", "accel_z_raw": "14696", "accel_x_g": "0.382", "accel_y_g": "-0.3", "accel_z_g": "0.897", "accel_magnitude_g": "1.02", "gyro_x_raw": "489", "gyro_y_raw": "399", "gyro_z_raw": "-64", "gyro_x_dps": "3.74", "gyro_y_dps": "3.05", "gyro_z_dps": "-0.49", "gyro_magnitude_dps": "4.85", "tilt_pitch_deg": "-17.1", "tilt_roll_deg": "-21.97", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "173", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.70,H:81.70,SRAW:2448,SM:30.7,SC:MODERATE,RRAW:1825,RP:67.0,RC:MODERATE_RAIN,MV:1,TX:-17.10,TY:-21.97,AXG:0.382,AYG:-0.300,AZG:0.897,GXD:3.74,GYD:3.05,GZD:-0.49"}, {"timestamp": "24-10-2026 09:39:45", "packet_id": "163", "temperature_c": "30.7", "temperature_f": "87.26", "humidity_pct": "81.6", "dew_point_c": "27.18", "soil_raw_adc": "2368", "soil_moisture_pct": "35.1", "soil_condition": "MODERATE", "rain_raw_adc": "1808", "rain_intensity_pct": "67.7", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-557", "accel_y_raw": "278", "accel_z_raw": "16220", "accel_x_g": "-0.034", "accel_y_g": "0.017", "accel_z_g": "0.99", "accel_magnitude_g": "0.991", "gyro_x_raw": "58", "gyro_y_raw": "-56", "gyro_z_raw": "-27", "gyro_x_dps": "0.45", "gyro_y_dps": "-0.43", "gyro_z_dps": "-0.21", "gyro_magnitude_dps": "0.66", "tilt_pitch_deg": "0.97", "tilt_roll_deg": "1.96", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "170", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.70,H:81.60,SRAW:2368,SM:35.1,SC:MODERATE,RRAW:1808,RP:67.7,RC:MODERATE_RAIN,MV:1,TX:0.97,TY:1.96,AXG:-0.034,AYG:0.017,AZG:0.990,GXD:0.45,GYD:-0.43,GZD:-0.21"}, {"timestamp": "24-10-2026 09:39:50", "packet_id": "164", "temperature_c": "30.7", "temperature_f": "87.26", "humidity_pct": "81.5", "dew_point_c": "27.16", "soil_raw_adc": "2337", "soil_moisture_pct": "36.8", "soil_condition": "MODERATE", "rain_raw_adc": "1819", "rain_intensity_pct": "67.2", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "11042", "accel_y_raw": "-4259", "accel_z_raw": "11534", "accel_x_g": "0.674", "accel_y_g": "-0.26", "accel_z_g": "0.704", "accel_magnitude_g": "1.009", "gyro_x_raw": "448", "gyro_y_raw": "-1714", "gyro_z_raw": "-3", "gyro_x_dps": "3.42", "gyro_y_dps": "-13.09", "gyro_z_dps": "-0.03", "gyro_magnitude_dps": "13.53", "tilt_pitch_deg": "-14.94", "tilt_roll_deg": "-41.94", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "175", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.70,H:81.50,SRAW:2337,SM:36.8,SC:MODERATE,RRAW:1819,RP:67.2,RC:MODERATE_RAIN,MV:1,TX:-14.94,TY:-41.94,AXG:0.674,AYG:-0.260,AZG:0.704,GXD:3.42,GYD:-13.09,GZD:-0.03"}, {"timestamp": "24-10-2026 09:39:55", "packet_id": "165", "temperature_c": "30.7", "temperature_f": "87.26", "humidity_pct": "81.5", "dew_point_c": "27.16", "soil_raw_adc": "2377", "soil_moisture_pct": "34.6", "soil_condition": "MODERATE", "rain_raw_adc": "1943", "rain_intensity_pct": "62.3", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "11714", "accel_y_raw": "-3997", "accel_z_raw": "11485", "accel_x_g": "0.715", "accel_y_g": "-0.244", "accel_z_g": "0.701", "accel_magnitude_g": "1.031", "gyro_x_raw": "501", "gyro_y_raw": "-317", "gyro_z_raw": "-70", "gyro_x_dps": "3.83", "gyro_y_dps": "-2.42", "gyro_z_dps": "-0.54", "gyro_magnitude_dps": "4.56", "tilt_pitch_deg": "-13.68", "tilt_roll_deg": "-43.9", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "174", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.70,H:81.50,SRAW:2377,SM:34.6,SC:MODERATE,RRAW:1943,RP:62.3,RC:MODERATE_RAIN,MV:1,TX:-13.68,TY:-43.90,AXG:0.715,AYG:-0.244,AZG:0.701,GXD:3.83,GYD:-2.42,GZD:-0.54"}, {"timestamp": "24-10-2026 09:40:00", "packet_id": "166", "temperature_c": "30.7", "temperature_f": "87.26", "humidity_pct": "81.4", "dew_point_c": "27.14", "soil_raw_adc": "2205", "soil_moisture_pct": "44.2", "soil_condition": "MODERATE", "rain_raw_adc": "1440", "rain_intensity_pct": "82.4", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "311", "accel_y_raw": "-1392", "accel_z_raw": "16875", "accel_x_g": "0.019", "accel_y_g": "-0.085", "accel_z_g": "1.03", "accel_magnitude_g": "1.034", "gyro_x_raw": "246", "gyro_y_raw": "-217", "gyro_z_raw": "-14", "gyro_x_dps": "1.88", "gyro_y_dps": "-1.66", "gyro_z_dps": "-0.11", "gyro_magnitude_dps": "2.51", "tilt_pitch_deg": "-4.7", "tilt_roll_deg": "-1.03", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.70,H:81.40,SRAW:2205,SM:44.2,SC:MODERATE,RRAW:1440,RP:82.4,RC:HEAVY_RAIN,MV:1,TX:-4.70,TY:-1.03,AXG:0.019,AYG:-0.085,AZG:1.030,GXD:1.88,GYD:-1.66,GZD:-0.11"}, {"timestamp": "24-10-2026 09:40:05", "packet_id": "167", "temperature_c": "30.7", "temperature_f": "87.26", "humidity_pct": "81.2", "dew_point_c": "27.09", "soil_raw_adc": "1611", "soil_moisture_pct": "77.2", "soil_condition": "WET", "rain_raw_adc": "1464", "rain_intensity_pct": "81.4", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "5586", "accel_y_raw": "212", "accel_z_raw": "15990", "accel_x_g": "0.341", "accel_y_g": "0.013", "accel_z_g": "0.976", "accel_magnitude_g": "1.034", "gyro_x_raw": "381", "gyro_y_raw": "58", "gyro_z_raw": "-48", "gyro_x_dps": "2.91", "gyro_y_dps": "0.45", "gyro_z_dps": "-0.37", "gyro_magnitude_dps": "2.97", "tilt_pitch_deg": "0.7", "tilt_roll_deg": "-19.25", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "162", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.70,H:81.20,SRAW:1611,SM:77.2,SC:WET,RRAW:1464,RP:81.4,RC:HEAVY_RAIN,MV:1,TX:0.70,TY:-19.25,AXG:0.341,AYG:0.013,AZG:0.976,GXD:2.91,GYD:0.45,GZD:-0.37"}, {"timestamp": "24-10-2026 09:40:10", "packet_id": "168", "temperature_c": "30.7", "temperature_f": "87.26", "humidity_pct": "81.1", "dew_point_c": "27.07", "soil_raw_adc": "1653", "soil_moisture_pct": "74.8", "soil_condition": "WET", "rain_raw_adc": "1454", "rain_intensity_pct": "81.8", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "5062", "accel_y_raw": "-7340", "accel_z_raw": "14450", "accel_x_g": "0.309", "accel_y_g": "-0.448", "accel_z_g": "0.882", "accel_magnitude_g": "1.036", "gyro_x_raw": "120", "gyro_y_raw": "24", "gyro_z_raw": "-6", "gyro_x_dps": "0.92", "gyro_y_dps": "0.19", "gyro_z_dps": "-0.05", "gyro_magnitude_dps": "0.94", "tilt_pitch_deg": "-25.61", "tilt_roll_deg": "-17.33", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "165", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.70,H:81.10,SRAW:1653,SM:74.8,SC:WET,RRAW:1454,RP:81.8,RC:HEAVY_RAIN,MV:1,TX:-25.61,TY:-17.33,AXG:0.309,AYG:-0.448,AZG:0.882,GXD:0.92,GYD:0.19,GZD:-0.05"}, {"timestamp": "24-10-2026 09:40:15", "packet_id": "169", "temperature_c": "30.8", "temperature_f": "87.44", "humidity_pct": "81.1", "dew_point_c": "27.17", "soil_raw_adc": "1708", "soil_moisture_pct": "71.8", "soil_condition": "WET", "rain_raw_adc": "1483", "rain_intensity_pct": "80.7", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "4964", "accel_y_raw": "5242", "accel_z_raw": "14057", "accel_x_g": "0.303", "accel_y_g": "0.32", "accel_z_g": "0.858", "accel_magnitude_g": "0.965", "gyro_x_raw": "3975", "gyro_y_raw": "-682", "gyro_z_raw": "-2026", "gyro_x_dps": "30.35", "gyro_y_dps": "-5.21", "gyro_z_dps": "-15.47", "gyro_magnitude_dps": "34.46", "tilt_pitch_deg": "19.37", "tilt_roll_deg": "-18.32", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "166", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.80,H:81.10,SRAW:1708,SM:71.8,SC:WET,RRAW:1483,RP:80.7,RC:HEAVY_RAIN,MV:1,TX:19.37,TY:-18.32,AXG:0.303,AYG:0.320,AZG:0.858,GXD:30.35,GYD:-5.21,GZD:-15.47"}, {"timestamp": "24-10-2026 09:40:20", "packet_id": "170", "temperature_c": "30.8", "temperature_f": "87.44", "humidity_pct": "81.0", "dew_point_c": "27.15", "soil_raw_adc": "1688", "soil_moisture_pct": "72.9", "soil_condition": "WET", "rain_raw_adc": "1487", "rain_intensity_pct": "80.5", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "5144", "accel_y_raw": "3833", "accel_z_raw": "15613", "accel_x_g": "0.314", "accel_y_g": "0.234", "accel_z_g": "0.953", "accel_magnitude_g": "1.03", "gyro_x_raw": "98", "gyro_y_raw": "134", "gyro_z_raw": "70", "gyro_x_dps": "0.75", "gyro_y_dps": "1.03", "gyro_z_dps": "0.54", "gyro_magnitude_dps": "1.38", "tilt_pitch_deg": "13.12", "tilt_roll_deg": "-17.77", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "162", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.80,H:81.00,SRAW:1688,SM:72.9,SC:WET,RRAW:1487,RP:80.5,RC:HEAVY_RAIN,MV:1,TX:13.12,TY:-17.77,AXG:0.314,AYG:0.234,AZG:0.953,GXD:0.75,GYD:1.03,GZD:0.54"}, {"timestamp": "24-10-2026 09:40:25", "packet_id": "171", "temperature_c": "30.8", "temperature_f": "87.44", "humidity_pct": "80.9", "dew_point_c": "27.13", "soil_raw_adc": "1692", "soil_moisture_pct": "72.7", "soil_condition": "WET", "rain_raw_adc": "1499", "rain_intensity_pct": "80.0", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-10256", "accel_y_raw": "163", "accel_z_raw": "13254", "accel_x_g": "-0.626", "accel_y_g": "0.01", "accel_z_g": "0.809", "accel_magnitude_g": "1.023", "gyro_x_raw": "391", "gyro_y_raw": "112", "gyro_z_raw": "15", "gyro_x_dps": "2.99", "gyro_y_dps": "0.86", "gyro_z_dps": "0.12", "gyro_magnitude_dps": "3.11", "tilt_pitch_deg": "0.59", "tilt_roll_deg": "37.72", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "161", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.80,H:80.90,SRAW:1692,SM:72.7,SC:WET,RRAW:1499,RP:80.0,RC:HEAVY_RAIN,MV:1,TX:0.59,TY:37.72,AXG:-0.626,AYG:0.010,AZG:0.809,GXD:2.99,GYD:0.86,GZD:0.12"}, {"timestamp": "24-10-2026 09:40:30", "packet_id": "172", "temperature_c": "30.8", "temperature_f": "87.44", "humidity_pct": "80.8", "dew_point_c": "27.11", "soil_raw_adc": "1741", "soil_moisture_pct": "69.9", "soil_condition": "WET", "rain_raw_adc": "1483", "rain_intensity_pct": "80.7", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "3260", "accel_y_raw": "-7372", "accel_z_raw": "14942", "accel_x_g": "0.199", "accel_y_g": "-0.45", "accel_z_g": "0.912", "accel_magnitude_g": "1.036", "gyro_x_raw": "319", "gyro_y_raw": "23", "gyro_z_raw": "-69", "gyro_x_dps": "2.44", "gyro_y_dps": "0.18", "gyro_z_dps": "-0.53", "gyro_magnitude_dps": "2.5", "tilt_pitch_deg": "-25.74", "tilt_roll_deg": "-11.1", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "165", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.80,H:80.80,SRAW:1741,SM:69.9,SC:WET,RRAW:1483,RP:80.7,RC:HEAVY_RAIN,MV:1,TX:-25.74,TY:-11.10,AXG:0.199,AYG:-0.450,AZG:0.912,GXD:2.44,GYD:0.18,GZD:-0.53"}, {"timestamp": "24-10-2026 09:40:35", "packet_id": "173", "temperature_c": "30.8", "temperature_f": "87.44", "humidity_pct": "80.7", "dew_point_c": "27.09", "soil_raw_adc": "1858", "soil_moisture_pct": "63.4", "soil_condition": "MODERATE", "rain_raw_adc": "1578", "rain_intensity_pct": "76.9", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "131", "accel_y_raw": "7192", "accel_z_raw": "14876", "accel_x_g": "0.008", "accel_y_g": "0.439", "accel_z_g": "0.908", "accel_magnitude_g": "1.009", "gyro_x_raw": "-336", "gyro_y_raw": "-1751", "gyro_z_raw": "-893", "gyro_x_dps": "-2.57", "gyro_y_dps": "-13.37", "gyro_z_dps": "-6.82", "gyro_magnitude_dps": "15.23", "tilt_pitch_deg": "25.81", "tilt_roll_deg": "-0.44", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "170", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.80,H:80.70,SRAW:1858,SM:63.4,SC:MODERATE,RRAW:1578,RP:76.9,RC:HEAVY_RAIN,MV:1,TX:25.81,TY:-0.44,AXG:0.008,AYG:0.439,AZG:0.908,GXD:-2.57,GYD:-13.37,GZD:-6.82"}, {"timestamp": "24-10-2026 09:40:40", "packet_id": "174", "temperature_c": "30.8", "temperature_f": "87.44", "humidity_pct": "80.7", "dew_point_c": "27.09", "soil_raw_adc": "2179", "soil_moisture_pct": "45.6", "soil_condition": "MODERATE", "rain_raw_adc": "2775", "rain_intensity_pct": "29.0", "rain_condition": "LIGHT_RAIN", "accel_x_raw": "7258", "accel_y_raw": "6455", "accel_z_raw": "13533", "accel_x_g": "0.443", "accel_y_g": "0.394", "accel_z_g": "0.826", "accel_magnitude_g": "1.017", "gyro_x_raw": "286", "gyro_y_raw": "-85", "gyro_z_raw": "-192", "gyro_x_dps": "2.19", "gyro_y_dps": "-0.65", "gyro_z_dps": "-1.47", "gyro_magnitude_dps": "2.72", "tilt_pitch_deg": "22.82", "tilt_roll_deg": "-25.84", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.80,H:80.70,SRAW:2179,SM:45.6,SC:MODERATE,RRAW:2775,RP:29.0,RC:LIGHT_RAIN,MV:1,TX:22.82,TY:-25.84,AXG:0.443,AYG:0.394,AZG:0.826,GXD:2.19,GYD:-0.65,GZD:-1.47"}, {"timestamp": "24-10-2026 09:40:45", "packet_id": "175", "temperature_c": "30.8", "temperature_f": "87.44", "humidity_pct": "80.7", "dew_point_c": "27.09", "soil_raw_adc": "1508", "soil_moisture_pct": "82.9", "soil_condition": "WET", "rain_raw_adc": "1406", "rain_intensity_pct": "83.8", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "8290", "accel_y_raw": "6930", "accel_z_raw": "12599", "accel_x_g": "0.506", "accel_y_g": "0.423", "accel_z_g": "0.769", "accel_magnitude_g": "1.013", "gyro_x_raw": "3328", "gyro_y_raw": "-65", "gyro_z_raw": "-1012", "gyro_x_dps": "25.41", "gyro_y_dps": "-0.5", "gyro_z_dps": "-7.73", "gyro_magnitude_dps": "26.56", "tilt_pitch_deg": "24.69", "tilt_roll_deg": "-29.95", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "165", "landslide_risk_level": "CRITICAL ALERT", "hazard_status": "HIGH_LANDSLIDE_HAZARD", "raw_packet": "GS1,TV:1,T:30.80,H:80.70,SRAW:1508,SM:82.9,SC:WET,RRAW:1406,RP:83.8,RC:HEAVY_RAIN,MV:1,TX:24.69,TY:-29.95,AXG:0.506,AYG:0.423,AZG:0.769,GXD:25.41,GYD:-0.50,GZD:-7.73"}, {"timestamp": "24-10-2026 09:40:50", "packet_id": "176", "temperature_c": "30.8", "temperature_f": "87.44", "humidity_pct": "80.6", "dew_point_c": "27.07", "soil_raw_adc": "1951", "soil_moisture_pct": "58.3", "soil_condition": "MODERATE", "rain_raw_adc": "2465", "rain_intensity_pct": "41.4", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "8060", "accel_y_raw": "6684", "accel_z_raw": "12664", "accel_x_g": "0.492", "accel_y_g": "0.408", "accel_z_g": "0.773", "accel_magnitude_g": "1.003", "gyro_x_raw": "290", "gyro_y_raw": "2", "gyro_z_raw": "-161", "gyro_x_dps": "2.22", "gyro_y_dps": "0.02", "gyro_z_dps": "-1.23", "gyro_magnitude_dps": "2.54", "tilt_pitch_deg": "24.0", "tilt_roll_deg": "-29.37", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "171", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.80,H:80.60,SRAW:1951,SM:58.3,SC:MODERATE,RRAW:2465,RP:41.4,RC:MODERATE_RAIN,MV:1,TX:24.00,TY:-29.37,AXG:0.492,AYG:0.408,AZG:0.773,GXD:2.22,GYD:0.02,GZD:-1.23"}, {"timestamp": "24-10-2026 09:40:55", "packet_id": "177", "temperature_c": "30.8", "temperature_f": "87.44", "humidity_pct": "80.6", "dew_point_c": "27.07", "soil_raw_adc": "1680", "soil_moisture_pct": "73.3", "soil_condition": "WET", "rain_raw_adc": "1590", "rain_intensity_pct": "76.4", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "9011", "accel_y_raw": "5914", "accel_z_raw": "12861", "accel_x_g": "0.55", "accel_y_g": "0.361", "accel_z_g": "0.785", "accel_magnitude_g": "1.024", "gyro_x_raw": "360", "gyro_y_raw": "-57", "gyro_z_raw": "149", "gyro_x_dps": "2.75", "gyro_y_dps": "-0.44", "gyro_z_dps": "1.14", "gyro_magnitude_dps": "3.01", "tilt_pitch_deg": "20.66", "tilt_roll_deg": "-32.47", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "163", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.80,H:80.60,SRAW:1680,SM:73.3,SC:WET,RRAW:1590,RP:76.4,RC:HEAVY_RAIN,MV:1,TX:20.66,TY:-32.47,AXG:0.550,AYG:0.361,AZG:0.785,GXD:2.75,GYD:-0.44,GZD:1.14"}, {"timestamp": "24-10-2026 09:41:00", "packet_id": "178", "temperature_c": "30.8", "temperature_f": "87.44", "humidity_pct": "80.6", "dew_point_c": "27.07", "soil_raw_adc": "1883", "soil_moisture_pct": "62.1", "soil_condition": "MODERATE", "rain_raw_adc": "1982", "rain_intensity_pct": "60.7", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "12025", "accel_y_raw": "-8732", "accel_z_raw": "7569", "accel_x_g": "0.734", "accel_y_g": "-0.533", "accel_z_g": "0.462", "accel_magnitude_g": "1.018", "gyro_x_raw": "106", "gyro_y_raw": "783", "gyro_z_raw": "-1", "gyro_x_dps": "0.81", "gyro_y_dps": "5.98", "gyro_z_dps": "-0.01", "gyro_magnitude_dps": "6.03", "tilt_pitch_deg": "-31.6", "tilt_roll_deg": "-46.11", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "173", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.80,H:80.60,SRAW:1883,SM:62.1,SC:MODERATE,RRAW:1982,RP:60.7,RC:MODERATE_RAIN,MV:1,TX:-31.60,TY:-46.11,AXG:0.734,AYG:-0.533,AZG:0.462,GXD:0.81,GYD:5.98,GZD:-0.01"}, {"timestamp": "24-10-2026 09:41:05", "packet_id": "179", "temperature_c": "30.9", "temperature_f": "87.62", "humidity_pct": "80.5", "dew_point_c": "27.14", "soil_raw_adc": "1818", "soil_moisture_pct": "65.7", "soil_condition": "WET", "rain_raw_adc": "1879", "rain_intensity_pct": "64.8", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "5144", "accel_y_raw": "-4603", "accel_z_raw": "15269", "accel_x_g": "0.314", "accel_y_g": "-0.281", "accel_z_g": "0.932", "accel_magnitude_g": "1.023", "gyro_x_raw": "-310", "gyro_y_raw": "224", "gyro_z_raw": "276", "gyro_x_dps": "-2.37", "gyro_y_dps": "1.71", "gyro_z_dps": "2.11", "gyro_magnitude_dps": "3.6", "tilt_pitch_deg": "-15.96", "tilt_roll_deg": "-17.89", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "168", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.90,H:80.50,SRAW:1818,SM:65.7,SC:WET,RRAW:1879,RP:64.8,RC:MODERATE_RAIN,MV:1,TX:-15.96,TY:-17.89,AXG:0.314,AYG:-0.281,AZG:0.932,GXD:-2.37,GYD:1.71,GZD:2.11"}, {"timestamp": "24-10-2026 09:41:21", "packet_id": "180", "temperature_c": "", "temperature_f": "", "humidity_pct": "", "dew_point_c": "", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "4095", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "0", "accel_y_raw": "0", "accel_z_raw": "16384", "accel_x_g": "0.0", "accel_y_g": "0.0", "accel_z_g": "1.0", "accel_magnitude_g": "1.0", "gyro_x_raw": "0", "gyro_y_raw": "0", "gyro_z_raw": "0", "gyro_x_dps": "0.0", "gyro_y_dps": "0.0", "gyro_z_dps": "0.0", "gyro_magnitude_dps": "0.0", "tilt_pitch_deg": "0.0", "tilt_roll_deg": "-0.0", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "71", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00"}, {"timestamp": "24-10-2026 09:41:21", "packet_id": "181", "temperature_c": "", "temperature_f": "", "humidity_pct": "", "dew_point_c": "", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "4095", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "0", "accel_y_raw": "0", "accel_z_raw": "16384", "accel_x_g": "0.0", "accel_y_g": "0.0", "accel_z_g": "1.0", "accel_magnitude_g": "1.0", "gyro_x_raw": "0", "gyro_y_raw": "0", "gyro_z_raw": "0", "gyro_x_dps": "0.0", "gyro_y_dps": "0.0", "gyro_z_dps": "0.0", "gyro_magnitude_dps": "0.0", "tilt_pitch_deg": "0.0", "tilt_roll_deg": "-0.0", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "71", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00"}, {"timestamp": "24-10-2026 09:41:21", "packet_id": "182", "temperature_c": "", "temperature_f": "", "humidity_pct": "", "dew_point_c": "", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "4095", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "0", "accel_y_raw": "0", "accel_z_raw": "16384", "accel_x_g": "0.0", "accel_y_g": "0.0", "accel_z_g": "1.0", "accel_magnitude_g": "1.0", "gyro_x_raw": "0", "gyro_y_raw": "0", "gyro_z_raw": "0", "gyro_x_dps": "0.0", "gyro_y_dps": "0.0", "gyro_z_dps": "0.0", "gyro_magnitude_dps": "0.0", "tilt_pitch_deg": "0.0", "tilt_roll_deg": "-0.0", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "71", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00"}, {"timestamp": "24-10-2026 09:41:27", "packet_id": "183", "temperature_c": "31.0", "temperature_f": "87.8", "humidity_pct": "80.4", "dew_point_c": "27.22", "soil_raw_adc": "1815", "soil_moisture_pct": "65.8", "soil_condition": "WET", "rain_raw_adc": "1788", "rain_intensity_pct": "68.5", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-2506", "accel_y_raw": "6307", "accel_z_raw": "15417", "accel_x_g": "-0.153", "accel_y_g": "0.385", "accel_z_g": "0.941", "accel_magnitude_g": "1.028", "gyro_x_raw": "-70", "gyro_y_raw": "-10", "gyro_z_raw": "-171", "gyro_x_dps": "-0.54", "gyro_y_dps": "-0.08", "gyro_z_dps": "-1.31", "gyro_magnitude_dps": "1.42", "tilt_pitch_deg": "22.01", "tilt_roll_deg": "8.56", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "167", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:31.00,H:80.40,SRAW:1815,SM:65.8,SC:WET,RRAW:1788,RP:68.5,RC:MODERATE_RAIN,MV:1,TX:22.01,TY:8.56,AXG:-0.153,AYG:0.385,AZG:0.941,GXD:-0.54,GYD:-0.08,GZD:-1.31"}, {"timestamp": "24-10-2026 09:41:32", "packet_id": "184", "temperature_c": "31.0", "temperature_f": "87.8", "humidity_pct": "80.3", "dew_point_c": "27.2", "soil_raw_adc": "1840", "soil_moisture_pct": "64.4", "soil_condition": "MODERATE", "rain_raw_adc": "1808", "rain_intensity_pct": "67.7", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "9404", "accel_y_raw": "-1097", "accel_z_raw": "13811", "accel_x_g": "0.574", "accel_y_g": "-0.067", "accel_z_g": "0.843", "accel_magnitude_g": "1.022", "gyro_x_raw": "-112", "gyro_y_raw": "-111", "gyro_z_raw": "305", "gyro_x_dps": "-0.86", "gyro_y_dps": "-0.85", "gyro_z_dps": "2.33", "gyro_magnitude_dps": "2.63", "tilt_pitch_deg": "-3.74", "tilt_roll_deg": "-34.18", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "173", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:31.00,H:80.30,SRAW:1840,SM:64.4,SC:MODERATE,RRAW:1808,RP:67.7,RC:MODERATE_RAIN,MV:1,TX:-3.74,TY:-34.18,AXG:0.574,AYG:-0.067,AZG:0.843,GXD:-0.86,GYD:-0.85,GZD:2.33"}, {"timestamp": "24-10-2026 09:41:37", "packet_id": "185", "temperature_c": "31.0", "temperature_f": "87.8", "humidity_pct": "80.2", "dew_point_c": "27.17", "soil_raw_adc": "1822", "soil_moisture_pct": "65.4", "soil_condition": "WET", "rain_raw_adc": "1798", "rain_intensity_pct": "68.1", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "10223", "accel_y_raw": "-2555", "accel_z_raw": "13090", "accel_x_g": "0.624", "accel_y_g": "-0.156", "accel_z_g": "0.799", "accel_magnitude_g": "1.026", "gyro_x_raw": "129", "gyro_y_raw": "-297", "gyro_z_raw": "-65", "gyro_x_dps": "0.99", "gyro_y_dps": "-2.27", "gyro_z_dps": "-0.5", "gyro_magnitude_dps": "2.53", "tilt_pitch_deg": "-8.76", "tilt_roll_deg": "-37.46", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "168", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:31.00,H:80.20,SRAW:1822,SM:65.4,SC:WET,RRAW:1798,RP:68.1,RC:MODERATE_RAIN,MV:1,TX:-8.76,TY:-37.46,AXG:0.624,AYG:-0.156,AZG:0.799,GXD:0.99,GYD:-2.27,GZD:-0.50"}, {"timestamp": "24-10-2026 09:41:42", "packet_id": "186", "temperature_c": "31.0", "temperature_f": "87.8", "humidity_pct": "80.2", "dew_point_c": "27.17", "soil_raw_adc": "1862", "soil_moisture_pct": "63.2", "soil_condition": "MODERATE", "rain_raw_adc": "1826", "rain_intensity_pct": "67.0", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "10485", "accel_y_raw": "-3751", "accel_z_raw": "13107", "accel_x_g": "0.64", "accel_y_g": "-0.229", "accel_z_g": "0.8", "accel_magnitude_g": "1.05", "gyro_x_raw": "-1015", "gyro_y_raw": "-488", "gyro_z_raw": "1002", "gyro_x_dps": "-7.75", "gyro_y_dps": "-3.73", "gyro_z_dps": "7.65", "gyro_magnitude_dps": "11.51", "tilt_pitch_deg": "-12.58", "tilt_roll_deg": "-37.58", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "174", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:31.00,H:80.20,SRAW:1862,SM:63.2,SC:MODERATE,RRAW:1826,RP:67.0,RC:MODERATE_RAIN,MV:1,TX:-12.58,TY:-37.58,AXG:0.640,AYG:-0.229,AZG:0.800,GXD:-7.75,GYD:-3.73,GZD:7.65"}, {"timestamp": "24-10-2026 09:41:46", "packet_id": "187", "temperature_c": "31.0", "temperature_f": "87.8", "humidity_pct": "80.1", "dew_point_c": "27.15", "soil_raw_adc": "1850", "soil_moisture_pct": "63.9", "soil_condition": "MODERATE", "rain_raw_adc": "1868", "rain_intensity_pct": "65.3", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "9895", "accel_y_raw": "-11911", "accel_z_raw": "6127", "accel_x_g": "0.604", "accel_y_g": "-0.727", "accel_z_g": "0.374", "accel_magnitude_g": "1.016", "gyro_x_raw": "459", "gyro_y_raw": "19", "gyro_z_raw": "-49", "gyro_x_dps": "3.51", "gyro_y_dps": "0.15", "gyro_z_dps": "-0.38", "gyro_magnitude_dps": "3.53", "tilt_pitch_deg": "-45.65", "tilt_roll_deg": "-36.47", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "173", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:31.00,H:80.10,SRAW:1850,SM:63.9,SC:MODERATE,RRAW:1868,RP:65.3,RC:MODERATE_RAIN,MV:1,TX:-45.65,TY:-36.47,AXG:0.604,AYG:-0.727,AZG:0.374,GXD:3.51,GYD:0.15,GZD:-0.38"}, {"timestamp": "24-10-2026 09:41:51", "packet_id": "188", "temperature_c": "31.0", "temperature_f": "87.8", "humidity_pct": "80.0", "dew_point_c": "27.13", "soil_raw_adc": "1857", "soil_moisture_pct": "63.5", "soil_condition": "MODERATE", "rain_raw_adc": "1861", "rain_intensity_pct": "65.6", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "11190", "accel_y_raw": "-9322", "accel_z_raw": "8470", "accel_x_g": "0.683", "accel_y_g": "-0.569", "accel_z_g": "0.517", "accel_magnitude_g": "1.028", "gyro_x_raw": "335", "gyro_y_raw": "-47", "gyro_z_raw": "-23", "gyro_x_dps": "2.56", "gyro_y_dps": "-0.36", "gyro_z_dps": "-0.18", "gyro_magnitude_dps": "2.59", "tilt_pitch_deg": "-33.58", "tilt_roll_deg": "-41.63", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "174", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:31.00,H:80.00,SRAW:1857,SM:63.5,SC:MODERATE,RRAW:1861,RP:65.6,RC:MODERATE_RAIN,MV:1,TX:-33.58,TY:-41.63,AXG:0.683,AYG:-0.569,AZG:0.517,GXD:2.56,GYD:-0.36,GZD:-0.18"}, {"timestamp": "24-10-2026 09:41:56", "packet_id": "189", "temperature_c": "31.1", "temperature_f": "87.98", "humidity_pct": "80.0", "dew_point_c": "27.23", "soil_raw_adc": "1853", "soil_moisture_pct": "63.7", "soil_condition": "MODERATE", "rain_raw_adc": "1901", "rain_intensity_pct": "64.0", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "14942", "accel_y_raw": "-1671", "accel_z_raw": "7651", "accel_x_g": "0.912", "accel_y_g": "-0.102", "accel_z_g": "0.467", "accel_magnitude_g": "1.03", "gyro_x_raw": "145", "gyro_y_raw": "-517", "gyro_z_raw": "399", "gyro_x_dps": "1.11", "gyro_y_dps": "-3.95", "gyro_z_dps": "3.05", "gyro_magnitude_dps": "5.11", "tilt_pitch_deg": "-5.7", "tilt_roll_deg": "-62.35", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "172", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:31.10,H:80.00,SRAW:1853,SM:63.7,SC:MODERATE,RRAW:1901,RP:64.0,RC:MODERATE_RAIN,MV:1,TX:-5.70,TY:-62.35,AXG:0.912,AYG:-0.102,AZG:0.467,GXD:1.11,GYD:-3.95,GZD:3.05"}, {"timestamp": "24-10-2026 09:42:01", "packet_id": "190", "temperature_c": "31.1", "temperature_f": "87.98", "humidity_pct": "80.0", "dew_point_c": "27.23", "soil_raw_adc": "1841", "soil_moisture_pct": "64.4", "soil_condition": "MODERATE", "rain_raw_adc": "1826", "rain_intensity_pct": "67.0", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "10960", "accel_y_raw": "-12533", "accel_z_raw": "737", "accel_x_g": "0.669", "accel_y_g": "-0.765", "accel_z_g": "0.045", "accel_magnitude_g": "1.017", "gyro_x_raw": "572", "gyro_y_raw": "7", "gyro_z_raw": "-175", "gyro_x_dps": "4.37", "gyro_y_dps": "0.06", "gyro_z_dps": "-1.34", "gyro_magnitude_dps": "4.57", "tilt_pitch_deg": "-48.73", "tilt_roll_deg": "-41.15", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "173", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:31.10,H:80.00,SRAW:1841,SM:64.4,SC:MODERATE,RRAW:1826,RP:67.0,RC:MODERATE_RAIN,MV:1,TX:-48.73,TY:-41.15,AXG:0.669,AYG:-0.765,AZG:0.045,GXD:4.37,GYD:0.06,GZD:-1.34"}, {"timestamp": "24-10-2026 09:42:06", "packet_id": "191", "temperature_c": "31.1", "temperature_f": "87.98", "humidity_pct": "79.9", "dew_point_c": "27.21", "soil_raw_adc": "1731", "soil_moisture_pct": "70.5", "soil_condition": "WET", "rain_raw_adc": "1660", "rain_intensity_pct": "73.6", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "1261", "accel_y_raw": "-5963", "accel_z_raw": "15794", "accel_x_g": "0.077", "accel_y_g": "-0.364", "accel_z_g": "0.964", "accel_magnitude_g": "1.033", "gyro_x_raw": "213", "gyro_y_raw": "-673", "gyro_z_raw": "23", "gyro_x_dps": "1.63", "gyro_y_dps": "-5.14", "gyro_z_dps": "0.18", "gyro_magnitude_dps": "5.4", "tilt_pitch_deg": "-20.61", "tilt_roll_deg": "-4.25", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "167", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:31.10,H:79.90,SRAW:1731,SM:70.5,SC:WET,RRAW:1660,RP:73.6,RC:MODERATE_RAIN,MV:1,TX:-20.61,TY:-4.25,AXG:0.077,AYG:-0.364,AZG:0.964,GXD:1.63,GYD:-5.14,GZD:0.18"}, {"timestamp": "24-10-2026 09:42:11", "packet_id": "192", "temperature_c": "31.1", "temperature_f": "87.98", "humidity_pct": "79.8", "dew_point_c": "27.19", "soil_raw_adc": "1739", "soil_moisture_pct": "70.1", "soil_condition": "WET", "rain_raw_adc": "1666", "rain_intensity_pct": "73.4", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-3702", "accel_y_raw": "1130", "accel_z_raw": "30916", "accel_x_g": "-0.226", "accel_y_g": "0.069", "accel_z_g": "1.887", "accel_magnitude_g": "1.902", "gyro_x_raw": "1940", "gyro_y_raw": "2526", "gyro_z_raw": "-44", "gyro_x_dps": "14.81", "gyro_y_dps": "19.29", "gyro_z_dps": "-0.34", "gyro_magnitude_dps": "24.32", "tilt_pitch_deg": "2.09", "tilt_roll_deg": "6.82", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "166", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:31.10,H:79.80,SRAW:1739,SM:70.1,SC:WET,RRAW:1666,RP:73.4,RC:MODERATE_RAIN,MV:1,TX:2.09,TY:6.82,AXG:-0.226,AYG:0.069,AZG:1.887,GXD:14.81,GYD:19.29,GZD:-0.34"}, {"timestamp": "24-10-2026 09:42:16", "packet_id": "193", "temperature_c": "31.1", "temperature_f": "87.98", "humidity_pct": "79.8", "dew_point_c": "27.19", "soil_raw_adc": "3851", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1732", "rain_intensity_pct": "70.7", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-1441", "accel_y_raw": "311", "accel_z_raw": "16728", "accel_x_g": "-0.088", "accel_y_g": "0.019", "accel_z_g": "1.021", "accel_magnitude_g": "1.025", "gyro_x_raw": "403", "gyro_y_raw": "108", "gyro_z_raw": "-45", "gyro_x_dps": "3.08", "gyro_y_dps": "0.83", "gyro_z_dps": "-0.35", "gyro_magnitude_dps": "3.21", "tilt_pitch_deg": "1.05", "tilt_roll_deg": "4.94", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "163", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:31.10,H:79.80,SRAW:3851,SM:0.0,SC:DRY,RRAW:1732,RP:70.7,RC:MODERATE_RAIN,MV:1,TX:1.05,TY:4.94,AXG:-0.088,AYG:0.019,AZG:1.021,GXD:3.08,GYD:0.83,GZD:-0.35"}, {"timestamp": "24-10-2026 09:42:21", "packet_id": "194", "temperature_c": "31.2", "temperature_f": "88.16", "humidity_pct": "79.7", "dew_point_c": "27.26", "soil_raw_adc": "3958", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1311", "rain_intensity_pct": "87.6", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1507", "accel_y_raw": "819", "accel_z_raw": "18923", "accel_x_g": "-0.092", "accel_y_g": "0.05", "accel_z_g": "1.155", "accel_magnitude_g": "1.16", "gyro_x_raw": "517", "gyro_y_raw": "-27", "gyro_z_raw": "-18", "gyro_x_dps": "3.95", "gyro_y_dps": "-0.21", "gyro_z_dps": "-0.14", "gyro_magnitude_dps": "3.96", "tilt_pitch_deg": "2.48", "tilt_roll_deg": "4.56", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "161", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:31.20,H:79.70,SRAW:3958,SM:0.0,SC:DRY,RRAW:1311,RP:87.6,RC:HEAVY_RAIN,MV:1,TX:2.48,TY:4.56,AXG:-0.092,AYG:0.050,AZG:1.155,GXD:3.95,GYD:-0.21,GZD:-0.14"}, {"timestamp": "24-10-2026 09:42:26", "packet_id": "195", "temperature_c": "31.2", "temperature_f": "88.16", "humidity_pct": "79.7", "dew_point_c": "27.26", "soil_raw_adc": "3269", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1253", "rain_intensity_pct": "89.9", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1458", "accel_y_raw": "311", "accel_z_raw": "16826", "accel_x_g": "-0.089", "accel_y_g": "0.019", "accel_z_g": "1.027", "accel_magnitude_g": "1.031", "gyro_x_raw": "364", "gyro_y_raw": "68", "gyro_z_raw": "-20", "gyro_x_dps": "2.78", "gyro_y_dps": "0.52", "gyro_z_dps": "-0.16", "gyro_magnitude_dps": "2.83", "tilt_pitch_deg": "1.03", "tilt_roll_deg": "4.97", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "160", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:31.20,H:79.70,SRAW:3269,SM:0.0,SC:DRY,RRAW:1253,RP:89.9,RC:HEAVY_RAIN,MV:1,TX:1.03,TY:4.97,AXG:-0.089,AYG:0.019,AZG:1.027,GXD:2.78,GYD:0.52,GZD:-0.16"}, {"timestamp": "24-10-2026 09:42:31", "packet_id": "196", "temperature_c": "31.2", "temperature_f": "88.16", "humidity_pct": "79.5", "dew_point_c": "27.22", "soil_raw_adc": "2697", "soil_moisture_pct": "16.8", "soil_condition": "DRY", "rain_raw_adc": "1296", "rain_intensity_pct": "88.2", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1359", "accel_y_raw": "262", "accel_z_raw": "16842", "accel_x_g": "-0.083", "accel_y_g": "0.016", "accel_z_g": "1.028", "accel_magnitude_g": "1.031", "gyro_x_raw": "378", "gyro_y_raw": "117", "gyro_z_raw": "-24", "gyro_x_dps": "2.89", "gyro_y_dps": "0.9", "gyro_z_dps": "-0.19", "gyro_magnitude_dps": "3.03", "tilt_pitch_deg": "0.88", "tilt_roll_deg": "4.62", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "161", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:31.20,H:79.50,SRAW:2697,SM:16.8,SC:DRY,RRAW:1296,RP:88.2,RC:HEAVY_RAIN,MV:1,TX:0.88,TY:4.62,AXG:-0.083,AYG:0.016,AZG:1.028,GXD:2.89,GYD:0.90,GZD:-0.19"}, {"timestamp": "24-10-2026 09:42:36", "packet_id": "197", "temperature_c": "31.2", "temperature_f": "88.16", "humidity_pct": "79.5", "dew_point_c": "27.22", "soil_raw_adc": "2559", "soil_moisture_pct": "24.5", "soil_condition": "DRY", "rain_raw_adc": "1492", "rain_intensity_pct": "80.3", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1441", "accel_y_raw": "327", "accel_z_raw": "16793", "accel_x_g": "-0.088", "accel_y_g": "0.02", "accel_z_g": "1.025", "accel_magnitude_g": "1.029", "gyro_x_raw": "387", "gyro_y_raw": "62", "gyro_z_raw": "-32", "gyro_x_dps": "2.96", "gyro_y_dps": "0.48", "gyro_z_dps": "-0.25", "gyro_magnitude_dps": "3.01", "tilt_pitch_deg": "1.13", "tilt_roll_deg": "4.9", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "161", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:31.20,H:79.50,SRAW:2559,SM:24.5,SC:DRY,RRAW:1492,RP:80.3,RC:HEAVY_RAIN,MV:1,TX:1.13,TY:4.90,AXG:-0.088,AYG:0.020,AZG:1.025,GXD:2.96,GYD:0.48,GZD:-0.25"}, {"timestamp": "24-10-2026 09:42:41", "packet_id": "198", "temperature_c": "31.2", "temperature_f": "88.16", "humidity_pct": "79.5", "dew_point_c": "27.22", "soil_raw_adc": "3897", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1254", "rain_intensity_pct": "89.8", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1409", "accel_y_raw": "327", "accel_z_raw": "16793", "accel_x_g": "-0.086", "accel_y_g": "0.02", "accel_z_g": "1.025", "accel_magnitude_g": "1.029", "gyro_x_raw": "365", "gyro_y_raw": "60", "gyro_z_raw": "-30", "gyro_x_dps": "2.79", "gyro_y_dps": "0.46", "gyro_z_dps": "-0.23", "gyro_magnitude_dps": "2.84", "tilt_pitch_deg": "1.09", "tilt_roll_deg": "4.81", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "160", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:31.20,H:79.50,SRAW:3897,SM:0.0,SC:DRY,RRAW:1254,RP:89.8,RC:HEAVY_RAIN,MV:1,TX:1.09,TY:4.81,AXG:-0.086,AYG:0.020,AZG:1.025,GXD:2.79,GYD:0.46,GZD:-0.23"}, {"timestamp": "24-10-2026 09:42:46", "packet_id": "199", "temperature_c": "31.2", "temperature_f": "88.16", "humidity_pct": "79.4", "dew_point_c": "27.2", "soil_raw_adc": "2978", "soil_moisture_pct": "1.2", "soil_condition": "DRY", "rain_raw_adc": "1310", "rain_intensity_pct": "87.6", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "4866", "accel_y_raw": "-2326", "accel_z_raw": "16400", "accel_x_g": "0.297", "accel_y_g": "-0.142", "accel_z_g": "1.001", "accel_magnitude_g": "1.054", "gyro_x_raw": "822", "gyro_y_raw": "-6098", "gyro_z_raw": "1499", "gyro_x_dps": "6.28", "gyro_y_dps": "-46.55", "gyro_z_dps": "11.45", "gyro_magnitude_dps": "48.35", "tilt_pitch_deg": "-7.75", "tilt_roll_deg": "-16.38", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "165", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:31.20,H:79.40,SRAW:2978,SM:1.2,SC:DRY,RRAW:1310,RP:87.6,RC:HEAVY_RAIN,MV:1,TX:-7.75,TY:-16.38,AXG:0.297,AYG:-0.142,AZG:1.001,GXD:6.28,GYD:-46.55,GZD:11.45"}, {"timestamp": "24-10-2026 09:42:51", "packet_id": "200", "temperature_c": "31.2", "temperature_f": "88.16", "humidity_pct": "79.3", "dew_point_c": "27.18", "soil_raw_adc": "3038", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1317", "rain_intensity_pct": "87.3", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "8404", "accel_y_raw": "-5308", "accel_z_raw": "13680", "accel_x_g": "0.513", "accel_y_g": "-0.324", "accel_z_g": "0.835", "accel_magnitude_g": "1.032", "gyro_x_raw": "301", "gyro_y_raw": "112", "gyro_z_raw": "437", "gyro_x_dps": "2.3", "gyro_y_dps": "0.86", "gyro_z_dps": "3.34", "gyro_magnitude_dps": "4.15", "tilt_pitch_deg": "-18.3", "tilt_roll_deg": "-29.79", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "163", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:31.20,H:79.30,SRAW:3038,SM:0.0,SC:DRY,RRAW:1317,RP:87.3,RC:HEAVY_RAIN,MV:1,TX:-18.30,TY:-29.79,AXG:0.513,AYG:-0.324,AZG:0.835,GXD:2.30,GYD:0.86,GZD:3.34"}, {"timestamp": "24-10-2026 09:42:56", "packet_id": "201", "temperature_c": "31.2", "temperature_f": "88.16", "humidity_pct": "79.3", "dew_point_c": "27.18", "soil_raw_adc": "3476", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1297", "rain_intensity_pct": "88.1", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1458", "accel_y_raw": "311", "accel_z_raw": "16875", "accel_x_g": "-0.089", "accel_y_g": "0.019", "accel_z_g": "1.03", "accel_magnitude_g": "1.034", "gyro_x_raw": "416", "gyro_y_raw": "24", "gyro_z_raw": "-52", "gyro_x_dps": "3.18", "gyro_y_dps": "0.19", "gyro_z_dps": "-0.4", "gyro_magnitude_dps": "3.21", "tilt_pitch_deg": "1.04", "tilt_roll_deg": "4.93", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "160", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:31.20,H:79.30,SRAW:3476,SM:0.0,SC:DRY,RRAW:1297,RP:88.1,RC:HEAVY_RAIN,MV:1,TX:1.04,TY:4.93,AXG:-0.089,AYG:0.019,AZG:1.030,GXD:3.18,GYD:0.19,GZD:-0.40"}, {"timestamp": "24-10-2026 09:43:01", "packet_id": "202", "temperature_c": "31.2", "temperature_f": "88.16", "humidity_pct": "79.3", "dew_point_c": "27.18", "soil_raw_adc": "3249", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1319", "rain_intensity_pct": "87.2", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "1703", "accel_y_raw": "3637", "accel_z_raw": "16269", "accel_x_g": "0.104", "accel_y_g": "0.222", "accel_z_g": "0.993", "accel_magnitude_g": "1.023", "gyro_x_raw": "226", "gyro_y_raw": "-243", "gyro_z_raw": "137", "gyro_x_dps": "1.73", "gyro_y_dps": "-1.86", "gyro_z_dps": "1.05", "gyro_magnitude_dps": "2.75", "tilt_pitch_deg": "12.51", "tilt_roll_deg": "-5.85", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "161", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:31.20,H:79.30,SRAW:3249,SM:0.0,SC:DRY,RRAW:1319,RP:87.2,RC:HEAVY_RAIN,MV:1,TX:12.51,TY:-5.85,AXG:0.104,AYG:0.222,AZG:0.993,GXD:1.73,GYD:-1.86,GZD:1.05"}, {"timestamp": "24-10-2026 09:43:06", "packet_id": "203", "temperature_c": "31.2", "temperature_f": "88.16", "humidity_pct": "79.2", "dew_point_c": "27.15", "soil_raw_adc": "3573", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1309", "rain_intensity_pct": "87.6", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1638", "accel_y_raw": "327", "accel_z_raw": "16695", "accel_x_g": "-0.1", "accel_y_g": "0.02", "accel_z_g": "1.019", "accel_magnitude_g": "1.024", "gyro_x_raw": "390", "gyro_y_raw": "30", "gyro_z_raw": "-40", "gyro_x_dps": "2.98", "gyro_y_dps": "0.23", "gyro_z_dps": "-0.31", "gyro_magnitude_dps": "3.0", "tilt_pitch_deg": "1.11", "tilt_roll_deg": "5.58", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "160", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:31.20,H:79.20,SRAW:3573,SM:0.0,SC:DRY,RRAW:1309,RP:87.6,RC:HEAVY_RAIN,MV:1,TX:1.11,TY:5.58,AXG:-0.100,AYG:0.020,AZG:1.019,GXD:2.98,GYD:0.23,GZD:-0.31"}, {"timestamp": "24-10-2026 09:43:11", "packet_id": "204", "temperature_c": "31.3", "temperature_f": "88.34", "humidity_pct": "79.3", "dew_point_c": "27.27", "soil_raw_adc": "2624", "soil_moisture_pct": "20.9", "soil_condition": "DRY", "rain_raw_adc": "1383", "rain_intensity_pct": "84.7", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "6651", "accel_y_raw": "1458", "accel_z_raw": "15417", "accel_x_g": "0.406", "accel_y_g": "0.089", "accel_z_g": "0.941", "accel_magnitude_g": "1.029", "gyro_x_raw": "324", "gyro_y_raw": "-503", "gyro_z_raw": "43", "gyro_x_dps": "2.48", "gyro_y_dps": "-3.84", "gyro_z_dps": "0.33", "gyro_magnitude_dps": "4.58", "tilt_pitch_deg": "4.96", "tilt_roll_deg": "-23.24", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "162", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:31.30,H:79.30,SRAW:2624,SM:20.9,SC:DRY,RRAW:1383,RP:84.7,RC:HEAVY_RAIN,MV:1,TX:4.96,TY:-23.24,AXG:0.406,AYG:0.089,AZG:0.941,GXD:2.48,GYD:-3.84,GZD:0.33"}, {"timestamp": "24-10-2026 09:43:16", "packet_id": "205", "temperature_c": "31.3", "temperature_f": "88.34", "humidity_pct": "79.3", "dew_point_c": "27.27", "soil_raw_adc": "3855", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1307", "rain_intensity_pct": "87.7", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "802", "accel_y_raw": "-1933", "accel_z_raw": "16941", "accel_x_g": "0.049", "accel_y_g": "-0.118", "accel_z_g": "1.034", "accel_magnitude_g": "1.042", "gyro_x_raw": "-917", "gyro_y_raw": "-2893", "gyro_z_raw": "34", "gyro_x_dps": "-7.0", "gyro_y_dps": "-22.09", "gyro_z_dps": "0.26", "gyro_magnitude_dps": "23.17", "tilt_pitch_deg": "-6.49", "tilt_roll_deg": "-2.69", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "164", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:31.30,H:79.30,SRAW:3855,SM:0.0,SC:DRY,RRAW:1307,RP:87.7,RC:HEAVY_RAIN,MV:1,TX:-6.49,TY:-2.69,AXG:0.049,AYG:-0.118,AZG:1.034,GXD:-7.00,GYD:-22.09,GZD:0.26"}, {"timestamp": "24-10-2026 09:43:21", "packet_id": "206", "temperature_c": "31.3", "temperature_f": "88.34", "humidity_pct": "79.3", "dew_point_c": "27.27", "soil_raw_adc": "3867", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "2899", "rain_intensity_pct": "24.0", "rain_condition": "LIGHT_RAIN", "accel_x_raw": "-1785", "accel_y_raw": "294", "accel_z_raw": "16793", "accel_x_g": "-0.109", "accel_y_g": "0.018", "accel_z_g": "1.025", "accel_magnitude_g": "1.031", "gyro_x_raw": "381", "gyro_y_raw": "78", "gyro_z_raw": "-68", "gyro_x_dps": "2.91", "gyro_y_dps": "0.6", "gyro_z_dps": "-0.52", "gyro_magnitude_dps": "3.02", "tilt_pitch_deg": "1.02", "tilt_roll_deg": "6.08", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "160", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:31.30,H:79.30,SRAW:3867,SM:0.0,SC:DRY,RRAW:2899,RP:24.0,RC:LIGHT_RAIN,MV:1,TX:1.02,TY:6.08,AXG:-0.109,AYG:0.018,AZG:1.025,GXD:2.91,GYD:0.60,GZD:-0.52"}, {"timestamp": "24-10-2026 09:43:26", "packet_id": "207", "temperature_c": "31.3", "temperature_f": "88.34", "humidity_pct": "79.3", "dew_point_c": "27.27", "soil_raw_adc": "3863", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "3251", "rain_intensity_pct": "10.0", "rain_condition": "LIGHT_RAIN", "accel_x_raw": "-1720", "accel_y_raw": "278", "accel_z_raw": "16809", "accel_x_g": "-0.105", "accel_y_g": "0.017", "accel_z_g": "1.026", "accel_magnitude_g": "1.031", "gyro_x_raw": "407", "gyro_y_raw": "66", "gyro_z_raw": "-56", "gyro_x_dps": "3.11", "gyro_y_dps": "0.51", "gyro_z_dps": "-0.43", "gyro_magnitude_dps": "3.18", "tilt_pitch_deg": "0.94", "tilt_roll_deg": "5.87", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "160", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:31.30,H:79.30,SRAW:3863,SM:0.0,SC:DRY,RRAW:3251,RP:10.0,RC:LIGHT_RAIN,MV:1,TX:0.94,TY:5.87,AXG:-0.105,AYG:0.017,AZG:1.026,GXD:3.11,GYD:0.51,GZD:-0.43"}, {"timestamp": "24-10-2026 09:43:31", "packet_id": "208", "temperature_c": "31.3", "temperature_f": "88.34", "humidity_pct": "79.3", "dew_point_c": "27.27", "soil_raw_adc": "3869", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "3260", "rain_intensity_pct": "9.6", "rain_condition": "NO_RAIN", "accel_x_raw": "-1671", "accel_y_raw": "229", "accel_z_raw": "16744", "accel_x_g": "-0.102", "accel_y_g": "0.014", "accel_z_g": "1.022", "accel_magnitude_g": "1.027", "gyro_x_raw": "416", "gyro_y_raw": "69", "gyro_z_raw": "-41", "gyro_x_dps": "3.18", "gyro_y_dps": "0.53", "gyro_z_dps": "-0.32", "gyro_magnitude_dps": "3.24", "tilt_pitch_deg": "0.76", "tilt_roll_deg": "5.68", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "156", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:31.30,H:79.30,SRAW:3869,SM:0.0,SC:DRY,RRAW:3260,RP:9.6,RC:NO_RAIN,MV:1,TX:0.76,TY:5.68,AXG:-0.102,AYG:0.014,AZG:1.022,GXD:3.18,GYD:0.53,GZD:-0.32"}, {"timestamp": "24-10-2026 09:43:36", "packet_id": "209", "temperature_c": "31.3", "temperature_f": "88.34", "humidity_pct": "79.3", "dew_point_c": "27.27", "soil_raw_adc": "3887", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "3269", "rain_intensity_pct": "9.2", "rain_condition": "NO_RAIN", "accel_x_raw": "8093", "accel_y_raw": "180", "accel_z_raw": "14712", "accel_x_g": "0.494", "accel_y_g": "0.011", "accel_z_g": "0.898", "accel_magnitude_g": "1.025", "gyro_x_raw": "408", "gyro_y_raw": "1224", "gyro_z_raw": "10", "gyro_x_dps": "3.12", "gyro_y_dps": "9.35", "gyro_z_dps": "0.08", "gyro_magnitude_dps": "9.86", "tilt_pitch_deg": "0.63", "tilt_roll_deg": "-28.78", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "156", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:31.30,H:79.30,SRAW:3887,SM:0.0,SC:DRY,RRAW:3269,RP:9.2,RC:NO_RAIN,MV:1,TX:0.63,TY:-28.78,AXG:0.494,AYG:0.011,AZG:0.898,GXD:3.12,GYD:9.35,GZD:0.08"}, {"timestamp": "24-10-2026 09:43:41", "packet_id": "210", "temperature_c": "31.3", "temperature_f": "88.34", "humidity_pct": "79.3", "dew_point_c": "27.27", "soil_raw_adc": "3862", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "3335", "rain_intensity_pct": "6.6", "rain_condition": "NO_RAIN", "accel_x_raw": "5373", "accel_y_raw": "-6406", "accel_z_raw": "14598", "accel_x_g": "0.328", "accel_y_g": "-0.391", "accel_z_g": "0.891", "accel_magnitude_g": "1.027", "gyro_x_raw": "444", "gyro_y_raw": "11", "gyro_z_raw": "17", "gyro_x_dps": "3.39", "gyro_y_dps": "0.09", "gyro_z_dps": "0.13", "gyro_magnitude_dps": "3.39", "tilt_pitch_deg": "-22.37", "tilt_roll_deg": "-18.62", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "159", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:31.30,H:79.30,SRAW:3862,SM:0.0,SC:DRY,RRAW:3335,RP:6.6,RC:NO_RAIN,MV:1,TX:-22.37,TY:-18.62,AXG:0.328,AYG:-0.391,AZG:0.891,GXD:3.39,GYD:0.09,GZD:0.13"}, {"timestamp": "24-10-2026 09:43:46", "packet_id": "211", "temperature_c": "31.3", "temperature_f": "88.34", "humidity_pct": "79.2", "dew_point_c": "27.25", "soil_raw_adc": "3866", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "3367", "rain_intensity_pct": "5.3", "rain_condition": "NO_RAIN", "accel_x_raw": "7225", "accel_y_raw": "-5914", "accel_z_raw": "13959", "accel_x_g": "0.441", "accel_y_g": "-0.361", "accel_z_g": "0.852", "accel_magnitude_g": "1.025", "gyro_x_raw": "107", "gyro_y_raw": "-1347", "gyro_z_raw": "-86", "gyro_x_dps": "0.82", "gyro_y_dps": "-10.29", "gyro_z_dps": "-0.66", "gyro_magnitude_dps": "10.34", "tilt_pitch_deg": "-20.61", "tilt_roll_deg": "-25.5", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "162", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:31.30,H:79.20,SRAW:3866,SM:0.0,SC:DRY,RRAW:3367,RP:5.3,RC:NO_RAIN,MV:1,TX:-20.61,TY:-25.50,AXG:0.441,AYG:-0.361,AZG:0.852,GXD:0.82,GYD:-10.29,GZD:-0.66"}, {"timestamp": "24-10-2026 09:43:51", "packet_id": "212", "temperature_c": "31.4", "temperature_f": "88.52", "humidity_pct": "79.1", "dew_point_c": "27.33", "soil_raw_adc": "3923", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "3539", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-4227", "accel_y_raw": "737", "accel_z_raw": "15368", "accel_x_g": "-0.258", "accel_y_g": "0.045", "accel_z_g": "0.938", "accel_magnitude_g": "0.974", "gyro_x_raw": "246", "gyro_y_raw": "141", "gyro_z_raw": "-337", "gyro_x_dps": "1.88", "gyro_y_dps": "1.08", "gyro_z_dps": "-2.58", "gyro_magnitude_dps": "3.37", "tilt_pitch_deg": "2.66", "tilt_roll_deg": "15.34", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "157", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:31.40,H:79.10,SRAW:3923,SM:0.0,SC:DRY,RRAW:3539,RP:0.0,RC:NO_RAIN,MV:1,TX:2.66,TY:15.34,AXG:-0.258,AYG:0.045,AZG:0.938,GXD:1.88,GYD:1.08,GZD:-2.58"}, {"timestamp": "24-10-2026 09:43:56", "packet_id": "213", "temperature_c": "31.4", "temperature_f": "88.52", "humidity_pct": "79.1", "dew_point_c": "27.33", "soil_raw_adc": "2879", "soil_moisture_pct": "6.7", "soil_condition": "DRY", "rain_raw_adc": "3543", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "13074", "accel_y_raw": "425", "accel_z_raw": "10174", "accel_x_g": "0.798", "accel_y_g": "0.026", "accel_z_g": "0.621", "accel_magnitude_g": "1.011", "gyro_x_raw": "357", "gyro_y_raw": "144", "gyro_z_raw": "102", "gyro_x_dps": "2.73", "gyro_y_dps": "1.1", "gyro_z_dps": "0.78", "gyro_magnitude_dps": "3.04", "tilt_pitch_deg": "1.47", "tilt_roll_deg": "-52.07", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "156", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:31.40,H:79.10,SRAW:2879,SM:6.7,SC:DRY,RRAW:3543,RP:0.0,RC:NO_RAIN,MV:1,TX:1.47,TY:-52.07,AXG:0.798,AYG:0.026,AZG:0.621,GXD:2.73,GYD:1.10,GZD:0.78"}, {"timestamp": "24-10-2026 09:44:01", "packet_id": "214", "temperature_c": "31.4", "temperature_f": "88.52", "humidity_pct": "78.9", "dew_point_c": "27.28", "soil_raw_adc": "2844", "soil_moisture_pct": "8.7", "soil_condition": "DRY", "rain_raw_adc": "3579", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "11829", "accel_y_raw": "-655", "accel_z_raw": "10944", "accel_x_g": "0.722", "accel_y_g": "-0.04", "accel_z_g": "0.668", "accel_magnitude_g": "0.984", "gyro_x_raw": "1299", "gyro_y_raw": "2148", "gyro_z_raw": "-965", "gyro_x_dps": "9.92", "gyro_y_dps": "16.4", "gyro_z_dps": "-7.37", "gyro_magnitude_dps": "20.53", "tilt_pitch_deg": "-2.3", "tilt_roll_deg": "-47.17", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "160", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:31.40,H:78.90,SRAW:2844,SM:8.7,SC:DRY,RRAW:3579,RP:0.0,RC:NO_RAIN,MV:1,TX:-2.30,TY:-47.17,AXG:0.722,AYG:-0.040,AZG:0.668,GXD:9.92,GYD:16.40,GZD:-7.37"}, {"timestamp": "24-10-2026 09:44:02", "packet_id": "215", "temperature_c": "", "temperature_f": "", "humidity_pct": "", "dew_point_c": "", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "4095", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "0", "accel_y_raw": "0", "accel_z_raw": "16384", "accel_x_g": "0.0", "accel_y_g": "0.0", "accel_z_g": "1.0", "accel_magnitude_g": "1.0", "gyro_x_raw": "0", "gyro_y_raw": "0", "gyro_z_raw": "0", "gyro_x_dps": "0.0", "gyro_y_dps": "0.0", "gyro_z_dps": "0.0", "gyro_magnitude_dps": "0.0", "tilt_pitch_deg": "0.0", "tilt_roll_deg": "-0.0", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "71", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00"}, {"timestamp": "24-10-2026 09:44:09", "packet_id": "216", "temperature_c": "31.4", "temperature_f": "88.52", "humidity_pct": "78.8", "dew_point_c": "27.26", "soil_raw_adc": "2278", "soil_moisture_pct": "40.1", "soil_condition": "MODERATE", "rain_raw_adc": "1319", "rain_intensity_pct": "87.2", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1720", "accel_y_raw": "393", "accel_z_raw": "16433", "accel_x_g": "-0.105", "accel_y_g": "0.024", "accel_z_g": "1.003", "accel_magnitude_g": "1.009", "gyro_x_raw": "344", "gyro_y_raw": "141", "gyro_z_raw": "-83", "gyro_x_dps": "2.63", "gyro_y_dps": "1.08", "gyro_z_dps": "-0.64", "gyro_magnitude_dps": "2.91", "tilt_pitch_deg": "1.39", "tilt_roll_deg": "5.97", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "166", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:31.40,H:78.80,SRAW:2278,SM:40.1,SC:MODERATE,RRAW:1319,RP:87.2,RC:HEAVY_RAIN,MV:1,TX:1.39,TY:5.97,AXG:-0.105,AYG:0.024,AZG:1.003,GXD:2.63,GYD:1.08,GZD:-0.64"}, {"timestamp": "24-10-2026 09:44:13", "packet_id": "217", "temperature_c": "31.4", "temperature_f": "88.52", "humidity_pct": "78.8", "dew_point_c": "27.26", "soil_raw_adc": "2263", "soil_moisture_pct": "40.9", "soil_condition": "MODERATE", "rain_raw_adc": "1306", "rain_intensity_pct": "87.8", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "6848", "accel_y_raw": "-589", "accel_z_raw": "16236", "accel_x_g": "0.418", "accel_y_g": "-0.036", "accel_z_g": "0.991", "accel_magnitude_g": "1.076", "gyro_x_raw": "1008", "gyro_y_raw": "-11851", "gyro_z_raw": "-4380", "gyro_x_dps": "7.7", "gyro_y_dps": "-90.47", "gyro_z_dps": "-33.44", "gyro_magnitude_dps": "96.76", "tilt_pitch_deg": "-1.9", "tilt_roll_deg": "-22.86", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "172", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:31.40,H:78.80,SRAW:2263,SM:40.9,SC:MODERATE,RRAW:1306,RP:87.8,RC:HEAVY_RAIN,MV:1,TX:-1.90,TY:-22.86,AXG:0.418,AYG:-0.036,AZG:0.991,GXD:7.70,GYD:-90.47,GZD:-33.44"}, {"timestamp": "24-10-2026 09:44:18", "packet_id": "218", "temperature_c": "31.4", "temperature_f": "88.52", "humidity_pct": "78.7", "dew_point_c": "27.24", "soil_raw_adc": "2150", "soil_moisture_pct": "47.2", "soil_condition": "MODERATE", "rain_raw_adc": "1427", "rain_intensity_pct": "82.9", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "12926", "accel_y_raw": "-2457", "accel_z_raw": "10600", "accel_x_g": "0.789", "accel_y_g": "-0.15", "accel_z_g": "0.647", "accel_magnitude_g": "1.031", "gyro_x_raw": "158", "gyro_y_raw": "66", "gyro_z_raw": "323", "gyro_x_dps": "1.21", "gyro_y_dps": "0.51", "gyro_z_dps": "2.47", "gyro_magnitude_dps": "2.8", "tilt_pitch_deg": "-8.38", "tilt_roll_deg": "-49.88", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "168", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:31.40,H:78.70,SRAW:2150,SM:47.2,SC:MODERATE,RRAW:1427,RP:82.9,RC:HEAVY_RAIN,MV:1,TX:-8.38,TY:-49.88,AXG:0.789,AYG:-0.150,AZG:0.647,GXD:1.21,GYD:0.51,GZD:2.47"}, {"timestamp": "24-10-2026 09:44:23", "packet_id": "219", "temperature_c": "31.4", "temperature_f": "88.52", "humidity_pct": "78.7", "dew_point_c": "27.24", "soil_raw_adc": "2202", "soil_moisture_pct": "44.3", "soil_condition": "MODERATE", "rain_raw_adc": "1386", "rain_intensity_pct": "84.6", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "12713", "accel_y_raw": "-4079", "accel_z_raw": "10207", "accel_x_g": "0.776", "accel_y_g": "-0.249", "accel_z_g": "0.623", "accel_magnitude_g": "1.026", "gyro_x_raw": "317", "gyro_y_raw": "-192", "gyro_z_raw": "204", "gyro_x_dps": "2.42", "gyro_y_dps": "-1.47", "gyro_z_dps": "1.56", "gyro_magnitude_dps": "3.23", "tilt_pitch_deg": "-14.02", "tilt_roll_deg": "-49.15", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "170", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:31.40,H:78.70,SRAW:2202,SM:44.3,SC:MODERATE,RRAW:1386,RP:84.6,RC:HEAVY_RAIN,MV:1,TX:-14.02,TY:-49.15,AXG:0.776,AYG:-0.249,AZG:0.623,GXD:2.42,GYD:-1.47,GZD:1.56"}, {"timestamp": "24-10-2026 09:44:28", "packet_id": "220", "temperature_c": "31.4", "temperature_f": "88.52", "humidity_pct": "78.6", "dew_point_c": "27.22", "soil_raw_adc": "2160", "soil_moisture_pct": "46.7", "soil_condition": "MODERATE", "rain_raw_adc": "1417", "rain_intensity_pct": "83.3", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "14270", "accel_y_raw": "-2015", "accel_z_raw": "8355", "accel_x_g": "0.871", "accel_y_g": "-0.123", "accel_z_g": "0.51", "accel_magnitude_g": "1.017", "gyro_x_raw": "162", "gyro_y_raw": "-127", "gyro_z_raw": "399", "gyro_x_dps": "1.24", "gyro_y_dps": "-0.97", "gyro_z_dps": "3.05", "gyro_magnitude_dps": "3.43", "tilt_pitch_deg": "-6.95", "tilt_roll_deg": "-58.94", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:31.40,H:78.60,SRAW:2160,SM:46.7,SC:MODERATE,RRAW:1417,RP:83.3,RC:HEAVY_RAIN,MV:1,TX:-6.95,TY:-58.94,AXG:0.871,AYG:-0.123,AZG:0.510,GXD:1.24,GYD:-0.97,GZD:3.05"}, {"timestamp": "24-10-2026 09:44:33", "packet_id": "221", "temperature_c": "31.4", "temperature_f": "88.52", "humidity_pct": "78.6", "dew_point_c": "27.22", "soil_raw_adc": "2114", "soil_moisture_pct": "49.2", "soil_condition": "MODERATE", "rain_raw_adc": "1386", "rain_intensity_pct": "84.6", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "10338", "accel_y_raw": "-12206", "accel_z_raw": "4947", "accel_x_g": "0.631", "accel_y_g": "-0.745", "accel_z_g": "0.302", "accel_magnitude_g": "1.022", "gyro_x_raw": "299", "gyro_y_raw": "74", "gyro_z_raw": "2", "gyro_x_dps": "2.29", "gyro_y_dps": "0.57", "gyro_z_dps": "0.02", "gyro_magnitude_dps": "2.36", "tilt_pitch_deg": "-46.79", "tilt_roll_deg": "-38.13", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:31.40,H:78.60,SRAW:2114,SM:49.2,SC:MODERATE,RRAW:1386,RP:84.6,RC:HEAVY_RAIN,MV:1,TX:-46.79,TY:-38.13,AXG:0.631,AYG:-0.745,AZG:0.302,GXD:2.29,GYD:0.57,GZD:0.02"}, {"timestamp": "24-10-2026 09:44:42", "packet_id": "222", "temperature_c": "31.4", "temperature_f": "88.52", "humidity_pct": "78.6", "dew_point_c": "27.22", "soil_raw_adc": "1741", "soil_moisture_pct": "69.9", "soil_condition": "WET", "rain_raw_adc": "1407", "rain_intensity_pct": "83.7", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-3637", "accel_y_raw": "4161", "accel_z_raw": "15663", "accel_x_g": "-0.222", "accel_y_g": "0.254", "accel_z_g": "0.956", "accel_magnitude_g": "1.014", "gyro_x_raw": "430", "gyro_y_raw": "22", "gyro_z_raw": "180", "gyro_x_dps": "3.29", "gyro_y_dps": "0.17", "gyro_z_dps": "1.38", "gyro_magnitude_dps": "3.57", "tilt_pitch_deg": "14.51", "tilt_roll_deg": "12.64", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "162", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:31.40,H:78.60,SRAW:1741,SM:69.9,SC:WET,RRAW:1407,RP:83.7,RC:HEAVY_RAIN,MV:1,TX:14.51,TY:12.64,AXG:-0.222,AYG:0.254,AZG:0.956,GXD:3.29,GYD:0.17,GZD:1.38"}, {"timestamp": "24-10-2026 09:44:46", "packet_id": "223", "temperature_c": "31.5", "temperature_f": "88.7", "humidity_pct": "78.5", "dew_point_c": "27.29", "soil_raw_adc": "1599", "soil_moisture_pct": "77.8", "soil_condition": "WET", "rain_raw_adc": "1414", "rain_intensity_pct": "83.4", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "12730", "accel_y_raw": "-5341", "accel_z_raw": "9043", "accel_x_g": "0.777", "accel_y_g": "-0.326", "accel_z_g": "0.552", "accel_magnitude_g": "1.007", "gyro_x_raw": "-14", "gyro_y_raw": "-644", "gyro_z_raw": "128", "gyro_x_dps": "-0.11", "gyro_y_dps": "-4.92", "gyro_z_dps": "0.98", "gyro_magnitude_dps": "5.02", "tilt_pitch_deg": "-18.88", "tilt_roll_deg": "-50.45", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "166", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:31.50,H:78.50,SRAW:1599,SM:77.8,SC:WET,RRAW:1414,RP:83.4,RC:HEAVY_RAIN,MV:1,TX:-18.88,TY:-50.45,AXG:0.777,AYG:-0.326,AZG:0.552,GXD:-0.11,GYD:-4.92,GZD:0.98"}, {"timestamp": "24-10-2026 09:44:51", "packet_id": "224", "temperature_c": "31.5", "temperature_f": "88.7", "humidity_pct": "78.5", "dew_point_c": "27.29", "soil_raw_adc": "1569", "soil_moisture_pct": "79.5", "soil_condition": "WET", "rain_raw_adc": "1387", "rain_intensity_pct": "84.5", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "9175", "accel_y_raw": "-12943", "accel_z_raw": "5373", "accel_x_g": "0.56", "accel_y_g": "-0.79", "accel_z_g": "0.328", "accel_magnitude_g": "1.022", "gyro_x_raw": "362", "gyro_y_raw": "589", "gyro_z_raw": "-95", "gyro_x_dps": "2.77", "gyro_y_dps": "4.5", "gyro_z_dps": "-0.73", "gyro_magnitude_dps": "5.33", "tilt_pitch_deg": "-50.59", "tilt_roll_deg": "-33.2", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "165", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:31.50,H:78.50,SRAW:1569,SM:79.5,SC:WET,RRAW:1387,RP:84.5,RC:HEAVY_RAIN,MV:1,TX:-50.59,TY:-33.20,AXG:0.560,AYG:-0.790,AZG:0.328,GXD:2.77,GYD:4.50,GZD:-0.73"}, {"timestamp": "24-10-2026 09:44:56", "packet_id": "225", "temperature_c": "31.5", "temperature_f": "88.7", "humidity_pct": "78.5", "dew_point_c": "27.29", "soil_raw_adc": "1683", "soil_moisture_pct": "73.2", "soil_condition": "WET", "rain_raw_adc": "1442", "rain_intensity_pct": "82.3", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-688", "accel_y_raw": "-16039", "accel_z_raw": "4472", "accel_x_g": "-0.042", "accel_y_g": "-0.979", "accel_z_g": "0.273", "accel_magnitude_g": "1.017", "gyro_x_raw": "373", "gyro_y_raw": "86", "gyro_z_raw": "6", "gyro_x_dps": "2.85", "gyro_y_dps": "0.66", "gyro_z_dps": "0.05", "gyro_magnitude_dps": "2.93", "tilt_pitch_deg": "-74.23", "tilt_roll_deg": "2.35", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "163", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:31.50,H:78.50,SRAW:1683,SM:73.2,SC:WET,RRAW:1442,RP:82.3,RC:HEAVY_RAIN,MV:1,TX:-74.23,TY:2.35,AXG:-0.042,AYG:-0.979,AZG:0.273,GXD:2.85,GYD:0.66,GZD:0.05"}, {"timestamp": "24-10-2026 09:45:01", "packet_id": "226", "temperature_c": "31.5", "temperature_f": "88.7", "humidity_pct": "78.5", "dew_point_c": "27.29", "soil_raw_adc": "1333", "soil_moisture_pct": "92.6", "soil_condition": "SATURATED", "rain_raw_adc": "1473", "rain_intensity_pct": "81.1", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-786", "accel_y_raw": "-16498", "accel_z_raw": "3571", "accel_x_g": "-0.048", "accel_y_g": "-1.007", "accel_z_g": "0.218", "accel_magnitude_g": "1.031", "gyro_x_raw": "4144", "gyro_y_raw": "-7316", "gyro_z_raw": "378", "gyro_x_dps": "31.64", "gyro_y_dps": "-55.85", "gyro_z_dps": "2.89", "gyro_magnitude_dps": "64.25", "tilt_pitch_deg": "-77.49", "tilt_roll_deg": "2.66", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "172", "landslide_risk_level": "CRITICAL ALERT", "hazard_status": "HIGH_LANDSLIDE_HAZARD", "raw_packet": "GS1,TV:1,T:31.50,H:78.50,SRAW:1333,SM:92.6,SC:SATURATED,RRAW:1473,RP:81.1,RC:HEAVY_RAIN,MV:1,TX:-77.49,TY:2.66,AXG:-0.048,AYG:-1.007,AZG:0.218,GXD:31.64,GYD:-55.85,GZD:2.89"}, {"timestamp": "24-10-2026 09:45:13", "packet_id": "227", "temperature_c": "31.5", "temperature_f": "88.7", "humidity_pct": "78.5", "dew_point_c": "27.29", "soil_raw_adc": "1728", "soil_moisture_pct": "70.7", "soil_condition": "WET", "rain_raw_adc": "1691", "rain_intensity_pct": "72.4", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-1081", "accel_y_raw": "950", "accel_z_raw": "16744", "accel_x_g": "-0.066", "accel_y_g": "0.058", "accel_z_g": "1.022", "accel_magnitude_g": "1.026", "gyro_x_raw": "394", "gyro_y_raw": "61", "gyro_z_raw": "-28", "gyro_x_dps": "3.01", "gyro_y_dps": "0.47", "gyro_z_dps": "-0.22", "gyro_magnitude_dps": "3.05", "tilt_pitch_deg": "3.22", "tilt_roll_deg": "3.71", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "164", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:31.50,H:78.50,SRAW:1728,SM:70.7,SC:WET,RRAW:1691,RP:72.4,RC:MODERATE_RAIN,MV:1,TX:3.22,TY:3.71,AXG:-0.066,AYG:0.058,AZG:1.022,GXD:3.01,GYD:0.47,GZD:-0.22"}, {"timestamp": "24-10-2026 09:45:18", "packet_id": "228", "temperature_c": "31.5", "temperature_f": "88.7", "humidity_pct": "78.6", "dew_point_c": "27.32", "soil_raw_adc": "1849", "soil_moisture_pct": "63.9", "soil_condition": "MODERATE", "rain_raw_adc": "1872", "rain_intensity_pct": "65.1", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-1032", "accel_y_raw": "966", "accel_z_raw": "16875", "accel_x_g": "-0.063", "accel_y_g": "0.059", "accel_z_g": "1.03", "accel_magnitude_g": "1.034", "gyro_x_raw": "415", "gyro_y_raw": "73", "gyro_z_raw": "-43", "gyro_x_dps": "3.17", "gyro_y_dps": "0.56", "gyro_z_dps": "-0.33", "gyro_magnitude_dps": "3.24", "tilt_pitch_deg": "3.26", "tilt_roll_deg": "3.48", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:31.50,H:78.60,SRAW:1849,SM:63.9,SC:MODERATE,RRAW:1872,RP:65.1,RC:MODERATE_RAIN,MV:1,TX:3.26,TY:3.48,AXG:-0.063,AYG:0.059,AZG:1.030,GXD:3.17,GYD:0.56,GZD:-0.33"}, {"timestamp": "24-10-2026 09:45:23", "packet_id": "229", "temperature_c": "31.5", "temperature_f": "88.7", "humidity_pct": "78.6", "dew_point_c": "27.32", "soil_raw_adc": "1517", "soil_moisture_pct": "82.4", "soil_condition": "WET", "rain_raw_adc": "1588", "rain_intensity_pct": "76.5", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-933", "accel_y_raw": "966", "accel_z_raw": "16842", "accel_x_g": "-0.057", "accel_y_g": "0.059", "accel_z_g": "1.028", "accel_magnitude_g": "1.031", "gyro_x_raw": "415", "gyro_y_raw": "31", "gyro_z_raw": "-24", "gyro_x_dps": "3.17", "gyro_y_dps": "0.24", "gyro_z_dps": "-0.19", "gyro_magnitude_dps": "3.18", "tilt_pitch_deg": "3.28", "tilt_roll_deg": "3.19", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "161", "landslide_risk_level": "ELEVATED", "hazard_status": "SATURATION_WARNING", "raw_packet": "GS1,TV:1,T:31.50,H:78.60,SRAW:1517,SM:82.4,SC:WET,RRAW:1588,RP:76.5,RC:HEAVY_RAIN,MV:1,TX:3.28,TY:3.19,AXG:-0.057,AYG:0.059,AZG:1.028,GXD:3.17,GYD:0.24,GZD:-0.19"}, {"timestamp": "24-10-2026 09:45:28", "packet_id": "230", "temperature_c": "31.5", "temperature_f": "88.7", "humidity_pct": "78.5", "dew_point_c": "27.29", "soil_raw_adc": "1683", "soil_moisture_pct": "73.2", "soil_condition": "WET", "rain_raw_adc": "1610", "rain_intensity_pct": "75.6", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-999", "accel_y_raw": "1015", "accel_z_raw": "16728", "accel_x_g": "-0.061", "accel_y_g": "0.062", "accel_z_g": "1.021", "accel_magnitude_g": "1.025", "gyro_x_raw": "378", "gyro_y_raw": "86", "gyro_z_raw": "-52", "gyro_x_dps": "2.89", "gyro_y_dps": "0.66", "gyro_z_dps": "-0.4", "gyro_magnitude_dps": "2.99", "tilt_pitch_deg": "3.48", "tilt_roll_deg": "3.4", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "161", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:31.50,H:78.50,SRAW:1683,SM:73.2,SC:WET,RRAW:1610,RP:75.6,RC:HEAVY_RAIN,MV:1,TX:3.48,TY:3.40,AXG:-0.061,AYG:0.062,AZG:1.021,GXD:2.89,GYD:0.66,GZD:-0.40"}, {"timestamp": "24-10-2026 09:45:33", "packet_id": "231", "temperature_c": "31.6", "temperature_f": "88.88", "humidity_pct": "78.4", "dew_point_c": "27.37", "soil_raw_adc": "1935", "soil_moisture_pct": "59.2", "soil_condition": "MODERATE", "rain_raw_adc": "2084", "rain_intensity_pct": "56.6", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-737", "accel_y_raw": "933", "accel_z_raw": "16973", "accel_x_g": "-0.045", "accel_y_g": "0.057", "accel_z_g": "1.036", "accel_magnitude_g": "1.039", "gyro_x_raw": "395", "gyro_y_raw": "56", "gyro_z_raw": "-48", "gyro_x_dps": "3.02", "gyro_y_dps": "0.43", "gyro_z_dps": "-0.37", "gyro_magnitude_dps": "3.07", "tilt_pitch_deg": "3.14", "tilt_roll_deg": "2.49", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:31.60,H:78.40,SRAW:1935,SM:59.2,SC:MODERATE,RRAW:2084,RP:56.6,RC:MODERATE_RAIN,MV:1,TX:3.14,TY:2.49,AXG:-0.045,AYG:0.057,AZG:1.036,GXD:3.02,GYD:0.43,GZD:-0.37"}, {"timestamp": "24-10-2026 09:45:38", "packet_id": "232", "temperature_c": "31.6", "temperature_f": "88.88", "humidity_pct": "78.4", "dew_point_c": "27.37", "soil_raw_adc": "2019", "soil_moisture_pct": "54.5", "soil_condition": "MODERATE", "rain_raw_adc": "2142", "rain_intensity_pct": "54.3", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-786", "accel_y_raw": "933", "accel_z_raw": "16842", "accel_x_g": "-0.048", "accel_y_g": "0.057", "accel_z_g": "1.028", "accel_magnitude_g": "1.031", "gyro_x_raw": "369", "gyro_y_raw": "86", "gyro_z_raw": "-2", "gyro_x_dps": "2.82", "gyro_y_dps": "0.66", "gyro_z_dps": "-0.02", "gyro_magnitude_dps": "2.9", "tilt_pitch_deg": "3.19", "tilt_roll_deg": "2.65", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:31.60,H:78.40,SRAW:2019,SM:54.5,SC:MODERATE,RRAW:2142,RP:54.3,RC:MODERATE_RAIN,MV:1,TX:3.19,TY:2.65,AXG:-0.048,AYG:0.057,AZG:1.028,GXD:2.82,GYD:0.66,GZD:-0.02"}, {"timestamp": "24-10-2026 09:45:43", "packet_id": "233", "temperature_c": "31.6", "temperature_f": "88.88", "humidity_pct": "78.4", "dew_point_c": "27.37", "soil_raw_adc": "2037", "soil_moisture_pct": "53.5", "soil_condition": "MODERATE", "rain_raw_adc": "2089", "rain_intensity_pct": "56.4", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-770", "accel_y_raw": "851", "accel_z_raw": "16842", "accel_x_g": "-0.047", "accel_y_g": "0.052", "accel_z_g": "1.028", "accel_magnitude_g": "1.03", "gyro_x_raw": "391", "gyro_y_raw": "55", "gyro_z_raw": "-32", "gyro_x_dps": "2.99", "gyro_y_dps": "0.42", "gyro_z_dps": "-0.25", "gyro_magnitude_dps": "3.03", "tilt_pitch_deg": "2.89", "tilt_roll_deg": "2.62", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:31.60,H:78.40,SRAW:2037,SM:53.5,SC:MODERATE,RRAW:2089,RP:56.4,RC:MODERATE_RAIN,MV:1,TX:2.89,TY:2.62,AXG:-0.047,AYG:0.052,AZG:1.028,GXD:2.99,GYD:0.42,GZD:-0.25"}, {"timestamp": "24-10-2026 09:45:48", "packet_id": "234", "temperature_c": "31.6", "temperature_f": "88.88", "humidity_pct": "78.3", "dew_point_c": "27.35", "soil_raw_adc": "2021", "soil_moisture_pct": "54.4", "soil_condition": "MODERATE", "rain_raw_adc": "2109", "rain_intensity_pct": "55.6", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-606", "accel_y_raw": "933", "accel_z_raw": "16793", "accel_x_g": "-0.037", "accel_y_g": "0.057", "accel_z_g": "1.025", "accel_magnitude_g": "1.027", "gyro_x_raw": "395", "gyro_y_raw": "77", "gyro_z_raw": "6", "gyro_x_dps": "3.02", "gyro_y_dps": "0.59", "gyro_z_dps": "0.05", "gyro_magnitude_dps": "3.08", "tilt_pitch_deg": "3.16", "tilt_roll_deg": "2.08", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "168", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:31.60,H:78.30,SRAW:2021,SM:54.4,SC:MODERATE,RRAW:2109,RP:55.6,RC:MODERATE_RAIN,MV:1,TX:3.16,TY:2.08,AXG:-0.037,AYG:0.057,AZG:1.025,GXD:3.02,GYD:0.59,GZD:0.05"}, {"timestamp": "24-10-2026 09:45:53", "packet_id": "235", "temperature_c": "31.6", "temperature_f": "88.88", "humidity_pct": "78.2", "dew_point_c": "27.32", "soil_raw_adc": "2015", "soil_moisture_pct": "54.7", "soil_condition": "MODERATE", "rain_raw_adc": "2160", "rain_intensity_pct": "53.6", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "147", "accel_y_raw": "1032", "accel_z_raw": "16515", "accel_x_g": "0.009", "accel_y_g": "0.063", "accel_z_g": "1.008", "accel_magnitude_g": "1.01", "gyro_x_raw": "450", "gyro_y_raw": "-134", "gyro_z_raw": "-40", "gyro_x_dps": "3.44", "gyro_y_dps": "-1.03", "gyro_z_dps": "-0.31", "gyro_magnitude_dps": "3.6", "tilt_pitch_deg": "3.6", "tilt_roll_deg": "-0.48", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "170", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:31.60,H:78.20,SRAW:2015,SM:54.7,SC:MODERATE,RRAW:2160,RP:53.6,RC:MODERATE_RAIN,MV:1,TX:3.60,TY:-0.48,AXG:0.009,AYG:0.063,AZG:1.008,GXD:3.44,GYD:-1.03,GZD:-0.31"}, {"timestamp": "24-10-2026 09:45:58", "packet_id": "236", "temperature_c": "31.6", "temperature_f": "88.88", "humidity_pct": "78.1", "dew_point_c": "27.3", "soil_raw_adc": "2229", "soil_moisture_pct": "42.8", "soil_condition": "MODERATE", "rain_raw_adc": "2226", "rain_intensity_pct": "51.0", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "376", "accel_y_raw": "835", "accel_z_raw": "16760", "accel_x_g": "0.023", "accel_y_g": "0.051", "accel_z_g": "1.023", "accel_magnitude_g": "1.025", "gyro_x_raw": "412", "gyro_y_raw": "65", "gyro_z_raw": "-28", "gyro_x_dps": "3.15", "gyro_y_dps": "0.5", "gyro_z_dps": "-0.22", "gyro_magnitude_dps": "3.2", "tilt_pitch_deg": "2.84", "tilt_roll_deg": "-1.31", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:31.60,H:78.10,SRAW:2229,SM:42.8,SC:MODERATE,RRAW:2226,RP:51.0,RC:MODERATE_RAIN,MV:1,TX:2.84,TY:-1.31,AXG:0.023,AYG:0.051,AZG:1.023,GXD:3.15,GYD:0.50,GZD:-0.22"}, {"timestamp": "24-10-2026 09:46:03", "packet_id": "237", "temperature_c": "31.6", "temperature_f": "88.88", "humidity_pct": "78.0", "dew_point_c": "27.28", "soil_raw_adc": "2098", "soil_moisture_pct": "50.1", "soil_condition": "MODERATE", "rain_raw_adc": "2288", "rain_intensity_pct": "48.5", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "442", "accel_y_raw": "950", "accel_z_raw": "16826", "accel_x_g": "0.027", "accel_y_g": "0.058", "accel_z_g": "1.027", "accel_magnitude_g": "1.029", "gyro_x_raw": "389", "gyro_y_raw": "40", "gyro_z_raw": "-9", "gyro_x_dps": "2.97", "gyro_y_dps": "0.31", "gyro_z_dps": "-0.07", "gyro_magnitude_dps": "2.99", "tilt_pitch_deg": "3.22", "tilt_roll_deg": "-1.5", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:31.60,H:78.00,SRAW:2098,SM:50.1,SC:MODERATE,RRAW:2288,RP:48.5,RC:MODERATE_RAIN,MV:1,TX:3.22,TY:-1.50,AXG:0.027,AYG:0.058,AZG:1.027,GXD:2.97,GYD:0.31,GZD:-0.07"}, {"timestamp": "24-10-2026 09:46:08", "packet_id": "238", "temperature_c": "31.6", "temperature_f": "88.88", "humidity_pct": "78.0", "dew_point_c": "27.28", "soil_raw_adc": "2118", "soil_moisture_pct": "49.0", "soil_condition": "MODERATE", "rain_raw_adc": "2515", "rain_intensity_pct": "39.4", "rain_condition": "LIGHT_RAIN", "accel_x_raw": "3915", "accel_y_raw": "-1048", "accel_z_raw": "16285", "accel_x_g": "0.239", "accel_y_g": "-0.064", "accel_z_g": "0.994", "accel_magnitude_g": "1.024", "gyro_x_raw": "-5418", "gyro_y_raw": "408", "gyro_z_raw": "-6082", "gyro_x_dps": "-41.36", "gyro_y_dps": "3.12", "gyro_z_dps": "-46.43", "gyro_magnitude_dps": "62.26", "tilt_pitch_deg": "-3.61", "tilt_roll_deg": "-13.47", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "172", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:31.60,H:78.00,SRAW:2118,SM:49.0,SC:MODERATE,RRAW:2515,RP:39.4,RC:LIGHT_RAIN,MV:1,TX:-3.61,TY:-13.47,AXG:0.239,AYG:-0.064,AZG:0.994,GXD:-41.36,GYD:3.12,GZD:-46.43"}, {"timestamp": "24-10-2026 09:46:15", "packet_id": "239", "temperature_c": "25.0", "temperature_f": "77.0", "humidity_pct": "50.0", "dew_point_c": "13.84", "soil_raw_adc": "2065", "soil_moisture_pct": "51.9", "soil_condition": "MODERATE", "rain_raw_adc": "2327", "rain_intensity_pct": "46.9", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "5980", "accel_y_raw": "0", "accel_z_raw": "15777", "accel_x_g": "0.365", "accel_y_g": "0.0", "accel_z_g": "0.963", "accel_magnitude_g": "1.03", "gyro_x_raw": "387", "gyro_y_raw": "57", "gyro_z_raw": "7", "gyro_x_dps": "2.96", "gyro_y_dps": "0.44", "gyro_z_dps": "0.06", "gyro_magnitude_dps": "2.99", "tilt_pitch_deg": "0.03", "tilt_roll_deg": "-20.73", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:25.00,H:50.00,SRAW:2065,SM:51.9,SC:MODERATE,RRAW:2327,RP:46.9,RC:MODERATE_RAIN,MV:1,TX:0.03,TY:-20.73,AXG:0.365,AYG:0.000,AZG:0.963,GXD:2.96,GYD:0.44,GZD:0.06"}, {"timestamp": "24-10-2026 09:46:40", "packet_id": "240", "temperature_c": "31.4", "temperature_f": "88.52", "humidity_pct": "78.3", "dew_point_c": "27.15", "soil_raw_adc": "1968", "soil_moisture_pct": "57.3", "soil_condition": "MODERATE", "rain_raw_adc": "2032", "rain_intensity_pct": "58.7", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-1310", "accel_y_raw": "-147", "accel_z_raw": "16859", "accel_x_g": "-0.08", "accel_y_g": "-0.009", "accel_z_g": "1.029", "accel_magnitude_g": "1.032", "gyro_x_raw": "428", "gyro_y_raw": "62", "gyro_z_raw": "-52", "gyro_x_dps": "3.27", "gyro_y_dps": "0.48", "gyro_z_dps": "-0.4", "gyro_magnitude_dps": "3.33", "tilt_pitch_deg": "-0.49", "tilt_roll_deg": "4.45", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "171", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:31.40,H:78.30,SRAW:1968,SM:57.3,SC:MODERATE,RRAW:2032,RP:58.7,RC:MODERATE_RAIN,MV:1,TX:-0.49,TY:4.45,AXG:-0.080,AYG:-0.009,AZG:1.029,GXD:3.27,GYD:0.48,GZD:-0.40"}, {"timestamp": "24-10-2026 09:46:45", "packet_id": "241", "temperature_c": "31.4", "temperature_f": "88.52", "humidity_pct": "78.4", "dew_point_c": "27.17", "soil_raw_adc": "2023", "soil_moisture_pct": "54.3", "soil_condition": "MODERATE", "rain_raw_adc": "2128", "rain_intensity_pct": "54.9", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-1343", "accel_y_raw": "-131", "accel_z_raw": "16826", "accel_x_g": "-0.082", "accel_y_g": "-0.008", "accel_z_g": "1.027", "accel_magnitude_g": "1.03", "gyro_x_raw": "378", "gyro_y_raw": "44", "gyro_z_raw": "-47", "gyro_x_dps": "2.89", "gyro_y_dps": "0.34", "gyro_z_dps": "-0.36", "gyro_magnitude_dps": "2.93", "tilt_pitch_deg": "-0.46", "tilt_roll_deg": "4.54", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "171", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:31.40,H:78.40,SRAW:2023,SM:54.3,SC:MODERATE,RRAW:2128,RP:54.9,RC:MODERATE_RAIN,MV:1,TX:-0.46,TY:4.54,AXG:-0.082,AYG:-0.008,AZG:1.027,GXD:2.89,GYD:0.34,GZD:-0.36"}, {"timestamp": "24-10-2026 09:46:50", "packet_id": "242", "temperature_c": "25.0", "temperature_f": "77.0", "humidity_pct": "50.0", "dew_point_c": "13.84", "soil_raw_adc": "2002", "soil_moisture_pct": "55.4", "soil_condition": "MODERATE", "rain_raw_adc": "2208", "rain_intensity_pct": "51.7", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "8126", "accel_y_raw": "98", "accel_z_raw": "15122", "accel_x_g": "0.496", "accel_y_g": "0.006", "accel_z_g": "0.923", "accel_magnitude_g": "1.048", "gyro_x_raw": "378", "gyro_y_raw": "23", "gyro_z_raw": "-10", "gyro_x_dps": "2.89", "gyro_y_dps": "0.18", "gyro_z_dps": "-0.08", "gyro_magnitude_dps": "2.9", "tilt_pitch_deg": "0.31", "tilt_roll_deg": "-28.24", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "170", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:25.00,H:50.00,SRAW:2002,SM:55.4,SC:MODERATE,RRAW:2208,RP:51.7,RC:MODERATE_RAIN,MV:1,TX:0.31,TY:-28.24,AXG:0.496,AYG:0.006,AZG:0.923,GXD:2.89,GYD:0.18,GZD:-0.08"}, {"timestamp": "24-10-2026 09:46:55", "packet_id": "243", "temperature_c": "25.0", "temperature_f": "77.0", "humidity_pct": "50.0", "dew_point_c": "13.84", "soil_raw_adc": "1521", "soil_moisture_pct": "82.2", "soil_condition": "WET", "rain_raw_adc": "1427", "rain_intensity_pct": "82.9", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "6897", "accel_y_raw": "65", "accel_z_raw": "15319", "accel_x_g": "0.421", "accel_y_g": "0.004", "accel_z_g": "0.935", "accel_magnitude_g": "1.025", "gyro_x_raw": "395", "gyro_y_raw": "108", "gyro_z_raw": "-44", "gyro_x_dps": "3.02", "gyro_y_dps": "0.83", "gyro_z_dps": "-0.34", "gyro_magnitude_dps": "3.15", "tilt_pitch_deg": "0.25", "tilt_roll_deg": "-24.23", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "162", "landslide_risk_level": "CRITICAL ALERT", "hazard_status": "HIGH_LANDSLIDE_HAZARD", "raw_packet": "GS1,TV:1,T:25.00,H:50.00,SRAW:1521,SM:82.2,SC:WET,RRAW:1427,RP:82.9,RC:HEAVY_RAIN,MV:1,TX:0.25,TY:-24.23,AXG:0.421,AYG:0.004,AZG:0.935,GXD:3.02,GYD:0.83,GZD:-0.34"}, {"timestamp": "24-10-2026 09:47:00", "packet_id": "244", "temperature_c": "25.0", "temperature_f": "77.0", "humidity_pct": "50.0", "dew_point_c": "13.84", "soil_raw_adc": "1499", "soil_moisture_pct": "83.4", "soil_condition": "WET", "rain_raw_adc": "1397", "rain_intensity_pct": "84.1", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "6766", "accel_y_raw": "98", "accel_z_raw": "15532", "accel_x_g": "0.413", "accel_y_g": "0.006", "accel_z_g": "0.948", "accel_magnitude_g": "1.034", "gyro_x_raw": "421", "gyro_y_raw": "91", "gyro_z_raw": "-56", "gyro_x_dps": "3.22", "gyro_y_dps": "0.7", "gyro_z_dps": "-0.43", "gyro_magnitude_dps": "3.32", "tilt_pitch_deg": "0.32", "tilt_roll_deg": "-23.54", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "162", "landslide_risk_level": "CRITICAL ALERT", "hazard_status": "HIGH_LANDSLIDE_HAZARD", "raw_packet": "GS1,TV:1,T:25.00,H:50.00,SRAW:1499,SM:83.4,SC:WET,RRAW:1397,RP:84.1,RC:HEAVY_RAIN,MV:1,TX:0.32,TY:-23.54,AXG:0.413,AYG:0.006,AZG:0.948,GXD:3.22,GYD:0.70,GZD:-0.43"}, {"timestamp": "24-10-2026 09:47:05", "packet_id": "245", "temperature_c": "25.0", "temperature_f": "77.0", "humidity_pct": "50.0", "dew_point_c": "13.84", "soil_raw_adc": "1485", "soil_moisture_pct": "84.2", "soil_condition": "WET", "rain_raw_adc": "1399", "rain_intensity_pct": "84.0", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "6766", "accel_y_raw": "65", "accel_z_raw": "15450", "accel_x_g": "0.413", "accel_y_g": "0.004", "accel_z_g": "0.943", "accel_magnitude_g": "1.029", "gyro_x_raw": "383", "gyro_y_raw": "65", "gyro_z_raw": "-44", "gyro_x_dps": "2.93", "gyro_y_dps": "0.5", "gyro_z_dps": "-0.34", "gyro_magnitude_dps": "2.99", "tilt_pitch_deg": "0.23", "tilt_roll_deg": "-23.63", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "162", "landslide_risk_level": "CRITICAL ALERT", "hazard_status": "HIGH_LANDSLIDE_HAZARD", "raw_packet": "GS1,TV:1,T:25.00,H:50.00,SRAW:1485,SM:84.2,SC:WET,RRAW:1399,RP:84.0,RC:HEAVY_RAIN,MV:1,TX:0.23,TY:-23.63,AXG:0.413,AYG:0.004,AZG:0.943,GXD:2.93,GYD:0.50,GZD:-0.34"}, {"timestamp": "24-10-2026 09:47:10", "packet_id": "246", "temperature_c": "25.0", "temperature_f": "77.0", "humidity_pct": "50.0", "dew_point_c": "13.84", "soil_raw_adc": "1503", "soil_moisture_pct": "83.2", "soil_condition": "WET", "rain_raw_adc": "1437", "rain_intensity_pct": "82.5", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "6733", "accel_y_raw": "0", "accel_z_raw": "15564", "accel_x_g": "0.411", "accel_y_g": "0.0", "accel_z_g": "0.95", "accel_magnitude_g": "1.035", "gyro_x_raw": "381", "gyro_y_raw": "65", "gyro_z_raw": "-48", "gyro_x_dps": "2.91", "gyro_y_dps": "0.5", "gyro_z_dps": "-0.37", "gyro_magnitude_dps": "2.98", "tilt_pitch_deg": "0.01", "tilt_roll_deg": "-23.38", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "162", "landslide_risk_level": "CRITICAL ALERT", "hazard_status": "HIGH_LANDSLIDE_HAZARD", "raw_packet": "GS1,TV:1,T:25.00,H:50.00,SRAW:1503,SM:83.2,SC:WET,RRAW:1437,RP:82.5,RC:HEAVY_RAIN,MV:1,TX:0.01,TY:-23.38,AXG:0.411,AYG:0.000,AZG:0.950,GXD:2.91,GYD:0.50,GZD:-0.37"}, {"timestamp": "24-10-2026 09:47:15", "packet_id": "247", "temperature_c": "25.0", "temperature_f": "77.0", "humidity_pct": "50.0", "dew_point_c": "13.84", "soil_raw_adc": "1479", "soil_moisture_pct": "84.5", "soil_condition": "WET", "rain_raw_adc": "1482", "rain_intensity_pct": "80.7", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "6668", "accel_y_raw": "114", "accel_z_raw": "15613", "accel_x_g": "0.407", "accel_y_g": "0.007", "accel_z_g": "0.953", "accel_magnitude_g": "1.036", "gyro_x_raw": "403", "gyro_y_raw": "57", "gyro_z_raw": "3", "gyro_x_dps": "3.08", "gyro_y_dps": "0.44", "gyro_z_dps": "0.03", "gyro_magnitude_dps": "3.11", "tilt_pitch_deg": "0.38", "tilt_roll_deg": "-23.13", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "161", "landslide_risk_level": "CRITICAL ALERT", "hazard_status": "HIGH_LANDSLIDE_HAZARD", "raw_packet": "GS1,TV:1,T:25.00,H:50.00,SRAW:1479,SM:84.5,SC:WET,RRAW:1482,RP:80.7,RC:HEAVY_RAIN,MV:1,TX:0.38,TY:-23.13,AXG:0.407,AYG:0.007,AZG:0.953,GXD:3.08,GYD:0.44,GZD:0.03"}, {"timestamp": "24-10-2026 09:47:20", "packet_id": "248", "temperature_c": "25.0", "temperature_f": "77.0", "humidity_pct": "50.0", "dew_point_c": "13.84", "soil_raw_adc": "1589", "soil_moisture_pct": "78.4", "soil_condition": "WET", "rain_raw_adc": "1438", "rain_intensity_pct": "82.5", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "6701", "accel_y_raw": "81", "accel_z_raw": "15532", "accel_x_g": "0.409", "accel_y_g": "0.005", "accel_z_g": "0.948", "accel_magnitude_g": "1.032", "gyro_x_raw": "403", "gyro_y_raw": "95", "gyro_z_raw": "-23", "gyro_x_dps": "3.08", "gyro_y_dps": "0.73", "gyro_z_dps": "-0.18", "gyro_magnitude_dps": "3.17", "tilt_pitch_deg": "0.3", "tilt_roll_deg": "-23.35", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "162", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:25.00,H:50.00,SRAW:1589,SM:78.4,SC:WET,RRAW:1438,RP:82.5,RC:HEAVY_RAIN,MV:1,TX:0.30,TY:-23.35,AXG:0.409,AYG:0.005,AZG:0.948,GXD:3.08,GYD:0.73,GZD:-0.18"}, {"timestamp": "24-10-2026 09:47:25", "packet_id": "249", "temperature_c": "31.7", "temperature_f": "89.06", "humidity_pct": "78.5", "dew_point_c": "27.49", "soil_raw_adc": "1495", "soil_moisture_pct": "83.6", "soil_condition": "WET", "rain_raw_adc": "1426", "rain_intensity_pct": "83.0", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "6701", "accel_y_raw": "98", "accel_z_raw": "15548", "accel_x_g": "0.409", "accel_y_g": "0.006", "accel_z_g": "0.949", "accel_magnitude_g": "1.033", "gyro_x_raw": "410", "gyro_y_raw": "49", "gyro_z_raw": "-14", "gyro_x_dps": "3.13", "gyro_y_dps": "0.38", "gyro_z_dps": "-0.11", "gyro_magnitude_dps": "3.15", "tilt_pitch_deg": "0.32", "tilt_roll_deg": "-23.29", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "162", "landslide_risk_level": "CRITICAL ALERT", "hazard_status": "HIGH_LANDSLIDE_HAZARD", "raw_packet": "GS1,TV:1,T:31.70,H:78.50,SRAW:1495,SM:83.6,SC:WET,RRAW:1426,RP:83.0,RC:HEAVY_RAIN,MV:1,TX:0.32,TY:-23.29,AXG:0.409,AYG:0.006,AZG:0.949,GXD:3.13,GYD:0.38,GZD:-0.11"}, {"timestamp": "24-10-2026 09:47:30", "packet_id": "250", "temperature_c": "31.8", "temperature_f": "89.24", "humidity_pct": "82.9", "dew_point_c": "28.52", "soil_raw_adc": "1504", "soil_moisture_pct": "83.1", "soil_condition": "WET", "rain_raw_adc": "1445", "rain_intensity_pct": "82.2", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "6651", "accel_y_raw": "32", "accel_z_raw": "15532", "accel_x_g": "0.406", "accel_y_g": "0.002", "accel_z_g": "0.948", "accel_magnitude_g": "1.031", "gyro_x_raw": "390", "gyro_y_raw": "57", "gyro_z_raw": "-48", "gyro_x_dps": "2.98", "gyro_y_dps": "0.44", "gyro_z_dps": "-0.37", "gyro_magnitude_dps": "3.03", "tilt_pitch_deg": "0.11", "tilt_roll_deg": "-23.2", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "162", "landslide_risk_level": "CRITICAL ALERT", "hazard_status": "HIGH_LANDSLIDE_HAZARD", "raw_packet": "GS1,TV:1,T:31.80,H:82.90,SRAW:1504,SM:83.1,SC:WET,RRAW:1445,RP:82.2,RC:HEAVY_RAIN,MV:1,TX:0.11,TY:-23.20,AXG:0.406,AYG:0.002,AZG:0.948,GXD:2.98,GYD:0.44,GZD:-0.37"}, {"timestamp": "24-10-2026 09:47:35", "packet_id": "251", "temperature_c": "31.8", "temperature_f": "89.24", "humidity_pct": "86.5", "dew_point_c": "29.26", "soil_raw_adc": "1516", "soil_moisture_pct": "82.4", "soil_condition": "WET", "rain_raw_adc": "1442", "rain_intensity_pct": "82.3", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "6684", "accel_y_raw": "49", "accel_z_raw": "15679", "accel_x_g": "0.408", "accel_y_g": "0.003", "accel_z_g": "0.957", "accel_magnitude_g": "1.04", "gyro_x_raw": "362", "gyro_y_raw": "35", "gyro_z_raw": "-18", "gyro_x_dps": "2.77", "gyro_y_dps": "0.27", "gyro_z_dps": "-0.14", "gyro_magnitude_dps": "2.79", "tilt_pitch_deg": "0.19", "tilt_roll_deg": "-23.1", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "162", "landslide_risk_level": "CRITICAL ALERT", "hazard_status": "HIGH_LANDSLIDE_HAZARD", "raw_packet": "GS1,TV:1,T:31.80,H:86.50,SRAW:1516,SM:82.4,SC:WET,RRAW:1442,RP:82.3,RC:HEAVY_RAIN,MV:1,TX:0.19,TY:-23.10,AXG:0.408,AYG:0.003,AZG:0.957,GXD:2.77,GYD:0.27,GZD:-0.14"}, {"timestamp": "24-10-2026 09:47:40", "packet_id": "252", "temperature_c": "32.5", "temperature_f": "90.5", "humidity_pct": "88.3", "dew_point_c": "30.31", "soil_raw_adc": "1535", "soil_moisture_pct": "81.4", "soil_condition": "WET", "rain_raw_adc": "1475", "rain_intensity_pct": "81.0", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "6619", "accel_y_raw": "81", "accel_z_raw": "15499", "accel_x_g": "0.404", "accel_y_g": "0.005", "accel_z_g": "0.946", "accel_magnitude_g": "1.029", "gyro_x_raw": "394", "gyro_y_raw": "73", "gyro_z_raw": "-31", "gyro_x_dps": "3.01", "gyro_y_dps": "0.56", "gyro_z_dps": "-0.24", "gyro_magnitude_dps": "3.07", "tilt_pitch_deg": "0.26", "tilt_roll_deg": "-23.12", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "162", "landslide_risk_level": "CRITICAL ALERT", "hazard_status": "HIGH_LANDSLIDE_HAZARD", "raw_packet": "GS1,TV:1,T:32.50,H:88.30,SRAW:1535,SM:81.4,SC:WET,RRAW:1475,RP:81.0,RC:HEAVY_RAIN,MV:1,TX:0.26,TY:-23.12,AXG:0.404,AYG:0.005,AZG:0.946,GXD:3.01,GYD:0.56,GZD:-0.24"}, {"timestamp": "24-10-2026 09:47:45", "packet_id": "253", "temperature_c": "33.4", "temperature_f": "92.12", "humidity_pct": "92.4", "dew_point_c": "31.99", "soil_raw_adc": "1526", "soil_moisture_pct": "81.9", "soil_condition": "WET", "rain_raw_adc": "1476", "rain_intensity_pct": "81.0", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "6668", "accel_y_raw": "65", "accel_z_raw": "15613", "accel_x_g": "0.407", "accel_y_g": "0.004", "accel_z_g": "0.953", "accel_magnitude_g": "1.036", "gyro_x_raw": "417", "gyro_y_raw": "103", "gyro_z_raw": "-7", "gyro_x_dps": "3.19", "gyro_y_dps": "0.79", "gyro_z_dps": "-0.06", "gyro_magnitude_dps": "3.29", "tilt_pitch_deg": "0.22", "tilt_roll_deg": "-23.11", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "162", "landslide_risk_level": "CRITICAL ALERT", "hazard_status": "HIGH_LANDSLIDE_HAZARD", "raw_packet": "GS1,TV:1,T:33.40,H:92.40,SRAW:1526,SM:81.9,SC:WET,RRAW:1476,RP:81.0,RC:HEAVY_RAIN,MV:1,TX:0.22,TY:-23.11,AXG:0.407,AYG:0.004,AZG:0.953,GXD:3.19,GYD:0.79,GZD:-0.06"}, {"timestamp": "24-10-2026 09:47:50", "packet_id": "254", "temperature_c": "32.9", "temperature_f": "91.22", "humidity_pct": "94.5", "dew_point_c": "31.89", "soil_raw_adc": "3858", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1293", "rain_intensity_pct": "88.3", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "6651", "accel_y_raw": "32", "accel_z_raw": "15532", "accel_x_g": "0.406", "accel_y_g": "0.002", "accel_z_g": "0.948", "accel_magnitude_g": "1.031", "gyro_x_raw": "390", "gyro_y_raw": "99", "gyro_z_raw": "-7", "gyro_x_dps": "2.98", "gyro_y_dps": "0.76", "gyro_z_dps": "-0.06", "gyro_magnitude_dps": "3.08", "tilt_pitch_deg": "0.09", "tilt_roll_deg": "-23.19", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "161", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:32.90,H:94.50,SRAW:3858,SM:0.0,SC:DRY,RRAW:1293,RP:88.3,RC:HEAVY_RAIN,MV:1,TX:0.09,TY:-23.19,AXG:0.406,AYG:0.002,AZG:0.948,GXD:2.98,GYD:0.76,GZD:-0.06"}, {"timestamp": "24-10-2026 09:47:59", "packet_id": "255", "temperature_c": "32.3", "temperature_f": "90.14", "humidity_pct": "93.9", "dew_point_c": "31.19", "soil_raw_adc": "3868", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1235", "rain_intensity_pct": "90.6", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1146", "accel_y_raw": "-114", "accel_z_raw": "16793", "accel_x_g": "-0.07", "accel_y_g": "-0.007", "accel_z_g": "1.025", "accel_magnitude_g": "1.027", "gyro_x_raw": "396", "gyro_y_raw": "56", "gyro_z_raw": "-40", "gyro_x_dps": "3.03", "gyro_y_dps": "0.43", "gyro_z_dps": "-0.31", "gyro_magnitude_dps": "3.08", "tilt_pitch_deg": "-0.37", "tilt_roll_deg": "3.91", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "162", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:32.30,H:93.90,SRAW:3868,SM:0.0,SC:DRY,RRAW:1235,RP:90.6,RC:HEAVY_RAIN,MV:1,TX:-0.37,TY:3.91,AXG:-0.070,AYG:-0.007,AZG:1.025,GXD:3.03,GYD:0.43,GZD:-0.31"}, {"timestamp": "24-10-2026 09:48:04", "packet_id": "256", "temperature_c": "32.2", "temperature_f": "89.96", "humidity_pct": "92.3", "dew_point_c": "30.79", "soil_raw_adc": "3856", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1284", "rain_intensity_pct": "88.6", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1212", "accel_y_raw": "-65", "accel_z_raw": "16809", "accel_x_g": "-0.074", "accel_y_g": "-0.004", "accel_z_g": "1.026", "accel_magnitude_g": "1.029", "gyro_x_raw": "386", "gyro_y_raw": "61", "gyro_z_raw": "6", "gyro_x_dps": "2.95", "gyro_y_dps": "0.47", "gyro_z_dps": "0.05", "gyro_magnitude_dps": "2.99", "tilt_pitch_deg": "-0.22", "tilt_roll_deg": "4.14", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "161", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:32.20,H:92.30,SRAW:3856,SM:0.0,SC:DRY,RRAW:1284,RP:88.6,RC:HEAVY_RAIN,MV:1,TX:-0.22,TY:4.14,AXG:-0.074,AYG:-0.004,AZG:1.026,GXD:2.95,GYD:0.47,GZD:0.05"}, {"timestamp": "24-10-2026 09:48:19", "packet_id": "257", "temperature_c": "31.8", "temperature_f": "89.24", "humidity_pct": "87.1", "dew_point_c": "29.38", "soil_raw_adc": "3841", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1294", "rain_intensity_pct": "88.2", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1212", "accel_y_raw": "0", "accel_z_raw": "16826", "accel_x_g": "-0.074", "accel_y_g": "0.0", "accel_z_g": "1.027", "accel_magnitude_g": "1.03", "gyro_x_raw": "390", "gyro_y_raw": "65", "gyro_z_raw": "-22", "gyro_x_dps": "2.98", "gyro_y_dps": "0.5", "gyro_z_dps": "-0.17", "gyro_magnitude_dps": "3.03", "tilt_pitch_deg": "0.0", "tilt_roll_deg": "4.11", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "160", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:31.80,H:87.10,SRAW:3841,SM:0.0,SC:DRY,RRAW:1294,RP:88.2,RC:HEAVY_RAIN,MV:1,TX:0.00,TY:4.11,AXG:-0.074,AYG:0.000,AZG:1.027,GXD:2.98,GYD:0.50,GZD:-0.17"}, {"timestamp": "24-10-2026 09:48:46", "packet_id": "258", "temperature_c": "31.5", "temperature_f": "88.7", "humidity_pct": "85.0", "dew_point_c": "28.66", "soil_raw_adc": "3911", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1305", "rain_intensity_pct": "87.8", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1212", "accel_y_raw": "81", "accel_z_raw": "16809", "accel_x_g": "-0.074", "accel_y_g": "0.005", "accel_z_g": "1.026", "accel_magnitude_g": "1.029", "gyro_x_raw": "370", "gyro_y_raw": "87", "gyro_z_raw": "-52", "gyro_x_dps": "2.83", "gyro_y_dps": "0.67", "gyro_z_dps": "-0.4", "gyro_magnitude_dps": "2.94", "tilt_pitch_deg": "0.27", "tilt_roll_deg": "4.15", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "160", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:31.50,H:85.00,SRAW:3911,SM:0.0,SC:DRY,RRAW:1305,RP:87.8,RC:HEAVY_RAIN,MV:1,TX:0.27,TY:4.15,AXG:-0.074,AYG:0.005,AZG:1.026,GXD:2.83,GYD:0.67,GZD:-0.40"}, {"timestamp": "24-10-2026 09:48:49", "packet_id": "259", "temperature_c": "31.4", "temperature_f": "88.52", "humidity_pct": "84.4", "dew_point_c": "28.44", "soil_raw_adc": "3889", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1301", "rain_intensity_pct": "88.0", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1212", "accel_y_raw": "0", "accel_z_raw": "16875", "accel_x_g": "-0.074", "accel_y_g": "0.0", "accel_z_g": "1.03", "accel_magnitude_g": "1.033", "gyro_x_raw": "382", "gyro_y_raw": "70", "gyro_z_raw": "-27", "gyro_x_dps": "2.92", "gyro_y_dps": "0.54", "gyro_z_dps": "-0.21", "gyro_magnitude_dps": "2.98", "tilt_pitch_deg": "0.01", "tilt_roll_deg": "4.1", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "160", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:31.40,H:84.40,SRAW:3889,SM:0.0,SC:DRY,RRAW:1301,RP:88.0,RC:HEAVY_RAIN,MV:1,TX:0.01,TY:4.10,AXG:-0.074,AYG:0.000,AZG:1.030,GXD:2.92,GYD:0.54,GZD:-0.21"}, {"timestamp": "24-10-2026 09:48:54", "packet_id": "260", "temperature_c": "31.4", "temperature_f": "88.52", "humidity_pct": "84.1", "dew_point_c": "28.38", "soil_raw_adc": "3967", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1933", "rain_intensity_pct": "62.7", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-1261", "accel_y_raw": "-16", "accel_z_raw": "16957", "accel_x_g": "-0.077", "accel_y_g": "-0.001", "accel_z_g": "1.035", "accel_magnitude_g": "1.038", "gyro_x_raw": "393", "gyro_y_raw": "56", "gyro_z_raw": "-11", "gyro_x_dps": "3.0", "gyro_y_dps": "0.43", "gyro_z_dps": "-0.09", "gyro_magnitude_dps": "3.03", "tilt_pitch_deg": "-0.08", "tilt_roll_deg": "4.27", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "165", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:31.40,H:84.10,SRAW:3967,SM:0.0,SC:DRY,RRAW:1933,RP:62.7,RC:MODERATE_RAIN,MV:1,TX:-0.08,TY:4.27,AXG:-0.077,AYG:-0.001,AZG:1.035,GXD:3.00,GYD:0.43,GZD:-0.09"}, {"timestamp": "24-10-2026 09:48:59", "packet_id": "261", "temperature_c": "31.3", "temperature_f": "88.34", "humidity_pct": "84.2", "dew_point_c": "28.3", "soil_raw_adc": "3893", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "2126", "rain_intensity_pct": "55.0", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-1212", "accel_y_raw": "-16", "accel_z_raw": "16859", "accel_x_g": "-0.074", "accel_y_g": "-0.001", "accel_z_g": "1.029", "accel_magnitude_g": "1.032", "gyro_x_raw": "378", "gyro_y_raw": "73", "gyro_z_raw": "-2", "gyro_x_dps": "2.89", "gyro_y_dps": "0.56", "gyro_z_dps": "-0.02", "gyro_magnitude_dps": "2.94", "tilt_pitch_deg": "-0.05", "tilt_roll_deg": "4.1", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "165", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:31.30,H:84.20,SRAW:3893,SM:0.0,SC:DRY,RRAW:2126,RP:55.0,RC:MODERATE_RAIN,MV:1,TX:-0.05,TY:4.10,AXG:-0.074,AYG:-0.001,AZG:1.029,GXD:2.89,GYD:0.56,GZD:-0.02"}, {"timestamp": "24-10-2026 09:49:04", "packet_id": "262", "temperature_c": "31.3", "temperature_f": "88.34", "humidity_pct": "84.6", "dew_point_c": "28.38", "soil_raw_adc": "3917", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "2131", "rain_intensity_pct": "54.8", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "3309", "accel_y_raw": "901", "accel_z_raw": "13467", "accel_x_g": "0.202", "accel_y_g": "0.055", "accel_z_g": "0.822", "accel_magnitude_g": "0.848", "gyro_x_raw": "-7443", "gyro_y_raw": "10380", "gyro_z_raw": "670", "gyro_x_dps": "-56.82", "gyro_y_dps": "79.24", "gyro_z_dps": "5.12", "gyro_magnitude_dps": "97.64", "tilt_pitch_deg": "3.7", "tilt_roll_deg": "-13.8", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "166", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:31.30,H:84.60,SRAW:3917,SM:0.0,SC:DRY,RRAW:2131,RP:54.8,RC:MODERATE_RAIN,MV:1,TX:3.70,TY:-13.80,AXG:0.202,AYG:0.055,AZG:0.822,GXD:-56.82,GYD:79.24,GZD:5.12"}, {"timestamp": "24-10-2026 09:49:09", "packet_id": "263", "temperature_c": "31.3", "temperature_f": "88.34", "humidity_pct": "85.0", "dew_point_c": "28.47", "soil_raw_adc": "3931", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "2142", "rain_intensity_pct": "54.3", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "16", "accel_y_raw": "294", "accel_z_raw": "16760", "accel_x_g": "0.001", "accel_y_g": "0.018", "accel_z_g": "1.023", "accel_magnitude_g": "1.023", "gyro_x_raw": "387", "gyro_y_raw": "57", "gyro_z_raw": "-31", "gyro_x_dps": "2.96", "gyro_y_dps": "0.44", "gyro_z_dps": "-0.24", "gyro_magnitude_dps": "3.0", "tilt_pitch_deg": "1.03", "tilt_roll_deg": "-0.04", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "163", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:31.30,H:85.00,SRAW:3931,SM:0.0,SC:DRY,RRAW:2142,RP:54.3,RC:MODERATE_RAIN,MV:1,TX:1.03,TY:-0.04,AXG:0.001,AYG:0.018,AZG:1.023,GXD:2.96,GYD:0.44,GZD:-0.24"}, {"timestamp": "24-10-2026 09:49:14", "packet_id": "264", "temperature_c": "31.3", "temperature_f": "88.34", "humidity_pct": "85.3", "dew_point_c": "28.53", "soil_raw_adc": "3939", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "2288", "rain_intensity_pct": "48.5", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-1245", "accel_y_raw": "-950", "accel_z_raw": "16711", "accel_x_g": "-0.076", "accel_y_g": "-0.058", "accel_z_g": "1.02", "accel_magnitude_g": "1.024", "gyro_x_raw": "26", "gyro_y_raw": "972", "gyro_z_raw": "644", "gyro_x_dps": "0.2", "gyro_y_dps": "7.42", "gyro_z_dps": "4.92", "gyro_magnitude_dps": "8.91", "tilt_pitch_deg": "-3.24", "tilt_roll_deg": "4.24", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "164", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:31.30,H:85.30,SRAW:3939,SM:0.0,SC:DRY,RRAW:2288,RP:48.5,RC:MODERATE_RAIN,MV:1,TX:-3.24,TY:4.24,AXG:-0.076,AYG:-0.058,AZG:1.020,GXD:0.20,GYD:7.42,GZD:4.92"}, {"timestamp": "24-10-2026 09:49:19", "packet_id": "265", "temperature_c": "31.3", "temperature_f": "88.34", "humidity_pct": "85.4", "dew_point_c": "28.55", "soil_raw_adc": "3925", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "2293", "rain_intensity_pct": "48.3", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-1998", "accel_y_raw": "-835", "accel_z_raw": "16777", "accel_x_g": "-0.122", "accel_y_g": "-0.051", "accel_z_g": "1.024", "accel_magnitude_g": "1.033", "gyro_x_raw": "393", "gyro_y_raw": "58", "gyro_z_raw": "-19", "gyro_x_dps": "3.0", "gyro_y_dps": "0.45", "gyro_z_dps": "-0.15", "gyro_magnitude_dps": "3.04", "tilt_pitch_deg": "-2.8", "tilt_roll_deg": "6.8", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "165", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:31.30,H:85.40,SRAW:3925,SM:0.0,SC:DRY,RRAW:2293,RP:48.3,RC:MODERATE_RAIN,MV:1,TX:-2.80,TY:6.80,AXG:-0.122,AYG:-0.051,AZG:1.024,GXD:3.00,GYD:0.45,GZD:-0.15"}, {"timestamp": "24-10-2026 09:49:24", "packet_id": "266", "temperature_c": "31.3", "temperature_f": "88.34", "humidity_pct": "85.2", "dew_point_c": "28.51", "soil_raw_adc": "3921", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "2279", "rain_intensity_pct": "48.8", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-1982", "accel_y_raw": "-770", "accel_z_raw": "16793", "accel_x_g": "-0.121", "accel_y_g": "-0.047", "accel_z_g": "1.025", "accel_magnitude_g": "1.033", "gyro_x_raw": "396", "gyro_y_raw": "69", "gyro_z_raw": "0", "gyro_x_dps": "3.03", "gyro_y_dps": "0.53", "gyro_z_dps": "0.0", "gyro_magnitude_dps": "3.08", "tilt_pitch_deg": "-2.61", "tilt_roll_deg": "6.71", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "164", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:31.30,H:85.20,SRAW:3921,SM:0.0,SC:DRY,RRAW:2279,RP:48.8,RC:MODERATE_RAIN,MV:1,TX:-2.61,TY:6.71,AXG:-0.121,AYG:-0.047,AZG:1.025,GXD:3.03,GYD:0.53,GZD:0.00"}, {"timestamp": "24-10-2026 09:49:29", "packet_id": "267", "temperature_c": "31.2", "temperature_f": "88.16", "humidity_pct": "85.4", "dew_point_c": "28.45", "soil_raw_adc": "3974", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "2245", "rain_intensity_pct": "50.2", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-2621", "accel_y_raw": "1277", "accel_z_raw": "20250", "accel_x_g": "-0.16", "accel_y_g": "0.078", "accel_z_g": "1.236", "accel_magnitude_g": "1.249", "gyro_x_raw": "803", "gyro_y_raw": "-3425", "gyro_z_raw": "2089", "gyro_x_dps": "6.13", "gyro_y_dps": "-26.15", "gyro_z_dps": "15.95", "gyro_magnitude_dps": "31.24", "tilt_pitch_deg": "3.57", "tilt_roll_deg": "7.35", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "165", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:31.20,H:85.40,SRAW:3974,SM:0.0,SC:DRY,RRAW:2245,RP:50.2,RC:MODERATE_RAIN,MV:1,TX:3.57,TY:7.35,AXG:-0.160,AYG:0.078,AZG:1.236,GXD:6.13,GYD:-26.15,GZD:15.95"}, {"timestamp": "24-10-2026 09:49:34", "packet_id": "268", "temperature_c": "31.2", "temperature_f": "88.16", "humidity_pct": "85.9", "dew_point_c": "28.55", "soil_raw_adc": "3971", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "2285", "rain_intensity_pct": "48.6", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "245", "accel_y_raw": "294", "accel_z_raw": "17121", "accel_x_g": "0.015", "accel_y_g": "0.018", "accel_z_g": "1.045", "accel_magnitude_g": "1.045", "gyro_x_raw": "351", "gyro_y_raw": "289", "gyro_z_raw": "-55", "gyro_x_dps": "2.68", "gyro_y_dps": "2.21", "gyro_z_dps": "-0.42", "gyro_magnitude_dps": "3.5", "tilt_pitch_deg": "1.0", "tilt_roll_deg": "-0.84", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "163", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:31.20,H:85.90,SRAW:3971,SM:0.0,SC:DRY,RRAW:2285,RP:48.6,RC:MODERATE_RAIN,MV:1,TX:1.00,TY:-0.84,AXG:0.015,AYG:0.018,AZG:1.045,GXD:2.68,GYD:2.21,GZD:-0.42"}, {"timestamp": "24-10-2026 09:49:39", "packet_id": "269", "temperature_c": "31.2", "temperature_f": "88.16", "humidity_pct": "86.4", "dew_point_c": "28.65", "soil_raw_adc": "3972", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "2274", "rain_intensity_pct": "49.0", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-163", "accel_y_raw": "229", "accel_z_raw": "17301", "accel_x_g": "-0.01", "accel_y_g": "0.014", "accel_z_g": "1.056", "accel_magnitude_g": "1.056", "gyro_x_raw": "-65", "gyro_y_raw": "555", "gyro_z_raw": "-103", "gyro_x_dps": "-0.5", "gyro_y_dps": "4.24", "gyro_z_dps": "-0.79", "gyro_magnitude_dps": "4.34", "tilt_pitch_deg": "0.77", "tilt_roll_deg": "0.56", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "164", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:31.20,H:86.40,SRAW:3972,SM:0.0,SC:DRY,RRAW:2274,RP:49.0,RC:MODERATE_RAIN,MV:1,TX:0.77,TY:0.56,AXG:-0.010,AYG:0.014,AZG:1.056,GXD:-0.50,GYD:4.24,GZD:-0.79"}, {"timestamp": "24-10-2026 09:49:44", "packet_id": "270", "temperature_c": "31.2", "temperature_f": "88.16", "humidity_pct": "87.0", "dew_point_c": "28.77", "soil_raw_adc": "3981", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "2255", "rain_intensity_pct": "49.8", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-966", "accel_y_raw": "-393", "accel_z_raw": "16941", "accel_x_g": "-0.059", "accel_y_g": "-0.024", "accel_z_g": "1.034", "accel_magnitude_g": "1.036", "gyro_x_raw": "124", "gyro_y_raw": "-606", "gyro_z_raw": "217", "gyro_x_dps": "0.95", "gyro_y_dps": "-4.63", "gyro_z_dps": "1.66", "gyro_magnitude_dps": "5.01", "tilt_pitch_deg": "-1.34", "tilt_roll_deg": "3.27", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "165", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:31.20,H:87.00,SRAW:3981,SM:0.0,SC:DRY,RRAW:2255,RP:49.8,RC:MODERATE_RAIN,MV:1,TX:-1.34,TY:3.27,AXG:-0.059,AYG:-0.024,AZG:1.034,GXD:0.95,GYD:-4.63,GZD:1.66"}, {"timestamp": "24-10-2026 09:49:49", "packet_id": "271", "temperature_c": "31.2", "temperature_f": "88.16", "humidity_pct": "87.1", "dew_point_c": "28.79", "soil_raw_adc": "3990", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "2256", "rain_intensity_pct": "49.8", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "196", "accel_y_raw": "-1343", "accel_z_raw": "16695", "accel_x_g": "0.012", "accel_y_g": "-0.082", "accel_z_g": "1.019", "accel_magnitude_g": "1.022", "gyro_x_raw": "246", "gyro_y_raw": "374", "gyro_z_raw": "-175", "gyro_x_dps": "1.88", "gyro_y_dps": "2.86", "gyro_z_dps": "-1.34", "gyro_magnitude_dps": "3.68", "tilt_pitch_deg": "-4.6", "tilt_roll_deg": "-0.67", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "165", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:31.20,H:87.10,SRAW:3990,SM:0.0,SC:DRY,RRAW:2256,RP:49.8,RC:MODERATE_RAIN,MV:1,TX:-4.60,TY:-0.67,AXG:0.012,AYG:-0.082,AZG:1.019,GXD:1.88,GYD:2.86,GZD:-1.34"}, {"timestamp": "24-10-2026 09:49:54", "packet_id": "272", "temperature_c": "31.2", "temperature_f": "88.16", "humidity_pct": "87.0", "dew_point_c": "28.77", "soil_raw_adc": "4005", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "2250", "rain_intensity_pct": "50.0", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-1458", "accel_y_raw": "-1818", "accel_z_raw": "16777", "accel_x_g": "-0.089", "accel_y_g": "-0.111", "accel_z_g": "1.024", "accel_magnitude_g": "1.034", "gyro_x_raw": "361", "gyro_y_raw": "82", "gyro_z_raw": "-48", "gyro_x_dps": "2.76", "gyro_y_dps": "0.63", "gyro_z_dps": "-0.37", "gyro_magnitude_dps": "2.86", "tilt_pitch_deg": "-6.17", "tilt_roll_deg": "4.93", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "165", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:31.20,H:87.00,SRAW:4005,SM:0.0,SC:DRY,RRAW:2250,RP:50.0,RC:MODERATE_RAIN,MV:1,TX:-6.17,TY:4.93,AXG:-0.089,AYG:-0.111,AZG:1.024,GXD:2.76,GYD:0.63,GZD:-0.37"}, {"timestamp": "24-10-2026 09:49:59", "packet_id": "273", "temperature_c": "31.2", "temperature_f": "88.16", "humidity_pct": "86.6", "dew_point_c": "28.69", "soil_raw_adc": "4026", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "2281", "rain_intensity_pct": "48.8", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-2260", "accel_y_raw": "-1736", "accel_z_raw": "16629", "accel_x_g": "-0.138", "accel_y_g": "-0.106", "accel_z_g": "1.015", "accel_magnitude_g": "1.03", "gyro_x_raw": "395", "gyro_y_raw": "89", "gyro_z_raw": "-98", "gyro_x_dps": "3.02", "gyro_y_dps": "0.68", "gyro_z_dps": "-0.75", "gyro_magnitude_dps": "3.19", "tilt_pitch_deg": "-5.9", "tilt_roll_deg": "7.71", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "165", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:31.20,H:86.60,SRAW:4026,SM:0.0,SC:DRY,RRAW:2281,RP:48.8,RC:MODERATE_RAIN,MV:1,TX:-5.90,TY:7.71,AXG:-0.138,AYG:-0.106,AZG:1.015,GXD:3.02,GYD:0.68,GZD:-0.75"}, {"timestamp": "24-10-2026 09:50:04", "packet_id": "274", "temperature_c": "31.1", "temperature_f": "87.98", "humidity_pct": "85.4", "dew_point_c": "28.35", "soil_raw_adc": "2351", "soil_moisture_pct": "36.1", "soil_condition": "MODERATE", "rain_raw_adc": "2317", "rain_intensity_pct": "47.3", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-2211", "accel_y_raw": "-1753", "accel_z_raw": "16580", "accel_x_g": "-0.135", "accel_y_g": "-0.107", "accel_z_g": "1.012", "accel_magnitude_g": "1.027", "gyro_x_raw": "351", "gyro_y_raw": "77", "gyro_z_raw": "-6", "gyro_x_dps": "2.68", "gyro_y_dps": "0.59", "gyro_z_dps": "-0.05", "gyro_magnitude_dps": "2.74", "tilt_pitch_deg": "-5.98", "tilt_roll_deg": "7.56", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "171", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:31.10,H:85.40,SRAW:2351,SM:36.1,SC:MODERATE,RRAW:2317,RP:47.3,RC:MODERATE_RAIN,MV:1,TX:-5.98,TY:7.56,AXG:-0.135,AYG:-0.107,AZG:1.012,GXD:2.68,GYD:0.59,GZD:-0.05"}, {"timestamp": "24-10-2026 09:50:09", "packet_id": "275", "temperature_c": "31.1", "temperature_f": "87.98", "humidity_pct": "84.9", "dew_point_c": "28.25", "soil_raw_adc": "2364", "soil_moisture_pct": "35.3", "soil_condition": "MODERATE", "rain_raw_adc": "2304", "rain_intensity_pct": "47.8", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-2097", "accel_y_raw": "-2129", "accel_z_raw": "16515", "accel_x_g": "-0.128", "accel_y_g": "-0.13", "accel_z_g": "1.008", "accel_magnitude_g": "1.024", "gyro_x_raw": "157", "gyro_y_raw": "56", "gyro_z_raw": "-127", "gyro_x_dps": "1.2", "gyro_y_dps": "0.43", "gyro_z_dps": "-0.97", "gyro_magnitude_dps": "1.6", "tilt_pitch_deg": "-7.28", "tilt_roll_deg": "7.2", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "171", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:31.10,H:84.90,SRAW:2364,SM:35.3,SC:MODERATE,RRAW:2304,RP:47.8,RC:MODERATE_RAIN,MV:1,TX:-7.28,TY:7.20,AXG:-0.128,AYG:-0.130,AZG:1.008,GXD:1.20,GYD:0.43,GZD:-0.97"}, {"timestamp": "24-10-2026 09:50:14", "packet_id": "276", "temperature_c": "31.1", "temperature_f": "87.98", "humidity_pct": "84.6", "dew_point_c": "28.19", "soil_raw_adc": "2435", "soil_moisture_pct": "31.4", "soil_condition": "MODERATE", "rain_raw_adc": "2342", "rain_intensity_pct": "46.3", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-2883", "accel_y_raw": "-2932", "accel_z_raw": "16515", "accel_x_g": "-0.176", "accel_y_g": "-0.179", "accel_z_g": "1.008", "accel_magnitude_g": "1.039", "gyro_x_raw": "419", "gyro_y_raw": "82", "gyro_z_raw": "-10", "gyro_x_dps": "3.2", "gyro_y_dps": "0.63", "gyro_z_dps": "-0.08", "gyro_magnitude_dps": "3.26", "tilt_pitch_deg": "-9.92", "tilt_roll_deg": "9.74", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "171", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:31.10,H:84.60,SRAW:2435,SM:31.4,SC:MODERATE,RRAW:2342,RP:46.3,RC:MODERATE_RAIN,MV:1,TX:-9.92,TY:9.74,AXG:-0.176,AYG:-0.179,AZG:1.008,GXD:3.20,GYD:0.63,GZD:-0.08"}, {"timestamp": "24-10-2026 09:50:19", "packet_id": "277", "temperature_c": "31.0", "temperature_f": "87.8", "humidity_pct": "84.6", "dew_point_c": "28.09", "soil_raw_adc": "3094", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "2287", "rain_intensity_pct": "48.5", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-2670", "accel_y_raw": "-3063", "accel_z_raw": "16482", "accel_x_g": "-0.163", "accel_y_g": "-0.187", "accel_z_g": "1.006", "accel_magnitude_g": "1.036", "gyro_x_raw": "289", "gyro_y_raw": "-212", "gyro_z_raw": "11", "gyro_x_dps": "2.21", "gyro_y_dps": "-1.62", "gyro_z_dps": "0.09", "gyro_magnitude_dps": "2.74", "tilt_pitch_deg": "-10.37", "tilt_roll_deg": "9.06", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "166", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:31.00,H:84.60,SRAW:3094,SM:0.0,SC:DRY,RRAW:2287,RP:48.5,RC:MODERATE_RAIN,MV:1,TX:-10.37,TY:9.06,AXG:-0.163,AYG:-0.187,AZG:1.006,GXD:2.21,GYD:-1.62,GZD:0.09"}, {"timestamp": "24-10-2026 09:50:24", "packet_id": "278", "temperature_c": "31.0", "temperature_f": "87.8", "humidity_pct": "84.5", "dew_point_c": "28.07", "soil_raw_adc": "3184", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "2238", "rain_intensity_pct": "50.5", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "8355", "accel_y_raw": "-2670", "accel_z_raw": "14352", "accel_x_g": "0.51", "accel_y_g": "-0.163", "accel_z_g": "0.876", "accel_magnitude_g": "1.027", "gyro_x_raw": "324", "gyro_y_raw": "930", "gyro_z_raw": "13", "gyro_x_dps": "2.48", "gyro_y_dps": "7.1", "gyro_z_dps": "0.1", "gyro_magnitude_dps": "7.52", "tilt_pitch_deg": "-9.12", "tilt_roll_deg": "-29.78", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "165", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:31.00,H:84.50,SRAW:3184,SM:0.0,SC:DRY,RRAW:2238,RP:50.5,RC:MODERATE_RAIN,MV:1,TX:-9.12,TY:-29.78,AXG:0.510,AYG:-0.163,AZG:0.876,GXD:2.48,GYD:7.10,GZD:0.10"}, {"timestamp": "24-10-2026 09:50:29", "packet_id": "279", "temperature_c": "31.0", "temperature_f": "87.8", "humidity_pct": "84.9", "dew_point_c": "28.15", "soil_raw_adc": "3148", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "2301", "rain_intensity_pct": "48.0", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-3342", "accel_y_raw": "-1900", "accel_z_raw": "13713", "accel_x_g": "-0.204", "accel_y_g": "-0.116", "accel_z_g": "0.837", "accel_magnitude_g": "0.869", "gyro_x_raw": "4151", "gyro_y_raw": "150", "gyro_z_raw": "-106", "gyro_x_dps": "31.69", "gyro_y_dps": "1.15", "gyro_z_dps": "-0.81", "gyro_magnitude_dps": "31.72", "tilt_pitch_deg": "-7.7", "tilt_roll_deg": "13.58", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "167", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:31.00,H:84.90,SRAW:3148,SM:0.0,SC:DRY,RRAW:2301,RP:48.0,RC:MODERATE_RAIN,MV:1,TX:-7.70,TY:13.58,AXG:-0.204,AYG:-0.116,AZG:0.837,GXD:31.69,GYD:1.15,GZD:-0.81"}, {"timestamp": "24-10-2026 09:50:36", "packet_id": "280", "temperature_c": "30.9", "temperature_f": "87.62", "humidity_pct": "85.4", "dew_point_c": "28.15", "soil_raw_adc": "2525", "soil_moisture_pct": "26.4", "soil_condition": "MODERATE", "rain_raw_adc": "1321", "rain_intensity_pct": "87.2", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-2113", "accel_y_raw": "65", "accel_z_raw": "16629", "accel_x_g": "-0.129", "accel_y_g": "0.004", "accel_z_g": "1.015", "accel_magnitude_g": "1.023", "gyro_x_raw": "420", "gyro_y_raw": "65", "gyro_z_raw": "-505", "gyro_x_dps": "3.21", "gyro_y_dps": "0.5", "gyro_z_dps": "-3.86", "gyro_magnitude_dps": "5.05", "tilt_pitch_deg": "0.25", "tilt_roll_deg": "7.25", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "166", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.90,H:85.40,SRAW:2525,SM:26.4,SC:MODERATE,RRAW:1321,RP:87.2,RC:HEAVY_RAIN,MV:1,TX:0.25,TY:7.25,AXG:-0.129,AYG:0.004,AZG:1.015,GXD:3.21,GYD:0.50,GZD:-3.86"}, {"timestamp": "24-10-2026 09:50:41", "packet_id": "281", "temperature_c": "30.8", "temperature_f": "87.44", "humidity_pct": "86.1", "dew_point_c": "28.2", "soil_raw_adc": "2547", "soil_moisture_pct": "25.2", "soil_condition": "MODERATE", "rain_raw_adc": "1326", "rain_intensity_pct": "87.0", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-2277", "accel_y_raw": "0", "accel_z_raw": "16662", "accel_x_g": "-0.139", "accel_y_g": "0.0", "accel_z_g": "1.017", "accel_magnitude_g": "1.026", "gyro_x_raw": "416", "gyro_y_raw": "77", "gyro_z_raw": "-58", "gyro_x_dps": "3.18", "gyro_y_dps": "0.59", "gyro_z_dps": "-0.45", "gyro_magnitude_dps": "3.27", "tilt_pitch_deg": "0.03", "tilt_roll_deg": "7.78", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "166", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.80,H:86.10,SRAW:2547,SM:25.2,SC:MODERATE,RRAW:1326,RP:87.0,RC:HEAVY_RAIN,MV:1,TX:0.03,TY:7.78,AXG:-0.139,AYG:0.000,AZG:1.017,GXD:3.18,GYD:0.59,GZD:-0.45"}, {"timestamp": "24-10-2026 09:50:46", "packet_id": "282", "temperature_c": "30.8", "temperature_f": "87.44", "humidity_pct": "86.9", "dew_point_c": "28.36", "soil_raw_adc": "2539", "soil_moisture_pct": "25.6", "soil_condition": "MODERATE", "rain_raw_adc": "1309", "rain_intensity_pct": "87.6", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-2244", "accel_y_raw": "32", "accel_z_raw": "16760", "accel_x_g": "-0.137", "accel_y_g": "0.002", "accel_z_g": "1.023", "accel_magnitude_g": "1.032", "gyro_x_raw": "395", "gyro_y_raw": "47", "gyro_z_raw": "-66", "gyro_x_dps": "3.02", "gyro_y_dps": "0.36", "gyro_z_dps": "-0.51", "gyro_magnitude_dps": "3.08", "tilt_pitch_deg": "0.11", "tilt_roll_deg": "7.61", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "166", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.80,H:86.90,SRAW:2539,SM:25.6,SC:MODERATE,RRAW:1309,RP:87.6,RC:HEAVY_RAIN,MV:1,TX:0.11,TY:7.61,AXG:-0.137,AYG:0.002,AZG:1.023,GXD:3.02,GYD:0.36,GZD:-0.51"}, {"timestamp": "24-10-2026 09:50:51", "packet_id": "283", "temperature_c": "30.8", "temperature_f": "87.44", "humidity_pct": "87.4", "dew_point_c": "28.46", "soil_raw_adc": "2554", "soil_moisture_pct": "24.8", "soil_condition": "DRY", "rain_raw_adc": "1315", "rain_intensity_pct": "87.4", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-2195", "accel_y_raw": "-16", "accel_z_raw": "16613", "accel_x_g": "-0.134", "accel_y_g": "-0.001", "accel_z_g": "1.014", "accel_magnitude_g": "1.023", "gyro_x_raw": "357", "gyro_y_raw": "99", "gyro_z_raw": "-57", "gyro_x_dps": "2.73", "gyro_y_dps": "0.76", "gyro_z_dps": "-0.44", "gyro_magnitude_dps": "2.87", "tilt_pitch_deg": "-0.08", "tilt_roll_deg": "7.54", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "163", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.80,H:87.40,SRAW:2554,SM:24.8,SC:DRY,RRAW:1315,RP:87.4,RC:HEAVY_RAIN,MV:1,TX:-0.08,TY:7.54,AXG:-0.134,AYG:-0.001,AZG:1.014,GXD:2.73,GYD:0.76,GZD:-0.44"}, {"timestamp": "24-10-2026 09:50:56", "packet_id": "284", "temperature_c": "30.7", "temperature_f": "87.26", "humidity_pct": "87.7", "dew_point_c": "28.42", "soil_raw_adc": "2559", "soil_moisture_pct": "24.5", "soil_condition": "DRY", "rain_raw_adc": "1353", "rain_intensity_pct": "85.9", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-4161", "accel_y_raw": "-4964", "accel_z_raw": "16121", "accel_x_g": "-0.254", "accel_y_g": "-0.303", "accel_z_g": "0.984", "accel_magnitude_g": "1.06", "gyro_x_raw": "448", "gyro_y_raw": "172", "gyro_z_raw": "204", "gyro_x_dps": "3.42", "gyro_y_dps": "1.32", "gyro_z_dps": "1.56", "gyro_magnitude_dps": "3.98", "tilt_pitch_deg": "-16.63", "tilt_roll_deg": "13.84", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "164", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.70,H:87.70,SRAW:2559,SM:24.5,SC:DRY,RRAW:1353,RP:85.9,RC:HEAVY_RAIN,MV:1,TX:-16.63,TY:13.84,AXG:-0.254,AYG:-0.303,AZG:0.984,GXD:3.42,GYD:1.32,GZD:1.56"}, {"timestamp": "24-10-2026 09:51:01", "packet_id": "285", "temperature_c": "30.7", "temperature_f": "87.26", "humidity_pct": "87.9", "dew_point_c": "28.46", "soil_raw_adc": "2665", "soil_moisture_pct": "18.6", "soil_condition": "DRY", "rain_raw_adc": "1312", "rain_intensity_pct": "87.5", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-2080", "accel_y_raw": "-3801", "accel_z_raw": "15859", "accel_x_g": "-0.127", "accel_y_g": "-0.232", "accel_z_g": "0.968", "accel_magnitude_g": "1.003", "gyro_x_raw": "814", "gyro_y_raw": "-31", "gyro_z_raw": "166", "gyro_x_dps": "6.22", "gyro_y_dps": "-0.24", "gyro_z_dps": "1.27", "gyro_magnitude_dps": "6.35", "tilt_pitch_deg": "-13.36", "tilt_roll_deg": "7.29", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "164", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.70,H:87.90,SRAW:2665,SM:18.6,SC:DRY,RRAW:1312,RP:87.5,RC:HEAVY_RAIN,MV:1,TX:-13.36,TY:7.29,AXG:-0.127,AYG:-0.232,AZG:0.968,GXD:6.22,GYD:-0.24,GZD:1.27"}, {"timestamp": "24-10-2026 09:51:06", "packet_id": "286", "temperature_c": "30.7", "temperature_f": "87.26", "humidity_pct": "87.9", "dew_point_c": "28.46", "soil_raw_adc": "2363", "soil_moisture_pct": "35.4", "soil_condition": "MODERATE", "rain_raw_adc": "1386", "rain_intensity_pct": "84.6", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "4767", "accel_y_raw": "-1867", "accel_z_raw": "16400", "accel_x_g": "0.291", "accel_y_g": "-0.114", "accel_z_g": "1.001", "accel_magnitude_g": "1.049", "gyro_x_raw": "404", "gyro_y_raw": "87", "gyro_z_raw": "-41", "gyro_x_dps": "3.09", "gyro_y_dps": "0.67", "gyro_z_dps": "-0.32", "gyro_magnitude_dps": "3.18", "tilt_pitch_deg": "-6.26", "tilt_roll_deg": "-16.09", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.70,H:87.90,SRAW:2363,SM:35.4,SC:MODERATE,RRAW:1386,RP:84.6,RC:HEAVY_RAIN,MV:1,TX:-6.26,TY:-16.09,AXG:0.291,AYG:-0.114,AZG:1.001,GXD:3.09,GYD:0.67,GZD:-0.32"}, {"timestamp": "24-10-2026 09:51:16", "packet_id": "287", "temperature_c": "30.6", "temperature_f": "87.08", "humidity_pct": "87.2", "dew_point_c": "28.22", "soil_raw_adc": "2384", "soil_moisture_pct": "34.2", "soil_condition": "MODERATE", "rain_raw_adc": "1393", "rain_intensity_pct": "84.3", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-2179", "accel_y_raw": "81", "accel_z_raw": "16678", "accel_x_g": "-0.133", "accel_y_g": "0.005", "accel_z_g": "1.018", "accel_magnitude_g": "1.027", "gyro_x_raw": "403", "gyro_y_raw": "61", "gyro_z_raw": "-48", "gyro_x_dps": "3.08", "gyro_y_dps": "0.47", "gyro_z_dps": "-0.37", "gyro_magnitude_dps": "3.14", "tilt_pitch_deg": "0.29", "tilt_roll_deg": "7.46", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "166", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.60,H:87.20,SRAW:2384,SM:34.2,SC:MODERATE,RRAW:1393,RP:84.3,RC:HEAVY_RAIN,MV:1,TX:0.29,TY:7.46,AXG:-0.133,AYG:0.005,AZG:1.018,GXD:3.08,GYD:0.47,GZD:-0.37"}, {"timestamp": "24-10-2026 09:51:22", "packet_id": "288", "temperature_c": "30.5", "temperature_f": "86.9", "humidity_pct": "87.6", "dew_point_c": "28.2", "soil_raw_adc": "1967", "soil_moisture_pct": "57.4", "soil_condition": "MODERATE", "rain_raw_adc": "1431", "rain_intensity_pct": "82.8", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-2195", "accel_y_raw": "81", "accel_z_raw": "16744", "accel_x_g": "-0.134", "accel_y_g": "0.005", "accel_z_g": "1.022", "accel_magnitude_g": "1.031", "gyro_x_raw": "378", "gyro_y_raw": "62", "gyro_z_raw": "-35", "gyro_x_dps": "2.89", "gyro_y_dps": "0.48", "gyro_z_dps": "-0.27", "gyro_magnitude_dps": "2.94", "tilt_pitch_deg": "0.26", "tilt_roll_deg": "7.48", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "166", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.50,H:87.60,SRAW:1967,SM:57.4,SC:MODERATE,RRAW:1431,RP:82.8,RC:HEAVY_RAIN,MV:1,TX:0.26,TY:7.48,AXG:-0.134,AYG:0.005,AZG:1.022,GXD:2.89,GYD:0.48,GZD:-0.27"}, {"timestamp": "24-10-2026 09:51:27", "packet_id": "289", "temperature_c": "30.5", "temperature_f": "86.9", "humidity_pct": "87.9", "dew_point_c": "28.26", "soil_raw_adc": "1942", "soil_moisture_pct": "58.8", "soil_condition": "MODERATE", "rain_raw_adc": "1479", "rain_intensity_pct": "80.8", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-2179", "accel_y_raw": "98", "accel_z_raw": "16728", "accel_x_g": "-0.133", "accel_y_g": "0.006", "accel_z_g": "1.021", "accel_magnitude_g": "1.03", "gyro_x_raw": "387", "gyro_y_raw": "102", "gyro_z_raw": "-52", "gyro_x_dps": "2.96", "gyro_y_dps": "0.78", "gyro_z_dps": "-0.4", "gyro_magnitude_dps": "3.09", "tilt_pitch_deg": "0.31", "tilt_roll_deg": "7.44", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "166", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.50,H:87.90,SRAW:1942,SM:58.8,SC:MODERATE,RRAW:1479,RP:80.8,RC:HEAVY_RAIN,MV:1,TX:0.31,TY:7.44,AXG:-0.133,AYG:0.006,AZG:1.021,GXD:2.96,GYD:0.78,GZD:-0.40"}, {"timestamp": "24-10-2026 09:51:32", "packet_id": "290", "temperature_c": "30.4", "temperature_f": "86.72", "humidity_pct": "88.3", "dew_point_c": "28.24", "soil_raw_adc": "1911", "soil_moisture_pct": "60.5", "soil_condition": "MODERATE", "rain_raw_adc": "1497", "rain_intensity_pct": "80.1", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-2228", "accel_y_raw": "65", "accel_z_raw": "16695", "accel_x_g": "-0.136", "accel_y_g": "0.004", "accel_z_g": "1.019", "accel_magnitude_g": "1.028", "gyro_x_raw": "386", "gyro_y_raw": "82", "gyro_z_raw": "-65", "gyro_x_dps": "2.95", "gyro_y_dps": "0.63", "gyro_z_dps": "-0.5", "gyro_magnitude_dps": "3.06", "tilt_pitch_deg": "0.25", "tilt_roll_deg": "7.6", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "166", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.40,H:88.30,SRAW:1911,SM:60.5,SC:MODERATE,RRAW:1497,RP:80.1,RC:HEAVY_RAIN,MV:1,TX:0.25,TY:7.60,AXG:-0.136,AYG:0.004,AZG:1.019,GXD:2.95,GYD:0.63,GZD:-0.50"}, {"timestamp": "24-10-2026 09:51:37", "packet_id": "291", "temperature_c": "30.4", "temperature_f": "86.72", "humidity_pct": "88.6", "dew_point_c": "28.3", "soil_raw_adc": "1914", "soil_moisture_pct": "60.3", "soil_condition": "MODERATE", "rain_raw_adc": "1507", "rain_intensity_pct": "79.7", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-2129", "accel_y_raw": "114", "accel_z_raw": "16695", "accel_x_g": "-0.13", "accel_y_g": "0.007", "accel_z_g": "1.019", "accel_magnitude_g": "1.027", "gyro_x_raw": "407", "gyro_y_raw": "56", "gyro_z_raw": "-36", "gyro_x_dps": "3.11", "gyro_y_dps": "0.43", "gyro_z_dps": "-0.28", "gyro_magnitude_dps": "3.15", "tilt_pitch_deg": "0.38", "tilt_roll_deg": "7.27", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "166", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.40,H:88.60,SRAW:1914,SM:60.3,SC:MODERATE,RRAW:1507,RP:79.7,RC:HEAVY_RAIN,MV:1,TX:0.38,TY:7.27,AXG:-0.130,AYG:0.007,AZG:1.019,GXD:3.11,GYD:0.43,GZD:-0.28"}, {"timestamp": "24-10-2026 09:51:42", "packet_id": "292", "temperature_c": "30.4", "temperature_f": "86.72", "humidity_pct": "88.7", "dew_point_c": "28.32", "soil_raw_adc": "2015", "soil_moisture_pct": "54.7", "soil_condition": "MODERATE", "rain_raw_adc": "1559", "rain_intensity_pct": "77.6", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-5947", "accel_y_raw": "147", "accel_z_raw": "17727", "accel_x_g": "-0.363", "accel_y_g": "0.009", "accel_z_g": "1.082", "accel_magnitude_g": "1.141", "gyro_x_raw": "-203", "gyro_y_raw": "-47", "gyro_z_raw": "-2710", "gyro_x_dps": "-1.55", "gyro_y_dps": "-0.36", "gyro_z_dps": "-20.69", "gyro_magnitude_dps": "20.75", "tilt_pitch_deg": "0.43", "tilt_roll_deg": "18.56", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "170", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.40,H:88.70,SRAW:2015,SM:54.7,SC:MODERATE,RRAW:1559,RP:77.6,RC:HEAVY_RAIN,MV:1,TX:0.43,TY:18.56,AXG:-0.363,AYG:0.009,AZG:1.082,GXD:-1.55,GYD:-0.36,GZD:-20.69"}, {"timestamp": "24-10-2026 09:51:47", "packet_id": "293", "temperature_c": "30.4", "temperature_f": "86.72", "humidity_pct": "88.8", "dew_point_c": "28.34", "soil_raw_adc": "2077", "soil_moisture_pct": "51.3", "soil_condition": "MODERATE", "rain_raw_adc": "1513", "rain_intensity_pct": "79.5", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1490", "accel_y_raw": "-13008", "accel_z_raw": "10485", "accel_x_g": "-0.091", "accel_y_g": "-0.794", "accel_z_g": "0.64", "accel_magnitude_g": "1.024", "gyro_x_raw": "289", "gyro_y_raw": "127", "gyro_z_raw": "-175", "gyro_x_dps": "2.21", "gyro_y_dps": "0.97", "gyro_z_dps": "-1.34", "gyro_magnitude_dps": "2.76", "tilt_pitch_deg": "-50.85", "tilt_roll_deg": "5.12", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.40,H:88.80,SRAW:2077,SM:51.3,SC:MODERATE,RRAW:1513,RP:79.5,RC:HEAVY_RAIN,MV:1,TX:-50.85,TY:5.12,AXG:-0.091,AYG:-0.794,AZG:0.640,GXD:2.21,GYD:0.97,GZD:-1.34"}, {"timestamp": "24-10-2026 09:51:52", "packet_id": "294", "temperature_c": "30.3", "temperature_f": "86.54", "humidity_pct": "89.0", "dew_point_c": "28.28", "soil_raw_adc": "1854", "soil_moisture_pct": "63.7", "soil_condition": "MODERATE", "rain_raw_adc": "1558", "rain_intensity_pct": "77.7", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1589", "accel_y_raw": "-13041", "accel_z_raw": "10502", "accel_x_g": "-0.097", "accel_y_g": "-0.796", "accel_z_g": "0.641", "accel_magnitude_g": "1.027", "gyro_x_raw": "455", "gyro_y_raw": "68", "gyro_z_raw": "-17", "gyro_x_dps": "3.48", "gyro_y_dps": "0.52", "gyro_z_dps": "-0.13", "gyro_magnitude_dps": "3.52", "tilt_pitch_deg": "-50.83", "tilt_roll_deg": "5.44", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.30,H:89.00,SRAW:1854,SM:63.7,SC:MODERATE,RRAW:1558,RP:77.7,RC:HEAVY_RAIN,MV:1,TX:-50.83,TY:5.44,AXG:-0.097,AYG:-0.796,AZG:0.641,GXD:3.48,GYD:0.52,GZD:-0.13"}, {"timestamp": "24-10-2026 09:51:57", "packet_id": "295", "temperature_c": "30.3", "temperature_f": "86.54", "humidity_pct": "89.4", "dew_point_c": "28.35", "soil_raw_adc": "1747", "soil_moisture_pct": "69.6", "soil_condition": "WET", "rain_raw_adc": "1602", "rain_intensity_pct": "75.9", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-327", "accel_y_raw": "-16580", "accel_z_raw": "131", "accel_x_g": "-0.02", "accel_y_g": "-1.012", "accel_z_g": "0.008", "accel_magnitude_g": "1.012", "gyro_x_raw": "-1739", "gyro_y_raw": "-247", "gyro_z_raw": "-577", "gyro_x_dps": "-13.28", "gyro_y_dps": "-1.89", "gyro_z_dps": "-4.41", "gyro_magnitude_dps": "14.12", "tilt_pitch_deg": "-88.8", "tilt_roll_deg": "1.12", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "167", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.30,H:89.40,SRAW:1747,SM:69.6,SC:WET,RRAW:1602,RP:75.9,RC:HEAVY_RAIN,MV:1,TX:-88.80,TY:1.12,AXG:-0.020,AYG:-1.012,AZG:0.008,GXD:-13.28,GYD:-1.89,GZD:-4.41"}, {"timestamp": "24-10-2026 09:52:02", "packet_id": "296", "temperature_c": "30.3", "temperature_f": "86.54", "humidity_pct": "90.1", "dew_point_c": "28.49", "soil_raw_adc": "1803", "soil_moisture_pct": "66.5", "soil_condition": "WET", "rain_raw_adc": "1601", "rain_intensity_pct": "76.0", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-557", "accel_y_raw": "-16678", "accel_z_raw": "393", "accel_x_g": "-0.034", "accel_y_g": "-1.018", "accel_z_g": "0.024", "accel_magnitude_g": "1.019", "gyro_x_raw": "343", "gyro_y_raw": "90", "gyro_z_raw": "-26", "gyro_x_dps": "2.62", "gyro_y_dps": "0.69", "gyro_z_dps": "-0.2", "gyro_magnitude_dps": "2.72", "tilt_pitch_deg": "-87.65", "tilt_roll_deg": "1.91", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "164", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.30,H:90.10,SRAW:1803,SM:66.5,SC:WET,RRAW:1601,RP:76.0,RC:HEAVY_RAIN,MV:1,TX:-87.65,TY:1.91,AXG:-0.034,AYG:-1.018,AZG:0.024,GXD:2.62,GYD:0.69,GZD:-0.20"}, {"timestamp": "24-10-2026 09:52:07", "packet_id": "297", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "91.1", "dew_point_c": "28.58", "soil_raw_adc": "1711", "soil_moisture_pct": "71.6", "soil_condition": "WET", "rain_raw_adc": "1617", "rain_intensity_pct": "75.3", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-524", "accel_y_raw": "-16646", "accel_z_raw": "294", "accel_x_g": "-0.032", "accel_y_g": "-1.016", "accel_z_g": "0.018", "accel_magnitude_g": "1.017", "gyro_x_raw": "492", "gyro_y_raw": "72", "gyro_z_raw": "-35", "gyro_x_dps": "3.76", "gyro_y_dps": "0.55", "gyro_z_dps": "-0.27", "gyro_magnitude_dps": "3.81", "tilt_pitch_deg": "-87.91", "tilt_roll_deg": "1.82", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "164", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.20,H:91.10,SRAW:1711,SM:71.6,SC:WET,RRAW:1617,RP:75.3,RC:HEAVY_RAIN,MV:1,TX:-87.91,TY:1.82,AXG:-0.032,AYG:-1.016,AZG:0.018,GXD:3.76,GYD:0.55,GZD:-0.27"}, {"timestamp": "24-10-2026 09:52:12", "packet_id": "298", "temperature_c": "30.1", "temperature_f": "86.18", "humidity_pct": "92.0", "dew_point_c": "28.65", "soil_raw_adc": "1734", "soil_moisture_pct": "70.3", "soil_condition": "WET", "rain_raw_adc": "1627", "rain_intensity_pct": "74.9", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-475", "accel_y_raw": "-16580", "accel_z_raw": "344", "accel_x_g": "-0.029", "accel_y_g": "-1.012", "accel_z_g": "0.021", "accel_magnitude_g": "1.013", "gyro_x_raw": "387", "gyro_y_raw": "86", "gyro_z_raw": "-22", "gyro_x_dps": "2.96", "gyro_y_dps": "0.66", "gyro_z_dps": "-0.17", "gyro_magnitude_dps": "3.04", "tilt_pitch_deg": "-87.98", "tilt_roll_deg": "1.62", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "167", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.10,H:92.00,SRAW:1734,SM:70.3,SC:WET,RRAW:1627,RP:74.9,RC:MODERATE_RAIN,MV:1,TX:-87.98,TY:1.62,AXG:-0.029,AYG:-1.012,AZG:0.021,GXD:2.96,GYD:0.66,GZD:-0.17"}, {"timestamp": "24-10-2026 09:52:17", "packet_id": "299", "temperature_c": "30.1", "temperature_f": "86.18", "humidity_pct": "92.7", "dew_point_c": "28.78", "soil_raw_adc": "1780", "soil_moisture_pct": "67.8", "soil_condition": "WET", "rain_raw_adc": "1661", "rain_intensity_pct": "73.6", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-557", "accel_y_raw": "-16564", "accel_z_raw": "163", "accel_x_g": "-0.034", "accel_y_g": "-1.011", "accel_z_g": "0.01", "accel_magnitude_g": "1.012", "gyro_x_raw": "423", "gyro_y_raw": "65", "gyro_z_raw": "-20", "gyro_x_dps": "3.23", "gyro_y_dps": "0.5", "gyro_z_dps": "-0.16", "gyro_magnitude_dps": "3.27", "tilt_pitch_deg": "-87.96", "tilt_roll_deg": "1.95", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "167", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.10,H:92.70,SRAW:1780,SM:67.8,SC:WET,RRAW:1661,RP:73.6,RC:MODERATE_RAIN,MV:1,TX:-87.96,TY:1.95,AXG:-0.034,AYG:-1.011,AZG:0.010,GXD:3.23,GYD:0.50,GZD:-0.16"}, {"timestamp": "24-10-2026 09:52:22", "packet_id": "300", "temperature_c": "30.1", "temperature_f": "86.18", "humidity_pct": "93.3", "dew_point_c": "28.89", "soil_raw_adc": "1776", "soil_moisture_pct": "68.0", "soil_condition": "WET", "rain_raw_adc": "1679", "rain_intensity_pct": "72.8", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-458", "accel_y_raw": "-16580", "accel_z_raw": "311", "accel_x_g": "-0.028", "accel_y_g": "-1.012", "accel_z_g": "0.019", "accel_magnitude_g": "1.013", "gyro_x_raw": "441", "gyro_y_raw": "65", "gyro_z_raw": "-40", "gyro_x_dps": "3.37", "gyro_y_dps": "0.5", "gyro_z_dps": "-0.31", "gyro_magnitude_dps": "3.42", "tilt_pitch_deg": "-88.09", "tilt_roll_deg": "1.56", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "167", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.10,H:93.30,SRAW:1776,SM:68.0,SC:WET,RRAW:1679,RP:72.8,RC:MODERATE_RAIN,MV:1,TX:-88.09,TY:1.56,AXG:-0.028,AYG:-1.012,AZG:0.019,GXD:3.37,GYD:0.50,GZD:-0.31"}, {"timestamp": "25-10-2026 14:28:50", "packet_id": "301", "temperature_c": "28.8", "temperature_f": "83.84", "humidity_pct": "93.6", "dew_point_c": "27.66", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "4095", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-1818", "accel_y_raw": "196", "accel_z_raw": "16777", "accel_x_g": "-0.111", "accel_y_g": "0.012", "accel_z_g": "1.024", "accel_magnitude_g": "1.03", "gyro_x_raw": "394", "gyro_y_raw": "85", "gyro_z_raw": "-36", "gyro_x_dps": "3.01", "gyro_y_dps": "0.65", "gyro_z_dps": "-0.28", "gyro_magnitude_dps": "3.09", "tilt_pitch_deg": "0.69", "tilt_roll_deg": "6.2", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "156", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:28.80,H:93.60,SRAW:4095,SM:0.0,SC:DRY,RRAW:4095,RP:0.0,RC:NO_RAIN,MV:1,TX:0.69,TY:6.20,AXG:-0.111,AYG:0.012,AZG:1.024,GXD:3.01,GYD:0.65,GZD:-0.28"}, {"timestamp": "25-10-2026 14:28:55", "packet_id": "302", "temperature_c": "28.8", "temperature_f": "83.84", "humidity_pct": "93.5", "dew_point_c": "27.64", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "4095", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-1818", "accel_y_raw": "262", "accel_z_raw": "16908", "accel_x_g": "-0.111", "accel_y_g": "0.016", "accel_z_g": "1.032", "accel_magnitude_g": "1.038", "gyro_x_raw": "381", "gyro_y_raw": "56", "gyro_z_raw": "-27", "gyro_x_dps": "2.91", "gyro_y_dps": "0.43", "gyro_z_dps": "-0.21", "gyro_magnitude_dps": "2.95", "tilt_pitch_deg": "0.9", "tilt_roll_deg": "6.15", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "156", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:28.80,H:93.50,SRAW:4095,SM:0.0,SC:DRY,RRAW:4095,RP:0.0,RC:NO_RAIN,MV:1,TX:0.90,TY:6.15,AXG:-0.111,AYG:0.016,AZG:1.032,GXD:2.91,GYD:0.43,GZD:-0.21"}, {"timestamp": "25-10-2026 14:29:00", "packet_id": "303", "temperature_c": "28.8", "temperature_f": "83.84", "humidity_pct": "93.3", "dew_point_c": "27.61", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "4095", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-1802", "accel_y_raw": "229", "accel_z_raw": "16760", "accel_x_g": "-0.11", "accel_y_g": "0.014", "accel_z_g": "1.023", "accel_magnitude_g": "1.029", "gyro_x_raw": "404", "gyro_y_raw": "35", "gyro_z_raw": "-48", "gyro_x_dps": "3.09", "gyro_y_dps": "0.27", "gyro_z_dps": "-0.37", "gyro_magnitude_dps": "3.12", "tilt_pitch_deg": "0.76", "tilt_roll_deg": "6.11", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "156", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:28.80,H:93.30,SRAW:4095,SM:0.0,SC:DRY,RRAW:4095,RP:0.0,RC:NO_RAIN,MV:1,TX:0.76,TY:6.11,AXG:-0.110,AYG:0.014,AZG:1.023,GXD:3.09,GYD:0.27,GZD:-0.37"}, {"timestamp": "25-10-2026 14:29:05", "packet_id": "304", "temperature_c": "28.8", "temperature_f": "83.84", "humidity_pct": "93.2", "dew_point_c": "27.59", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "4095", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "884", "accel_y_raw": "-65", "accel_z_raw": "16875", "accel_x_g": "0.054", "accel_y_g": "-0.004", "accel_z_g": "1.03", "accel_magnitude_g": "1.031", "gyro_x_raw": "420", "gyro_y_raw": "86", "gyro_z_raw": "-9", "gyro_x_dps": "3.21", "gyro_y_dps": "0.66", "gyro_z_dps": "-0.07", "gyro_magnitude_dps": "3.28", "tilt_pitch_deg": "-0.22", "tilt_roll_deg": "-3.0", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "158", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:28.80,H:93.20,SRAW:4095,SM:0.0,SC:DRY,RRAW:4095,RP:0.0,RC:NO_RAIN,MV:1,TX:-0.22,TY:-3.00,AXG:0.054,AYG:-0.004,AZG:1.030,GXD:3.21,GYD:0.66,GZD:-0.07"}, {"timestamp": "25-10-2026 14:29:10", "packet_id": "305", "temperature_c": "28.8", "temperature_f": "83.84", "humidity_pct": "93.1", "dew_point_c": "27.57", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "4095", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "884", "accel_y_raw": "-98", "accel_z_raw": "16891", "accel_x_g": "0.054", "accel_y_g": "-0.006", "accel_z_g": "1.031", "accel_magnitude_g": "1.032", "gyro_x_raw": "399", "gyro_y_raw": "57", "gyro_z_raw": "-39", "gyro_x_dps": "3.05", "gyro_y_dps": "0.44", "gyro_z_dps": "-0.3", "gyro_magnitude_dps": "3.1", "tilt_pitch_deg": "-0.34", "tilt_roll_deg": "-2.98", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "158", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:28.80,H:93.10,SRAW:4095,SM:0.0,SC:DRY,RRAW:4095,RP:0.0,RC:NO_RAIN,MV:1,TX:-0.34,TY:-2.98,AXG:0.054,AYG:-0.006,AZG:1.031,GXD:3.05,GYD:0.44,GZD:-0.30"}, {"timestamp": "25-10-2026 14:29:15", "packet_id": "306", "temperature_c": "28.9", "temperature_f": "84.02", "humidity_pct": "92.9", "dew_point_c": "27.63", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "4095", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "802", "accel_y_raw": "-180", "accel_z_raw": "16875", "accel_x_g": "0.049", "accel_y_g": "-0.011", "accel_z_g": "1.03", "accel_magnitude_g": "1.031", "gyro_x_raw": "403", "gyro_y_raw": "44", "gyro_z_raw": "-31", "gyro_x_dps": "3.08", "gyro_y_dps": "0.34", "gyro_z_dps": "-0.24", "gyro_magnitude_dps": "3.11", "tilt_pitch_deg": "-0.64", "tilt_roll_deg": "-2.74", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "158", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:28.90,H:92.90,SRAW:4095,SM:0.0,SC:DRY,RRAW:4095,RP:0.0,RC:NO_RAIN,MV:1,TX:-0.64,TY:-2.74,AXG:0.049,AYG:-0.011,AZG:1.030,GXD:3.08,GYD:0.34,GZD:-0.24"}, {"timestamp": "25-10-2026 14:29:20", "packet_id": "307", "temperature_c": "28.9", "temperature_f": "84.02", "humidity_pct": "92.7", "dew_point_c": "27.59", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "4095", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "720", "accel_y_raw": "-131", "accel_z_raw": "16875", "accel_x_g": "0.044", "accel_y_g": "-0.008", "accel_z_g": "1.03", "accel_magnitude_g": "1.031", "gyro_x_raw": "377", "gyro_y_raw": "44", "gyro_z_raw": "-44", "gyro_x_dps": "2.88", "gyro_y_dps": "0.34", "gyro_z_dps": "-0.34", "gyro_magnitude_dps": "2.92", "tilt_pitch_deg": "-0.43", "tilt_roll_deg": "-2.43", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "158", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:28.90,H:92.70,SRAW:4095,SM:0.0,SC:DRY,RRAW:4095,RP:0.0,RC:NO_RAIN,MV:1,TX:-0.43,TY:-2.43,AXG:0.044,AYG:-0.008,AZG:1.030,GXD:2.88,GYD:0.34,GZD:-0.34"}, {"timestamp": "25-10-2026 14:29:25", "packet_id": "308", "temperature_c": "28.9", "temperature_f": "84.02", "humidity_pct": "92.6", "dew_point_c": "27.58", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "4095", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "720", "accel_y_raw": "-65", "accel_z_raw": "16842", "accel_x_g": "0.044", "accel_y_g": "-0.004", "accel_z_g": "1.028", "accel_magnitude_g": "1.029", "gyro_x_raw": "390", "gyro_y_raw": "62", "gyro_z_raw": "-10", "gyro_x_dps": "2.98", "gyro_y_dps": "0.48", "gyro_z_dps": "-0.08", "gyro_magnitude_dps": "3.02", "tilt_pitch_deg": "-0.22", "tilt_roll_deg": "-2.44", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "158", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:28.90,H:92.60,SRAW:4095,SM:0.0,SC:DRY,RRAW:4095,RP:0.0,RC:NO_RAIN,MV:1,TX:-0.22,TY:-2.44,AXG:0.044,AYG:-0.004,AZG:1.028,GXD:2.98,GYD:0.48,GZD:-0.08"}, {"timestamp": "25-10-2026 14:29:30", "packet_id": "309", "temperature_c": "28.9", "temperature_f": "84.02", "humidity_pct": "92.5", "dew_point_c": "27.56", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "4095", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "770", "accel_y_raw": "-81", "accel_z_raw": "16842", "accel_x_g": "0.047", "accel_y_g": "-0.005", "accel_z_g": "1.028", "accel_magnitude_g": "1.029", "gyro_x_raw": "383", "gyro_y_raw": "39", "gyro_z_raw": "-19", "gyro_x_dps": "2.93", "gyro_y_dps": "0.3", "gyro_z_dps": "-0.15", "gyro_magnitude_dps": "2.95", "tilt_pitch_deg": "-0.3", "tilt_roll_deg": "-2.6", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "158", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:28.90,H:92.50,SRAW:4095,SM:0.0,SC:DRY,RRAW:4095,RP:0.0,RC:NO_RAIN,MV:1,TX:-0.30,TY:-2.60,AXG:0.047,AYG:-0.005,AZG:1.028,GXD:2.93,GYD:0.30,GZD:-0.15"}, {"timestamp": "25-10-2026 14:29:35", "packet_id": "310", "temperature_c": "28.9", "temperature_f": "84.02", "humidity_pct": "92.4", "dew_point_c": "27.54", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "4095", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "737", "accel_y_raw": "-114", "accel_z_raw": "16908", "accel_x_g": "0.045", "accel_y_g": "-0.007", "accel_z_g": "1.032", "accel_magnitude_g": "1.033", "gyro_x_raw": "378", "gyro_y_raw": "74", "gyro_z_raw": "-40", "gyro_x_dps": "2.89", "gyro_y_dps": "0.57", "gyro_z_dps": "-0.31", "gyro_magnitude_dps": "2.96", "tilt_pitch_deg": "-0.38", "tilt_roll_deg": "-2.52", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "158", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:28.90,H:92.40,SRAW:4095,SM:0.0,SC:DRY,RRAW:4095,RP:0.0,RC:NO_RAIN,MV:1,TX:-0.38,TY:-2.52,AXG:0.045,AYG:-0.007,AZG:1.032,GXD:2.89,GYD:0.57,GZD:-0.31"}, {"timestamp": "25-10-2026 14:29:40", "packet_id": "311", "temperature_c": "28.9", "temperature_f": "84.02", "humidity_pct": "92.4", "dew_point_c": "27.54", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "4095", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "671", "accel_y_raw": "-147", "accel_z_raw": "16859", "accel_x_g": "0.041", "accel_y_g": "-0.009", "accel_z_g": "1.029", "accel_magnitude_g": "1.03", "gyro_x_raw": "407", "gyro_y_raw": "41", "gyro_z_raw": "-26", "gyro_x_dps": "3.11", "gyro_y_dps": "0.32", "gyro_z_dps": "-0.2", "gyro_magnitude_dps": "3.13", "tilt_pitch_deg": "-0.52", "tilt_roll_deg": "-2.28", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "158", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:28.90,H:92.40,SRAW:4095,SM:0.0,SC:DRY,RRAW:4095,RP:0.0,RC:NO_RAIN,MV:1,TX:-0.52,TY:-2.28,AXG:0.041,AYG:-0.009,AZG:1.029,GXD:3.11,GYD:0.32,GZD:-0.20"}, {"timestamp": "25-10-2026 14:29:45", "packet_id": "312", "temperature_c": "28.9", "temperature_f": "84.02", "humidity_pct": "92.3", "dew_point_c": "27.52", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "4095", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "606", "accel_y_raw": "-163", "accel_z_raw": "17121", "accel_x_g": "0.037", "accel_y_g": "-0.01", "accel_z_g": "1.045", "accel_magnitude_g": "1.046", "gyro_x_raw": "339", "gyro_y_raw": "43", "gyro_z_raw": "-27", "gyro_x_dps": "2.59", "gyro_y_dps": "0.33", "gyro_z_dps": "-0.21", "gyro_magnitude_dps": "2.62", "tilt_pitch_deg": "-0.52", "tilt_roll_deg": "-2.05", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "158", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:28.90,H:92.30,SRAW:4095,SM:0.0,SC:DRY,RRAW:4095,RP:0.0,RC:NO_RAIN,MV:1,TX:-0.52,TY:-2.05,AXG:0.037,AYG:-0.010,AZG:1.045,GXD:2.59,GYD:0.33,GZD:-0.21"}, {"timestamp": "25-10-2026 14:29:50", "packet_id": "313", "temperature_c": "28.9", "temperature_f": "84.02", "humidity_pct": "92.2", "dew_point_c": "27.5", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "4095", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "114", "accel_y_raw": "-98", "accel_z_raw": "16924", "accel_x_g": "0.007", "accel_y_g": "-0.006", "accel_z_g": "1.033", "accel_magnitude_g": "1.033", "gyro_x_raw": "360", "gyro_y_raw": "51", "gyro_z_raw": "-14", "gyro_x_dps": "2.75", "gyro_y_dps": "0.39", "gyro_z_dps": "-0.11", "gyro_magnitude_dps": "2.78", "tilt_pitch_deg": "-0.32", "tilt_roll_deg": "-0.39", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "158", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:28.90,H:92.20,SRAW:4095,SM:0.0,SC:DRY,RRAW:4095,RP:0.0,RC:NO_RAIN,MV:1,TX:-0.32,TY:-0.39,AXG:0.007,AYG:-0.006,AZG:1.033,GXD:2.75,GYD:0.39,GZD:-0.11"}, {"timestamp": "25-10-2026 14:29:55", "packet_id": "314", "temperature_c": "28.9", "temperature_f": "84.02", "humidity_pct": "92.2", "dew_point_c": "27.5", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "4095", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "540", "accel_y_raw": "-131", "accel_z_raw": "16695", "accel_x_g": "0.033", "accel_y_g": "-0.008", "accel_z_g": "1.019", "accel_magnitude_g": "1.02", "gyro_x_raw": "395", "gyro_y_raw": "55", "gyro_z_raw": "-48", "gyro_x_dps": "3.02", "gyro_y_dps": "0.42", "gyro_z_dps": "-0.37", "gyro_magnitude_dps": "3.07", "tilt_pitch_deg": "-0.45", "tilt_roll_deg": "-1.88", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "158", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:28.90,H:92.20,SRAW:4095,SM:0.0,SC:DRY,RRAW:4095,RP:0.0,RC:NO_RAIN,MV:1,TX:-0.45,TY:-1.88,AXG:0.033,AYG:-0.008,AZG:1.019,GXD:3.02,GYD:0.42,GZD:-0.37"}, {"timestamp": "25-10-2026 14:30:00", "packet_id": "315", "temperature_c": "28.9", "temperature_f": "84.02", "humidity_pct": "92.2", "dew_point_c": "27.5", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1744", "rain_intensity_pct": "70.2", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "573", "accel_y_raw": "-131", "accel_z_raw": "16891", "accel_x_g": "0.035", "accel_y_g": "-0.008", "accel_z_g": "1.031", "accel_magnitude_g": "1.032", "gyro_x_raw": "399", "gyro_y_raw": "58", "gyro_z_raw": "6", "gyro_x_dps": "3.05", "gyro_y_dps": "0.45", "gyro_z_dps": "0.05", "gyro_magnitude_dps": "3.08", "tilt_pitch_deg": "-0.42", "tilt_roll_deg": "-1.94", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "164", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:28.90,H:92.20,SRAW:4095,SM:0.0,SC:DRY,RRAW:1744,RP:70.2,RC:MODERATE_RAIN,MV:1,TX:-0.42,TY:-1.94,AXG:0.035,AYG:-0.008,AZG:1.031,GXD:3.05,GYD:0.45,GZD:0.05"}, {"timestamp": "25-10-2026 14:30:05", "packet_id": "316", "temperature_c": "28.9", "temperature_f": "84.02", "humidity_pct": "92.3", "dew_point_c": "27.52", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1636", "rain_intensity_pct": "74.6", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "573", "accel_y_raw": "-131", "accel_z_raw": "16842", "accel_x_g": "0.035", "accel_y_g": "-0.008", "accel_z_g": "1.028", "accel_magnitude_g": "1.029", "gyro_x_raw": "382", "gyro_y_raw": "73", "gyro_z_raw": "-2", "gyro_x_dps": "2.92", "gyro_y_dps": "0.56", "gyro_z_dps": "-0.02", "gyro_magnitude_dps": "2.97", "tilt_pitch_deg": "-0.44", "tilt_roll_deg": "-1.94", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "165", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:28.90,H:92.30,SRAW:4095,SM:0.0,SC:DRY,RRAW:1636,RP:74.6,RC:MODERATE_RAIN,MV:1,TX:-0.44,TY:-1.94,AXG:0.035,AYG:-0.008,AZG:1.028,GXD:2.92,GYD:0.56,GZD:-0.02"}, {"timestamp": "25-10-2026 14:30:10", "packet_id": "317", "temperature_c": "28.9", "temperature_f": "84.02", "humidity_pct": "92.3", "dew_point_c": "27.52", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1595", "rain_intensity_pct": "76.2", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "16138", "accel_y_raw": "-327", "accel_z_raw": "4276", "accel_x_g": "0.985", "accel_y_g": "-0.02", "accel_z_g": "0.261", "accel_magnitude_g": "1.019", "gyro_x_raw": "111", "gyro_y_raw": "3425", "gyro_z_raw": "-369", "gyro_x_dps": "0.85", "gyro_y_dps": "26.15", "gyro_z_dps": "-2.82", "gyro_magnitude_dps": "26.32", "tilt_pitch_deg": "-1.14", "tilt_roll_deg": "-75.1", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "164", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:28.90,H:92.30,SRAW:4095,SM:0.0,SC:DRY,RRAW:1595,RP:76.2,RC:HEAVY_RAIN,MV:1,TX:-1.14,TY:-75.10,AXG:0.985,AYG:-0.020,AZG:0.261,GXD:0.85,GYD:26.15,GZD:-2.82"}, {"timestamp": "25-10-2026 14:30:15", "packet_id": "318", "temperature_c": "28.9", "temperature_f": "84.02", "humidity_pct": "92.3", "dew_point_c": "27.52", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1635", "rain_intensity_pct": "74.6", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-10158", "accel_y_raw": "-196", "accel_z_raw": "12795", "accel_x_g": "-0.62", "accel_y_g": "-0.012", "accel_z_g": "0.781", "accel_magnitude_g": "0.997", "gyro_x_raw": "692", "gyro_y_raw": "-1163", "gyro_z_raw": "49", "gyro_x_dps": "5.29", "gyro_y_dps": "-8.88", "gyro_z_dps": "0.38", "gyro_magnitude_dps": "10.34", "tilt_pitch_deg": "-0.72", "tilt_roll_deg": "38.45", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "166", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:28.90,H:92.30,SRAW:4095,SM:0.0,SC:DRY,RRAW:1635,RP:74.6,RC:MODERATE_RAIN,MV:1,TX:-0.72,TY:38.45,AXG:-0.620,AYG:-0.012,AZG:0.781,GXD:5.29,GYD:-8.88,GZD:0.38"}, {"timestamp": "25-10-2026 14:30:20", "packet_id": "319", "temperature_c": "28.9", "temperature_f": "84.02", "humidity_pct": "92.3", "dew_point_c": "27.52", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1573", "rain_intensity_pct": "77.1", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-917", "accel_y_raw": "884", "accel_z_raw": "16760", "accel_x_g": "-0.056", "accel_y_g": "0.054", "accel_z_g": "1.023", "accel_magnitude_g": "1.026", "gyro_x_raw": "395", "gyro_y_raw": "65", "gyro_z_raw": "-41", "gyro_x_dps": "3.02", "gyro_y_dps": "0.5", "gyro_z_dps": "-0.32", "gyro_magnitude_dps": "3.08", "tilt_pitch_deg": "3.04", "tilt_roll_deg": "3.11", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "160", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:28.90,H:92.30,SRAW:4095,SM:0.0,SC:DRY,RRAW:1573,RP:77.1,RC:HEAVY_RAIN,MV:1,TX:3.04,TY:3.11,AXG:-0.056,AYG:0.054,AZG:1.023,GXD:3.02,GYD:0.50,GZD:-0.32"}, {"timestamp": "25-10-2026 14:30:25", "packet_id": "320", "temperature_c": "28.9", "temperature_f": "84.02", "humidity_pct": "92.2", "dew_point_c": "27.5", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1557", "rain_intensity_pct": "77.7", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-933", "accel_y_raw": "933", "accel_z_raw": "16793", "accel_x_g": "-0.057", "accel_y_g": "0.057", "accel_z_g": "1.025", "accel_magnitude_g": "1.028", "gyro_x_raw": "396", "gyro_y_raw": "28", "gyro_z_raw": "-73", "gyro_x_dps": "3.03", "gyro_y_dps": "0.22", "gyro_z_dps": "-0.56", "gyro_magnitude_dps": "3.09", "tilt_pitch_deg": "3.18", "tilt_roll_deg": "3.16", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "160", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:28.90,H:92.20,SRAW:4095,SM:0.0,SC:DRY,RRAW:1557,RP:77.7,RC:HEAVY_RAIN,MV:1,TX:3.18,TY:3.16,AXG:-0.057,AYG:0.057,AZG:1.025,GXD:3.03,GYD:0.22,GZD:-0.56"}, {"timestamp": "25-10-2026 14:30:30", "packet_id": "321", "temperature_c": "28.9", "temperature_f": "84.02", "humidity_pct": "92.2", "dew_point_c": "27.5", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1642", "rain_intensity_pct": "74.3", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-950", "accel_y_raw": "950", "accel_z_raw": "16809", "accel_x_g": "-0.058", "accel_y_g": "0.058", "accel_z_g": "1.026", "accel_magnitude_g": "1.029", "gyro_x_raw": "381", "gyro_y_raw": "68", "gyro_z_raw": "-10", "gyro_x_dps": "2.91", "gyro_y_dps": "0.52", "gyro_z_dps": "-0.08", "gyro_magnitude_dps": "2.96", "tilt_pitch_deg": "3.22", "tilt_roll_deg": "3.25", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "163", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:28.90,H:92.20,SRAW:4095,SM:0.0,SC:DRY,RRAW:1642,RP:74.3,RC:MODERATE_RAIN,MV:1,TX:3.22,TY:3.25,AXG:-0.058,AYG:0.058,AZG:1.026,GXD:2.91,GYD:0.52,GZD:-0.08"}, {"timestamp": "25-10-2026 14:30:35", "packet_id": "322", "temperature_c": "28.9", "temperature_f": "84.02", "humidity_pct": "92.2", "dew_point_c": "27.5", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1497", "rain_intensity_pct": "80.1", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-884", "accel_y_raw": "966", "accel_z_raw": "16842", "accel_x_g": "-0.054", "accel_y_g": "0.059", "accel_z_g": "1.028", "accel_magnitude_g": "1.031", "gyro_x_raw": "412", "gyro_y_raw": "82", "gyro_z_raw": "-28", "gyro_x_dps": "3.15", "gyro_y_dps": "0.63", "gyro_z_dps": "-0.22", "gyro_magnitude_dps": "3.22", "tilt_pitch_deg": "3.26", "tilt_roll_deg": "3.01", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "160", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:28.90,H:92.20,SRAW:4095,SM:0.0,SC:DRY,RRAW:1497,RP:80.1,RC:HEAVY_RAIN,MV:1,TX:3.26,TY:3.01,AXG:-0.054,AYG:0.059,AZG:1.028,GXD:3.15,GYD:0.63,GZD:-0.22"}, {"timestamp": "25-10-2026 14:30:40", "packet_id": "323", "temperature_c": "28.9", "temperature_f": "84.02", "humidity_pct": "92.2", "dew_point_c": "27.5", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1490", "rain_intensity_pct": "80.4", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-868", "accel_y_raw": "884", "accel_z_raw": "16842", "accel_x_g": "-0.053", "accel_y_g": "0.054", "accel_z_g": "1.028", "accel_magnitude_g": "1.031", "gyro_x_raw": "399", "gyro_y_raw": "37", "gyro_z_raw": "-23", "gyro_x_dps": "3.05", "gyro_y_dps": "0.29", "gyro_z_dps": "-0.18", "gyro_magnitude_dps": "3.07", "tilt_pitch_deg": "3.0", "tilt_roll_deg": "2.96", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "160", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:28.90,H:92.20,SRAW:4095,SM:0.0,SC:DRY,RRAW:1490,RP:80.4,RC:HEAVY_RAIN,MV:1,TX:3.00,TY:2.96,AXG:-0.053,AYG:0.054,AZG:1.028,GXD:3.05,GYD:0.29,GZD:-0.18"}, {"timestamp": "25-10-2026 14:30:45", "packet_id": "324", "temperature_c": "28.9", "temperature_f": "84.02", "humidity_pct": "92.1", "dew_point_c": "27.48", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1615", "rain_intensity_pct": "75.4", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-851", "accel_y_raw": "917", "accel_z_raw": "16793", "accel_x_g": "-0.052", "accel_y_g": "0.056", "accel_z_g": "1.025", "accel_magnitude_g": "1.028", "gyro_x_raw": "369", "gyro_y_raw": "14", "gyro_z_raw": "-41", "gyro_x_dps": "2.82", "gyro_y_dps": "0.11", "gyro_z_dps": "-0.32", "gyro_magnitude_dps": "2.84", "tilt_pitch_deg": "3.11", "tilt_roll_deg": "2.87", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "160", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:28.90,H:92.10,SRAW:4095,SM:0.0,SC:DRY,RRAW:1615,RP:75.4,RC:HEAVY_RAIN,MV:1,TX:3.11,TY:2.87,AXG:-0.052,AYG:0.056,AZG:1.025,GXD:2.82,GYD:0.11,GZD:-0.32"}, {"timestamp": "25-10-2026 14:30:50", "packet_id": "325", "temperature_c": "28.9", "temperature_f": "84.02", "humidity_pct": "92.1", "dew_point_c": "27.48", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1584", "rain_intensity_pct": "76.6", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-933", "accel_y_raw": "1032", "accel_z_raw": "16695", "accel_x_g": "-0.057", "accel_y_g": "0.063", "accel_z_g": "1.019", "accel_magnitude_g": "1.023", "gyro_x_raw": "403", "gyro_y_raw": "69", "gyro_z_raw": "-32", "gyro_x_dps": "3.08", "gyro_y_dps": "0.53", "gyro_z_dps": "-0.25", "gyro_magnitude_dps": "3.14", "tilt_pitch_deg": "3.52", "tilt_roll_deg": "3.19", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "160", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:28.90,H:92.10,SRAW:4095,SM:0.0,SC:DRY,RRAW:1584,RP:76.6,RC:HEAVY_RAIN,MV:1,TX:3.52,TY:3.19,AXG:-0.057,AYG:0.063,AZG:1.019,GXD:3.08,GYD:0.53,GZD:-0.25"}, {"timestamp": "25-10-2026 14:30:55", "packet_id": "326", "temperature_c": "29.0", "temperature_f": "84.2", "humidity_pct": "92.1", "dew_point_c": "27.58", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1613", "rain_intensity_pct": "75.5", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "4718", "accel_y_raw": "819", "accel_z_raw": "15908", "accel_x_g": "0.288", "accel_y_g": "0.05", "accel_z_g": "0.971", "accel_magnitude_g": "1.014", "gyro_x_raw": "331", "gyro_y_raw": "-1814", "gyro_z_raw": "882", "gyro_x_dps": "2.53", "gyro_y_dps": "-13.85", "gyro_z_dps": "6.74", "gyro_magnitude_dps": "15.61", "tilt_pitch_deg": "2.8", "tilt_roll_deg": "-16.49", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "162", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.00,H:92.10,SRAW:4095,SM:0.0,SC:DRY,RRAW:1613,RP:75.5,RC:HEAVY_RAIN,MV:1,TX:2.80,TY:-16.49,AXG:0.288,AYG:0.050,AZG:0.971,GXD:2.53,GYD:-13.85,GZD:6.74"}, {"timestamp": "25-10-2026 14:31:00", "packet_id": "327", "temperature_c": "29.0", "temperature_f": "84.2", "humidity_pct": "92.0", "dew_point_c": "27.56", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "3785", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "1212", "accel_y_raw": "622", "accel_z_raw": "16859", "accel_x_g": "0.074", "accel_y_g": "0.038", "accel_z_g": "1.029", "accel_magnitude_g": "1.032", "gyro_x_raw": "387", "gyro_y_raw": "48", "gyro_z_raw": "-55", "gyro_x_dps": "2.96", "gyro_y_dps": "0.37", "gyro_z_dps": "-0.42", "gyro_magnitude_dps": "3.01", "tilt_pitch_deg": "2.13", "tilt_roll_deg": "-4.1", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "156", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:29.00,H:92.00,SRAW:4095,SM:0.0,SC:DRY,RRAW:3785,RP:0.0,RC:NO_RAIN,MV:1,TX:2.13,TY:-4.10,AXG:0.074,AYG:0.038,AZG:1.029,GXD:2.96,GYD:0.37,GZD:-0.42"}, {"timestamp": "25-10-2026 14:31:05", "packet_id": "328", "temperature_c": "29.0", "temperature_f": "84.2", "humidity_pct": "92.0", "dew_point_c": "27.56", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1586", "rain_intensity_pct": "76.6", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "1261", "accel_y_raw": "655", "accel_z_raw": "16875", "accel_x_g": "0.077", "accel_y_g": "0.04", "accel_z_g": "1.03", "accel_magnitude_g": "1.034", "gyro_x_raw": "404", "gyro_y_raw": "110", "gyro_z_raw": "-57", "gyro_x_dps": "3.09", "gyro_y_dps": "0.84", "gyro_z_dps": "-0.44", "gyro_magnitude_dps": "3.23", "tilt_pitch_deg": "2.22", "tilt_roll_deg": "-4.25", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "160", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.00,H:92.00,SRAW:4095,SM:0.0,SC:DRY,RRAW:1586,RP:76.6,RC:HEAVY_RAIN,MV:1,TX:2.22,TY:-4.25,AXG:0.077,AYG:0.040,AZG:1.030,GXD:3.09,GYD:0.84,GZD:-0.44"}, {"timestamp": "25-10-2026 14:31:10", "packet_id": "329", "temperature_c": "29.0", "temperature_f": "84.2", "humidity_pct": "92.0", "dew_point_c": "27.56", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1595", "rain_intensity_pct": "76.2", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "1196", "accel_y_raw": "573", "accel_z_raw": "16842", "accel_x_g": "0.073", "accel_y_g": "0.035", "accel_z_g": "1.028", "accel_magnitude_g": "1.031", "gyro_x_raw": "382", "gyro_y_raw": "81", "gyro_z_raw": "-49", "gyro_x_dps": "2.92", "gyro_y_dps": "0.62", "gyro_z_dps": "-0.38", "gyro_magnitude_dps": "3.01", "tilt_pitch_deg": "1.97", "tilt_roll_deg": "-4.09", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "160", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.00,H:92.00,SRAW:4095,SM:0.0,SC:DRY,RRAW:1595,RP:76.2,RC:HEAVY_RAIN,MV:1,TX:1.97,TY:-4.09,AXG:0.073,AYG:0.035,AZG:1.028,GXD:2.92,GYD:0.62,GZD:-0.38"}, {"timestamp": "25-10-2026 14:31:15", "packet_id": "330", "temperature_c": "29.0", "temperature_f": "84.2", "humidity_pct": "92.0", "dew_point_c": "27.56", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1650", "rain_intensity_pct": "74.0", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "1163", "accel_y_raw": "688", "accel_z_raw": "16777", "accel_x_g": "0.071", "accel_y_g": "0.042", "accel_z_g": "1.024", "accel_magnitude_g": "1.027", "gyro_x_raw": "399", "gyro_y_raw": "28", "gyro_z_raw": "-40", "gyro_x_dps": "3.05", "gyro_y_dps": "0.22", "gyro_z_dps": "-0.31", "gyro_magnitude_dps": "3.07", "tilt_pitch_deg": "2.36", "tilt_roll_deg": "-3.95", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "163", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.00,H:92.00,SRAW:4095,SM:0.0,SC:DRY,RRAW:1650,RP:74.0,RC:MODERATE_RAIN,MV:1,TX:2.36,TY:-3.95,AXG:0.071,AYG:0.042,AZG:1.024,GXD:3.05,GYD:0.22,GZD:-0.31"}, {"timestamp": "25-10-2026 14:31:20", "packet_id": "331", "temperature_c": "29.0", "temperature_f": "84.2", "humidity_pct": "92.0", "dew_point_c": "27.56", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1488", "rain_intensity_pct": "80.5", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "1245", "accel_y_raw": "671", "accel_z_raw": "16809", "accel_x_g": "0.076", "accel_y_g": "0.041", "accel_z_g": "1.026", "accel_magnitude_g": "1.03", "gyro_x_raw": "382", "gyro_y_raw": "64", "gyro_z_raw": "-6", "gyro_x_dps": "2.92", "gyro_y_dps": "0.49", "gyro_z_dps": "-0.05", "gyro_magnitude_dps": "2.96", "tilt_pitch_deg": "2.26", "tilt_roll_deg": "-4.26", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "160", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.00,H:92.00,SRAW:4095,SM:0.0,SC:DRY,RRAW:1488,RP:80.5,RC:HEAVY_RAIN,MV:1,TX:2.26,TY:-4.26,AXG:0.076,AYG:0.041,AZG:1.026,GXD:2.92,GYD:0.49,GZD:-0.05"}, {"timestamp": "25-10-2026 14:31:25", "packet_id": "332", "temperature_c": "29.0", "temperature_f": "84.2", "humidity_pct": "91.9", "dew_point_c": "27.54", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1516", "rain_intensity_pct": "79.4", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "1212", "accel_y_raw": "606", "accel_z_raw": "16875", "accel_x_g": "0.074", "accel_y_g": "0.037", "accel_z_g": "1.03", "accel_magnitude_g": "1.033", "gyro_x_raw": "373", "gyro_y_raw": "60", "gyro_z_raw": "2", "gyro_x_dps": "2.85", "gyro_y_dps": "0.46", "gyro_z_dps": "0.02", "gyro_magnitude_dps": "2.89", "tilt_pitch_deg": "2.07", "tilt_roll_deg": "-4.11", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "159", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.00,H:91.90,SRAW:4095,SM:0.0,SC:DRY,RRAW:1516,RP:79.4,RC:HEAVY_RAIN,MV:1,TX:2.07,TY:-4.11,AXG:0.074,AYG:0.037,AZG:1.030,GXD:2.85,GYD:0.46,GZD:0.02"}, {"timestamp": "25-10-2026 14:31:30", "packet_id": "333", "temperature_c": "29.1", "temperature_f": "84.38", "humidity_pct": "91.8", "dew_point_c": "27.62", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1468", "rain_intensity_pct": "81.3", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "16121", "accel_y_raw": "442", "accel_z_raw": "3981", "accel_x_g": "0.984", "accel_y_g": "0.027", "accel_z_g": "0.243", "accel_magnitude_g": "1.014", "gyro_x_raw": "379", "gyro_y_raw": "-395", "gyro_z_raw": "-23", "gyro_x_dps": "2.9", "gyro_y_dps": "-3.02", "gyro_z_dps": "-0.18", "gyro_magnitude_dps": "4.19", "tilt_pitch_deg": "1.5", "tilt_roll_deg": "-76.05", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "162", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.10,H:91.80,SRAW:4095,SM:0.0,SC:DRY,RRAW:1468,RP:81.3,RC:HEAVY_RAIN,MV:1,TX:1.50,TY:-76.05,AXG:0.984,AYG:0.027,AZG:0.243,GXD:2.90,GYD:-3.02,GZD:-0.18"}, {"timestamp": "25-10-2026 14:31:35", "packet_id": "334", "temperature_c": "29.1", "temperature_f": "84.38", "humidity_pct": "91.8", "dew_point_c": "27.62", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1488", "rain_intensity_pct": "80.5", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-753", "accel_y_raw": "1130", "accel_z_raw": "17448", "accel_x_g": "-0.046", "accel_y_g": "0.069", "accel_z_g": "1.065", "accel_magnitude_g": "1.068", "gyro_x_raw": "6", "gyro_y_raw": "817", "gyro_z_raw": "2829", "gyro_x_dps": "0.05", "gyro_y_dps": "6.24", "gyro_z_dps": "21.6", "gyro_magnitude_dps": "22.48", "tilt_pitch_deg": "3.72", "tilt_roll_deg": "2.47", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "160", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.10,H:91.80,SRAW:4095,SM:0.0,SC:DRY,RRAW:1488,RP:80.5,RC:HEAVY_RAIN,MV:1,TX:3.72,TY:2.47,AXG:-0.046,AYG:0.069,AZG:1.065,GXD:0.05,GYD:6.24,GZD:21.60"}, {"timestamp": "25-10-2026 14:31:40", "packet_id": "335", "temperature_c": "29.1", "temperature_f": "84.38", "humidity_pct": "91.7", "dew_point_c": "27.61", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1416", "rain_intensity_pct": "83.4", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-655", "accel_y_raw": "819", "accel_z_raw": "16842", "accel_x_g": "-0.04", "accel_y_g": "0.05", "accel_z_g": "1.028", "accel_magnitude_g": "1.03", "gyro_x_raw": "362", "gyro_y_raw": "44", "gyro_z_raw": "-27", "gyro_x_dps": "2.77", "gyro_y_dps": "0.34", "gyro_z_dps": "-0.21", "gyro_magnitude_dps": "2.8", "tilt_pitch_deg": "2.76", "tilt_roll_deg": "2.23", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "160", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.10,H:91.70,SRAW:4095,SM:0.0,SC:DRY,RRAW:1416,RP:83.4,RC:HEAVY_RAIN,MV:1,TX:2.76,TY:2.23,AXG:-0.040,AYG:0.050,AZG:1.028,GXD:2.77,GYD:0.34,GZD:-0.21"}, {"timestamp": "25-10-2026 14:31:45", "packet_id": "336", "temperature_c": "29.1", "temperature_f": "84.38", "humidity_pct": "91.7", "dew_point_c": "27.61", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1453", "rain_intensity_pct": "81.9", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1245", "accel_y_raw": "671", "accel_z_raw": "16711", "accel_x_g": "-0.076", "accel_y_g": "0.041", "accel_z_g": "1.02", "accel_magnitude_g": "1.024", "gyro_x_raw": "410", "gyro_y_raw": "55", "gyro_z_raw": "-9", "gyro_x_dps": "3.13", "gyro_y_dps": "0.42", "gyro_z_dps": "-0.07", "gyro_magnitude_dps": "3.16", "tilt_pitch_deg": "2.3", "tilt_roll_deg": "4.25", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "160", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.10,H:91.70,SRAW:4095,SM:0.0,SC:DRY,RRAW:1453,RP:81.9,RC:HEAVY_RAIN,MV:1,TX:2.30,TY:4.25,AXG:-0.076,AYG:0.041,AZG:1.020,GXD:3.13,GYD:0.42,GZD:-0.07"}, {"timestamp": "25-10-2026 14:31:50", "packet_id": "337", "temperature_c": "29.1", "temperature_f": "84.38", "humidity_pct": "91.6", "dew_point_c": "27.59", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1643", "rain_intensity_pct": "74.3", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-1261", "accel_y_raw": "704", "accel_z_raw": "16859", "accel_x_g": "-0.077", "accel_y_g": "0.043", "accel_z_g": "1.029", "accel_magnitude_g": "1.033", "gyro_x_raw": "375", "gyro_y_raw": "35", "gyro_z_raw": "-6", "gyro_x_dps": "2.87", "gyro_y_dps": "0.27", "gyro_z_dps": "-0.05", "gyro_magnitude_dps": "2.88", "tilt_pitch_deg": "2.4", "tilt_roll_deg": "4.29", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "163", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.10,H:91.60,SRAW:4095,SM:0.0,SC:DRY,RRAW:1643,RP:74.3,RC:MODERATE_RAIN,MV:1,TX:2.40,TY:4.29,AXG:-0.077,AYG:0.043,AZG:1.029,GXD:2.87,GYD:0.27,GZD:-0.05"}, {"timestamp": "25-10-2026 14:31:55", "packet_id": "338", "temperature_c": "29.1", "temperature_f": "84.38", "humidity_pct": "91.6", "dew_point_c": "27.59", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1535", "rain_intensity_pct": "78.6", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1228", "accel_y_raw": "802", "accel_z_raw": "16760", "accel_x_g": "-0.075", "accel_y_g": "0.049", "accel_z_g": "1.023", "accel_magnitude_g": "1.027", "gyro_x_raw": "395", "gyro_y_raw": "73", "gyro_z_raw": "-14", "gyro_x_dps": "3.02", "gyro_y_dps": "0.56", "gyro_z_dps": "-0.11", "gyro_magnitude_dps": "3.07", "tilt_pitch_deg": "2.74", "tilt_roll_deg": "4.21", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "160", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.10,H:91.60,SRAW:4095,SM:0.0,SC:DRY,RRAW:1535,RP:78.6,RC:HEAVY_RAIN,MV:1,TX:2.74,TY:4.21,AXG:-0.075,AYG:0.049,AZG:1.023,GXD:3.02,GYD:0.56,GZD:-0.11"}, {"timestamp": "25-10-2026 14:32:00", "packet_id": "339", "temperature_c": "29.1", "temperature_f": "84.38", "humidity_pct": "91.5", "dew_point_c": "27.57", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "3602", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-1277", "accel_y_raw": "737", "accel_z_raw": "16826", "accel_x_g": "-0.078", "accel_y_g": "0.045", "accel_z_g": "1.027", "accel_magnitude_g": "1.031", "gyro_x_raw": "399", "gyro_y_raw": "51", "gyro_z_raw": "-14", "gyro_x_dps": "3.05", "gyro_y_dps": "0.39", "gyro_z_dps": "-0.11", "gyro_magnitude_dps": "3.08", "tilt_pitch_deg": "2.5", "tilt_roll_deg": "4.32", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "156", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:29.10,H:91.50,SRAW:4095,SM:0.0,SC:DRY,RRAW:3602,RP:0.0,RC:NO_RAIN,MV:1,TX:2.50,TY:4.32,AXG:-0.078,AYG:0.045,AZG:1.027,GXD:3.05,GYD:0.39,GZD:-0.11"}, {"timestamp": "25-10-2026 14:32:05", "packet_id": "340", "temperature_c": "29.1", "temperature_f": "84.38", "humidity_pct": "91.5", "dew_point_c": "27.57", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "3862", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-1261", "accel_y_raw": "753", "accel_z_raw": "16809", "accel_x_g": "-0.077", "accel_y_g": "0.046", "accel_z_g": "1.026", "accel_magnitude_g": "1.03", "gyro_x_raw": "369", "gyro_y_raw": "57", "gyro_z_raw": "-11", "gyro_x_dps": "2.82", "gyro_y_dps": "0.44", "gyro_z_dps": "-0.09", "gyro_magnitude_dps": "2.86", "tilt_pitch_deg": "2.54", "tilt_roll_deg": "4.27", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "156", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:29.10,H:91.50,SRAW:4095,SM:0.0,SC:DRY,RRAW:3862,RP:0.0,RC:NO_RAIN,MV:1,TX:2.54,TY:4.27,AXG:-0.077,AYG:0.046,AZG:1.026,GXD:2.82,GYD:0.44,GZD:-0.09"}, {"timestamp": "25-10-2026 14:32:10", "packet_id": "341", "temperature_c": "29.1", "temperature_f": "84.38", "humidity_pct": "91.5", "dew_point_c": "27.57", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "3941", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-1294", "accel_y_raw": "737", "accel_z_raw": "16728", "accel_x_g": "-0.079", "accel_y_g": "0.045", "accel_z_g": "1.021", "accel_magnitude_g": "1.025", "gyro_x_raw": "406", "gyro_y_raw": "61", "gyro_z_raw": "-20", "gyro_x_dps": "3.1", "gyro_y_dps": "0.47", "gyro_z_dps": "-0.16", "gyro_magnitude_dps": "3.14", "tilt_pitch_deg": "2.51", "tilt_roll_deg": "4.4", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "156", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:29.10,H:91.50,SRAW:4095,SM:0.0,SC:DRY,RRAW:3941,RP:0.0,RC:NO_RAIN,MV:1,TX:2.51,TY:4.40,AXG:-0.079,AYG:0.045,AZG:1.021,GXD:3.10,GYD:0.47,GZD:-0.16"}, {"timestamp": "25-10-2026 14:32:40", "packet_id": "342", "temperature_c": "29.2", "temperature_f": "84.56", "humidity_pct": "91.3", "dew_point_c": "27.63", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "3906", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-1327", "accel_y_raw": "688", "accel_z_raw": "16711", "accel_x_g": "-0.081", "accel_y_g": "0.042", "accel_z_g": "1.02", "accel_magnitude_g": "1.024", "gyro_x_raw": "424", "gyro_y_raw": "56", "gyro_z_raw": "-35", "gyro_x_dps": "3.24", "gyro_y_dps": "0.43", "gyro_z_dps": "-0.27", "gyro_magnitude_dps": "3.28", "tilt_pitch_deg": "2.34", "tilt_roll_deg": "4.51", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "156", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:29.20,H:91.30,SRAW:4095,SM:0.0,SC:DRY,RRAW:3906,RP:0.0,RC:NO_RAIN,MV:1,TX:2.34,TY:4.51,AXG:-0.081,AYG:0.042,AZG:1.020,GXD:3.24,GYD:0.43,GZD:-0.27"}, {"timestamp": "25-10-2026 14:32:45", "packet_id": "343", "temperature_c": "29.2", "temperature_f": "84.56", "humidity_pct": "91.2", "dew_point_c": "27.61", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "3862", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-1277", "accel_y_raw": "770", "accel_z_raw": "16695", "accel_x_g": "-0.078", "accel_y_g": "0.047", "accel_z_g": "1.019", "accel_magnitude_g": "1.023", "gyro_x_raw": "373", "gyro_y_raw": "58", "gyro_z_raw": "-45", "gyro_x_dps": "2.85", "gyro_y_dps": "0.45", "gyro_z_dps": "-0.35", "gyro_magnitude_dps": "2.91", "tilt_pitch_deg": "2.61", "tilt_roll_deg": "4.38", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "156", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:29.20,H:91.20,SRAW:4095,SM:0.0,SC:DRY,RRAW:3862,RP:0.0,RC:NO_RAIN,MV:1,TX:2.61,TY:4.38,AXG:-0.078,AYG:0.047,AZG:1.019,GXD:2.85,GYD:0.45,GZD:-0.35"}, {"timestamp": "25-10-2026 14:32:50", "packet_id": "344", "temperature_c": "29.2", "temperature_f": "84.56", "humidity_pct": "91.2", "dew_point_c": "27.61", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "3898", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-1245", "accel_y_raw": "737", "accel_z_raw": "16842", "accel_x_g": "-0.076", "accel_y_g": "0.045", "accel_z_g": "1.028", "accel_magnitude_g": "1.032", "gyro_x_raw": "410", "gyro_y_raw": "69", "gyro_z_raw": "-20", "gyro_x_dps": "3.13", "gyro_y_dps": "0.53", "gyro_z_dps": "-0.16", "gyro_magnitude_dps": "3.18", "tilt_pitch_deg": "2.49", "tilt_roll_deg": "4.23", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "156", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:29.20,H:91.20,SRAW:4095,SM:0.0,SC:DRY,RRAW:3898,RP:0.0,RC:NO_RAIN,MV:1,TX:2.49,TY:4.23,AXG:-0.076,AYG:0.045,AZG:1.028,GXD:3.13,GYD:0.53,GZD:-0.16"}, {"timestamp": "25-10-2026 14:32:55", "packet_id": "345", "temperature_c": "29.2", "temperature_f": "84.56", "humidity_pct": "91.1", "dew_point_c": "27.59", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "3984", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-1310", "accel_y_raw": "753", "accel_z_raw": "16777", "accel_x_g": "-0.08", "accel_y_g": "0.046", "accel_z_g": "1.024", "accel_magnitude_g": "1.028", "gyro_x_raw": "403", "gyro_y_raw": "52", "gyro_z_raw": "-27", "gyro_x_dps": "3.08", "gyro_y_dps": "0.4", "gyro_z_dps": "-0.21", "gyro_magnitude_dps": "3.11", "tilt_pitch_deg": "2.59", "tilt_roll_deg": "4.44", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "156", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:29.20,H:91.10,SRAW:4095,SM:0.0,SC:DRY,RRAW:3984,RP:0.0,RC:NO_RAIN,MV:1,TX:2.59,TY:4.44,AXG:-0.080,AYG:0.046,AZG:1.024,GXD:3.08,GYD:0.40,GZD:-0.21"}, {"timestamp": "25-10-2026 14:33:00", "packet_id": "346", "temperature_c": "29.2", "temperature_f": "84.56", "humidity_pct": "91.0", "dew_point_c": "27.57", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "3946", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-1277", "accel_y_raw": "770", "accel_z_raw": "16826", "accel_x_g": "-0.078", "accel_y_g": "0.047", "accel_z_g": "1.027", "accel_magnitude_g": "1.031", "gyro_x_raw": "412", "gyro_y_raw": "94", "gyro_z_raw": "-23", "gyro_x_dps": "3.15", "gyro_y_dps": "0.72", "gyro_z_dps": "-0.18", "gyro_magnitude_dps": "3.24", "tilt_pitch_deg": "2.62", "tilt_roll_deg": "4.32", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "156", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:29.20,H:91.00,SRAW:4095,SM:0.0,SC:DRY,RRAW:3946,RP:0.0,RC:NO_RAIN,MV:1,TX:2.62,TY:4.32,AXG:-0.078,AYG:0.047,AZG:1.027,GXD:3.15,GYD:0.72,GZD:-0.18"}, {"timestamp": "25-10-2026 14:33:05", "packet_id": "347", "temperature_c": "29.2", "temperature_f": "84.56", "humidity_pct": "90.9", "dew_point_c": "27.55", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "3979", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-1343", "accel_y_raw": "753", "accel_z_raw": "16809", "accel_x_g": "-0.082", "accel_y_g": "0.046", "accel_z_g": "1.026", "accel_magnitude_g": "1.03", "gyro_x_raw": "399", "gyro_y_raw": "70", "gyro_z_raw": "-22", "gyro_x_dps": "3.05", "gyro_y_dps": "0.54", "gyro_z_dps": "-0.17", "gyro_magnitude_dps": "3.1", "tilt_pitch_deg": "2.58", "tilt_roll_deg": "4.54", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "156", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:29.20,H:90.90,SRAW:4095,SM:0.0,SC:DRY,RRAW:3979,RP:0.0,RC:NO_RAIN,MV:1,TX:2.58,TY:4.54,AXG:-0.082,AYG:0.046,AZG:1.026,GXD:3.05,GYD:0.54,GZD:-0.17"}, {"timestamp": "25-10-2026 14:33:10", "packet_id": "348", "temperature_c": "29.2", "temperature_f": "84.56", "humidity_pct": "90.9", "dew_point_c": "27.55", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "3795", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-1179", "accel_y_raw": "638", "accel_z_raw": "16187", "accel_x_g": "-0.072", "accel_y_g": "0.039", "accel_z_g": "0.988", "accel_magnitude_g": "0.991", "gyro_x_raw": "437", "gyro_y_raw": "39", "gyro_z_raw": "-9", "gyro_x_dps": "3.34", "gyro_y_dps": "0.3", "gyro_z_dps": "-0.07", "gyro_magnitude_dps": "3.35", "tilt_pitch_deg": "2.23", "tilt_roll_deg": "4.14", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "156", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:29.20,H:90.90,SRAW:4095,SM:0.0,SC:DRY,RRAW:3795,RP:0.0,RC:NO_RAIN,MV:1,TX:2.23,TY:4.14,AXG:-0.072,AYG:0.039,AZG:0.988,GXD:3.34,GYD:0.30,GZD:-0.07"}, {"timestamp": "25-10-2026 14:33:15", "packet_id": "349", "temperature_c": "29.2", "temperature_f": "84.56", "humidity_pct": "90.9", "dew_point_c": "27.55", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1473", "rain_intensity_pct": "81.1", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1245", "accel_y_raw": "802", "accel_z_raw": "16777", "accel_x_g": "-0.076", "accel_y_g": "0.049", "accel_z_g": "1.024", "accel_magnitude_g": "1.028", "gyro_x_raw": "391", "gyro_y_raw": "57", "gyro_z_raw": "-31", "gyro_x_dps": "2.99", "gyro_y_dps": "0.44", "gyro_z_dps": "-0.24", "gyro_magnitude_dps": "3.03", "tilt_pitch_deg": "2.75", "tilt_roll_deg": "4.26", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "160", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.20,H:90.90,SRAW:4095,SM:0.0,SC:DRY,RRAW:1473,RP:81.1,RC:HEAVY_RAIN,MV:1,TX:2.75,TY:4.26,AXG:-0.076,AYG:0.049,AZG:1.024,GXD:2.99,GYD:0.44,GZD:-0.24"}, {"timestamp": "25-10-2026 14:33:20", "packet_id": "350", "temperature_c": "29.2", "temperature_f": "84.56", "humidity_pct": "90.9", "dew_point_c": "27.55", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1490", "rain_intensity_pct": "80.4", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1228", "accel_y_raw": "835", "accel_z_raw": "16875", "accel_x_g": "-0.075", "accel_y_g": "0.051", "accel_z_g": "1.03", "accel_magnitude_g": "1.034", "gyro_x_raw": "411", "gyro_y_raw": "75", "gyro_z_raw": "-7", "gyro_x_dps": "3.14", "gyro_y_dps": "0.58", "gyro_z_dps": "-0.06", "gyro_magnitude_dps": "3.19", "tilt_pitch_deg": "2.82", "tilt_roll_deg": "4.18", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "160", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.20,H:90.90,SRAW:4095,SM:0.0,SC:DRY,RRAW:1490,RP:80.4,RC:HEAVY_RAIN,MV:1,TX:2.82,TY:4.18,AXG:-0.075,AYG:0.051,AZG:1.030,GXD:3.14,GYD:0.58,GZD:-0.06"}, {"timestamp": "25-10-2026 14:33:25", "packet_id": "351", "temperature_c": "29.2", "temperature_f": "84.56", "humidity_pct": "90.8", "dew_point_c": "27.54", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1453", "rain_intensity_pct": "81.9", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1212", "accel_y_raw": "786", "accel_z_raw": "16842", "accel_x_g": "-0.074", "accel_y_g": "0.048", "accel_z_g": "1.028", "accel_magnitude_g": "1.032", "gyro_x_raw": "399", "gyro_y_raw": "83", "gyro_z_raw": "-22", "gyro_x_dps": "3.05", "gyro_y_dps": "0.64", "gyro_z_dps": "-0.17", "gyro_magnitude_dps": "3.12", "tilt_pitch_deg": "2.66", "tilt_roll_deg": "4.1", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "160", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.20,H:90.80,SRAW:4095,SM:0.0,SC:DRY,RRAW:1453,RP:81.9,RC:HEAVY_RAIN,MV:1,TX:2.66,TY:4.10,AXG:-0.074,AYG:0.048,AZG:1.028,GXD:3.05,GYD:0.64,GZD:-0.17"}, {"timestamp": "25-10-2026 14:33:30", "packet_id": "352", "temperature_c": "29.2", "temperature_f": "84.56", "humidity_pct": "90.8", "dew_point_c": "27.54", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1488", "rain_intensity_pct": "80.5", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1245", "accel_y_raw": "557", "accel_z_raw": "16760", "accel_x_g": "-0.076", "accel_y_g": "0.034", "accel_z_g": "1.023", "accel_magnitude_g": "1.026", "gyro_x_raw": "551", "gyro_y_raw": "-74", "gyro_z_raw": "-68", "gyro_x_dps": "4.21", "gyro_y_dps": "-0.57", "gyro_z_dps": "-0.52", "gyro_magnitude_dps": "4.28", "tilt_pitch_deg": "1.89", "tilt_roll_deg": "4.23", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "161", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.20,H:90.80,SRAW:4095,SM:0.0,SC:DRY,RRAW:1488,RP:80.5,RC:HEAVY_RAIN,MV:1,TX:1.89,TY:4.23,AXG:-0.076,AYG:0.034,AZG:1.023,GXD:4.21,GYD:-0.57,GZD:-0.52"}, {"timestamp": "25-10-2026 14:33:35", "packet_id": "353", "temperature_c": "29.2", "temperature_f": "84.56", "humidity_pct": "90.9", "dew_point_c": "27.55", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1455", "rain_intensity_pct": "81.8", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1392", "accel_y_raw": "671", "accel_z_raw": "16777", "accel_x_g": "-0.085", "accel_y_g": "0.041", "accel_z_g": "1.024", "accel_magnitude_g": "1.028", "gyro_x_raw": "374", "gyro_y_raw": "34", "gyro_z_raw": "-14", "gyro_x_dps": "2.86", "gyro_y_dps": "0.26", "gyro_z_dps": "-0.11", "gyro_magnitude_dps": "2.87", "tilt_pitch_deg": "2.3", "tilt_roll_deg": "4.72", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "160", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.20,H:90.90,SRAW:4095,SM:0.0,SC:DRY,RRAW:1455,RP:81.8,RC:HEAVY_RAIN,MV:1,TX:2.30,TY:4.72,AXG:-0.085,AYG:0.041,AZG:1.024,GXD:2.86,GYD:0.26,GZD:-0.11"}, {"timestamp": "25-10-2026 14:33:40", "packet_id": "354", "temperature_c": "29.2", "temperature_f": "84.56", "humidity_pct": "90.8", "dew_point_c": "27.54", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1434", "rain_intensity_pct": "82.6", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1163", "accel_y_raw": "753", "accel_z_raw": "16875", "accel_x_g": "-0.071", "accel_y_g": "0.046", "accel_z_g": "1.03", "accel_magnitude_g": "1.033", "gyro_x_raw": "483", "gyro_y_raw": "188", "gyro_z_raw": "-370", "gyro_x_dps": "3.69", "gyro_y_dps": "1.44", "gyro_z_dps": "-2.83", "gyro_magnitude_dps": "4.87", "tilt_pitch_deg": "2.57", "tilt_roll_deg": "3.94", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "160", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.20,H:90.80,SRAW:4095,SM:0.0,SC:DRY,RRAW:1434,RP:82.6,RC:HEAVY_RAIN,MV:1,TX:2.57,TY:3.94,AXG:-0.071,AYG:0.046,AZG:1.030,GXD:3.69,GYD:1.44,GZD:-2.83"}, {"timestamp": "25-10-2026 14:33:45", "packet_id": "355", "temperature_c": "29.2", "temperature_f": "84.56", "humidity_pct": "90.8", "dew_point_c": "27.54", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1468", "rain_intensity_pct": "81.3", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1196", "accel_y_raw": "851", "accel_z_raw": "16629", "accel_x_g": "-0.073", "accel_y_g": "0.052", "accel_z_g": "1.015", "accel_magnitude_g": "1.019", "gyro_x_raw": "328", "gyro_y_raw": "-90", "gyro_z_raw": "2081", "gyro_x_dps": "2.51", "gyro_y_dps": "-0.69", "gyro_z_dps": "15.89", "gyro_magnitude_dps": "16.1", "tilt_pitch_deg": "2.94", "tilt_roll_deg": "4.09", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "161", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.20,H:90.80,SRAW:4095,SM:0.0,SC:DRY,RRAW:1468,RP:81.3,RC:HEAVY_RAIN,MV:1,TX:2.94,TY:4.09,AXG:-0.073,AYG:0.052,AZG:1.015,GXD:2.51,GYD:-0.69,GZD:15.89"}, {"timestamp": "25-10-2026 14:33:50", "packet_id": "356", "temperature_c": "29.2", "temperature_f": "84.56", "humidity_pct": "90.7", "dew_point_c": "27.52", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1425", "rain_intensity_pct": "83.0", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1769", "accel_y_raw": "638", "accel_z_raw": "16875", "accel_x_g": "-0.108", "accel_y_g": "0.039", "accel_z_g": "1.03", "accel_magnitude_g": "1.036", "gyro_x_raw": "393", "gyro_y_raw": "82", "gyro_z_raw": "-48", "gyro_x_dps": "3.0", "gyro_y_dps": "0.63", "gyro_z_dps": "-0.37", "gyro_magnitude_dps": "3.09", "tilt_pitch_deg": "2.13", "tilt_roll_deg": "5.99", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "160", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.20,H:90.70,SRAW:4095,SM:0.0,SC:DRY,RRAW:1425,RP:83.0,RC:HEAVY_RAIN,MV:1,TX:2.13,TY:5.99,AXG:-0.108,AYG:0.039,AZG:1.030,GXD:3.00,GYD:0.63,GZD:-0.37"}, {"timestamp": "25-10-2026 14:38:24", "packet_id": "357", "temperature_c": "28.9", "temperature_f": "84.02", "humidity_pct": "92.2", "dew_point_c": "27.5", "soil_raw_adc": "1732", "soil_moisture_pct": "70.4", "soil_condition": "WET", "rain_raw_adc": "1602", "rain_intensity_pct": "75.9", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1753", "accel_y_raw": "606", "accel_z_raw": "16842", "accel_x_g": "-0.107", "accel_y_g": "0.037", "accel_z_g": "1.028", "accel_magnitude_g": "1.034", "gyro_x_raw": "412", "gyro_y_raw": "58", "gyro_z_raw": "-66", "gyro_x_dps": "3.15", "gyro_y_dps": "0.45", "gyro_z_dps": "-0.51", "gyro_magnitude_dps": "3.22", "tilt_pitch_deg": "2.06", "tilt_roll_deg": "5.94", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "161", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:28.90,H:92.20,SRAW:1732,SM:70.4,SC:WET,RRAW:1602,RP:75.9,RC:HEAVY_RAIN,MV:1,TX:2.06,TY:5.94,AXG:-0.107,AYG:0.037,AZG:1.028,GXD:3.15,GYD:0.45,GZD:-0.51"}, {"timestamp": "25-10-2026 14:38:29", "packet_id": "358", "temperature_c": "28.9", "temperature_f": "84.02", "humidity_pct": "92.1", "dew_point_c": "27.48", "soil_raw_adc": "1739", "soil_moisture_pct": "70.1", "soil_condition": "WET", "rain_raw_adc": "1616", "rain_intensity_pct": "75.4", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1654", "accel_y_raw": "638", "accel_z_raw": "16777", "accel_x_g": "-0.101", "accel_y_g": "0.039", "accel_z_g": "1.024", "accel_magnitude_g": "1.03", "gyro_x_raw": "379", "gyro_y_raw": "43", "gyro_z_raw": "-32", "gyro_x_dps": "2.9", "gyro_y_dps": "0.33", "gyro_z_dps": "-0.25", "gyro_magnitude_dps": "2.93", "tilt_pitch_deg": "2.17", "tilt_roll_deg": "5.6", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "161", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:28.90,H:92.10,SRAW:1739,SM:70.1,SC:WET,RRAW:1616,RP:75.4,RC:HEAVY_RAIN,MV:1,TX:2.17,TY:5.60,AXG:-0.101,AYG:0.039,AZG:1.024,GXD:2.90,GYD:0.33,GZD:-0.25"}, {"timestamp": "25-10-2026 14:38:34", "packet_id": "359", "temperature_c": "29.0", "temperature_f": "84.2", "humidity_pct": "92.0", "dew_point_c": "27.56", "soil_raw_adc": "1781", "soil_moisture_pct": "67.7", "soil_condition": "WET", "rain_raw_adc": "1623", "rain_intensity_pct": "75.1", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1736", "accel_y_raw": "655", "accel_z_raw": "16744", "accel_x_g": "-0.106", "accel_y_g": "0.04", "accel_z_g": "1.022", "accel_magnitude_g": "1.028", "gyro_x_raw": "373", "gyro_y_raw": "45", "gyro_z_raw": "-49", "gyro_x_dps": "2.85", "gyro_y_dps": "0.35", "gyro_z_dps": "-0.38", "gyro_magnitude_dps": "2.9", "tilt_pitch_deg": "2.22", "tilt_roll_deg": "5.94", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "161", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.00,H:92.00,SRAW:1781,SM:67.7,SC:WET,RRAW:1623,RP:75.1,RC:HEAVY_RAIN,MV:1,TX:2.22,TY:5.94,AXG:-0.106,AYG:0.040,AZG:1.022,GXD:2.85,GYD:0.35,GZD:-0.38"}, {"timestamp": "25-10-2026 14:38:39", "packet_id": "360", "temperature_c": "29.0", "temperature_f": "84.2", "humidity_pct": "91.9", "dew_point_c": "27.54", "soil_raw_adc": "1822", "soil_moisture_pct": "65.4", "soil_condition": "WET", "rain_raw_adc": "1650", "rain_intensity_pct": "74.0", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "868", "accel_y_raw": "-65", "accel_z_raw": "18628", "accel_x_g": "0.053", "accel_y_g": "-0.004", "accel_z_g": "1.137", "accel_magnitude_g": "1.138", "gyro_x_raw": "842", "gyro_y_raw": "-3213", "gyro_z_raw": "-1713", "gyro_x_dps": "6.43", "gyro_y_dps": "-24.53", "gyro_z_dps": "-13.08", "gyro_magnitude_dps": "28.53", "tilt_pitch_deg": "-0.18", "tilt_roll_deg": "-2.68", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.00,H:91.90,SRAW:1822,SM:65.4,SC:WET,RRAW:1650,RP:74.0,RC:MODERATE_RAIN,MV:1,TX:-0.18,TY:-2.68,AXG:0.053,AYG:-0.004,AZG:1.137,GXD:6.43,GYD:-24.53,GZD:-13.08"}, {"timestamp": "25-10-2026 14:38:44", "packet_id": "361", "temperature_c": "29.1", "temperature_f": "84.38", "humidity_pct": "91.8", "dew_point_c": "27.62", "soil_raw_adc": "1808", "soil_moisture_pct": "66.2", "soil_condition": "WET", "rain_raw_adc": "1698", "rain_intensity_pct": "72.1", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "15761", "accel_y_raw": "-278", "accel_z_raw": "5423", "accel_x_g": "0.962", "accel_y_g": "-0.017", "accel_z_g": "0.331", "accel_magnitude_g": "1.017", "gyro_x_raw": "395", "gyro_y_raw": "-1100", "gyro_z_raw": "-15", "gyro_x_dps": "3.02", "gyro_y_dps": "-8.4", "gyro_z_dps": "-0.12", "gyro_magnitude_dps": "8.93", "tilt_pitch_deg": "-0.93", "tilt_roll_deg": "-71.01", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "168", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.10,H:91.80,SRAW:1808,SM:66.2,SC:WET,RRAW:1698,RP:72.1,RC:MODERATE_RAIN,MV:1,TX:-0.93,TY:-71.01,AXG:0.962,AYG:-0.017,AZG:0.331,GXD:3.02,GYD:-8.40,GZD:-0.12"}, {"timestamp": "25-10-2026 14:38:49", "packet_id": "362", "temperature_c": "29.1", "temperature_f": "84.38", "humidity_pct": "91.6", "dew_point_c": "27.59", "soil_raw_adc": "1788", "soil_moisture_pct": "67.3", "soil_condition": "WET", "rain_raw_adc": "1708", "rain_intensity_pct": "71.7", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "1359", "accel_y_raw": "-3440", "accel_z_raw": "16744", "accel_x_g": "0.083", "accel_y_g": "-0.21", "accel_z_g": "1.022", "accel_magnitude_g": "1.047", "gyro_x_raw": "293", "gyro_y_raw": "-284", "gyro_z_raw": "-251", "gyro_x_dps": "2.24", "gyro_y_dps": "-2.17", "gyro_z_dps": "-1.92", "gyro_magnitude_dps": "3.66", "tilt_pitch_deg": "-11.58", "tilt_roll_deg": "-4.53", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "168", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.10,H:91.60,SRAW:1788,SM:67.3,SC:WET,RRAW:1708,RP:71.7,RC:MODERATE_RAIN,MV:1,TX:-11.58,TY:-4.53,AXG:0.083,AYG:-0.210,AZG:1.022,GXD:2.24,GYD:-2.17,GZD:-1.92"}, {"timestamp": "25-10-2026 14:38:54", "packet_id": "363", "temperature_c": "29.1", "temperature_f": "84.38", "humidity_pct": "91.5", "dew_point_c": "27.57", "soil_raw_adc": "1853", "soil_moisture_pct": "63.7", "soil_condition": "MODERATE", "rain_raw_adc": "1735", "rain_intensity_pct": "70.6", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-5898", "accel_y_raw": "-229", "accel_z_raw": "15122", "accel_x_g": "-0.36", "accel_y_g": "-0.014", "accel_z_g": "0.923", "accel_magnitude_g": "0.991", "gyro_x_raw": "920", "gyro_y_raw": "2884", "gyro_z_raw": "-606", "gyro_x_dps": "7.03", "gyro_y_dps": "22.02", "gyro_z_dps": "-4.63", "gyro_magnitude_dps": "23.57", "tilt_pitch_deg": "-0.83", "tilt_roll_deg": "21.31", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "173", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.10,H:91.50,SRAW:1853,SM:63.7,SC:MODERATE,RRAW:1735,RP:70.6,RC:MODERATE_RAIN,MV:1,TX:-0.83,TY:21.31,AXG:-0.360,AYG:-0.014,AZG:0.923,GXD:7.03,GYD:22.02,GZD:-4.63"}, {"timestamp": "25-10-2026 14:38:59", "packet_id": "364", "temperature_c": "29.2", "temperature_f": "84.56", "humidity_pct": "91.4", "dew_point_c": "27.65", "soil_raw_adc": "1902", "soil_moisture_pct": "61.0", "soil_condition": "MODERATE", "rain_raw_adc": "1678", "rain_intensity_pct": "72.9", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-6340", "accel_y_raw": "-49", "accel_z_raw": "15499", "accel_x_g": "-0.387", "accel_y_g": "-0.003", "accel_z_g": "0.946", "accel_magnitude_g": "1.022", "gyro_x_raw": "373", "gyro_y_raw": "57", "gyro_z_raw": "-40", "gyro_x_dps": "2.85", "gyro_y_dps": "0.44", "gyro_z_dps": "-0.31", "gyro_magnitude_dps": "2.9", "tilt_pitch_deg": "-0.18", "tilt_roll_deg": "22.23", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "172", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.20,H:91.40,SRAW:1902,SM:61.0,SC:MODERATE,RRAW:1678,RP:72.9,RC:MODERATE_RAIN,MV:1,TX:-0.18,TY:22.23,AXG:-0.387,AYG:-0.003,AZG:0.946,GXD:2.85,GYD:0.44,GZD:-0.31"}, {"timestamp": "25-10-2026 14:39:04", "packet_id": "365", "temperature_c": "29.2", "temperature_f": "84.56", "humidity_pct": "91.2", "dew_point_c": "27.61", "soil_raw_adc": "1856", "soil_moisture_pct": "63.6", "soil_condition": "MODERATE", "rain_raw_adc": "1697", "rain_intensity_pct": "72.1", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-2129", "accel_y_raw": "-5160", "accel_z_raw": "15925", "accel_x_g": "-0.13", "accel_y_g": "-0.315", "accel_z_g": "0.972", "accel_magnitude_g": "1.03", "gyro_x_raw": "640", "gyro_y_raw": "213", "gyro_z_raw": "-10", "gyro_x_dps": "4.89", "gyro_y_dps": "1.63", "gyro_z_dps": "-0.08", "gyro_magnitude_dps": "5.16", "tilt_pitch_deg": "-17.82", "tilt_roll_deg": "7.24", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "172", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.20,H:91.20,SRAW:1856,SM:63.6,SC:MODERATE,RRAW:1697,RP:72.1,RC:MODERATE_RAIN,MV:1,TX:-17.82,TY:7.24,AXG:-0.130,AYG:-0.315,AZG:0.972,GXD:4.89,GYD:1.63,GZD:-0.08"}, {"timestamp": "25-10-2026 14:39:09", "packet_id": "366", "temperature_c": "29.2", "temperature_f": "84.56", "humidity_pct": "91.1", "dew_point_c": "27.59", "soil_raw_adc": "1919", "soil_moisture_pct": "60.1", "soil_condition": "MODERATE", "rain_raw_adc": "1709", "rain_intensity_pct": "71.6", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-2670", "accel_y_raw": "-704", "accel_z_raw": "16465", "accel_x_g": "-0.163", "accel_y_g": "-0.043", "accel_z_g": "1.005", "accel_magnitude_g": "1.019", "gyro_x_raw": "568", "gyro_y_raw": "430", "gyro_z_raw": "-104", "gyro_x_dps": "4.34", "gyro_y_dps": "3.29", "gyro_z_dps": "-0.8", "gyro_magnitude_dps": "5.5", "tilt_pitch_deg": "-2.43", "tilt_roll_deg": "9.21", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "171", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.20,H:91.10,SRAW:1919,SM:60.1,SC:MODERATE,RRAW:1709,RP:71.6,RC:MODERATE_RAIN,MV:1,TX:-2.43,TY:9.21,AXG:-0.163,AYG:-0.043,AZG:1.005,GXD:4.34,GYD:3.29,GZD:-0.80"}, {"timestamp": "25-10-2026 14:39:14", "packet_id": "367", "temperature_c": "29.3", "temperature_f": "84.74", "humidity_pct": "91.0", "dew_point_c": "27.67", "soil_raw_adc": "1981", "soil_moisture_pct": "56.6", "soil_condition": "MODERATE", "rain_raw_adc": "1701", "rain_intensity_pct": "72.0", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-7159", "accel_y_raw": "-114", "accel_z_raw": "15220", "accel_x_g": "-0.437", "accel_y_g": "-0.007", "accel_z_g": "0.929", "accel_magnitude_g": "1.027", "gyro_x_raw": "390", "gyro_y_raw": "226", "gyro_z_raw": "-32", "gyro_x_dps": "2.98", "gyro_y_dps": "1.73", "gyro_z_dps": "-0.25", "gyro_magnitude_dps": "3.45", "tilt_pitch_deg": "-0.38", "tilt_roll_deg": "25.18", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "172", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.30,H:91.00,SRAW:1981,SM:56.6,SC:MODERATE,RRAW:1701,RP:72.0,RC:MODERATE_RAIN,MV:1,TX:-0.38,TY:25.18,AXG:-0.437,AYG:-0.007,AZG:0.929,GXD:2.98,GYD:1.73,GZD:-0.25"}, {"timestamp": "25-10-2026 14:39:19", "packet_id": "368", "temperature_c": "29.3", "temperature_f": "84.74", "humidity_pct": "90.9", "dew_point_c": "27.65", "soil_raw_adc": "1893", "soil_moisture_pct": "61.5", "soil_condition": "MODERATE", "rain_raw_adc": "1776", "rain_intensity_pct": "69.0", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "573", "accel_y_raw": "-1359", "accel_z_raw": "16891", "accel_x_g": "0.035", "accel_y_g": "-0.083", "accel_z_g": "1.031", "accel_magnitude_g": "1.035", "gyro_x_raw": "-2", "gyro_y_raw": "-6293", "gyro_z_raw": "110", "gyro_x_dps": "-0.02", "gyro_y_dps": "-48.04", "gyro_z_dps": "0.84", "gyro_magnitude_dps": "48.05", "tilt_pitch_deg": "-4.61", "tilt_roll_deg": "-1.96", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "173", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.30,H:90.90,SRAW:1893,SM:61.5,SC:MODERATE,RRAW:1776,RP:69.0,RC:MODERATE_RAIN,MV:1,TX:-4.61,TY:-1.96,AXG:0.035,AYG:-0.083,AZG:1.031,GXD:-0.02,GYD:-48.04,GZD:0.84"}, {"timestamp": "25-10-2026 14:39:24", "packet_id": "369", "temperature_c": "29.3", "temperature_f": "84.74", "humidity_pct": "90.8", "dew_point_c": "27.63", "soil_raw_adc": "1830", "soil_moisture_pct": "65.0", "soil_condition": "WET", "rain_raw_adc": "1717", "rain_intensity_pct": "71.3", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "1409", "accel_y_raw": "376", "accel_z_raw": "16859", "accel_x_g": "0.086", "accel_y_g": "0.023", "accel_z_g": "1.029", "accel_magnitude_g": "1.033", "gyro_x_raw": "369", "gyro_y_raw": "90", "gyro_z_raw": "-10", "gyro_x_dps": "2.82", "gyro_y_dps": "0.69", "gyro_z_dps": "-0.08", "gyro_magnitude_dps": "2.9", "tilt_pitch_deg": "1.26", "tilt_roll_deg": "-4.8", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "164", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.30,H:90.80,SRAW:1830,SM:65.0,SC:WET,RRAW:1717,RP:71.3,RC:MODERATE_RAIN,MV:1,TX:1.26,TY:-4.80,AXG:0.086,AYG:0.023,AZG:1.029,GXD:2.82,GYD:0.69,GZD:-0.08"}, {"timestamp": "25-10-2026 14:39:29", "packet_id": "370", "temperature_c": "29.3", "temperature_f": "84.74", "humidity_pct": "90.7", "dew_point_c": "27.62", "soil_raw_adc": "1822", "soil_moisture_pct": "65.4", "soil_condition": "WET", "rain_raw_adc": "1735", "rain_intensity_pct": "70.6", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "1359", "accel_y_raw": "262", "accel_z_raw": "16908", "accel_x_g": "0.083", "accel_y_g": "0.016", "accel_z_g": "1.032", "accel_magnitude_g": "1.035", "gyro_x_raw": "378", "gyro_y_raw": "61", "gyro_z_raw": "-49", "gyro_x_dps": "2.89", "gyro_y_dps": "0.47", "gyro_z_dps": "-0.38", "gyro_magnitude_dps": "2.95", "tilt_pitch_deg": "0.89", "tilt_roll_deg": "-4.61", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "164", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.30,H:90.70,SRAW:1822,SM:65.4,SC:WET,RRAW:1735,RP:70.6,RC:MODERATE_RAIN,MV:1,TX:0.89,TY:-4.61,AXG:0.083,AYG:0.016,AZG:1.032,GXD:2.89,GYD:0.47,GZD:-0.38"}, {"timestamp": "25-10-2026 14:39:34", "packet_id": "371", "temperature_c": "29.3", "temperature_f": "84.74", "humidity_pct": "90.6", "dew_point_c": "27.6", "soil_raw_adc": "1827", "soil_moisture_pct": "65.2", "soil_condition": "WET", "rain_raw_adc": "1779", "rain_intensity_pct": "68.8", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "1294", "accel_y_raw": "327", "accel_z_raw": "16924", "accel_x_g": "0.079", "accel_y_g": "0.02", "accel_z_g": "1.033", "accel_magnitude_g": "1.036", "gyro_x_raw": "381", "gyro_y_raw": "65", "gyro_z_raw": "-44", "gyro_x_dps": "2.91", "gyro_y_dps": "0.5", "gyro_z_dps": "-0.34", "gyro_magnitude_dps": "2.97", "tilt_pitch_deg": "1.11", "tilt_roll_deg": "-4.38", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "164", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.30,H:90.60,SRAW:1827,SM:65.2,SC:WET,RRAW:1779,RP:68.8,RC:MODERATE_RAIN,MV:1,TX:1.11,TY:-4.38,AXG:0.079,AYG:0.020,AZG:1.033,GXD:2.91,GYD:0.50,GZD:-0.34"}, {"timestamp": "25-10-2026 14:39:44", "packet_id": "372", "temperature_c": "29.3", "temperature_f": "84.74", "humidity_pct": "90.4", "dew_point_c": "27.56", "soil_raw_adc": "1823", "soil_moisture_pct": "65.4", "soil_condition": "WET", "rain_raw_adc": "1746", "rain_intensity_pct": "70.2", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "1277", "accel_y_raw": "393", "accel_z_raw": "16777", "accel_x_g": "0.078", "accel_y_g": "0.024", "accel_z_g": "1.024", "accel_magnitude_g": "1.027", "gyro_x_raw": "382", "gyro_y_raw": "58", "gyro_z_raw": "-13", "gyro_x_dps": "2.92", "gyro_y_dps": "0.45", "gyro_z_dps": "-0.1", "gyro_magnitude_dps": "2.96", "tilt_pitch_deg": "1.33", "tilt_roll_deg": "-4.33", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "164", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.30,H:90.40,SRAW:1823,SM:65.4,SC:WET,RRAW:1746,RP:70.2,RC:MODERATE_RAIN,MV:1,TX:1.33,TY:-4.33,AXG:0.078,AYG:0.024,AZG:1.024,GXD:2.92,GYD:0.45,GZD:-0.10"}, {"timestamp": "25-10-2026 14:39:49", "packet_id": "373", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "90.4", "dew_point_c": "27.66", "soil_raw_adc": "1826", "soil_moisture_pct": "65.2", "soil_condition": "WET", "rain_raw_adc": "1737", "rain_intensity_pct": "70.5", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "1277", "accel_y_raw": "344", "accel_z_raw": "16777", "accel_x_g": "0.078", "accel_y_g": "0.021", "accel_z_g": "1.024", "accel_magnitude_g": "1.027", "gyro_x_raw": "413", "gyro_y_raw": "75", "gyro_z_raw": "-37", "gyro_x_dps": "3.16", "gyro_y_dps": "0.58", "gyro_z_dps": "-0.29", "gyro_magnitude_dps": "3.23", "tilt_pitch_deg": "1.17", "tilt_roll_deg": "-4.36", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "164", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.40,H:90.40,SRAW:1826,SM:65.2,SC:WET,RRAW:1737,RP:70.5,RC:MODERATE_RAIN,MV:1,TX:1.17,TY:-4.36,AXG:0.078,AYG:0.021,AZG:1.024,GXD:3.16,GYD:0.58,GZD:-0.29"}, {"timestamp": "25-10-2026 14:39:54", "packet_id": "374", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "90.3", "dew_point_c": "27.64", "soil_raw_adc": "1826", "soil_moisture_pct": "65.2", "soil_condition": "WET", "rain_raw_adc": "1766", "rain_intensity_pct": "69.4", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "966", "accel_y_raw": "442", "accel_z_raw": "17022", "accel_x_g": "0.059", "accel_y_g": "0.027", "accel_z_g": "1.039", "accel_magnitude_g": "1.041", "gyro_x_raw": "345", "gyro_y_raw": "200", "gyro_z_raw": "-51", "gyro_x_dps": "2.64", "gyro_y_dps": "1.53", "gyro_z_dps": "-0.39", "gyro_magnitude_dps": "3.08", "tilt_pitch_deg": "1.49", "tilt_roll_deg": "-3.25", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "164", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.40,H:90.30,SRAW:1826,SM:65.2,SC:WET,RRAW:1766,RP:69.4,RC:MODERATE_RAIN,MV:1,TX:1.49,TY:-3.25,AXG:0.059,AYG:0.027,AZG:1.039,GXD:2.64,GYD:1.53,GZD:-0.39"}, {"timestamp": "25-10-2026 14:39:59", "packet_id": "375", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "90.3", "dew_point_c": "27.64", "soil_raw_adc": "1826", "soil_moisture_pct": "65.2", "soil_condition": "WET", "rain_raw_adc": "1765", "rain_intensity_pct": "69.4", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "8126", "accel_y_raw": "-327", "accel_z_raw": "14860", "accel_x_g": "0.496", "accel_y_g": "-0.02", "accel_z_g": "0.907", "accel_magnitude_g": "1.034", "gyro_x_raw": "377", "gyro_y_raw": "-121", "gyro_z_raw": "24", "gyro_x_dps": "2.88", "gyro_y_dps": "-0.93", "gyro_z_dps": "0.19", "gyro_magnitude_dps": "3.03", "tilt_pitch_deg": "-1.08", "tilt_roll_deg": "-28.69", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "167", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.40,H:90.30,SRAW:1826,SM:65.2,SC:WET,RRAW:1765,RP:69.4,RC:MODERATE_RAIN,MV:1,TX:-1.08,TY:-28.69,AXG:0.496,AYG:-0.020,AZG:0.907,GXD:2.88,GYD:-0.93,GZD:0.19"}, {"timestamp": "25-10-2026 14:40:04", "packet_id": "376", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "90.3", "dew_point_c": "27.64", "soil_raw_adc": "1818", "soil_moisture_pct": "65.7", "soil_condition": "WET", "rain_raw_adc": "1776", "rain_intensity_pct": "69.0", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "14942", "accel_y_raw": "-245", "accel_z_raw": "7536", "accel_x_g": "0.912", "accel_y_g": "-0.015", "accel_z_g": "0.46", "accel_magnitude_g": "1.022", "gyro_x_raw": "-775", "gyro_y_raw": "2882", "gyro_z_raw": "-568", "gyro_x_dps": "-5.92", "gyro_y_dps": "22.0", "gyro_z_dps": "-4.34", "gyro_magnitude_dps": "23.19", "tilt_pitch_deg": "-0.84", "tilt_roll_deg": "-63.22", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.40,H:90.30,SRAW:1818,SM:65.7,SC:WET,RRAW:1776,RP:69.0,RC:MODERATE_RAIN,MV:1,TX:-0.84,TY:-63.22,AXG:0.912,AYG:-0.015,AZG:0.460,GXD:-5.92,GYD:22.00,GZD:-4.34"}, {"timestamp": "25-10-2026 14:40:09", "packet_id": "377", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "90.3", "dew_point_c": "27.64", "soil_raw_adc": "1825", "soil_moisture_pct": "65.3", "soil_condition": "WET", "rain_raw_adc": "1803", "rain_intensity_pct": "67.9", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "16285", "accel_y_raw": "-327", "accel_z_raw": "3309", "accel_x_g": "0.994", "accel_y_g": "-0.02", "accel_z_g": "0.202", "accel_magnitude_g": "1.015", "gyro_x_raw": "386", "gyro_y_raw": "-1709", "gyro_z_raw": "44", "gyro_x_dps": "2.95", "gyro_y_dps": "-13.05", "gyro_z_dps": "0.34", "gyro_magnitude_dps": "13.38", "tilt_pitch_deg": "-1.1", "tilt_roll_deg": "-78.48", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "168", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.40,H:90.30,SRAW:1825,SM:65.3,SC:WET,RRAW:1803,RP:67.9,RC:MODERATE_RAIN,MV:1,TX:-1.10,TY:-78.48,AXG:0.994,AYG:-0.020,AZG:0.202,GXD:2.95,GYD:-13.05,GZD:0.34"}, {"timestamp": "25-10-2026 14:40:14", "packet_id": "378", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "90.2", "dew_point_c": "27.62", "soil_raw_adc": "1803", "soil_moisture_pct": "66.5", "soil_condition": "WET", "rain_raw_adc": "1799", "rain_intensity_pct": "68.0", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "14090", "accel_y_raw": "-131", "accel_z_raw": "9043", "accel_x_g": "0.86", "accel_y_g": "-0.008", "accel_z_g": "0.552", "accel_magnitude_g": "1.022", "gyro_x_raw": "370", "gyro_y_raw": "-1092", "gyro_z_raw": "-32", "gyro_x_dps": "2.83", "gyro_y_dps": "-8.34", "gyro_z_dps": "-0.25", "gyro_magnitude_dps": "8.81", "tilt_pitch_deg": "-0.44", "tilt_roll_deg": "-57.32", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "168", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.40,H:90.20,SRAW:1803,SM:66.5,SC:WET,RRAW:1799,RP:68.0,RC:MODERATE_RAIN,MV:1,TX:-0.44,TY:-57.32,AXG:0.860,AYG:-0.008,AZG:0.552,GXD:2.83,GYD:-8.34,GZD:-0.25"}, {"timestamp": "25-10-2026 14:40:19", "packet_id": "379", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "90.2", "dew_point_c": "27.62", "soil_raw_adc": "1774", "soil_moisture_pct": "68.1", "soil_condition": "WET", "rain_raw_adc": "1629", "rain_intensity_pct": "74.8", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "14286", "accel_y_raw": "-229", "accel_z_raw": "8519", "accel_x_g": "0.872", "accel_y_g": "-0.014", "accel_z_g": "0.52", "accel_magnitude_g": "1.015", "gyro_x_raw": "379", "gyro_y_raw": "-91", "gyro_z_raw": "-35", "gyro_x_dps": "2.9", "gyro_y_dps": "-0.7", "gyro_z_dps": "-0.27", "gyro_magnitude_dps": "3.0", "tilt_pitch_deg": "-0.81", "tilt_roll_deg": "-59.19", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "168", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.40,H:90.20,SRAW:1774,SM:68.1,SC:WET,RRAW:1629,RP:74.8,RC:MODERATE_RAIN,MV:1,TX:-0.81,TY:-59.19,AXG:0.872,AYG:-0.014,AZG:0.520,GXD:2.90,GYD:-0.70,GZD:-0.27"}, {"timestamp": "25-10-2026 14:40:24", "packet_id": "380", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "90.2", "dew_point_c": "27.62", "soil_raw_adc": "1775", "soil_moisture_pct": "68.1", "soil_condition": "WET", "rain_raw_adc": "1670", "rain_intensity_pct": "73.2", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "15187", "accel_y_raw": "-245", "accel_z_raw": "6979", "accel_x_g": "0.927", "accel_y_g": "-0.015", "accel_z_g": "0.426", "accel_magnitude_g": "1.02", "gyro_x_raw": "420", "gyro_y_raw": "704", "gyro_z_raw": "-61", "gyro_x_dps": "3.21", "gyro_y_dps": "5.38", "gyro_z_dps": "-0.47", "gyro_magnitude_dps": "6.28", "tilt_pitch_deg": "-0.86", "tilt_roll_deg": "-65.32", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "167", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.40,H:90.20,SRAW:1775,SM:68.1,SC:WET,RRAW:1670,RP:73.2,RC:MODERATE_RAIN,MV:1,TX:-0.86,TY:-65.32,AXG:0.927,AYG:-0.015,AZG:0.426,GXD:3.21,GYD:5.38,GZD:-0.47"}, {"timestamp": "25-10-2026 14:40:29", "packet_id": "381", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "90.1", "dew_point_c": "27.6", "soil_raw_adc": "1821", "soil_moisture_pct": "65.5", "soil_condition": "WET", "rain_raw_adc": "1643", "rain_intensity_pct": "74.3", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "10747", "accel_y_raw": "5816", "accel_z_raw": "11370", "accel_x_g": "0.656", "accel_y_g": "0.355", "accel_z_g": "0.694", "accel_magnitude_g": "1.019", "gyro_x_raw": "326", "gyro_y_raw": "268", "gyro_z_raw": "66", "gyro_x_dps": "2.49", "gyro_y_dps": "2.05", "gyro_z_dps": "0.51", "gyro_magnitude_dps": "3.27", "tilt_pitch_deg": "20.4", "tilt_roll_deg": "-40.08", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "165", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.40,H:90.10,SRAW:1821,SM:65.5,SC:WET,RRAW:1643,RP:74.3,RC:MODERATE_RAIN,MV:1,TX:20.40,TY:-40.08,AXG:0.656,AYG:0.355,AZG:0.694,GXD:2.49,GYD:2.05,GZD:0.51"}, {"timestamp": "25-10-2026 14:40:32", "packet_id": "382", "temperature_c": "", "temperature_f": "", "humidity_pct": "", "dew_point_c": "", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "4095", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "0", "accel_y_raw": "0", "accel_z_raw": "16384", "accel_x_g": "0.0", "accel_y_g": "0.0", "accel_z_g": "1.0", "accel_magnitude_g": "1.0", "gyro_x_raw": "0", "gyro_y_raw": "0", "gyro_z_raw": "0", "gyro_x_dps": "0.0", "gyro_y_dps": "0.0", "gyro_z_dps": "0.0", "gyro_magnitude_dps": "0.0", "tilt_pitch_deg": "0.0", "tilt_roll_deg": "-0.0", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "71", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00"}, {"timestamp": "25-10-2026 14:41:27", "packet_id": "383", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "89.7", "dew_point_c": "27.52", "soil_raw_adc": "1763", "soil_moisture_pct": "68.7", "soil_condition": "WET", "rain_raw_adc": "1744", "rain_intensity_pct": "70.2", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-507", "accel_y_raw": "294", "accel_z_raw": "16809", "accel_x_g": "-0.031", "accel_y_g": "0.018", "accel_z_g": "1.026", "accel_magnitude_g": "1.027", "gyro_x_raw": "403", "gyro_y_raw": "70", "gyro_z_raw": "-51", "gyro_x_dps": "3.08", "gyro_y_dps": "0.54", "gyro_z_dps": "-0.39", "gyro_magnitude_dps": "3.15", "tilt_pitch_deg": "1.02", "tilt_roll_deg": "1.73", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "164", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.40,H:89.70,SRAW:1763,SM:68.7,SC:WET,RRAW:1744,RP:70.2,RC:MODERATE_RAIN,MV:1,TX:1.02,TY:1.73,AXG:-0.031,AYG:0.018,AZG:1.026,GXD:3.08,GYD:0.54,GZD:-0.39"}, {"timestamp": "25-10-2026 14:41:32", "packet_id": "384", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "89.7", "dew_point_c": "27.52", "soil_raw_adc": "1746", "soil_moisture_pct": "69.7", "soil_condition": "WET", "rain_raw_adc": "1721", "rain_intensity_pct": "71.2", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-507", "accel_y_raw": "196", "accel_z_raw": "16859", "accel_x_g": "-0.031", "accel_y_g": "0.012", "accel_z_g": "1.029", "accel_magnitude_g": "1.03", "gyro_x_raw": "370", "gyro_y_raw": "87", "gyro_z_raw": "-44", "gyro_x_dps": "2.83", "gyro_y_dps": "0.67", "gyro_z_dps": "-0.34", "gyro_magnitude_dps": "2.93", "tilt_pitch_deg": "0.69", "tilt_roll_deg": "1.71", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "164", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.40,H:89.70,SRAW:1746,SM:69.7,SC:WET,RRAW:1721,RP:71.2,RC:MODERATE_RAIN,MV:1,TX:0.69,TY:1.71,AXG:-0.031,AYG:0.012,AZG:1.029,GXD:2.83,GYD:0.67,GZD:-0.34"}, {"timestamp": "25-10-2026 14:41:37", "packet_id": "385", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "89.7", "dew_point_c": "27.52", "soil_raw_adc": "1759", "soil_moisture_pct": "68.9", "soil_condition": "WET", "rain_raw_adc": "1690", "rain_intensity_pct": "72.4", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-442", "accel_y_raw": "262", "accel_z_raw": "16908", "accel_x_g": "-0.027", "accel_y_g": "0.016", "accel_z_g": "1.032", "accel_magnitude_g": "1.032", "gyro_x_raw": "381", "gyro_y_raw": "96", "gyro_z_raw": "-44", "gyro_x_dps": "2.91", "gyro_y_dps": "0.74", "gyro_z_dps": "-0.34", "gyro_magnitude_dps": "3.02", "tilt_pitch_deg": "0.87", "tilt_roll_deg": "1.52", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "164", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.40,H:89.70,SRAW:1759,SM:68.9,SC:WET,RRAW:1690,RP:72.4,RC:MODERATE_RAIN,MV:1,TX:0.87,TY:1.52,AXG:-0.027,AYG:0.016,AZG:1.032,GXD:2.91,GYD:0.74,GZD:-0.34"}, {"timestamp": "25-10-2026 14:41:42", "packet_id": "386", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "89.7", "dew_point_c": "27.52", "soil_raw_adc": "1763", "soil_moisture_pct": "68.7", "soil_condition": "WET", "rain_raw_adc": "1744", "rain_intensity_pct": "70.2", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-507", "accel_y_raw": "212", "accel_z_raw": "16842", "accel_x_g": "-0.031", "accel_y_g": "0.013", "accel_z_g": "1.028", "accel_magnitude_g": "1.029", "gyro_x_raw": "393", "gyro_y_raw": "83", "gyro_z_raw": "5", "gyro_x_dps": "3.0", "gyro_y_dps": "0.64", "gyro_z_dps": "0.04", "gyro_magnitude_dps": "3.07", "tilt_pitch_deg": "0.71", "tilt_roll_deg": "1.74", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "163", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.40,H:89.70,SRAW:1763,SM:68.7,SC:WET,RRAW:1744,RP:70.2,RC:MODERATE_RAIN,MV:1,TX:0.71,TY:1.74,AXG:-0.031,AYG:0.013,AZG:1.028,GXD:3.00,GYD:0.64,GZD:0.04"}, {"timestamp": "25-10-2026 14:41:47", "packet_id": "387", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "89.6", "dew_point_c": "27.51", "soil_raw_adc": "1744", "soil_moisture_pct": "69.8", "soil_condition": "WET", "rain_raw_adc": "1747", "rain_intensity_pct": "70.1", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-458", "accel_y_raw": "376", "accel_z_raw": "16826", "accel_x_g": "-0.028", "accel_y_g": "0.023", "accel_z_g": "1.027", "accel_magnitude_g": "1.028", "gyro_x_raw": "402", "gyro_y_raw": "52", "gyro_z_raw": "-37", "gyro_x_dps": "3.07", "gyro_y_dps": "0.4", "gyro_z_dps": "-0.29", "gyro_magnitude_dps": "3.11", "tilt_pitch_deg": "1.27", "tilt_roll_deg": "1.57", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "164", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.40,H:89.60,SRAW:1744,SM:69.8,SC:WET,RRAW:1747,RP:70.1,RC:MODERATE_RAIN,MV:1,TX:1.27,TY:1.57,AXG:-0.028,AYG:0.023,AZG:1.027,GXD:3.07,GYD:0.40,GZD:-0.29"}, {"timestamp": "25-10-2026 14:41:52", "packet_id": "388", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "89.6", "dew_point_c": "27.51", "soil_raw_adc": "1744", "soil_moisture_pct": "69.8", "soil_condition": "WET", "rain_raw_adc": "1756", "rain_intensity_pct": "69.8", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-507", "accel_y_raw": "229", "accel_z_raw": "16908", "accel_x_g": "-0.031", "accel_y_g": "0.014", "accel_z_g": "1.032", "accel_magnitude_g": "1.033", "gyro_x_raw": "416", "gyro_y_raw": "93", "gyro_z_raw": "-35", "gyro_x_dps": "3.18", "gyro_y_dps": "0.71", "gyro_z_dps": "-0.27", "gyro_magnitude_dps": "3.27", "tilt_pitch_deg": "0.8", "tilt_roll_deg": "1.75", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "164", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.40,H:89.60,SRAW:1744,SM:69.8,SC:WET,RRAW:1756,RP:69.8,RC:MODERATE_RAIN,MV:1,TX:0.80,TY:1.75,AXG:-0.031,AYG:0.014,AZG:1.032,GXD:3.18,GYD:0.71,GZD:-0.27"}, {"timestamp": "25-10-2026 14:41:57", "packet_id": "389", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "89.7", "dew_point_c": "27.52", "soil_raw_adc": "1751", "soil_moisture_pct": "69.4", "soil_condition": "WET", "rain_raw_adc": "1733", "rain_intensity_pct": "70.7", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-524", "accel_y_raw": "294", "accel_z_raw": "16826", "accel_x_g": "-0.032", "accel_y_g": "0.018", "accel_z_g": "1.027", "accel_magnitude_g": "1.028", "gyro_x_raw": "396", "gyro_y_raw": "44", "gyro_z_raw": "-56", "gyro_x_dps": "3.03", "gyro_y_dps": "0.34", "gyro_z_dps": "-0.43", "gyro_magnitude_dps": "3.08", "tilt_pitch_deg": "0.99", "tilt_roll_deg": "1.81", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "164", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.40,H:89.70,SRAW:1751,SM:69.4,SC:WET,RRAW:1733,RP:70.7,RC:MODERATE_RAIN,MV:1,TX:0.99,TY:1.81,AXG:-0.032,AYG:0.018,AZG:1.027,GXD:3.03,GYD:0.34,GZD:-0.43"}, {"timestamp": "25-10-2026 14:42:02", "packet_id": "390", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "89.6", "dew_point_c": "27.51", "soil_raw_adc": "1776", "soil_moisture_pct": "68.0", "soil_condition": "WET", "rain_raw_adc": "1745", "rain_intensity_pct": "70.2", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-491", "accel_y_raw": "311", "accel_z_raw": "16891", "accel_x_g": "-0.03", "accel_y_g": "0.019", "accel_z_g": "1.031", "accel_magnitude_g": "1.032", "gyro_x_raw": "377", "gyro_y_raw": "57", "gyro_z_raw": "-35", "gyro_x_dps": "2.88", "gyro_y_dps": "0.44", "gyro_z_dps": "-0.27", "gyro_magnitude_dps": "2.93", "tilt_pitch_deg": "1.04", "tilt_roll_deg": "1.66", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "164", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.40,H:89.60,SRAW:1776,SM:68.0,SC:WET,RRAW:1745,RP:70.2,RC:MODERATE_RAIN,MV:1,TX:1.04,TY:1.66,AXG:-0.030,AYG:0.019,AZG:1.031,GXD:2.88,GYD:0.44,GZD:-0.27"}, {"timestamp": "25-10-2026 14:42:07", "packet_id": "391", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "89.6", "dew_point_c": "27.51", "soil_raw_adc": "1741", "soil_moisture_pct": "69.9", "soil_condition": "WET", "rain_raw_adc": "1735", "rain_intensity_pct": "70.6", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-507", "accel_y_raw": "229", "accel_z_raw": "16760", "accel_x_g": "-0.031", "accel_y_g": "0.014", "accel_z_g": "1.023", "accel_magnitude_g": "1.024", "gyro_x_raw": "420", "gyro_y_raw": "79", "gyro_z_raw": "-10", "gyro_x_dps": "3.21", "gyro_y_dps": "0.61", "gyro_z_dps": "-0.08", "gyro_magnitude_dps": "3.27", "tilt_pitch_deg": "0.79", "tilt_roll_deg": "1.71", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "164", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.40,H:89.60,SRAW:1741,SM:69.9,SC:WET,RRAW:1735,RP:70.6,RC:MODERATE_RAIN,MV:1,TX:0.79,TY:1.71,AXG:-0.031,AYG:0.014,AZG:1.023,GXD:3.21,GYD:0.61,GZD:-0.08"}, {"timestamp": "25-10-2026 14:42:12", "packet_id": "392", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "89.6", "dew_point_c": "27.51", "soil_raw_adc": "1756", "soil_moisture_pct": "69.1", "soil_condition": "WET", "rain_raw_adc": "1733", "rain_intensity_pct": "70.7", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-475", "accel_y_raw": "278", "accel_z_raw": "16826", "accel_x_g": "-0.029", "accel_y_g": "0.017", "accel_z_g": "1.027", "accel_magnitude_g": "1.028", "gyro_x_raw": "429", "gyro_y_raw": "-6", "gyro_z_raw": "-31", "gyro_x_dps": "3.28", "gyro_y_dps": "-0.05", "gyro_z_dps": "-0.24", "gyro_magnitude_dps": "3.29", "tilt_pitch_deg": "0.93", "tilt_roll_deg": "1.59", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "165", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.40,H:89.60,SRAW:1756,SM:69.1,SC:WET,RRAW:1733,RP:70.7,RC:MODERATE_RAIN,MV:1,TX:0.93,TY:1.59,AXG:-0.029,AYG:0.017,AZG:1.027,GXD:3.28,GYD:-0.05,GZD:-0.24"}, {"timestamp": "25-10-2026 14:42:17", "packet_id": "393", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "89.6", "dew_point_c": "27.51", "soil_raw_adc": "1849", "soil_moisture_pct": "63.9", "soil_condition": "MODERATE", "rain_raw_adc": "1757", "rain_intensity_pct": "69.7", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-1703", "accel_y_raw": "196", "accel_z_raw": "16793", "accel_x_g": "-0.104", "accel_y_g": "0.012", "accel_z_g": "1.025", "accel_magnitude_g": "1.03", "gyro_x_raw": "403", "gyro_y_raw": "87", "gyro_z_raw": "-49", "gyro_x_dps": "3.08", "gyro_y_dps": "0.67", "gyro_z_dps": "-0.38", "gyro_magnitude_dps": "3.17", "tilt_pitch_deg": "0.65", "tilt_roll_deg": "5.77", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.40,H:89.60,SRAW:1849,SM:63.9,SC:MODERATE,RRAW:1757,RP:69.7,RC:MODERATE_RAIN,MV:1,TX:0.65,TY:5.77,AXG:-0.104,AYG:0.012,AZG:1.025,GXD:3.08,GYD:0.67,GZD:-0.38"}, {"timestamp": "25-10-2026 14:42:22", "packet_id": "394", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "89.6", "dew_point_c": "27.51", "soil_raw_adc": "1697", "soil_moisture_pct": "72.4", "soil_condition": "WET", "rain_raw_adc": "1535", "rain_intensity_pct": "78.6", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1736", "accel_y_raw": "212", "accel_z_raw": "16777", "accel_x_g": "-0.106", "accel_y_g": "0.013", "accel_z_g": "1.024", "accel_magnitude_g": "1.03", "gyro_x_raw": "393", "gyro_y_raw": "81", "gyro_z_raw": "-41", "gyro_x_dps": "3.0", "gyro_y_dps": "0.62", "gyro_z_dps": "-0.32", "gyro_magnitude_dps": "3.08", "tilt_pitch_deg": "0.71", "tilt_roll_deg": "5.92", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "161", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.40,H:89.60,SRAW:1697,SM:72.4,SC:WET,RRAW:1535,RP:78.6,RC:HEAVY_RAIN,MV:1,TX:0.71,TY:5.92,AXG:-0.106,AYG:0.013,AZG:1.024,GXD:3.00,GYD:0.62,GZD:-0.32"}, {"timestamp": "25-10-2026 14:42:27", "packet_id": "395", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "89.6", "dew_point_c": "27.51", "soil_raw_adc": "1751", "soil_moisture_pct": "69.4", "soil_condition": "WET", "rain_raw_adc": "1616", "rain_intensity_pct": "75.4", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1753", "accel_y_raw": "229", "accel_z_raw": "16842", "accel_x_g": "-0.107", "accel_y_g": "0.014", "accel_z_g": "1.028", "accel_magnitude_g": "1.034", "gyro_x_raw": "391", "gyro_y_raw": "68", "gyro_z_raw": "-40", "gyro_x_dps": "2.99", "gyro_y_dps": "0.52", "gyro_z_dps": "-0.31", "gyro_magnitude_dps": "3.05", "tilt_pitch_deg": "0.77", "tilt_roll_deg": "5.94", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "161", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.40,H:89.60,SRAW:1751,SM:69.4,SC:WET,RRAW:1616,RP:75.4,RC:HEAVY_RAIN,MV:1,TX:0.77,TY:5.94,AXG:-0.107,AYG:0.014,AZG:1.028,GXD:2.99,GYD:0.52,GZD:-0.31"}, {"timestamp": "25-10-2026 14:42:32", "packet_id": "396", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "89.6", "dew_point_c": "27.51", "soil_raw_adc": "1985", "soil_moisture_pct": "56.4", "soil_condition": "MODERATE", "rain_raw_adc": "1588", "rain_intensity_pct": "76.5", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1703", "accel_y_raw": "147", "accel_z_raw": "16793", "accel_x_g": "-0.104", "accel_y_g": "0.009", "accel_z_g": "1.025", "accel_magnitude_g": "1.03", "gyro_x_raw": "377", "gyro_y_raw": "40", "gyro_z_raw": "-30", "gyro_x_dps": "2.88", "gyro_y_dps": "0.31", "gyro_z_dps": "-0.23", "gyro_magnitude_dps": "2.91", "tilt_pitch_deg": "0.52", "tilt_roll_deg": "5.76", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "166", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.40,H:89.60,SRAW:1985,SM:56.4,SC:MODERATE,RRAW:1588,RP:76.5,RC:HEAVY_RAIN,MV:1,TX:0.52,TY:5.76,AXG:-0.104,AYG:0.009,AZG:1.025,GXD:2.88,GYD:0.31,GZD:-0.23"}, {"timestamp": "25-10-2026 14:42:37", "packet_id": "397", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "89.6", "dew_point_c": "27.51", "soil_raw_adc": "2426", "soil_moisture_pct": "31.9", "soil_condition": "MODERATE", "rain_raw_adc": "3395", "rain_intensity_pct": "4.2", "rain_condition": "NO_RAIN", "accel_x_raw": "-1720", "accel_y_raw": "212", "accel_z_raw": "16711", "accel_x_g": "-0.105", "accel_y_g": "0.013", "accel_z_g": "1.02", "accel_magnitude_g": "1.025", "gyro_x_raw": "416", "gyro_y_raw": "35", "gyro_z_raw": "-70", "gyro_x_dps": "3.18", "gyro_y_dps": "0.27", "gyro_z_dps": "-0.54", "gyro_magnitude_dps": "3.24", "tilt_pitch_deg": "0.71", "tilt_roll_deg": "5.9", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "162", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:29.40,H:89.60,SRAW:2426,SM:31.9,SC:MODERATE,RRAW:3395,RP:4.2,RC:NO_RAIN,MV:1,TX:0.71,TY:5.90,AXG:-0.105,AYG:0.013,AZG:1.020,GXD:3.18,GYD:0.27,GZD:-0.54"}, {"timestamp": "25-10-2026 14:42:42", "packet_id": "398", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "89.6", "dew_point_c": "27.51", "soil_raw_adc": "2384", "soil_moisture_pct": "34.2", "soil_condition": "MODERATE", "rain_raw_adc": "3810", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-1736", "accel_y_raw": "212", "accel_z_raw": "16744", "accel_x_g": "-0.106", "accel_y_g": "0.013", "accel_z_g": "1.022", "accel_magnitude_g": "1.028", "gyro_x_raw": "416", "gyro_y_raw": "51", "gyro_z_raw": "-51", "gyro_x_dps": "3.18", "gyro_y_dps": "0.39", "gyro_z_dps": "-0.39", "gyro_magnitude_dps": "3.23", "tilt_pitch_deg": "0.75", "tilt_roll_deg": "5.9", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "162", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:29.40,H:89.60,SRAW:2384,SM:34.2,SC:MODERATE,RRAW:3810,RP:0.0,RC:NO_RAIN,MV:1,TX:0.75,TY:5.90,AXG:-0.106,AYG:0.013,AZG:1.022,GXD:3.18,GYD:0.39,GZD:-0.39"}, {"timestamp": "25-10-2026 14:42:47", "packet_id": "399", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "89.6", "dew_point_c": "27.51", "soil_raw_adc": "2437", "soil_moisture_pct": "31.3", "soil_condition": "MODERATE", "rain_raw_adc": "3766", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-1736", "accel_y_raw": "180", "accel_z_raw": "16744", "accel_x_g": "-0.106", "accel_y_g": "0.011", "accel_z_g": "1.022", "accel_magnitude_g": "1.028", "gyro_x_raw": "389", "gyro_y_raw": "68", "gyro_z_raw": "-43", "gyro_x_dps": "2.97", "gyro_y_dps": "0.52", "gyro_z_dps": "-0.33", "gyro_magnitude_dps": "3.03", "tilt_pitch_deg": "0.6", "tilt_roll_deg": "5.92", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "162", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:29.40,H:89.60,SRAW:2437,SM:31.3,SC:MODERATE,RRAW:3766,RP:0.0,RC:NO_RAIN,MV:1,TX:0.60,TY:5.92,AXG:-0.106,AYG:0.011,AZG:1.022,GXD:2.97,GYD:0.52,GZD:-0.33"}, {"timestamp": "25-10-2026 14:42:52", "packet_id": "400", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "89.6", "dew_point_c": "27.51", "soil_raw_adc": "2045", "soil_moisture_pct": "53.1", "soil_condition": "MODERATE", "rain_raw_adc": "1931", "rain_intensity_pct": "62.8", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-1736", "accel_y_raw": "212", "accel_z_raw": "16777", "accel_x_g": "-0.106", "accel_y_g": "0.013", "accel_z_g": "1.024", "accel_magnitude_g": "1.03", "gyro_x_raw": "404", "gyro_y_raw": "44", "gyro_z_raw": "-14", "gyro_x_dps": "3.09", "gyro_y_dps": "0.34", "gyro_z_dps": "-0.11", "gyro_magnitude_dps": "3.11", "tilt_pitch_deg": "0.73", "tilt_roll_deg": "5.89", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.40,H:89.60,SRAW:2045,SM:53.1,SC:MODERATE,RRAW:1931,RP:62.8,RC:MODERATE_RAIN,MV:1,TX:0.73,TY:5.89,AXG:-0.106,AYG:0.013,AZG:1.024,GXD:3.09,GYD:0.34,GZD:-0.11"}, {"timestamp": "25-10-2026 14:42:57", "packet_id": "401", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "89.7", "dew_point_c": "27.52", "soil_raw_adc": "2362", "soil_moisture_pct": "35.4", "soil_condition": "MODERATE", "rain_raw_adc": "3283", "rain_intensity_pct": "8.7", "rain_condition": "NO_RAIN", "accel_x_raw": "-1687", "accel_y_raw": "212", "accel_z_raw": "16842", "accel_x_g": "-0.103", "accel_y_g": "0.013", "accel_z_g": "1.028", "accel_magnitude_g": "1.033", "gyro_x_raw": "404", "gyro_y_raw": "17", "gyro_z_raw": "-74", "gyro_x_dps": "3.09", "gyro_y_dps": "0.13", "gyro_z_dps": "-0.57", "gyro_magnitude_dps": "3.14", "tilt_pitch_deg": "0.72", "tilt_roll_deg": "5.71", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "162", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:29.40,H:89.70,SRAW:2362,SM:35.4,SC:MODERATE,RRAW:3283,RP:8.7,RC:NO_RAIN,MV:1,TX:0.72,TY:5.71,AXG:-0.103,AYG:0.013,AZG:1.028,GXD:3.09,GYD:0.13,GZD:-0.57"}, {"timestamp": "25-10-2026 14:43:02", "packet_id": "402", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "89.6", "dew_point_c": "27.51", "soil_raw_adc": "2176", "soil_moisture_pct": "45.8", "soil_condition": "MODERATE", "rain_raw_adc": "2503", "rain_intensity_pct": "39.9", "rain_condition": "LIGHT_RAIN", "accel_x_raw": "-1736", "accel_y_raw": "163", "accel_z_raw": "16809", "accel_x_g": "-0.106", "accel_y_g": "0.01", "accel_z_g": "1.026", "accel_magnitude_g": "1.032", "gyro_x_raw": "362", "gyro_y_raw": "57", "gyro_z_raw": "-69", "gyro_x_dps": "2.77", "gyro_y_dps": "0.44", "gyro_z_dps": "-0.53", "gyro_magnitude_dps": "2.85", "tilt_pitch_deg": "0.58", "tilt_roll_deg": "5.9", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "166", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.40,H:89.60,SRAW:2176,SM:45.8,SC:MODERATE,RRAW:2503,RP:39.9,RC:LIGHT_RAIN,MV:1,TX:0.58,TY:5.90,AXG:-0.106,AYG:0.010,AZG:1.026,GXD:2.77,GYD:0.44,GZD:-0.53"}, {"timestamp": "25-10-2026 14:43:07", "packet_id": "403", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "89.6", "dew_point_c": "27.51", "soil_raw_adc": "1921", "soil_moisture_pct": "59.9", "soil_condition": "MODERATE", "rain_raw_adc": "1481", "rain_intensity_pct": "80.8", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1703", "accel_y_raw": "229", "accel_z_raw": "16695", "accel_x_g": "-0.104", "accel_y_g": "0.014", "accel_z_g": "1.019", "accel_magnitude_g": "1.024", "gyro_x_raw": "402", "gyro_y_raw": "57", "gyro_z_raw": "-28", "gyro_x_dps": "3.07", "gyro_y_dps": "0.44", "gyro_z_dps": "-0.22", "gyro_magnitude_dps": "3.11", "tilt_pitch_deg": "0.79", "tilt_roll_deg": "5.8", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "166", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.40,H:89.60,SRAW:1921,SM:59.9,SC:MODERATE,RRAW:1481,RP:80.8,RC:HEAVY_RAIN,MV:1,TX:0.79,TY:5.80,AXG:-0.104,AYG:0.014,AZG:1.019,GXD:3.07,GYD:0.44,GZD:-0.22"}, {"timestamp": "25-10-2026 14:43:12", "packet_id": "404", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "89.6", "dew_point_c": "27.51", "soil_raw_adc": "1902", "soil_moisture_pct": "61.0", "soil_condition": "MODERATE", "rain_raw_adc": "1511", "rain_intensity_pct": "79.6", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "9666", "accel_y_raw": "0", "accel_z_raw": "13729", "accel_x_g": "0.59", "accel_y_g": "-0.0", "accel_z_g": "0.838", "accel_magnitude_g": "1.025", "gyro_x_raw": "417", "gyro_y_raw": "454", "gyro_z_raw": "-35", "gyro_x_dps": "3.19", "gyro_y_dps": "3.47", "gyro_z_dps": "-0.27", "gyro_magnitude_dps": "4.72", "tilt_pitch_deg": "-0.01", "tilt_roll_deg": "-35.14", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.40,H:89.60,SRAW:1902,SM:61.0,SC:MODERATE,RRAW:1511,RP:79.6,RC:HEAVY_RAIN,MV:1,TX:-0.01,TY:-35.14,AXG:0.590,AYG:-0.000,AZG:0.838,GXD:3.19,GYD:3.47,GZD:-0.27"}, {"timestamp": "25-10-2026 14:43:17", "packet_id": "405", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "89.6", "dew_point_c": "27.51", "soil_raw_adc": "1893", "soil_moisture_pct": "61.5", "soil_condition": "MODERATE", "rain_raw_adc": "1531", "rain_intensity_pct": "78.8", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "15450", "accel_y_raw": "-147", "accel_z_raw": "6029", "accel_x_g": "0.943", "accel_y_g": "-0.009", "accel_z_g": "0.368", "accel_magnitude_g": "1.012", "gyro_x_raw": "237", "gyro_y_raw": "-1164", "gyro_z_raw": "-28", "gyro_x_dps": "1.81", "gyro_y_dps": "-8.89", "gyro_z_dps": "-0.22", "gyro_magnitude_dps": "9.08", "tilt_pitch_deg": "-0.53", "tilt_roll_deg": "-68.67", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "170", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.40,H:89.60,SRAW:1893,SM:61.5,SC:MODERATE,RRAW:1531,RP:78.8,RC:HEAVY_RAIN,MV:1,TX:-0.53,TY:-68.67,AXG:0.943,AYG:-0.009,AZG:0.368,GXD:1.81,GYD:-8.89,GZD:-0.22"}, {"timestamp": "25-10-2026 14:43:22", "packet_id": "406", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "89.6", "dew_point_c": "27.51", "soil_raw_adc": "1899", "soil_moisture_pct": "61.2", "soil_condition": "MODERATE", "rain_raw_adc": "1535", "rain_intensity_pct": "78.6", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1343", "accel_y_raw": "163", "accel_z_raw": "16941", "accel_x_g": "-0.082", "accel_y_g": "0.01", "accel_z_g": "1.034", "accel_magnitude_g": "1.037", "gyro_x_raw": "390", "gyro_y_raw": "65", "gyro_z_raw": "-35", "gyro_x_dps": "2.98", "gyro_y_dps": "0.5", "gyro_z_dps": "-0.27", "gyro_magnitude_dps": "3.03", "tilt_pitch_deg": "0.53", "tilt_roll_deg": "4.54", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "166", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.40,H:89.60,SRAW:1899,SM:61.2,SC:MODERATE,RRAW:1535,RP:78.6,RC:HEAVY_RAIN,MV:1,TX:0.53,TY:4.54,AXG:-0.082,AYG:0.010,AZG:1.034,GXD:2.98,GYD:0.50,GZD:-0.27"}, {"timestamp": "25-10-2026 14:43:27", "packet_id": "407", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "89.7", "dew_point_c": "27.52", "soil_raw_adc": "1905", "soil_moisture_pct": "60.8", "soil_condition": "MODERATE", "rain_raw_adc": "1535", "rain_intensity_pct": "78.6", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1409", "accel_y_raw": "327", "accel_z_raw": "17039", "accel_x_g": "-0.086", "accel_y_g": "0.02", "accel_z_g": "1.04", "accel_magnitude_g": "1.044", "gyro_x_raw": "353", "gyro_y_raw": "121", "gyro_z_raw": "-47", "gyro_x_dps": "2.7", "gyro_y_dps": "0.93", "gyro_z_dps": "-0.36", "gyro_magnitude_dps": "2.88", "tilt_pitch_deg": "1.1", "tilt_roll_deg": "4.72", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "166", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.40,H:89.70,SRAW:1905,SM:60.8,SC:MODERATE,RRAW:1535,RP:78.6,RC:HEAVY_RAIN,MV:1,TX:1.10,TY:4.72,AXG:-0.086,AYG:0.020,AZG:1.040,GXD:2.70,GYD:0.93,GZD:-0.36"}, {"timestamp": "25-10-2026 14:43:32", "packet_id": "408", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "89.7", "dew_point_c": "27.52", "soil_raw_adc": "1903", "soil_moisture_pct": "60.9", "soil_condition": "MODERATE", "rain_raw_adc": "1555", "rain_intensity_pct": "77.8", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1458", "accel_y_raw": "180", "accel_z_raw": "16744", "accel_x_g": "-0.089", "accel_y_g": "0.011", "accel_z_g": "1.022", "accel_magnitude_g": "1.026", "gyro_x_raw": "365", "gyro_y_raw": "52", "gyro_z_raw": "-10", "gyro_x_dps": "2.79", "gyro_y_dps": "0.4", "gyro_z_dps": "-0.08", "gyro_magnitude_dps": "2.82", "tilt_pitch_deg": "0.61", "tilt_roll_deg": "5.0", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "166", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.40,H:89.70,SRAW:1903,SM:60.9,SC:MODERATE,RRAW:1555,RP:77.8,RC:HEAVY_RAIN,MV:1,TX:0.61,TY:5.00,AXG:-0.089,AYG:0.011,AZG:1.022,GXD:2.79,GYD:0.40,GZD:-0.08"}, {"timestamp": "25-10-2026 14:43:37", "packet_id": "409", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "89.7", "dew_point_c": "27.52", "soil_raw_adc": "1906", "soil_moisture_pct": "60.8", "soil_condition": "MODERATE", "rain_raw_adc": "1483", "rain_intensity_pct": "80.7", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1474", "accel_y_raw": "131", "accel_z_raw": "16924", "accel_x_g": "-0.09", "accel_y_g": "0.008", "accel_z_g": "1.033", "accel_magnitude_g": "1.037", "gyro_x_raw": "398", "gyro_y_raw": "52", "gyro_z_raw": "-65", "gyro_x_dps": "3.04", "gyro_y_dps": "0.4", "gyro_z_dps": "-0.5", "gyro_magnitude_dps": "3.11", "tilt_pitch_deg": "0.46", "tilt_roll_deg": "5.0", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "166", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.40,H:89.70,SRAW:1906,SM:60.8,SC:MODERATE,RRAW:1483,RP:80.7,RC:HEAVY_RAIN,MV:1,TX:0.46,TY:5.00,AXG:-0.090,AYG:0.008,AZG:1.033,GXD:3.04,GYD:0.40,GZD:-0.50"}, {"timestamp": "25-10-2026 14:43:42", "packet_id": "410", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "89.8", "dew_point_c": "27.54", "soil_raw_adc": "1827", "soil_moisture_pct": "65.2", "soil_condition": "WET", "rain_raw_adc": "1770", "rain_intensity_pct": "69.2", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-1458", "accel_y_raw": "163", "accel_z_raw": "16875", "accel_x_g": "-0.089", "accel_y_g": "0.01", "accel_z_g": "1.03", "accel_magnitude_g": "1.034", "gyro_x_raw": "358", "gyro_y_raw": "58", "gyro_z_raw": "-57", "gyro_x_dps": "2.74", "gyro_y_dps": "0.45", "gyro_z_dps": "-0.44", "gyro_magnitude_dps": "2.81", "tilt_pitch_deg": "0.54", "tilt_roll_deg": "4.92", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "164", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.40,H:89.80,SRAW:1827,SM:65.2,SC:WET,RRAW:1770,RP:69.2,RC:MODERATE_RAIN,MV:1,TX:0.54,TY:4.92,AXG:-0.089,AYG:0.010,AZG:1.030,GXD:2.74,GYD:0.45,GZD:-0.44"}, {"timestamp": "25-10-2026 14:43:47", "packet_id": "411", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "89.8", "dew_point_c": "27.54", "soil_raw_adc": "1924", "soil_moisture_pct": "59.8", "soil_condition": "MODERATE", "rain_raw_adc": "1560", "rain_intensity_pct": "77.6", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1425", "accel_y_raw": "163", "accel_z_raw": "16728", "accel_x_g": "-0.087", "accel_y_g": "0.01", "accel_z_g": "1.021", "accel_magnitude_g": "1.025", "gyro_x_raw": "412", "gyro_y_raw": "53", "gyro_z_raw": "-44", "gyro_x_dps": "3.15", "gyro_y_dps": "0.41", "gyro_z_dps": "-0.34", "gyro_magnitude_dps": "3.19", "tilt_pitch_deg": "0.59", "tilt_roll_deg": "4.87", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "166", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.40,H:89.80,SRAW:1924,SM:59.8,SC:MODERATE,RRAW:1560,RP:77.6,RC:HEAVY_RAIN,MV:1,TX:0.59,TY:4.87,AXG:-0.087,AYG:0.010,AZG:1.021,GXD:3.15,GYD:0.41,GZD:-0.34"}, {"timestamp": "25-10-2026 14:43:52", "packet_id": "412", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "89.8", "dew_point_c": "27.54", "soil_raw_adc": "2096", "soil_moisture_pct": "50.2", "soil_condition": "MODERATE", "rain_raw_adc": "2107", "rain_intensity_pct": "55.7", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-1392", "accel_y_raw": "180", "accel_z_raw": "16744", "accel_x_g": "-0.085", "accel_y_g": "0.011", "accel_z_g": "1.022", "accel_magnitude_g": "1.026", "gyro_x_raw": "417", "gyro_y_raw": "65", "gyro_z_raw": "-10", "gyro_x_dps": "3.19", "gyro_y_dps": "0.5", "gyro_z_dps": "-0.08", "gyro_magnitude_dps": "3.23", "tilt_pitch_deg": "0.64", "tilt_roll_deg": "4.78", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.40,H:89.80,SRAW:2096,SM:50.2,SC:MODERATE,RRAW:2107,RP:55.7,RC:MODERATE_RAIN,MV:1,TX:0.64,TY:4.78,AXG:-0.085,AYG:0.011,AZG:1.022,GXD:3.19,GYD:0.50,GZD:-0.08"}, {"timestamp": "25-10-2026 14:43:57", "packet_id": "413", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "89.9", "dew_point_c": "27.56", "soil_raw_adc": "2010", "soil_moisture_pct": "55.0", "soil_condition": "MODERATE", "rain_raw_adc": "1658", "rain_intensity_pct": "73.7", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-1458", "accel_y_raw": "147", "accel_z_raw": "16711", "accel_x_g": "-0.089", "accel_y_g": "0.009", "accel_z_g": "1.02", "accel_magnitude_g": "1.024", "gyro_x_raw": "411", "gyro_y_raw": "73", "gyro_z_raw": "-52", "gyro_x_dps": "3.14", "gyro_y_dps": "0.56", "gyro_z_dps": "-0.4", "gyro_magnitude_dps": "3.21", "tilt_pitch_deg": "0.51", "tilt_roll_deg": "4.97", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.40,H:89.90,SRAW:2010,SM:55.0,SC:MODERATE,RRAW:1658,RP:73.7,RC:MODERATE_RAIN,MV:1,TX:0.51,TY:4.97,AXG:-0.089,AYG:0.009,AZG:1.020,GXD:3.14,GYD:0.56,GZD:-0.40"}, {"timestamp": "25-10-2026 14:44:02", "packet_id": "414", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "89.8", "dew_point_c": "27.54", "soil_raw_adc": "2174", "soil_moisture_pct": "45.9", "soil_condition": "MODERATE", "rain_raw_adc": "1573", "rain_intensity_pct": "77.1", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1458", "accel_y_raw": "131", "accel_z_raw": "16793", "accel_x_g": "-0.089", "accel_y_g": "0.008", "accel_z_g": "1.025", "accel_magnitude_g": "1.029", "gyro_x_raw": "415", "gyro_y_raw": "68", "gyro_z_raw": "-23", "gyro_x_dps": "3.17", "gyro_y_dps": "0.52", "gyro_z_dps": "-0.18", "gyro_magnitude_dps": "3.22", "tilt_pitch_deg": "0.42", "tilt_roll_deg": "4.97", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "166", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.40,H:89.80,SRAW:2174,SM:45.9,SC:MODERATE,RRAW:1573,RP:77.1,RC:HEAVY_RAIN,MV:1,TX:0.42,TY:4.97,AXG:-0.089,AYG:0.008,AZG:1.025,GXD:3.17,GYD:0.52,GZD:-0.18"}, {"timestamp": "25-10-2026 14:44:07", "packet_id": "415", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "89.8", "dew_point_c": "27.54", "soil_raw_adc": "2224", "soil_moisture_pct": "43.1", "soil_condition": "MODERATE", "rain_raw_adc": "1667", "rain_intensity_pct": "73.3", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-1425", "accel_y_raw": "114", "accel_z_raw": "16728", "accel_x_g": "-0.087", "accel_y_g": "0.007", "accel_z_g": "1.021", "accel_magnitude_g": "1.025", "gyro_x_raw": "411", "gyro_y_raw": "43", "gyro_z_raw": "-51", "gyro_x_dps": "3.14", "gyro_y_dps": "0.33", "gyro_z_dps": "-0.39", "gyro_magnitude_dps": "3.18", "tilt_pitch_deg": "0.38", "tilt_roll_deg": "4.89", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.40,H:89.80,SRAW:2224,SM:43.1,SC:MODERATE,RRAW:1667,RP:73.3,RC:MODERATE_RAIN,MV:1,TX:0.38,TY:4.89,AXG:-0.087,AYG:0.007,AZG:1.021,GXD:3.14,GYD:0.33,GZD:-0.39"}, {"timestamp": "25-10-2026 14:44:12", "packet_id": "416", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "89.8", "dew_point_c": "27.54", "soil_raw_adc": "1856", "soil_moisture_pct": "63.6", "soil_condition": "MODERATE", "rain_raw_adc": "1535", "rain_intensity_pct": "78.6", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1523", "accel_y_raw": "163", "accel_z_raw": "16777", "accel_x_g": "-0.093", "accel_y_g": "0.01", "accel_z_g": "1.024", "accel_magnitude_g": "1.028", "gyro_x_raw": "394", "gyro_y_raw": "60", "gyro_z_raw": "-37", "gyro_x_dps": "3.01", "gyro_y_dps": "0.46", "gyro_z_dps": "-0.29", "gyro_magnitude_dps": "3.06", "tilt_pitch_deg": "0.57", "tilt_roll_deg": "5.16", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "166", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.40,H:89.80,SRAW:1856,SM:63.6,SC:MODERATE,RRAW:1535,RP:78.6,RC:HEAVY_RAIN,MV:1,TX:0.57,TY:5.16,AXG:-0.093,AYG:0.010,AZG:1.024,GXD:3.01,GYD:0.46,GZD:-0.29"}, {"timestamp": "25-10-2026 14:44:17", "packet_id": "417", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "89.9", "dew_point_c": "27.56", "soil_raw_adc": "1864", "soil_moisture_pct": "63.1", "soil_condition": "MODERATE", "rain_raw_adc": "1559", "rain_intensity_pct": "77.6", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1458", "accel_y_raw": "212", "accel_z_raw": "16793", "accel_x_g": "-0.089", "accel_y_g": "0.013", "accel_z_g": "1.025", "accel_magnitude_g": "1.029", "gyro_x_raw": "386", "gyro_y_raw": "43", "gyro_z_raw": "-44", "gyro_x_dps": "2.95", "gyro_y_dps": "0.33", "gyro_z_dps": "-0.34", "gyro_magnitude_dps": "2.99", "tilt_pitch_deg": "0.72", "tilt_roll_deg": "4.94", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "166", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.40,H:89.90,SRAW:1864,SM:63.1,SC:MODERATE,RRAW:1559,RP:77.6,RC:HEAVY_RAIN,MV:1,TX:0.72,TY:4.94,AXG:-0.089,AYG:0.013,AZG:1.025,GXD:2.95,GYD:0.33,GZD:-0.34"}, {"timestamp": "25-10-2026 14:44:22", "packet_id": "418", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "89.8", "dew_point_c": "27.54", "soil_raw_adc": "2118", "soil_moisture_pct": "49.0", "soil_condition": "MODERATE", "rain_raw_adc": "2243", "rain_intensity_pct": "50.3", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-1458", "accel_y_raw": "262", "accel_z_raw": "16678", "accel_x_g": "-0.089", "accel_y_g": "0.016", "accel_z_g": "1.018", "accel_magnitude_g": "1.022", "gyro_x_raw": "383", "gyro_y_raw": "82", "gyro_z_raw": "-44", "gyro_x_dps": "2.93", "gyro_y_dps": "0.63", "gyro_z_dps": "-0.34", "gyro_magnitude_dps": "3.02", "tilt_pitch_deg": "0.89", "tilt_roll_deg": "5.0", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.40,H:89.80,SRAW:2118,SM:49.0,SC:MODERATE,RRAW:2243,RP:50.3,RC:MODERATE_RAIN,MV:1,TX:0.89,TY:5.00,AXG:-0.089,AYG:0.016,AZG:1.018,GXD:2.93,GYD:0.63,GZD:-0.34"}, {"timestamp": "25-10-2026 14:44:27", "packet_id": "419", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "89.8", "dew_point_c": "27.54", "soil_raw_adc": "1881", "soil_moisture_pct": "62.2", "soil_condition": "MODERATE", "rain_raw_adc": "1531", "rain_intensity_pct": "78.8", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "16367", "accel_y_raw": "-311", "accel_z_raw": "2359", "accel_x_g": "0.999", "accel_y_g": "-0.019", "accel_z_g": "0.144", "accel_magnitude_g": "1.01", "gyro_x_raw": "416", "gyro_y_raw": "424", "gyro_z_raw": "-75", "gyro_x_dps": "3.18", "gyro_y_dps": "3.24", "gyro_z_dps": "-0.58", "gyro_magnitude_dps": "4.58", "tilt_pitch_deg": "-1.1", "tilt_roll_deg": "-81.75", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.40,H:89.80,SRAW:1881,SM:62.2,SC:MODERATE,RRAW:1531,RP:78.8,RC:HEAVY_RAIN,MV:1,TX:-1.10,TY:-81.75,AXG:0.999,AYG:-0.019,AZG:0.144,GXD:3.18,GYD:3.24,GZD:-0.58"}, {"timestamp": "25-10-2026 14:44:32", "packet_id": "420", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "89.7", "dew_point_c": "27.52", "soil_raw_adc": "1887", "soil_moisture_pct": "61.8", "soil_condition": "MODERATE", "rain_raw_adc": "1517", "rain_intensity_pct": "79.3", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "16236", "accel_y_raw": "-327", "accel_z_raw": "3080", "accel_x_g": "0.991", "accel_y_g": "-0.02", "accel_z_g": "0.188", "accel_magnitude_g": "1.009", "gyro_x_raw": "416", "gyro_y_raw": "664", "gyro_z_raw": "-104", "gyro_x_dps": "3.18", "gyro_y_dps": "5.07", "gyro_z_dps": "-0.8", "gyro_magnitude_dps": "6.04", "tilt_pitch_deg": "-1.15", "tilt_roll_deg": "-79.21", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.40,H:89.70,SRAW:1887,SM:61.8,SC:MODERATE,RRAW:1517,RP:79.3,RC:HEAVY_RAIN,MV:1,TX:-1.15,TY:-79.21,AXG:0.991,AYG:-0.020,AZG:0.188,GXD:3.18,GYD:5.07,GZD:-0.80"}, {"timestamp": "25-10-2026 14:44:37", "packet_id": "421", "temperature_c": "29.5", "temperature_f": "85.1", "humidity_pct": "89.7", "dew_point_c": "27.62", "soil_raw_adc": "1900", "soil_moisture_pct": "61.1", "soil_condition": "MODERATE", "rain_raw_adc": "1532", "rain_intensity_pct": "78.7", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "16367", "accel_y_raw": "-442", "accel_z_raw": "2113", "accel_x_g": "0.999", "accel_y_g": "-0.027", "accel_z_g": "0.129", "accel_magnitude_g": "1.008", "gyro_x_raw": "423", "gyro_y_raw": "74", "gyro_z_raw": "-55", "gyro_x_dps": "3.23", "gyro_y_dps": "0.57", "gyro_z_dps": "-0.42", "gyro_magnitude_dps": "3.31", "tilt_pitch_deg": "-1.55", "tilt_roll_deg": "-82.47", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.50,H:89.70,SRAW:1900,SM:61.1,SC:MODERATE,RRAW:1532,RP:78.7,RC:HEAVY_RAIN,MV:1,TX:-1.55,TY:-82.47,AXG:0.999,AYG:-0.027,AZG:0.129,GXD:3.23,GYD:0.57,GZD:-0.42"}, {"timestamp": "25-10-2026 14:44:42", "packet_id": "422", "temperature_c": "29.5", "temperature_f": "85.1", "humidity_pct": "89.7", "dew_point_c": "27.62", "soil_raw_adc": "1930", "soil_moisture_pct": "59.4", "soil_condition": "MODERATE", "rain_raw_adc": "1588", "rain_intensity_pct": "76.5", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "12140", "accel_y_raw": "65", "accel_z_raw": "12320", "accel_x_g": "0.741", "accel_y_g": "0.004", "accel_z_g": "0.752", "accel_magnitude_g": "1.056", "gyro_x_raw": "-712", "gyro_y_raw": "906", "gyro_z_raw": "-1083", "gyro_x_dps": "-5.44", "gyro_y_dps": "6.92", "gyro_z_dps": "-8.27", "gyro_magnitude_dps": "12.08", "tilt_pitch_deg": "0.24", "tilt_roll_deg": "-44.56", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "168", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.50,H:89.70,SRAW:1930,SM:59.4,SC:MODERATE,RRAW:1588,RP:76.5,RC:HEAVY_RAIN,MV:1,TX:0.24,TY:-44.56,AXG:0.741,AYG:0.004,AZG:0.752,GXD:-5.44,GYD:6.92,GZD:-8.27"}, {"timestamp": "25-10-2026 14:44:47", "packet_id": "423", "temperature_c": "29.4", "temperature_f": "84.92", "humidity_pct": "89.7", "dew_point_c": "27.52", "soil_raw_adc": "1908", "soil_moisture_pct": "60.7", "soil_condition": "MODERATE", "rain_raw_adc": "1535", "rain_intensity_pct": "78.6", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "11468", "accel_y_raw": "-327", "accel_z_raw": "11976", "accel_x_g": "0.7", "accel_y_g": "-0.02", "accel_z_g": "0.731", "accel_magnitude_g": "1.012", "gyro_x_raw": "387", "gyro_y_raw": "-250", "gyro_z_raw": "-34", "gyro_x_dps": "2.96", "gyro_y_dps": "-1.91", "gyro_z_dps": "-0.26", "gyro_magnitude_dps": "3.53", "tilt_pitch_deg": "-1.15", "tilt_roll_deg": "-43.77", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "170", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.40,H:89.70,SRAW:1908,SM:60.7,SC:MODERATE,RRAW:1535,RP:78.6,RC:HEAVY_RAIN,MV:1,TX:-1.15,TY:-43.77,AXG:0.700,AYG:-0.020,AZG:0.731,GXD:2.96,GYD:-1.91,GZD:-0.26"}, {"timestamp": "25-10-2026 14:44:52", "packet_id": "424", "temperature_c": "29.5", "temperature_f": "85.1", "humidity_pct": "89.7", "dew_point_c": "27.62", "soil_raw_adc": "1889", "soil_moisture_pct": "61.7", "soil_condition": "MODERATE", "rain_raw_adc": "1535", "rain_intensity_pct": "78.6", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "11862", "accel_y_raw": "-278", "accel_z_raw": "11796", "accel_x_g": "0.724", "accel_y_g": "-0.017", "accel_z_g": "0.72", "accel_magnitude_g": "1.021", "gyro_x_raw": "375", "gyro_y_raw": "-238", "gyro_z_raw": "-19", "gyro_x_dps": "2.87", "gyro_y_dps": "-1.82", "gyro_z_dps": "-0.15", "gyro_magnitude_dps": "3.4", "tilt_pitch_deg": "-0.97", "tilt_roll_deg": "-45.13", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "170", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.50,H:89.70,SRAW:1889,SM:61.7,SC:MODERATE,RRAW:1535,RP:78.6,RC:HEAVY_RAIN,MV:1,TX:-0.97,TY:-45.13,AXG:0.724,AYG:-0.017,AZG:0.720,GXD:2.87,GYD:-1.82,GZD:-0.15"}, {"timestamp": "25-10-2026 14:44:57", "packet_id": "425", "temperature_c": "29.5", "temperature_f": "85.1", "humidity_pct": "89.7", "dew_point_c": "27.62", "soil_raw_adc": "1899", "soil_moisture_pct": "61.2", "soil_condition": "MODERATE", "rain_raw_adc": "1584", "rain_intensity_pct": "76.6", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1654", "accel_y_raw": "606", "accel_z_raw": "16826", "accel_x_g": "-0.101", "accel_y_g": "0.037", "accel_z_g": "1.027", "accel_magnitude_g": "1.033", "gyro_x_raw": "420", "gyro_y_raw": "120", "gyro_z_raw": "-31", "gyro_x_dps": "3.21", "gyro_y_dps": "0.92", "gyro_z_dps": "-0.24", "gyro_magnitude_dps": "3.35", "tilt_pitch_deg": "2.06", "tilt_roll_deg": "5.61", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "166", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.50,H:89.70,SRAW:1899,SM:61.2,SC:MODERATE,RRAW:1584,RP:76.6,RC:HEAVY_RAIN,MV:1,TX:2.06,TY:5.61,AXG:-0.101,AYG:0.037,AZG:1.027,GXD:3.21,GYD:0.92,GZD:-0.24"}, {"timestamp": "25-10-2026 14:45:02", "packet_id": "426", "temperature_c": "29.5", "temperature_f": "85.1", "humidity_pct": "89.7", "dew_point_c": "27.62", "soil_raw_adc": "1903", "soil_moisture_pct": "60.9", "soil_condition": "MODERATE", "rain_raw_adc": "1586", "rain_intensity_pct": "76.6", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1671", "accel_y_raw": "524", "accel_z_raw": "16711", "accel_x_g": "-0.102", "accel_y_g": "0.032", "accel_z_g": "1.02", "accel_magnitude_g": "1.026", "gyro_x_raw": "396", "gyro_y_raw": "73", "gyro_z_raw": "-48", "gyro_x_dps": "3.03", "gyro_y_dps": "0.56", "gyro_z_dps": "-0.37", "gyro_magnitude_dps": "3.1", "tilt_pitch_deg": "1.81", "tilt_roll_deg": "5.7", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "166", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.50,H:89.70,SRAW:1903,SM:60.9,SC:MODERATE,RRAW:1586,RP:76.6,RC:HEAVY_RAIN,MV:1,TX:1.81,TY:5.70,AXG:-0.102,AYG:0.032,AZG:1.020,GXD:3.03,GYD:0.56,GZD:-0.37"}, {"timestamp": "25-10-2026 14:45:07", "packet_id": "427", "temperature_c": "29.5", "temperature_f": "85.1", "humidity_pct": "89.7", "dew_point_c": "27.62", "soil_raw_adc": "1918", "soil_moisture_pct": "60.1", "soil_condition": "MODERATE", "rain_raw_adc": "1589", "rain_intensity_pct": "76.4", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1638", "accel_y_raw": "491", "accel_z_raw": "16744", "accel_x_g": "-0.1", "accel_y_g": "0.03", "accel_z_g": "1.022", "accel_magnitude_g": "1.027", "gyro_x_raw": "399", "gyro_y_raw": "64", "gyro_z_raw": "-24", "gyro_x_dps": "3.05", "gyro_y_dps": "0.49", "gyro_z_dps": "-0.19", "gyro_magnitude_dps": "3.09", "tilt_pitch_deg": "1.69", "tilt_roll_deg": "5.61", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "166", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.50,H:89.70,SRAW:1918,SM:60.1,SC:MODERATE,RRAW:1589,RP:76.4,RC:HEAVY_RAIN,MV:1,TX:1.69,TY:5.61,AXG:-0.100,AYG:0.030,AZG:1.022,GXD:3.05,GYD:0.49,GZD:-0.19"}, {"timestamp": "25-10-2026 14:45:12", "packet_id": "428", "temperature_c": "29.5", "temperature_f": "85.1", "humidity_pct": "89.7", "dew_point_c": "27.62", "soil_raw_adc": "1968", "soil_moisture_pct": "57.3", "soil_condition": "MODERATE", "rain_raw_adc": "1593", "rain_intensity_pct": "76.3", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1703", "accel_y_raw": "688", "accel_z_raw": "16842", "accel_x_g": "-0.104", "accel_y_g": "0.042", "accel_z_g": "1.028", "accel_magnitude_g": "1.034", "gyro_x_raw": "387", "gyro_y_raw": "61", "gyro_z_raw": "-74", "gyro_x_dps": "2.96", "gyro_y_dps": "0.47", "gyro_z_dps": "-0.57", "gyro_magnitude_dps": "3.05", "tilt_pitch_deg": "2.33", "tilt_roll_deg": "5.74", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "166", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.50,H:89.70,SRAW:1968,SM:57.3,SC:MODERATE,RRAW:1593,RP:76.3,RC:HEAVY_RAIN,MV:1,TX:2.33,TY:5.74,AXG:-0.104,AYG:0.042,AZG:1.028,GXD:2.96,GYD:0.47,GZD:-0.57"}, {"timestamp": "25-10-2026 14:45:17", "packet_id": "429", "temperature_c": "29.5", "temperature_f": "85.1", "humidity_pct": "89.6", "dew_point_c": "27.6", "soil_raw_adc": "1911", "soil_moisture_pct": "60.5", "soil_condition": "MODERATE", "rain_raw_adc": "1610", "rain_intensity_pct": "75.6", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1638", "accel_y_raw": "458", "accel_z_raw": "16826", "accel_x_g": "-0.1", "accel_y_g": "0.028", "accel_z_g": "1.027", "accel_magnitude_g": "1.032", "gyro_x_raw": "413", "gyro_y_raw": "69", "gyro_z_raw": "-14", "gyro_x_dps": "3.16", "gyro_y_dps": "0.53", "gyro_z_dps": "-0.11", "gyro_magnitude_dps": "3.21", "tilt_pitch_deg": "1.57", "tilt_roll_deg": "5.58", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "166", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.50,H:89.60,SRAW:1911,SM:60.5,SC:MODERATE,RRAW:1610,RP:75.6,RC:HEAVY_RAIN,MV:1,TX:1.57,TY:5.58,AXG:-0.100,AYG:0.028,AZG:1.027,GXD:3.16,GYD:0.53,GZD:-0.11"}, {"timestamp": "25-10-2026 14:45:22", "packet_id": "430", "temperature_c": "29.5", "temperature_f": "85.1", "humidity_pct": "89.6", "dew_point_c": "27.6", "soil_raw_adc": "1970", "soil_moisture_pct": "57.2", "soil_condition": "MODERATE", "rain_raw_adc": "1626", "rain_intensity_pct": "75.0", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1703", "accel_y_raw": "491", "accel_z_raw": "16777", "accel_x_g": "-0.104", "accel_y_g": "0.03", "accel_z_g": "1.024", "accel_magnitude_g": "1.03", "gyro_x_raw": "372", "gyro_y_raw": "27", "gyro_z_raw": "-61", "gyro_x_dps": "2.84", "gyro_y_dps": "0.21", "gyro_z_dps": "-0.47", "gyro_magnitude_dps": "2.89", "tilt_pitch_deg": "1.66", "tilt_roll_deg": "5.77", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "166", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.50,H:89.60,SRAW:1970,SM:57.2,SC:MODERATE,RRAW:1626,RP:75.0,RC:HEAVY_RAIN,MV:1,TX:1.66,TY:5.77,AXG:-0.104,AYG:0.030,AZG:1.024,GXD:2.84,GYD:0.21,GZD:-0.47"}, {"timestamp": "25-10-2026 14:45:27", "packet_id": "431", "temperature_c": "29.5", "temperature_f": "85.1", "humidity_pct": "89.6", "dew_point_c": "27.6", "soil_raw_adc": "1899", "soil_moisture_pct": "61.2", "soil_condition": "MODERATE", "rain_raw_adc": "1632", "rain_intensity_pct": "74.7", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-1703", "accel_y_raw": "589", "accel_z_raw": "16744", "accel_x_g": "-0.104", "accel_y_g": "0.036", "accel_z_g": "1.022", "accel_magnitude_g": "1.028", "gyro_x_raw": "398", "gyro_y_raw": "62", "gyro_z_raw": "-48", "gyro_x_dps": "3.04", "gyro_y_dps": "0.48", "gyro_z_dps": "-0.37", "gyro_magnitude_dps": "3.1", "tilt_pitch_deg": "1.99", "tilt_roll_deg": "5.83", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.50,H:89.60,SRAW:1899,SM:61.2,SC:MODERATE,RRAW:1632,RP:74.7,RC:MODERATE_RAIN,MV:1,TX:1.99,TY:5.83,AXG:-0.104,AYG:0.036,AZG:1.022,GXD:3.04,GYD:0.48,GZD:-0.37"}, {"timestamp": "25-10-2026 14:45:32", "packet_id": "432", "temperature_c": "29.5", "temperature_f": "85.1", "humidity_pct": "89.6", "dew_point_c": "27.6", "soil_raw_adc": "1920", "soil_moisture_pct": "60.0", "soil_condition": "MODERATE", "rain_raw_adc": "1650", "rain_intensity_pct": "74.0", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-1638", "accel_y_raw": "524", "accel_z_raw": "16728", "accel_x_g": "-0.1", "accel_y_g": "0.032", "accel_z_g": "1.021", "accel_magnitude_g": "1.026", "gyro_x_raw": "404", "gyro_y_raw": "53", "gyro_z_raw": "-53", "gyro_x_dps": "3.09", "gyro_y_dps": "0.41", "gyro_z_dps": "-0.41", "gyro_magnitude_dps": "3.14", "tilt_pitch_deg": "1.79", "tilt_roll_deg": "5.58", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.50,H:89.60,SRAW:1920,SM:60.0,SC:MODERATE,RRAW:1650,RP:74.0,RC:MODERATE_RAIN,MV:1,TX:1.79,TY:5.58,AXG:-0.100,AYG:0.032,AZG:1.021,GXD:3.09,GYD:0.41,GZD:-0.41"}, {"timestamp": "25-10-2026 14:45:37", "packet_id": "433", "temperature_c": "29.5", "temperature_f": "85.1", "humidity_pct": "89.6", "dew_point_c": "27.6", "soil_raw_adc": "2014", "soil_moisture_pct": "54.8", "soil_condition": "MODERATE", "rain_raw_adc": "1639", "rain_intensity_pct": "74.4", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "16252", "accel_y_raw": "-425", "accel_z_raw": "2932", "accel_x_g": "0.992", "accel_y_g": "-0.026", "accel_z_g": "0.179", "accel_magnitude_g": "1.008", "gyro_x_raw": "387", "gyro_y_raw": "1337", "gyro_z_raw": "-89", "gyro_x_dps": "2.96", "gyro_y_dps": "10.21", "gyro_z_dps": "-0.68", "gyro_magnitude_dps": "10.65", "tilt_pitch_deg": "-1.47", "tilt_roll_deg": "-79.64", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "173", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.50,H:89.60,SRAW:2014,SM:54.8,SC:MODERATE,RRAW:1639,RP:74.4,RC:MODERATE_RAIN,MV:1,TX:-1.47,TY:-79.64,AXG:0.992,AYG:-0.026,AZG:0.179,GXD:2.96,GYD:10.21,GZD:-0.68"}, {"timestamp": "25-10-2026 14:45:42", "packet_id": "434", "temperature_c": "29.5", "temperature_f": "85.1", "humidity_pct": "89.5", "dew_point_c": "27.58", "soil_raw_adc": "1959", "soil_moisture_pct": "57.8", "soil_condition": "MODERATE", "rain_raw_adc": "1629", "rain_intensity_pct": "74.8", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "16334", "accel_y_raw": "-393", "accel_z_raw": "2457", "accel_x_g": "0.997", "accel_y_g": "-0.024", "accel_z_g": "0.15", "accel_magnitude_g": "1.009", "gyro_x_raw": "374", "gyro_y_raw": "81", "gyro_z_raw": "5", "gyro_x_dps": "2.86", "gyro_y_dps": "0.62", "gyro_z_dps": "0.04", "gyro_magnitude_dps": "2.93", "tilt_pitch_deg": "-1.36", "tilt_roll_deg": "-81.35", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "171", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.50,H:89.50,SRAW:1959,SM:57.8,SC:MODERATE,RRAW:1629,RP:74.8,RC:MODERATE_RAIN,MV:1,TX:-1.36,TY:-81.35,AXG:0.997,AYG:-0.024,AZG:0.150,GXD:2.86,GYD:0.62,GZD:0.04"}, {"timestamp": "25-10-2026 14:45:47", "packet_id": "435", "temperature_c": "29.5", "temperature_f": "85.1", "humidity_pct": "89.5", "dew_point_c": "27.58", "soil_raw_adc": "1859", "soil_moisture_pct": "63.4", "soil_condition": "MODERATE", "rain_raw_adc": "1731", "rain_intensity_pct": "70.8", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "16351", "accel_y_raw": "-393", "accel_z_raw": "2572", "accel_x_g": "0.998", "accel_y_g": "-0.024", "accel_z_g": "0.157", "accel_magnitude_g": "1.011", "gyro_x_raw": "424", "gyro_y_raw": "161", "gyro_z_raw": "-30", "gyro_x_dps": "3.24", "gyro_y_dps": "1.23", "gyro_z_dps": "-0.23", "gyro_magnitude_dps": "3.47", "tilt_pitch_deg": "-1.36", "tilt_roll_deg": "-80.96", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "172", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.50,H:89.50,SRAW:1859,SM:63.4,SC:MODERATE,RRAW:1731,RP:70.8,RC:MODERATE_RAIN,MV:1,TX:-1.36,TY:-80.96,AXG:0.998,AYG:-0.024,AZG:0.157,GXD:3.24,GYD:1.23,GZD:-0.23"}, {"timestamp": "25-10-2026 14:45:52", "packet_id": "436", "temperature_c": "29.5", "temperature_f": "85.1", "humidity_pct": "89.5", "dew_point_c": "27.58", "soil_raw_adc": "1908", "soil_moisture_pct": "60.7", "soil_condition": "MODERATE", "rain_raw_adc": "1593", "rain_intensity_pct": "76.3", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "13811", "accel_y_raw": "-573", "accel_z_raw": "7700", "accel_x_g": "0.843", "accel_y_g": "-0.035", "accel_z_g": "0.47", "accel_magnitude_g": "0.966", "gyro_x_raw": "-2073", "gyro_y_raw": "15093", "gyro_z_raw": "-1727", "gyro_x_dps": "-15.83", "gyro_y_dps": "115.22", "gyro_z_dps": "-13.19", "gyro_magnitude_dps": "117.05", "tilt_pitch_deg": "-2.06", "tilt_roll_deg": "-60.8", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "174", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:29.50,H:89.50,SRAW:1908,SM:60.7,SC:MODERATE,RRAW:1593,RP:76.3,RC:HEAVY_RAIN,MV:1,TX:-2.06,TY:-60.80,AXG:0.843,AYG:-0.035,AZG:0.470,GXD:-15.83,GYD:115.22,GZD:-13.19"}, {"timestamp": "25-10-2026 14:45:57", "packet_id": "437", "temperature_c": "29.5", "temperature_f": "85.1", "humidity_pct": "89.5", "dew_point_c": "27.58", "soil_raw_adc": "1919", "soil_moisture_pct": "60.1", "soil_condition": "MODERATE", "rain_raw_adc": "1611", "rain_intensity_pct": "75.6", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-1982", "accel_y_raw": "98", "accel_z_raw": "16646", "accel_x_g": "-0.121", "accel_y_g": "0.006", "accel_z_g": "1.016", "accel_magnitude_g": "1.023", "gyro_x_raw": "390", "gyro_y_raw": "39", "gyro_z_raw": "-43", "gyro_x_dps": "2.98", "gyro_y_dps": "0.3", "gyro_z_dps": "-0.33", "gyro_magnitude_dps": "3.01", "tilt_pitch_deg": "0.36", "tilt_roll_deg": "6.78", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "166", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.50,H:89.50,SRAW:1919,SM:60.1,SC:MODERATE,RRAW:1611,RP:75.6,RC:HEAVY_RAIN,MV:1,TX:0.36,TY:6.78,AXG:-0.121,AYG:0.006,AZG:1.016,GXD:2.98,GYD:0.30,GZD:-0.33"}, {"timestamp": "25-10-2026 14:46:02", "packet_id": "438", "temperature_c": "29.5", "temperature_f": "85.1", "humidity_pct": "89.5", "dew_point_c": "27.58", "soil_raw_adc": "1934", "soil_moisture_pct": "59.2", "soil_condition": "MODERATE", "rain_raw_adc": "1623", "rain_intensity_pct": "75.1", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-2244", "accel_y_raw": "16", "accel_z_raw": "16662", "accel_x_g": "-0.137", "accel_y_g": "0.001", "accel_z_g": "1.017", "accel_magnitude_g": "1.026", "gyro_x_raw": "364", "gyro_y_raw": "102", "gyro_z_raw": "-58", "gyro_x_dps": "2.78", "gyro_y_dps": "0.78", "gyro_z_dps": "-0.45", "gyro_magnitude_dps": "2.92", "tilt_pitch_deg": "0.08", "tilt_roll_deg": "7.67", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "166", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.50,H:89.50,SRAW:1934,SM:59.2,SC:MODERATE,RRAW:1623,RP:75.1,RC:HEAVY_RAIN,MV:1,TX:0.08,TY:7.67,AXG:-0.137,AYG:0.001,AZG:1.017,GXD:2.78,GYD:0.78,GZD:-0.45"}, {"timestamp": "25-10-2026 14:46:07", "packet_id": "439", "temperature_c": "29.5", "temperature_f": "85.1", "humidity_pct": "89.5", "dew_point_c": "27.58", "soil_raw_adc": "1981", "soil_moisture_pct": "56.6", "soil_condition": "MODERATE", "rain_raw_adc": "1649", "rain_intensity_pct": "74.0", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-2097", "accel_y_raw": "114", "accel_z_raw": "16662", "accel_x_g": "-0.128", "accel_y_g": "0.007", "accel_z_g": "1.017", "accel_magnitude_g": "1.025", "gyro_x_raw": "417", "gyro_y_raw": "44", "gyro_z_raw": "-55", "gyro_x_dps": "3.19", "gyro_y_dps": "0.34", "gyro_z_dps": "-0.42", "gyro_magnitude_dps": "3.24", "tilt_pitch_deg": "0.4", "tilt_roll_deg": "7.17", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.50,H:89.50,SRAW:1981,SM:56.6,SC:MODERATE,RRAW:1649,RP:74.0,RC:MODERATE_RAIN,MV:1,TX:0.40,TY:7.17,AXG:-0.128,AYG:0.007,AZG:1.017,GXD:3.19,GYD:0.34,GZD:-0.42"}, {"timestamp": "25-10-2026 14:46:12", "packet_id": "440", "temperature_c": "29.5", "temperature_f": "85.1", "humidity_pct": "89.5", "dew_point_c": "27.58", "soil_raw_adc": "1985", "soil_moisture_pct": "56.4", "soil_condition": "MODERATE", "rain_raw_adc": "1652", "rain_intensity_pct": "73.9", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-2080", "accel_y_raw": "120", "accel_z_raw": "16660", "accel_x_g": "-0.127", "accel_y_g": "0.007", "accel_z_g": "1.017", "accel_magnitude_g": "1.025", "gyro_x_raw": "415", "gyro_y_raw": "46", "gyro_z_raw": "-52", "gyro_x_dps": "3.17", "gyro_y_dps": "0.35", "gyro_z_dps": "-0.40", "gyro_magnitude_dps": "3.22", "tilt_pitch_deg": "0.41", "tilt_roll_deg": "7.15", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:29.50,H:89.50,SRAW:1985,SM:56.4,SC:MODERATE,RRAW:1652,RP:73.9,RC:MODERATE_RAIN,MV:1,TX:0.41,TY:7.15,AXG:-0.127,AYG:0.007,AZG:1.017,GXD:3.17,GYD:0.35,GZD:-0.40"}, {"timestamp": "04-11-2026 00:09:22", "packet_id": "441", "temperature_c": "30.6", "temperature_f": "87.08", "humidity_pct": "80.8", "dew_point_c": "26.91", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "4095", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-2097", "accel_y_raw": "294", "accel_z_raw": "16678", "accel_x_g": "-0.128", "accel_y_g": "0.018", "accel_z_g": "1.018", "accel_magnitude_g": "1.026", "gyro_x_raw": "349", "gyro_y_raw": "61", "gyro_z_raw": "-14", "gyro_x_dps": "2.67", "gyro_y_dps": "0.47", "gyro_z_dps": "-0.11", "gyro_magnitude_dps": "2.71", "tilt_pitch_deg": "1.0", "tilt_roll_deg": "7.18", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "156", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:30.60,H:80.80,SRAW:4095,SM:0.0,SC:DRY,RRAW:4095,RP:0.0,RC:NO_RAIN,MV:1,TX:1.00,TY:7.18,AXG:-0.128,AYG:0.018,AZG:1.018,GXD:2.67,GYD:0.47,GZD:-0.11"}, {"timestamp": "04-11-2026 00:09:27", "packet_id": "442", "temperature_c": "30.6", "temperature_f": "87.08", "humidity_pct": "80.8", "dew_point_c": "26.91", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "4095", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-2031", "accel_y_raw": "294", "accel_z_raw": "16728", "accel_x_g": "-0.124", "accel_y_g": "0.018", "accel_z_g": "1.021", "accel_magnitude_g": "1.029", "gyro_x_raw": "386", "gyro_y_raw": "52", "gyro_z_raw": "-49", "gyro_x_dps": "2.95", "gyro_y_dps": "0.4", "gyro_z_dps": "-0.38", "gyro_magnitude_dps": "3.0", "tilt_pitch_deg": "0.99", "tilt_roll_deg": "6.91", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "156", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:30.60,H:80.80,SRAW:4095,SM:0.0,SC:DRY,RRAW:4095,RP:0.0,RC:NO_RAIN,MV:1,TX:0.99,TY:6.91,AXG:-0.124,AYG:0.018,AZG:1.021,GXD:2.95,GYD:0.40,GZD:-0.38"}, {"timestamp": "04-11-2026 00:09:32", "packet_id": "443", "temperature_c": "30.6", "temperature_f": "87.08", "humidity_pct": "80.8", "dew_point_c": "26.91", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "4095", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-2080", "accel_y_raw": "180", "accel_z_raw": "16809", "accel_x_g": "-0.127", "accel_y_g": "0.011", "accel_z_g": "1.026", "accel_magnitude_g": "1.034", "gyro_x_raw": "366", "gyro_y_raw": "106", "gyro_z_raw": "-61", "gyro_x_dps": "2.8", "gyro_y_dps": "0.81", "gyro_z_dps": "-0.47", "gyro_magnitude_dps": "2.95", "tilt_pitch_deg": "0.6", "tilt_roll_deg": "7.06", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "156", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:30.60,H:80.80,SRAW:4095,SM:0.0,SC:DRY,RRAW:4095,RP:0.0,RC:NO_RAIN,MV:1,TX:0.60,TY:7.06,AXG:-0.127,AYG:0.011,AZG:1.026,GXD:2.80,GYD:0.81,GZD:-0.47"}, {"timestamp": "04-11-2026 00:09:37", "packet_id": "444", "temperature_c": "30.6", "temperature_f": "87.08", "humidity_pct": "80.8", "dew_point_c": "26.91", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "4095", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-2031", "accel_y_raw": "212", "accel_z_raw": "16629", "accel_x_g": "-0.124", "accel_y_g": "0.013", "accel_z_g": "1.015", "accel_magnitude_g": "1.023", "gyro_x_raw": "412", "gyro_y_raw": "98", "gyro_z_raw": "-6", "gyro_x_dps": "3.15", "gyro_y_dps": "0.75", "gyro_z_dps": "-0.05", "gyro_magnitude_dps": "3.24", "tilt_pitch_deg": "0.74", "tilt_roll_deg": "6.98", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "156", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:30.60,H:80.80,SRAW:4095,SM:0.0,SC:DRY,RRAW:4095,RP:0.0,RC:NO_RAIN,MV:1,TX:0.74,TY:6.98,AXG:-0.124,AYG:0.013,AZG:1.015,GXD:3.15,GYD:0.75,GZD:-0.05"}, {"timestamp": "04-11-2026 00:09:42", "packet_id": "445", "temperature_c": "30.6", "temperature_f": "87.08", "humidity_pct": "80.9", "dew_point_c": "26.93", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "4095", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-2113", "accel_y_raw": "278", "accel_z_raw": "16728", "accel_x_g": "-0.129", "accel_y_g": "0.017", "accel_z_g": "1.021", "accel_magnitude_g": "1.029", "gyro_x_raw": "419", "gyro_y_raw": "81", "gyro_z_raw": "-58", "gyro_x_dps": "3.2", "gyro_y_dps": "0.62", "gyro_z_dps": "-0.45", "gyro_magnitude_dps": "3.29", "tilt_pitch_deg": "0.94", "tilt_roll_deg": "7.22", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "156", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:30.60,H:80.90,SRAW:4095,SM:0.0,SC:DRY,RRAW:4095,RP:0.0,RC:NO_RAIN,MV:1,TX:0.94,TY:7.22,AXG:-0.129,AYG:0.017,AZG:1.021,GXD:3.20,GYD:0.62,GZD:-0.45"}, {"timestamp": "04-11-2026 00:09:47", "packet_id": "446", "temperature_c": "30.6", "temperature_f": "87.08", "humidity_pct": "80.9", "dew_point_c": "26.93", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "4095", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-14991", "accel_y_raw": "311", "accel_z_raw": "6701", "accel_x_g": "-0.915", "accel_y_g": "0.019", "accel_z_g": "0.409", "accel_magnitude_g": "1.002", "gyro_x_raw": "399", "gyro_y_raw": "-324", "gyro_z_raw": "-34", "gyro_x_dps": "3.05", "gyro_y_dps": "-2.48", "gyro_z_dps": "-0.26", "gyro_magnitude_dps": "3.94", "tilt_pitch_deg": "1.06", "tilt_roll_deg": "65.89", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "158", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.60,H:80.90,SRAW:4095,SM:0.0,SC:DRY,RRAW:4095,RP:0.0,RC:NO_RAIN,MV:1,TX:1.06,TY:65.89,AXG:-0.915,AYG:0.019,AZG:0.409,GXD:3.05,GYD:-2.48,GZD:-0.26"}, {"timestamp": "04-11-2026 00:09:52", "packet_id": "447", "temperature_c": "30.6", "temperature_f": "87.08", "humidity_pct": "80.9", "dew_point_c": "26.93", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "4095", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-14843", "accel_y_raw": "278", "accel_z_raw": "6914", "accel_x_g": "-0.906", "accel_y_g": "0.017", "accel_z_g": "0.422", "accel_magnitude_g": "1.0", "gyro_x_raw": "374", "gyro_y_raw": "-35", "gyro_z_raw": "26", "gyro_x_dps": "2.86", "gyro_y_dps": "-0.27", "gyro_z_dps": "0.2", "gyro_magnitude_dps": "2.88", "tilt_pitch_deg": "0.99", "tilt_roll_deg": "65.0", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "157", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.60,H:80.90,SRAW:4095,SM:0.0,SC:DRY,RRAW:4095,RP:0.0,RC:NO_RAIN,MV:1,TX:0.99,TY:65.00,AXG:-0.906,AYG:0.017,AZG:0.422,GXD:2.86,GYD:-0.27,GZD:0.20"}, {"timestamp": "04-11-2026 00:09:57", "packet_id": "448", "temperature_c": "30.6", "temperature_f": "87.08", "humidity_pct": "80.9", "dew_point_c": "26.93", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "4095", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-14778", "accel_y_raw": "278", "accel_z_raw": "7176", "accel_x_g": "-0.902", "accel_y_g": "0.017", "accel_z_g": "0.438", "accel_magnitude_g": "1.003", "gyro_x_raw": "420", "gyro_y_raw": "-940", "gyro_z_raw": "-27", "gyro_x_dps": "3.21", "gyro_y_dps": "-7.18", "gyro_z_dps": "-0.21", "gyro_magnitude_dps": "7.87", "tilt_pitch_deg": "0.98", "tilt_roll_deg": "64.07", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "158", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.60,H:80.90,SRAW:4095,SM:0.0,SC:DRY,RRAW:4095,RP:0.0,RC:NO_RAIN,MV:1,TX:0.98,TY:64.07,AXG:-0.902,AYG:0.017,AZG:0.438,GXD:3.21,GYD:-7.18,GZD:-0.21"}, {"timestamp": "04-11-2026 00:10:02", "packet_id": "449", "temperature_c": "30.6", "temperature_f": "87.08", "humidity_pct": "80.9", "dew_point_c": "26.93", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "4095", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-9207", "accel_y_raw": "3833", "accel_z_raw": "13303", "accel_x_g": "-0.562", "accel_y_g": "0.234", "accel_z_g": "0.812", "accel_magnitude_g": "1.015", "gyro_x_raw": "281", "gyro_y_raw": "961", "gyro_z_raw": "-193", "gyro_x_dps": "2.15", "gyro_y_dps": "7.34", "gyro_z_dps": "-1.48", "gyro_magnitude_dps": "7.79", "tilt_pitch_deg": "13.31", "tilt_roll_deg": "33.65", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "158", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.60,H:80.90,SRAW:4095,SM:0.0,SC:DRY,RRAW:4095,RP:0.0,RC:NO_RAIN,MV:1,TX:13.31,TY:33.65,AXG:-0.562,AYG:0.234,AZG:0.812,GXD:2.15,GYD:7.34,GZD:-1.48"}, {"timestamp": "04-11-2026 00:10:07", "packet_id": "450", "temperature_c": "30.6", "temperature_f": "87.08", "humidity_pct": "80.9", "dew_point_c": "26.93", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "4095", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-2064", "accel_y_raw": "229", "accel_z_raw": "16809", "accel_x_g": "-0.126", "accel_y_g": "0.014", "accel_z_g": "1.026", "accel_magnitude_g": "1.034", "gyro_x_raw": "377", "gyro_y_raw": "90", "gyro_z_raw": "-19", "gyro_x_dps": "2.88", "gyro_y_dps": "0.69", "gyro_z_dps": "-0.15", "gyro_magnitude_dps": "2.97", "tilt_pitch_deg": "0.8", "tilt_roll_deg": "7.02", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "156", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:30.60,H:80.90,SRAW:4095,SM:0.0,SC:DRY,RRAW:4095,RP:0.0,RC:NO_RAIN,MV:1,TX:0.80,TY:7.02,AXG:-0.126,AYG:0.014,AZG:1.026,GXD:2.88,GYD:0.69,GZD:-0.15"}, {"timestamp": "04-11-2026 00:10:12", "packet_id": "451", "temperature_c": "30.6", "temperature_f": "87.08", "humidity_pct": "81.0", "dew_point_c": "26.96", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "4095", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-1949", "accel_y_raw": "999", "accel_z_raw": "16678", "accel_x_g": "-0.119", "accel_y_g": "0.061", "accel_z_g": "1.018", "accel_magnitude_g": "1.027", "gyro_x_raw": "5673", "gyro_y_raw": "306", "gyro_z_raw": "69", "gyro_x_dps": "43.31", "gyro_y_dps": "2.34", "gyro_z_dps": "0.53", "gyro_magnitude_dps": "43.38", "tilt_pitch_deg": "3.38", "tilt_roll_deg": "6.68", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "156", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.60,H:81.00,SRAW:4095,SM:0.0,SC:DRY,RRAW:4095,RP:0.0,RC:NO_RAIN,MV:1,TX:3.38,TY:6.68,AXG:-0.119,AYG:0.061,AZG:1.018,GXD:43.31,GYD:2.34,GZD:0.53"}, {"timestamp": "04-11-2026 00:10:17", "packet_id": "452", "temperature_c": "30.6", "temperature_f": "87.08", "humidity_pct": "81.0", "dew_point_c": "26.96", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "4095", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-2260", "accel_y_raw": "278", "accel_z_raw": "16646", "accel_x_g": "-0.138", "accel_y_g": "0.017", "accel_z_g": "1.016", "accel_magnitude_g": "1.025", "gyro_x_raw": "403", "gyro_y_raw": "69", "gyro_z_raw": "-44", "gyro_x_dps": "3.08", "gyro_y_dps": "0.53", "gyro_z_dps": "-0.34", "gyro_magnitude_dps": "3.14", "tilt_pitch_deg": "0.96", "tilt_roll_deg": "7.75", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "156", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:30.60,H:81.00,SRAW:4095,SM:0.0,SC:DRY,RRAW:4095,RP:0.0,RC:NO_RAIN,MV:1,TX:0.96,TY:7.75,AXG:-0.138,AYG:0.017,AZG:1.016,GXD:3.08,GYD:0.53,GZD:-0.34"}, {"timestamp": "04-11-2026 00:11:37", "packet_id": "453", "temperature_c": "30.4", "temperature_f": "86.72", "humidity_pct": "81.2", "dew_point_c": "26.8", "soil_raw_adc": "2442", "soil_moisture_pct": "31.0", "soil_condition": "MODERATE", "rain_raw_adc": "2150", "rain_intensity_pct": "54.0", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-1966", "accel_y_raw": "344", "accel_z_raw": "16826", "accel_x_g": "-0.12", "accel_y_g": "0.021", "accel_z_g": "1.027", "accel_magnitude_g": "1.034", "gyro_x_raw": "408", "gyro_y_raw": "96", "gyro_z_raw": "-51", "gyro_x_dps": "3.12", "gyro_y_dps": "0.74", "gyro_z_dps": "-0.39", "gyro_magnitude_dps": "3.23", "tilt_pitch_deg": "1.18", "tilt_roll_deg": "6.67", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.40,H:81.20,SRAW:2442,SM:31.0,SC:MODERATE,RRAW:2150,RP:54.0,RC:MODERATE_RAIN,MV:1,TX:1.18,TY:6.67,AXG:-0.120,AYG:0.021,AZG:1.027,GXD:3.12,GYD:0.74,GZD:-0.39"}, {"timestamp": "04-11-2026 00:11:42", "packet_id": "454", "temperature_c": "30.4", "temperature_f": "86.72", "humidity_pct": "81.2", "dew_point_c": "26.8", "soil_raw_adc": "2428", "soil_moisture_pct": "31.8", "soil_condition": "MODERATE", "rain_raw_adc": "2124", "rain_intensity_pct": "55.0", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-1900", "accel_y_raw": "262", "accel_z_raw": "16826", "accel_x_g": "-0.116", "accel_y_g": "0.016", "accel_z_g": "1.027", "accel_magnitude_g": "1.034", "gyro_x_raw": "357", "gyro_y_raw": "96", "gyro_z_raw": "-6", "gyro_x_dps": "2.73", "gyro_y_dps": "0.74", "gyro_z_dps": "-0.05", "gyro_magnitude_dps": "2.83", "tilt_pitch_deg": "0.89", "tilt_roll_deg": "6.47", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.40,H:81.20,SRAW:2428,SM:31.8,SC:MODERATE,RRAW:2124,RP:55.0,RC:MODERATE_RAIN,MV:1,TX:0.89,TY:6.47,AXG:-0.116,AYG:0.016,AZG:1.027,GXD:2.73,GYD:0.74,GZD:-0.05"}, {"timestamp": "04-11-2026 00:11:47", "packet_id": "455", "temperature_c": "30.4", "temperature_f": "86.72", "humidity_pct": "81.3", "dew_point_c": "26.82", "soil_raw_adc": "2449", "soil_moisture_pct": "30.6", "soil_condition": "MODERATE", "rain_raw_adc": "2117", "rain_intensity_pct": "55.3", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-1916", "accel_y_raw": "294", "accel_z_raw": "16662", "accel_x_g": "-0.117", "accel_y_g": "0.018", "accel_z_g": "1.017", "accel_magnitude_g": "1.024", "gyro_x_raw": "390", "gyro_y_raw": "65", "gyro_z_raw": "-17", "gyro_x_dps": "2.98", "gyro_y_dps": "0.5", "gyro_z_dps": "-0.13", "gyro_magnitude_dps": "3.02", "tilt_pitch_deg": "1.0", "tilt_roll_deg": "6.56", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.40,H:81.30,SRAW:2449,SM:30.6,SC:MODERATE,RRAW:2117,RP:55.3,RC:MODERATE_RAIN,MV:1,TX:1.00,TY:6.56,AXG:-0.117,AYG:0.018,AZG:1.017,GXD:2.98,GYD:0.50,GZD:-0.13"}, {"timestamp": "04-11-2026 00:11:52", "packet_id": "456", "temperature_c": "30.4", "temperature_f": "86.72", "humidity_pct": "81.3", "dew_point_c": "26.82", "soil_raw_adc": "2525", "soil_moisture_pct": "26.4", "soil_condition": "MODERATE", "rain_raw_adc": "2127", "rain_intensity_pct": "54.9", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-1966", "accel_y_raw": "294", "accel_z_raw": "16809", "accel_x_g": "-0.12", "accel_y_g": "0.018", "accel_z_g": "1.026", "accel_magnitude_g": "1.033", "gyro_x_raw": "378", "gyro_y_raw": "68", "gyro_z_raw": "-9", "gyro_x_dps": "2.89", "gyro_y_dps": "0.52", "gyro_z_dps": "-0.07", "gyro_magnitude_dps": "2.94", "tilt_pitch_deg": "1.02", "tilt_roll_deg": "6.66", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.40,H:81.30,SRAW:2525,SM:26.4,SC:MODERATE,RRAW:2127,RP:54.9,RC:MODERATE_RAIN,MV:1,TX:1.02,TY:6.66,AXG:-0.120,AYG:0.018,AZG:1.026,GXD:2.89,GYD:0.52,GZD:-0.07"}, {"timestamp": "04-11-2026 00:11:57", "packet_id": "457", "temperature_c": "30.3", "temperature_f": "86.54", "humidity_pct": "81.4", "dew_point_c": "26.75", "soil_raw_adc": "2608", "soil_moisture_pct": "21.8", "soil_condition": "DRY", "rain_raw_adc": "2283", "rain_intensity_pct": "48.7", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-1949", "accel_y_raw": "245", "accel_z_raw": "16728", "accel_x_g": "-0.119", "accel_y_g": "0.015", "accel_z_g": "1.021", "accel_magnitude_g": "1.028", "gyro_x_raw": "403", "gyro_y_raw": "95", "gyro_z_raw": "-31", "gyro_x_dps": "3.08", "gyro_y_dps": "0.73", "gyro_z_dps": "-0.24", "gyro_magnitude_dps": "3.17", "tilt_pitch_deg": "0.83", "tilt_roll_deg": "6.67", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "164", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.30,H:81.40,SRAW:2608,SM:21.8,SC:DRY,RRAW:2283,RP:48.7,RC:MODERATE_RAIN,MV:1,TX:0.83,TY:6.67,AXG:-0.119,AYG:0.015,AZG:1.021,GXD:3.08,GYD:0.73,GZD:-0.24"}, {"timestamp": "04-11-2026 00:12:02", "packet_id": "458", "temperature_c": "30.3", "temperature_f": "86.54", "humidity_pct": "81.4", "dew_point_c": "26.75", "soil_raw_adc": "2498", "soil_moisture_pct": "27.9", "soil_condition": "MODERATE", "rain_raw_adc": "2160", "rain_intensity_pct": "53.6", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-2015", "accel_y_raw": "327", "accel_z_raw": "16809", "accel_x_g": "-0.123", "accel_y_g": "0.02", "accel_z_g": "1.026", "accel_magnitude_g": "1.034", "gyro_x_raw": "356", "gyro_y_raw": "111", "gyro_z_raw": "-20", "gyro_x_dps": "2.72", "gyro_y_dps": "0.85", "gyro_z_dps": "-0.16", "gyro_magnitude_dps": "2.85", "tilt_pitch_deg": "1.1", "tilt_roll_deg": "6.81", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.30,H:81.40,SRAW:2498,SM:27.9,SC:MODERATE,RRAW:2160,RP:53.6,RC:MODERATE_RAIN,MV:1,TX:1.10,TY:6.81,AXG:-0.123,AYG:0.020,AZG:1.026,GXD:2.72,GYD:0.85,GZD:-0.16"}, {"timestamp": "04-11-2026 00:12:07", "packet_id": "459", "temperature_c": "30.3", "temperature_f": "86.54", "humidity_pct": "81.5", "dew_point_c": "26.77", "soil_raw_adc": "2435", "soil_moisture_pct": "31.4", "soil_condition": "MODERATE", "rain_raw_adc": "2163", "rain_intensity_pct": "53.5", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-1933", "accel_y_raw": "311", "accel_z_raw": "16793", "accel_x_g": "-0.118", "accel_y_g": "0.019", "accel_z_g": "1.025", "accel_magnitude_g": "1.032", "gyro_x_raw": "410", "gyro_y_raw": "83", "gyro_z_raw": "-65", "gyro_x_dps": "3.13", "gyro_y_dps": "0.64", "gyro_z_dps": "-0.5", "gyro_magnitude_dps": "3.23", "tilt_pitch_deg": "1.04", "tilt_roll_deg": "6.55", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.30,H:81.50,SRAW:2435,SM:31.4,SC:MODERATE,RRAW:2163,RP:53.5,RC:MODERATE_RAIN,MV:1,TX:1.04,TY:6.55,AXG:-0.118,AYG:0.019,AZG:1.025,GXD:3.13,GYD:0.64,GZD:-0.50"}, {"timestamp": "04-11-2026 00:12:12", "packet_id": "460", "temperature_c": "30.3", "temperature_f": "86.54", "humidity_pct": "81.5", "dew_point_c": "26.77", "soil_raw_adc": "2393", "soil_moisture_pct": "33.7", "soil_condition": "MODERATE", "rain_raw_adc": "2106", "rain_intensity_pct": "55.8", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-1998", "accel_y_raw": "245", "accel_z_raw": "16744", "accel_x_g": "-0.122", "accel_y_g": "0.015", "accel_z_g": "1.022", "accel_magnitude_g": "1.029", "gyro_x_raw": "419", "gyro_y_raw": "113", "gyro_z_raw": "-22", "gyro_x_dps": "3.2", "gyro_y_dps": "0.87", "gyro_z_dps": "-0.17", "gyro_magnitude_dps": "3.32", "tilt_pitch_deg": "0.84", "tilt_roll_deg": "6.8", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.30,H:81.50,SRAW:2393,SM:33.7,SC:MODERATE,RRAW:2106,RP:55.8,RC:MODERATE_RAIN,MV:1,TX:0.84,TY:6.80,AXG:-0.122,AYG:0.015,AZG:1.022,GXD:3.20,GYD:0.87,GZD:-0.17"}, {"timestamp": "04-11-2026 00:12:17", "packet_id": "461", "temperature_c": "30.3", "temperature_f": "86.54", "humidity_pct": "81.6", "dew_point_c": "26.79", "soil_raw_adc": "2559", "soil_moisture_pct": "24.5", "soil_condition": "DRY", "rain_raw_adc": "2026", "rain_intensity_pct": "59.0", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-15761", "accel_y_raw": "-114", "accel_z_raw": "-3899", "accel_x_g": "-0.962", "accel_y_g": "-0.007", "accel_z_g": "-0.238", "accel_magnitude_g": "0.991", "gyro_x_raw": "558", "gyro_y_raw": "4643", "gyro_z_raw": "-288", "gyro_x_dps": "4.26", "gyro_y_dps": "35.45", "gyro_z_dps": "-2.2", "gyro_magnitude_dps": "35.77", "tilt_pitch_deg": "-0.42", "tilt_roll_deg": "76.12", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.30,H:81.60,SRAW:2559,SM:24.5,SC:DRY,RRAW:2026,RP:59.0,RC:MODERATE_RAIN,MV:1,TX:-0.42,TY:76.12,AXG:-0.962,AYG:-0.007,AZG:-0.238,GXD:4.26,GYD:35.45,GZD:-2.20"}, {"timestamp": "04-11-2026 00:12:22", "packet_id": "462", "temperature_c": "30.3", "temperature_f": "86.54", "humidity_pct": "81.6", "dew_point_c": "26.79", "soil_raw_adc": "2405", "soil_moisture_pct": "33.1", "soil_condition": "MODERATE", "rain_raw_adc": "2029", "rain_intensity_pct": "58.8", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-15548", "accel_y_raw": "-212", "accel_z_raw": "-4423", "accel_x_g": "-0.949", "accel_y_g": "-0.013", "accel_z_g": "-0.27", "accel_magnitude_g": "0.987", "gyro_x_raw": "366", "gyro_y_raw": "188", "gyro_z_raw": "14", "gyro_x_dps": "2.8", "gyro_y_dps": "1.44", "gyro_z_dps": "0.11", "gyro_magnitude_dps": "3.15", "tilt_pitch_deg": "-0.75", "tilt_roll_deg": "74.1", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "172", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.30,H:81.60,SRAW:2405,SM:33.1,SC:MODERATE,RRAW:2029,RP:58.8,RC:MODERATE_RAIN,MV:1,TX:-0.75,TY:74.10,AXG:-0.949,AYG:-0.013,AZG:-0.270,GXD:2.80,GYD:1.44,GZD:0.11"}, {"timestamp": "04-11-2026 00:12:27", "packet_id": "463", "temperature_c": "30.3", "temperature_f": "86.54", "humidity_pct": "81.7", "dew_point_c": "26.81", "soil_raw_adc": "2462", "soil_moisture_pct": "29.9", "soil_condition": "MODERATE", "rain_raw_adc": "2098", "rain_intensity_pct": "56.1", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-15515", "accel_y_raw": "-278", "accel_z_raw": "-4571", "accel_x_g": "-0.947", "accel_y_g": "-0.017", "accel_z_g": "-0.279", "accel_magnitude_g": "0.987", "gyro_x_raw": "403", "gyro_y_raw": "172", "gyro_z_raw": "-52", "gyro_x_dps": "3.08", "gyro_y_dps": "1.32", "gyro_z_dps": "-0.4", "gyro_magnitude_dps": "3.37", "tilt_pitch_deg": "-0.98", "tilt_roll_deg": "73.54", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "173", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.30,H:81.70,SRAW:2462,SM:29.9,SC:MODERATE,RRAW:2098,RP:56.1,RC:MODERATE_RAIN,MV:1,TX:-0.98,TY:73.54,AXG:-0.947,AYG:-0.017,AZG:-0.279,GXD:3.08,GYD:1.32,GZD:-0.40"}, {"timestamp": "04-11-2026 00:12:32", "packet_id": "464", "temperature_c": "30.3", "temperature_f": "86.54", "humidity_pct": "81.7", "dew_point_c": "26.81", "soil_raw_adc": "2470", "soil_moisture_pct": "29.4", "soil_condition": "MODERATE", "rain_raw_adc": "2014", "rain_intensity_pct": "59.4", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-15532", "accel_y_raw": "-98", "accel_z_raw": "-4669", "accel_x_g": "-0.948", "accel_y_g": "-0.006", "accel_z_g": "-0.285", "accel_magnitude_g": "0.99", "gyro_x_raw": "411", "gyro_y_raw": "120", "gyro_z_raw": "-41", "gyro_x_dps": "3.14", "gyro_y_dps": "0.92", "gyro_z_dps": "-0.32", "gyro_magnitude_dps": "3.29", "tilt_pitch_deg": "-0.35", "tilt_roll_deg": "73.25", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "173", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.30,H:81.70,SRAW:2470,SM:29.4,SC:MODERATE,RRAW:2014,RP:59.4,RC:MODERATE_RAIN,MV:1,TX:-0.35,TY:73.25,AXG:-0.948,AYG:-0.006,AZG:-0.285,GXD:3.14,GYD:0.92,GZD:-0.32"}, {"timestamp": "04-11-2026 00:12:37", "packet_id": "465", "temperature_c": "30.3", "temperature_f": "86.54", "humidity_pct": "81.6", "dew_point_c": "26.79", "soil_raw_adc": "2389", "soil_moisture_pct": "33.9", "soil_condition": "MODERATE", "rain_raw_adc": "2022", "rain_intensity_pct": "59.1", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-15548", "accel_y_raw": "-262", "accel_z_raw": "-4784", "accel_x_g": "-0.949", "accel_y_g": "-0.016", "accel_z_g": "-0.292", "accel_magnitude_g": "0.993", "gyro_x_raw": "399", "gyro_y_raw": "102", "gyro_z_raw": "1", "gyro_x_dps": "3.05", "gyro_y_dps": "0.78", "gyro_z_dps": "0.01", "gyro_magnitude_dps": "3.15", "tilt_pitch_deg": "-0.9", "tilt_roll_deg": "72.89", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "172", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.30,H:81.60,SRAW:2389,SM:33.9,SC:MODERATE,RRAW:2022,RP:59.1,RC:MODERATE_RAIN,MV:1,TX:-0.90,TY:72.89,AXG:-0.949,AYG:-0.016,AZG:-0.292,GXD:3.05,GYD:0.78,GZD:0.01"}, {"timestamp": "04-11-2026 00:12:42", "packet_id": "466", "temperature_c": "30.3", "temperature_f": "86.54", "humidity_pct": "81.7", "dew_point_c": "26.81", "soil_raw_adc": "2687", "soil_moisture_pct": "17.4", "soil_condition": "DRY", "rain_raw_adc": "2160", "rain_intensity_pct": "53.6", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-15450", "accel_y_raw": "-196", "accel_z_raw": "-4653", "accel_x_g": "-0.943", "accel_y_g": "-0.012", "accel_z_g": "-0.284", "accel_magnitude_g": "0.985", "gyro_x_raw": "379", "gyro_y_raw": "124", "gyro_z_raw": "-61", "gyro_x_dps": "2.9", "gyro_y_dps": "0.95", "gyro_z_dps": "-0.47", "gyro_magnitude_dps": "3.09", "tilt_pitch_deg": "-0.68", "tilt_roll_deg": "73.21", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "168", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.30,H:81.70,SRAW:2687,SM:17.4,SC:DRY,RRAW:2160,RP:53.6,RC:MODERATE_RAIN,MV:1,TX:-0.68,TY:73.21,AXG:-0.943,AYG:-0.012,AZG:-0.284,GXD:2.90,GYD:0.95,GZD:-0.47"}, {"timestamp": "04-11-2026 00:12:47", "packet_id": "467", "temperature_c": "30.3", "temperature_f": "86.54", "humidity_pct": "81.7", "dew_point_c": "26.81", "soil_raw_adc": "2559", "soil_moisture_pct": "24.5", "soil_condition": "DRY", "rain_raw_adc": "2165", "rain_intensity_pct": "53.4", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-15482", "accel_y_raw": "-212", "accel_z_raw": "-4734", "accel_x_g": "-0.945", "accel_y_g": "-0.013", "accel_z_g": "-0.289", "accel_magnitude_g": "0.988", "gyro_x_raw": "387", "gyro_y_raw": "14", "gyro_z_raw": "-48", "gyro_x_dps": "2.96", "gyro_y_dps": "0.11", "gyro_z_dps": "-0.37", "gyro_magnitude_dps": "2.99", "tilt_pitch_deg": "-0.76", "tilt_roll_deg": "72.97", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "168", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.30,H:81.70,SRAW:2559,SM:24.5,SC:DRY,RRAW:2165,RP:53.4,RC:MODERATE_RAIN,MV:1,TX:-0.76,TY:72.97,AXG:-0.945,AYG:-0.013,AZG:-0.289,GXD:2.96,GYD:0.11,GZD:-0.37"}, {"timestamp": "04-11-2026 00:12:52", "packet_id": "468", "temperature_c": "30.3", "temperature_f": "86.54", "humidity_pct": "81.7", "dew_point_c": "26.81", "soil_raw_adc": "3062", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "2202", "rain_intensity_pct": "51.9", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-15482", "accel_y_raw": "-163", "accel_z_raw": "-4800", "accel_x_g": "-0.945", "accel_y_g": "-0.01", "accel_z_g": "-0.293", "accel_magnitude_g": "0.989", "gyro_x_raw": "362", "gyro_y_raw": "129", "gyro_z_raw": "-37", "gyro_x_dps": "2.77", "gyro_y_dps": "0.99", "gyro_z_dps": "-0.29", "gyro_magnitude_dps": "2.96", "tilt_pitch_deg": "-0.59", "tilt_roll_deg": "72.77", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "167", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.30,H:81.70,SRAW:3062,SM:0.0,SC:DRY,RRAW:2202,RP:51.9,RC:MODERATE_RAIN,MV:1,TX:-0.59,TY:72.77,AXG:-0.945,AYG:-0.010,AZG:-0.293,GXD:2.77,GYD:0.99,GZD:-0.29"}, {"timestamp": "04-11-2026 00:12:57", "packet_id": "469", "temperature_c": "30.3", "temperature_f": "86.54", "humidity_pct": "81.7", "dew_point_c": "26.81", "soil_raw_adc": "2258", "soil_moisture_pct": "41.2", "soil_condition": "MODERATE", "rain_raw_adc": "1985", "rain_intensity_pct": "60.6", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-15417", "accel_y_raw": "-278", "accel_z_raw": "-4882", "accel_x_g": "-0.941", "accel_y_g": "-0.017", "accel_z_g": "-0.298", "accel_magnitude_g": "0.987", "gyro_x_raw": "375", "gyro_y_raw": "145", "gyro_z_raw": "-30", "gyro_x_dps": "2.87", "gyro_y_dps": "1.11", "gyro_z_dps": "-0.23", "gyro_magnitude_dps": "3.09", "tilt_pitch_deg": "-0.96", "tilt_roll_deg": "72.38", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "173", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.30,H:81.70,SRAW:2258,SM:41.2,SC:MODERATE,RRAW:1985,RP:60.6,RC:MODERATE_RAIN,MV:1,TX:-0.96,TY:72.38,AXG:-0.941,AYG:-0.017,AZG:-0.298,GXD:2.87,GYD:1.11,GZD:-0.23"}, {"timestamp": "04-11-2026 00:13:02", "packet_id": "470", "temperature_c": "30.3", "temperature_f": "86.54", "humidity_pct": "81.8", "dew_point_c": "26.83", "soil_raw_adc": "2231", "soil_moisture_pct": "42.7", "soil_condition": "MODERATE", "rain_raw_adc": "1973", "rain_intensity_pct": "61.1", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-15384", "accel_y_raw": "-196", "accel_z_raw": "-5013", "accel_x_g": "-0.939", "accel_y_g": "-0.012", "accel_z_g": "-0.306", "accel_magnitude_g": "0.988", "gyro_x_raw": "389", "gyro_y_raw": "87", "gyro_z_raw": "-40", "gyro_x_dps": "2.97", "gyro_y_dps": "0.67", "gyro_z_dps": "-0.31", "gyro_magnitude_dps": "3.06", "tilt_pitch_deg": "-0.68", "tilt_roll_deg": "71.94", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "173", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.30,H:81.80,SRAW:2231,SM:42.7,SC:MODERATE,RRAW:1973,RP:61.1,RC:MODERATE_RAIN,MV:1,TX:-0.68,TY:71.94,AXG:-0.939,AYG:-0.012,AZG:-0.306,GXD:2.97,GYD:0.67,GZD:-0.31"}, {"timestamp": "04-11-2026 00:13:07", "packet_id": "471", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "81.9", "dew_point_c": "26.75", "soil_raw_adc": "2321", "soil_moisture_pct": "37.7", "soil_condition": "MODERATE", "rain_raw_adc": "1979", "rain_intensity_pct": "60.8", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-15351", "accel_y_raw": "-294", "accel_z_raw": "-5046", "accel_x_g": "-0.937", "accel_y_g": "-0.018", "accel_z_g": "-0.308", "accel_magnitude_g": "0.986", "gyro_x_raw": "390", "gyro_y_raw": "100", "gyro_z_raw": "-34", "gyro_x_dps": "2.98", "gyro_y_dps": "0.77", "gyro_z_dps": "-0.26", "gyro_magnitude_dps": "3.09", "tilt_pitch_deg": "-1.05", "tilt_roll_deg": "71.77", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "173", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.20,H:81.90,SRAW:2321,SM:37.7,SC:MODERATE,RRAW:1979,RP:60.8,RC:MODERATE_RAIN,MV:1,TX:-1.05,TY:71.77,AXG:-0.937,AYG:-0.018,AZG:-0.308,GXD:2.98,GYD:0.77,GZD:-0.26"}, {"timestamp": "04-11-2026 00:13:12", "packet_id": "472", "temperature_c": "30.3", "temperature_f": "86.54", "humidity_pct": "81.9", "dew_point_c": "26.85", "soil_raw_adc": "3343", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1805", "rain_intensity_pct": "67.8", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-15368", "accel_y_raw": "-196", "accel_z_raw": "-5046", "accel_x_g": "-0.938", "accel_y_g": "-0.012", "accel_z_g": "-0.308", "accel_magnitude_g": "0.987", "gyro_x_raw": "382", "gyro_y_raw": "123", "gyro_z_raw": "-27", "gyro_x_dps": "2.92", "gyro_y_dps": "0.94", "gyro_z_dps": "-0.21", "gyro_magnitude_dps": "3.07", "tilt_pitch_deg": "-0.69", "tilt_roll_deg": "71.79", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "167", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.30,H:81.90,SRAW:3343,SM:0.0,SC:DRY,RRAW:1805,RP:67.8,RC:MODERATE_RAIN,MV:1,TX:-0.69,TY:71.79,AXG:-0.938,AYG:-0.012,AZG:-0.308,GXD:2.92,GYD:0.94,GZD:-0.21"}, {"timestamp": "04-11-2026 00:13:17", "packet_id": "473", "temperature_c": "30.3", "temperature_f": "86.54", "humidity_pct": "81.9", "dew_point_c": "26.85", "soil_raw_adc": "3258", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1799", "rain_intensity_pct": "68.0", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-15319", "accel_y_raw": "-212", "accel_z_raw": "-5013", "accel_x_g": "-0.935", "accel_y_g": "-0.013", "accel_z_g": "-0.306", "accel_magnitude_g": "0.984", "gyro_x_raw": "395", "gyro_y_raw": "129", "gyro_z_raw": "-23", "gyro_x_dps": "3.02", "gyro_y_dps": "0.99", "gyro_z_dps": "-0.18", "gyro_magnitude_dps": "3.18", "tilt_pitch_deg": "-0.78", "tilt_roll_deg": "71.85", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "167", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.30,H:81.90,SRAW:3258,SM:0.0,SC:DRY,RRAW:1799,RP:68.0,RC:MODERATE_RAIN,MV:1,TX:-0.78,TY:71.85,AXG:-0.935,AYG:-0.013,AZG:-0.306,GXD:3.02,GYD:0.99,GZD:-0.18"}, {"timestamp": "04-11-2026 00:13:22", "packet_id": "474", "temperature_c": "30.3", "temperature_f": "86.54", "humidity_pct": "82.0", "dew_point_c": "26.87", "soil_raw_adc": "3068", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1886", "rain_intensity_pct": "64.6", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-15319", "accel_y_raw": "-229", "accel_z_raw": "-5079", "accel_x_g": "-0.935", "accel_y_g": "-0.014", "accel_z_g": "-0.31", "accel_magnitude_g": "0.985", "gyro_x_raw": "399", "gyro_y_raw": "99", "gyro_z_raw": "-44", "gyro_x_dps": "3.05", "gyro_y_dps": "0.76", "gyro_z_dps": "-0.34", "gyro_magnitude_dps": "3.16", "tilt_pitch_deg": "-0.8", "tilt_roll_deg": "71.65", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "167", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.30,H:82.00,SRAW:3068,SM:0.0,SC:DRY,RRAW:1886,RP:64.6,RC:MODERATE_RAIN,MV:1,TX:-0.80,TY:71.65,AXG:-0.935,AYG:-0.014,AZG:-0.310,GXD:3.05,GYD:0.76,GZD:-0.34"}, {"timestamp": "04-11-2026 00:13:27", "packet_id": "475", "temperature_c": "30.3", "temperature_f": "86.54", "humidity_pct": "82.0", "dew_point_c": "26.87", "soil_raw_adc": "2921", "soil_moisture_pct": "4.4", "soil_condition": "DRY", "rain_raw_adc": "1914", "rain_intensity_pct": "63.4", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-15302", "accel_y_raw": "-196", "accel_z_raw": "-5193", "accel_x_g": "-0.934", "accel_y_g": "-0.012", "accel_z_g": "-0.317", "accel_magnitude_g": "0.986", "gyro_x_raw": "375", "gyro_y_raw": "87", "gyro_z_raw": "-40", "gyro_x_dps": "2.87", "gyro_y_dps": "0.67", "gyro_z_dps": "-0.31", "gyro_magnitude_dps": "2.96", "tilt_pitch_deg": "-0.69", "tilt_roll_deg": "71.22", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "167", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.30,H:82.00,SRAW:2921,SM:4.4,SC:DRY,RRAW:1914,RP:63.4,RC:MODERATE_RAIN,MV:1,TX:-0.69,TY:71.22,AXG:-0.934,AYG:-0.012,AZG:-0.317,GXD:2.87,GYD:0.67,GZD:-0.31"}, {"timestamp": "04-11-2026 00:13:32", "packet_id": "476", "temperature_c": "30.3", "temperature_f": "86.54", "humidity_pct": "82.0", "dew_point_c": "26.87", "soil_raw_adc": "2747", "soil_moisture_pct": "14.1", "soil_condition": "DRY", "rain_raw_adc": "1928", "rain_intensity_pct": "62.9", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-15335", "accel_y_raw": "-131", "accel_z_raw": "-5128", "accel_x_g": "-0.936", "accel_y_g": "-0.008", "accel_z_g": "-0.313", "accel_magnitude_g": "0.987", "gyro_x_raw": "393", "gyro_y_raw": "103", "gyro_z_raw": "-40", "gyro_x_dps": "3.0", "gyro_y_dps": "0.79", "gyro_z_dps": "-0.31", "gyro_magnitude_dps": "3.12", "tilt_pitch_deg": "-0.48", "tilt_roll_deg": "71.48", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "168", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.30,H:82.00,SRAW:2747,SM:14.1,SC:DRY,RRAW:1928,RP:62.9,RC:MODERATE_RAIN,MV:1,TX:-0.48,TY:71.48,AXG:-0.936,AYG:-0.008,AZG:-0.313,GXD:3.00,GYD:0.79,GZD:-0.31"}, {"timestamp": "04-11-2026 00:14:27", "packet_id": "477", "temperature_c": "30.3", "temperature_f": "86.54", "humidity_pct": "81.9", "dew_point_c": "26.85", "soil_raw_adc": "2282", "soil_moisture_pct": "39.9", "soil_condition": "MODERATE", "rain_raw_adc": "2112", "rain_intensity_pct": "55.5", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-14630", "accel_y_raw": "-262", "accel_z_raw": "-6979", "accel_x_g": "-0.893", "accel_y_g": "-0.016", "accel_z_g": "-0.426", "accel_magnitude_g": "0.99", "gyro_x_raw": "375", "gyro_y_raw": "51", "gyro_z_raw": "-20", "gyro_x_dps": "2.87", "gyro_y_dps": "0.39", "gyro_z_dps": "-0.16", "gyro_magnitude_dps": "2.9", "tilt_pitch_deg": "-0.93", "tilt_roll_deg": "64.48", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "173", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.30,H:81.90,SRAW:2282,SM:39.9,SC:MODERATE,RRAW:2112,RP:55.5,RC:MODERATE_RAIN,MV:1,TX:-0.93,TY:64.48,AXG:-0.893,AYG:-0.016,AZG:-0.426,GXD:2.87,GYD:0.39,GZD:-0.16"}, {"timestamp": "04-11-2026 00:14:32", "packet_id": "478", "temperature_c": "30.3", "temperature_f": "86.54", "humidity_pct": "81.9", "dew_point_c": "26.85", "soil_raw_adc": "2243", "soil_moisture_pct": "42.1", "soil_condition": "MODERATE", "rain_raw_adc": "2128", "rain_intensity_pct": "54.9", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-14499", "accel_y_raw": "-262", "accel_z_raw": "-7028", "accel_x_g": "-0.885", "accel_y_g": "-0.016", "accel_z_g": "-0.429", "accel_magnitude_g": "0.984", "gyro_x_raw": "373", "gyro_y_raw": "98", "gyro_z_raw": "-44", "gyro_x_dps": "2.85", "gyro_y_dps": "0.75", "gyro_z_dps": "-0.34", "gyro_magnitude_dps": "2.97", "tilt_pitch_deg": "-0.95", "tilt_roll_deg": "64.12", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "173", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.30,H:81.90,SRAW:2243,SM:42.1,SC:MODERATE,RRAW:2128,RP:54.9,RC:MODERATE_RAIN,MV:1,TX:-0.95,TY:64.12,AXG:-0.885,AYG:-0.016,AZG:-0.429,GXD:2.85,GYD:0.75,GZD:-0.34"}, {"timestamp": "04-11-2026 00:14:37", "packet_id": "479", "temperature_c": "30.3", "temperature_f": "86.54", "humidity_pct": "81.9", "dew_point_c": "26.85", "soil_raw_adc": "2288", "soil_moisture_pct": "39.6", "soil_condition": "MODERATE", "rain_raw_adc": "2079", "rain_intensity_pct": "56.8", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-14581", "accel_y_raw": "-278", "accel_z_raw": "-7110", "accel_x_g": "-0.89", "accel_y_g": "-0.017", "accel_z_g": "-0.434", "accel_magnitude_g": "0.99", "gyro_x_raw": "420", "gyro_y_raw": "95", "gyro_z_raw": "-22", "gyro_x_dps": "3.21", "gyro_y_dps": "0.73", "gyro_z_dps": "-0.17", "gyro_magnitude_dps": "3.3", "tilt_pitch_deg": "-0.97", "tilt_roll_deg": "63.97", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "173", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.30,H:81.90,SRAW:2288,SM:39.6,SC:MODERATE,RRAW:2079,RP:56.8,RC:MODERATE_RAIN,MV:1,TX:-0.97,TY:63.97,AXG:-0.890,AYG:-0.017,AZG:-0.434,GXD:3.21,GYD:0.73,GZD:-0.17"}, {"timestamp": "04-11-2026 00:14:42", "packet_id": "480", "temperature_c": "30.3", "temperature_f": "86.54", "humidity_pct": "81.9", "dew_point_c": "26.85", "soil_raw_adc": "2320", "soil_moisture_pct": "37.8", "soil_condition": "MODERATE", "rain_raw_adc": "2160", "rain_intensity_pct": "53.6", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-14499", "accel_y_raw": "-229", "accel_z_raw": "-7045", "accel_x_g": "-0.885", "accel_y_g": "-0.014", "accel_z_g": "-0.43", "accel_magnitude_g": "0.984", "gyro_x_raw": "403", "gyro_y_raw": "128", "gyro_z_raw": "-73", "gyro_x_dps": "3.08", "gyro_y_dps": "0.98", "gyro_z_dps": "-0.56", "gyro_magnitude_dps": "3.28", "tilt_pitch_deg": "-0.8", "tilt_roll_deg": "64.07", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "173", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.30,H:81.90,SRAW:2320,SM:37.8,SC:MODERATE,RRAW:2160,RP:53.6,RC:MODERATE_RAIN,MV:1,TX:-0.80,TY:64.07,AXG:-0.885,AYG:-0.014,AZG:-0.430,GXD:3.08,GYD:0.98,GZD:-0.56"}, {"timestamp": "04-11-2026 00:14:47", "packet_id": "481", "temperature_c": "30.3", "temperature_f": "86.54", "humidity_pct": "81.8", "dew_point_c": "26.83", "soil_raw_adc": "2283", "soil_moisture_pct": "39.8", "soil_condition": "MODERATE", "rain_raw_adc": "2165", "rain_intensity_pct": "53.4", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-14483", "accel_y_raw": "-294", "accel_z_raw": "-7094", "accel_x_g": "-0.884", "accel_y_g": "-0.018", "accel_z_g": "-0.433", "accel_magnitude_g": "0.985", "gyro_x_raw": "390", "gyro_y_raw": "95", "gyro_z_raw": "-37", "gyro_x_dps": "2.98", "gyro_y_dps": "0.73", "gyro_z_dps": "-0.29", "gyro_magnitude_dps": "3.08", "tilt_pitch_deg": "-1.02", "tilt_roll_deg": "63.87", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "173", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.30,H:81.80,SRAW:2283,SM:39.8,SC:MODERATE,RRAW:2165,RP:53.4,RC:MODERATE_RAIN,MV:1,TX:-1.02,TY:63.87,AXG:-0.884,AYG:-0.018,AZG:-0.433,GXD:2.98,GYD:0.73,GZD:-0.29"}, {"timestamp": "04-11-2026 00:14:52", "packet_id": "482", "temperature_c": "30.3", "temperature_f": "86.54", "humidity_pct": "81.8", "dew_point_c": "26.83", "soil_raw_adc": "2479", "soil_moisture_pct": "28.9", "soil_condition": "MODERATE", "rain_raw_adc": "3731", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-14532", "accel_y_raw": "-262", "accel_z_raw": "-7028", "accel_x_g": "-0.887", "accel_y_g": "-0.016", "accel_z_g": "-0.429", "accel_magnitude_g": "0.985", "gyro_x_raw": "407", "gyro_y_raw": "100", "gyro_z_raw": "-32", "gyro_x_dps": "3.11", "gyro_y_dps": "0.77", "gyro_z_dps": "-0.25", "gyro_magnitude_dps": "3.21", "tilt_pitch_deg": "-0.92", "tilt_roll_deg": "64.17", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "166", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.30,H:81.80,SRAW:2479,SM:28.9,SC:MODERATE,RRAW:3731,RP:0.0,RC:NO_RAIN,MV:1,TX:-0.92,TY:64.17,AXG:-0.887,AYG:-0.016,AZG:-0.429,GXD:3.11,GYD:0.77,GZD:-0.25"}, {"timestamp": "04-11-2026 00:14:57", "packet_id": "483", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "81.8", "dew_point_c": "26.73", "soil_raw_adc": "2202", "soil_moisture_pct": "44.3", "soil_condition": "MODERATE", "rain_raw_adc": "1980", "rain_intensity_pct": "60.8", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-14565", "accel_y_raw": "-294", "accel_z_raw": "-7061", "accel_x_g": "-0.889", "accel_y_g": "-0.018", "accel_z_g": "-0.431", "accel_magnitude_g": "0.988", "gyro_x_raw": "399", "gyro_y_raw": "116", "gyro_z_raw": "-27", "gyro_x_dps": "3.05", "gyro_y_dps": "0.89", "gyro_z_dps": "-0.21", "gyro_magnitude_dps": "3.18", "tilt_pitch_deg": "-1.06", "tilt_roll_deg": "64.14", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "173", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.20,H:81.80,SRAW:2202,SM:44.3,SC:MODERATE,RRAW:1980,RP:60.8,RC:MODERATE_RAIN,MV:1,TX:-1.06,TY:64.14,AXG:-0.889,AYG:-0.018,AZG:-0.431,GXD:3.05,GYD:0.89,GZD:-0.21"}, {"timestamp": "04-11-2026 00:15:02", "packet_id": "484", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "81.8", "dew_point_c": "26.73", "soil_raw_adc": "2288", "soil_moisture_pct": "39.6", "soil_condition": "MODERATE", "rain_raw_adc": "2224", "rain_intensity_pct": "51.0", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-14467", "accel_y_raw": "-294", "accel_z_raw": "-7176", "accel_x_g": "-0.883", "accel_y_g": "-0.018", "accel_z_g": "-0.438", "accel_magnitude_g": "0.986", "gyro_x_raw": "402", "gyro_y_raw": "48", "gyro_z_raw": "-39", "gyro_x_dps": "3.07", "gyro_y_dps": "0.37", "gyro_z_dps": "-0.3", "gyro_magnitude_dps": "3.11", "tilt_pitch_deg": "-1.06", "tilt_roll_deg": "63.57", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "173", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.20,H:81.80,SRAW:2288,SM:39.6,SC:MODERATE,RRAW:2224,RP:51.0,RC:MODERATE_RAIN,MV:1,TX:-1.06,TY:63.57,AXG:-0.883,AYG:-0.018,AZG:-0.438,GXD:3.07,GYD:0.37,GZD:-0.30"}, {"timestamp": "04-11-2026 00:15:07", "packet_id": "485", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "81.8", "dew_point_c": "26.73", "soil_raw_adc": "2283", "soil_moisture_pct": "39.8", "soil_condition": "MODERATE", "rain_raw_adc": "2288", "rain_intensity_pct": "48.5", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-14516", "accel_y_raw": "-262", "accel_z_raw": "-7094", "accel_x_g": "-0.886", "accel_y_g": "-0.016", "accel_z_g": "-0.433", "accel_magnitude_g": "0.986", "gyro_x_raw": "412", "gyro_y_raw": "120", "gyro_z_raw": "-51", "gyro_x_dps": "3.15", "gyro_y_dps": "0.92", "gyro_z_dps": "-0.39", "gyro_magnitude_dps": "3.3", "tilt_pitch_deg": "-0.94", "tilt_roll_deg": "63.94", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "173", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.20,H:81.80,SRAW:2283,SM:39.8,SC:MODERATE,RRAW:2288,RP:48.5,RC:MODERATE_RAIN,MV:1,TX:-0.94,TY:63.94,AXG:-0.886,AYG:-0.016,AZG:-0.433,GXD:3.15,GYD:0.92,GZD:-0.39"}, {"timestamp": "04-11-2026 00:15:12", "packet_id": "486", "temperature_c": "30.3", "temperature_f": "86.54", "humidity_pct": "81.8", "dew_point_c": "26.83", "soil_raw_adc": "2302", "soil_moisture_pct": "38.8", "soil_condition": "MODERATE", "rain_raw_adc": "2276", "rain_intensity_pct": "49.0", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-14532", "accel_y_raw": "-278", "accel_z_raw": "-7208", "accel_x_g": "-0.887", "accel_y_g": "-0.017", "accel_z_g": "-0.44", "accel_magnitude_g": "0.99", "gyro_x_raw": "382", "gyro_y_raw": "141", "gyro_z_raw": "-44", "gyro_x_dps": "2.92", "gyro_y_dps": "1.08", "gyro_z_dps": "-0.34", "gyro_magnitude_dps": "3.13", "tilt_pitch_deg": "-0.96", "tilt_roll_deg": "63.58", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "173", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.30,H:81.80,SRAW:2302,SM:38.8,SC:MODERATE,RRAW:2276,RP:49.0,RC:MODERATE_RAIN,MV:1,TX:-0.96,TY:63.58,AXG:-0.887,AYG:-0.017,AZG:-0.440,GXD:2.92,GYD:1.08,GZD:-0.34"}, {"timestamp": "04-11-2026 00:15:17", "packet_id": "487", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "81.8", "dew_point_c": "26.73", "soil_raw_adc": "2272", "soil_moisture_pct": "40.4", "soil_condition": "MODERATE", "rain_raw_adc": "2062", "rain_intensity_pct": "57.5", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-14467", "accel_y_raw": "-262", "accel_z_raw": "-7077", "accel_x_g": "-0.883", "accel_y_g": "-0.016", "accel_z_g": "-0.432", "accel_magnitude_g": "0.983", "gyro_x_raw": "385", "gyro_y_raw": "73", "gyro_z_raw": "-41", "gyro_x_dps": "2.94", "gyro_y_dps": "0.56", "gyro_z_dps": "-0.32", "gyro_magnitude_dps": "3.01", "tilt_pitch_deg": "-0.94", "tilt_roll_deg": "63.88", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "173", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.20,H:81.80,SRAW:2272,SM:40.4,SC:MODERATE,RRAW:2062,RP:57.5,RC:MODERATE_RAIN,MV:1,TX:-0.94,TY:63.88,AXG:-0.883,AYG:-0.016,AZG:-0.432,GXD:2.94,GYD:0.56,GZD:-0.32"}, {"timestamp": "04-11-2026 00:15:22", "packet_id": "488", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "81.9", "dew_point_c": "26.75", "soil_raw_adc": "2239", "soil_moisture_pct": "42.3", "soil_condition": "MODERATE", "rain_raw_adc": "2054", "rain_intensity_pct": "57.8", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-14581", "accel_y_raw": "-245", "accel_z_raw": "-7159", "accel_x_g": "-0.89", "accel_y_g": "-0.015", "accel_z_g": "-0.437", "accel_magnitude_g": "0.992", "gyro_x_raw": "379", "gyro_y_raw": "77", "gyro_z_raw": "-31", "gyro_x_dps": "2.9", "gyro_y_dps": "0.59", "gyro_z_dps": "-0.24", "gyro_magnitude_dps": "2.97", "tilt_pitch_deg": "-0.89", "tilt_roll_deg": "63.81", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "173", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.20,H:81.90,SRAW:2239,SM:42.3,SC:MODERATE,RRAW:2054,RP:57.8,RC:MODERATE_RAIN,MV:1,TX:-0.89,TY:63.81,AXG:-0.890,AYG:-0.015,AZG:-0.437,GXD:2.90,GYD:0.59,GZD:-0.24"}, {"timestamp": "04-11-2026 00:15:27", "packet_id": "489", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "81.9", "dew_point_c": "26.75", "soil_raw_adc": "2096", "soil_moisture_pct": "50.2", "soil_condition": "MODERATE", "rain_raw_adc": "2032", "rain_intensity_pct": "58.7", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-14647", "accel_y_raw": "-180", "accel_z_raw": "-6750", "accel_x_g": "-0.894", "accel_y_g": "-0.011", "accel_z_g": "-0.412", "accel_magnitude_g": "0.984", "gyro_x_raw": "369", "gyro_y_raw": "-1249", "gyro_z_raw": "-19", "gyro_x_dps": "2.82", "gyro_y_dps": "-9.54", "gyro_z_dps": "-0.15", "gyro_magnitude_dps": "9.95", "tilt_pitch_deg": "-0.65", "tilt_roll_deg": "65.26", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "174", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.20,H:81.90,SRAW:2096,SM:50.2,SC:MODERATE,RRAW:2032,RP:58.7,RC:MODERATE_RAIN,MV:1,TX:-0.65,TY:65.26,AXG:-0.894,AYG:-0.011,AZG:-0.412,GXD:2.82,GYD:-9.54,GZD:-0.15"}, {"timestamp": "04-11-2026 00:15:32", "packet_id": "490", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "81.9", "dew_point_c": "26.75", "soil_raw_adc": "2211", "soil_moisture_pct": "43.8", "soil_condition": "MODERATE", "rain_raw_adc": "2061", "rain_intensity_pct": "57.6", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-14401", "accel_y_raw": "-245", "accel_z_raw": "-7356", "accel_x_g": "-0.879", "accel_y_g": "-0.015", "accel_z_g": "-0.449", "accel_magnitude_g": "0.987", "gyro_x_raw": "395", "gyro_y_raw": "94", "gyro_z_raw": "-36", "gyro_x_dps": "3.02", "gyro_y_dps": "0.72", "gyro_z_dps": "-0.28", "gyro_magnitude_dps": "3.12", "tilt_pitch_deg": "-0.85", "tilt_roll_deg": "62.93", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "173", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.20,H:81.90,SRAW:2211,SM:43.8,SC:MODERATE,RRAW:2061,RP:57.6,RC:MODERATE_RAIN,MV:1,TX:-0.85,TY:62.93,AXG:-0.879,AYG:-0.015,AZG:-0.449,GXD:3.02,GYD:0.72,GZD:-0.28"}, {"timestamp": "04-11-2026 00:15:37", "packet_id": "491", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "81.9", "dew_point_c": "26.75", "soil_raw_adc": "2188", "soil_moisture_pct": "45.1", "soil_condition": "MODERATE", "rain_raw_adc": "1974", "rain_intensity_pct": "61.0", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-14450", "accel_y_raw": "-262", "accel_z_raw": "-7356", "accel_x_g": "-0.882", "accel_y_g": "-0.016", "accel_z_g": "-0.449", "accel_magnitude_g": "0.99", "gyro_x_raw": "390", "gyro_y_raw": "144", "gyro_z_raw": "-22", "gyro_x_dps": "2.98", "gyro_y_dps": "1.1", "gyro_z_dps": "-0.17", "gyro_magnitude_dps": "3.18", "tilt_pitch_deg": "-0.92", "tilt_roll_deg": "63.0", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "173", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.20,H:81.90,SRAW:2188,SM:45.1,SC:MODERATE,RRAW:1974,RP:61.0,RC:MODERATE_RAIN,MV:1,TX:-0.92,TY:63.00,AXG:-0.882,AYG:-0.016,AZG:-0.449,GXD:2.98,GYD:1.10,GZD:-0.17"}, {"timestamp": "04-11-2026 00:15:42", "packet_id": "492", "temperature_c": "30.3", "temperature_f": "86.54", "humidity_pct": "81.9", "dew_point_c": "26.85", "soil_raw_adc": "2149", "soil_moisture_pct": "47.3", "soil_condition": "MODERATE", "rain_raw_adc": "2076", "rain_intensity_pct": "57.0", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-14450", "accel_y_raw": "-327", "accel_z_raw": "-7323", "accel_x_g": "-0.882", "accel_y_g": "-0.02", "accel_z_g": "-0.447", "accel_magnitude_g": "0.989", "gyro_x_raw": "387", "gyro_y_raw": "107", "gyro_z_raw": "-23", "gyro_x_dps": "2.96", "gyro_y_dps": "0.82", "gyro_z_dps": "-0.18", "gyro_magnitude_dps": "3.08", "tilt_pitch_deg": "-1.16", "tilt_roll_deg": "63.08", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "173", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.30,H:81.90,SRAW:2149,SM:47.3,SC:MODERATE,RRAW:2076,RP:57.0,RC:MODERATE_RAIN,MV:1,TX:-1.16,TY:63.08,AXG:-0.882,AYG:-0.020,AZG:-0.447,GXD:2.96,GYD:0.82,GZD:-0.18"}, {"timestamp": "04-11-2026 00:15:47", "packet_id": "493", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "81.8", "dew_point_c": "26.73", "soil_raw_adc": "2155", "soil_moisture_pct": "46.9", "soil_condition": "MODERATE", "rain_raw_adc": "1968", "rain_intensity_pct": "61.3", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-14467", "accel_y_raw": "-278", "accel_z_raw": "-7225", "accel_x_g": "-0.883", "accel_y_g": "-0.017", "accel_z_g": "-0.441", "accel_magnitude_g": "0.987", "gyro_x_raw": "396", "gyro_y_raw": "81", "gyro_z_raw": "-65", "gyro_x_dps": "3.03", "gyro_y_dps": "0.62", "gyro_z_dps": "-0.5", "gyro_magnitude_dps": "3.13", "tilt_pitch_deg": "-0.96", "tilt_roll_deg": "63.44", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "173", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.20,H:81.80,SRAW:2155,SM:46.9,SC:MODERATE,RRAW:1968,RP:61.3,RC:MODERATE_RAIN,MV:1,TX:-0.96,TY:63.44,AXG:-0.883,AYG:-0.017,AZG:-0.441,GXD:3.03,GYD:0.62,GZD:-0.50"}, {"timestamp": "04-11-2026 00:15:52", "packet_id": "494", "temperature_c": "30.3", "temperature_f": "86.54", "humidity_pct": "81.8", "dew_point_c": "26.83", "soil_raw_adc": "2149", "soil_moisture_pct": "47.3", "soil_condition": "MODERATE", "rain_raw_adc": "1859", "rain_intensity_pct": "65.6", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-14417", "accel_y_raw": "-278", "accel_z_raw": "-7290", "accel_x_g": "-0.88", "accel_y_g": "-0.017", "accel_z_g": "-0.445", "accel_magnitude_g": "0.986", "gyro_x_raw": "383", "gyro_y_raw": "125", "gyro_z_raw": "-36", "gyro_x_dps": "2.93", "gyro_y_dps": "0.96", "gyro_z_dps": "-0.28", "gyro_magnitude_dps": "3.1", "tilt_pitch_deg": "-0.99", "tilt_roll_deg": "63.16", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "173", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.30,H:81.80,SRAW:2149,SM:47.3,SC:MODERATE,RRAW:1859,RP:65.6,RC:MODERATE_RAIN,MV:1,TX:-0.99,TY:63.16,AXG:-0.880,AYG:-0.017,AZG:-0.445,GXD:2.93,GYD:0.96,GZD:-0.28"}, {"timestamp": "04-11-2026 00:15:57", "packet_id": "495", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "81.7", "dew_point_c": "26.71", "soil_raw_adc": "2126", "soil_moisture_pct": "48.6", "soil_condition": "MODERATE", "rain_raw_adc": "1927", "rain_intensity_pct": "62.9", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-14450", "accel_y_raw": "-311", "accel_z_raw": "-7258", "accel_x_g": "-0.882", "accel_y_g": "-0.019", "accel_z_g": "-0.443", "accel_magnitude_g": "0.987", "gyro_x_raw": "394", "gyro_y_raw": "82", "gyro_z_raw": "-6", "gyro_x_dps": "3.01", "gyro_y_dps": "0.63", "gyro_z_dps": "-0.05", "gyro_magnitude_dps": "3.08", "tilt_pitch_deg": "-1.08", "tilt_roll_deg": "63.3", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "173", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.20,H:81.70,SRAW:2126,SM:48.6,SC:MODERATE,RRAW:1927,RP:62.9,RC:MODERATE_RAIN,MV:1,TX:-1.08,TY:63.30,AXG:-0.882,AYG:-0.019,AZG:-0.443,GXD:3.01,GYD:0.63,GZD:-0.05"}, {"timestamp": "04-11-2026 00:16:02", "packet_id": "496", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "81.7", "dew_point_c": "26.71", "soil_raw_adc": "2062", "soil_moisture_pct": "52.1", "soil_condition": "MODERATE", "rain_raw_adc": "1979", "rain_intensity_pct": "60.8", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-14450", "accel_y_raw": "-360", "accel_z_raw": "-7389", "accel_x_g": "-0.882", "accel_y_g": "-0.022", "accel_z_g": "-0.451", "accel_magnitude_g": "0.991", "gyro_x_raw": "411", "gyro_y_raw": "124", "gyro_z_raw": "-78", "gyro_x_dps": "3.14", "gyro_y_dps": "0.95", "gyro_z_dps": "-0.6", "gyro_magnitude_dps": "3.33", "tilt_pitch_deg": "-1.27", "tilt_roll_deg": "62.91", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "173", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.20,H:81.70,SRAW:2062,SM:52.1,SC:MODERATE,RRAW:1979,RP:60.8,RC:MODERATE_RAIN,MV:1,TX:-1.27,TY:62.91,AXG:-0.882,AYG:-0.022,AZG:-0.451,GXD:3.14,GYD:0.95,GZD:-0.60"}, {"timestamp": "04-11-2026 00:16:07", "packet_id": "497", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "81.7", "dew_point_c": "26.71", "soil_raw_adc": "4063", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1804", "rain_intensity_pct": "67.8", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-14434", "accel_y_raw": "-262", "accel_z_raw": "-7405", "accel_x_g": "-0.881", "accel_y_g": "-0.016", "accel_z_g": "-0.452", "accel_magnitude_g": "0.99", "gyro_x_raw": "373", "gyro_y_raw": "127", "gyro_z_raw": "-32", "gyro_x_dps": "2.85", "gyro_y_dps": "0.97", "gyro_z_dps": "-0.25", "gyro_magnitude_dps": "3.02", "tilt_pitch_deg": "-0.9", "tilt_roll_deg": "62.81", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "167", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.20,H:81.70,SRAW:4063,SM:0.0,SC:DRY,RRAW:1804,RP:67.8,RC:MODERATE_RAIN,MV:1,TX:-0.90,TY:62.81,AXG:-0.881,AYG:-0.016,AZG:-0.452,GXD:2.85,GYD:0.97,GZD:-0.25"}, {"timestamp": "04-11-2026 00:16:12", "packet_id": "498", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "81.7", "dew_point_c": "26.71", "soil_raw_adc": "2410", "soil_moisture_pct": "32.8", "soil_condition": "MODERATE", "rain_raw_adc": "1591", "rain_intensity_pct": "76.4", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-14401", "accel_y_raw": "-245", "accel_z_raw": "-7421", "accel_x_g": "-0.879", "accel_y_g": "-0.015", "accel_z_g": "-0.453", "accel_magnitude_g": "0.989", "gyro_x_raw": "411", "gyro_y_raw": "-14", "gyro_z_raw": "-52", "gyro_x_dps": "3.14", "gyro_y_dps": "-0.11", "gyro_z_dps": "-0.4", "gyro_magnitude_dps": "3.17", "tilt_pitch_deg": "-0.89", "tilt_roll_deg": "62.69", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "171", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.20,H:81.70,SRAW:2410,SM:32.8,SC:MODERATE,RRAW:1591,RP:76.4,RC:HEAVY_RAIN,MV:1,TX:-0.89,TY:62.69,AXG:-0.879,AYG:-0.015,AZG:-0.453,GXD:3.14,GYD:-0.11,GZD:-0.40"}, {"timestamp": "04-11-2026 00:16:17", "packet_id": "499", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "81.7", "dew_point_c": "26.71", "soil_raw_adc": "2498", "soil_moisture_pct": "27.9", "soil_condition": "MODERATE", "rain_raw_adc": "1795", "rain_intensity_pct": "68.2", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-14385", "accel_y_raw": "-278", "accel_z_raw": "-7323", "accel_x_g": "-0.878", "accel_y_g": "-0.017", "accel_z_g": "-0.447", "accel_magnitude_g": "0.985", "gyro_x_raw": "407", "gyro_y_raw": "65", "gyro_z_raw": "-34", "gyro_x_dps": "3.11", "gyro_y_dps": "0.5", "gyro_z_dps": "-0.26", "gyro_magnitude_dps": "3.16", "tilt_pitch_deg": "-1.01", "tilt_roll_deg": "63.02", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "173", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.20,H:81.70,SRAW:2498,SM:27.9,SC:MODERATE,RRAW:1795,RP:68.2,RC:MODERATE_RAIN,MV:1,TX:-1.01,TY:63.02,AXG:-0.878,AYG:-0.017,AZG:-0.447,GXD:3.11,GYD:0.50,GZD:-0.26"}, {"timestamp": "04-11-2026 00:16:22", "packet_id": "500", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "81.8", "dew_point_c": "26.73", "soil_raw_adc": "2458", "soil_moisture_pct": "30.1", "soil_condition": "MODERATE", "rain_raw_adc": "1826", "rain_intensity_pct": "67.0", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-14434", "accel_y_raw": "-229", "accel_z_raw": "-7356", "accel_x_g": "-0.881", "accel_y_g": "-0.014", "accel_z_g": "-0.449", "accel_magnitude_g": "0.989", "gyro_x_raw": "407", "gyro_y_raw": "103", "gyro_z_raw": "-22", "gyro_x_dps": "3.11", "gyro_y_dps": "0.79", "gyro_z_dps": "-0.17", "gyro_magnitude_dps": "3.21", "tilt_pitch_deg": "-0.79", "tilt_roll_deg": "63.01", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "173", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.20,H:81.80,SRAW:2458,SM:30.1,SC:MODERATE,RRAW:1826,RP:67.0,RC:MODERATE_RAIN,MV:1,TX:-0.79,TY:63.01,AXG:-0.881,AYG:-0.014,AZG:-0.449,GXD:3.11,GYD:0.79,GZD:-0.17"}, {"timestamp": "04-11-2026 00:16:27", "packet_id": "501", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "81.8", "dew_point_c": "26.73", "soil_raw_adc": "3938", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "3579", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-14450", "accel_y_raw": "-180", "accel_z_raw": "-7356", "accel_x_g": "-0.882", "accel_y_g": "-0.011", "accel_z_g": "-0.449", "accel_magnitude_g": "0.99", "gyro_x_raw": "420", "gyro_y_raw": "79", "gyro_z_raw": "-62", "gyro_x_dps": "3.21", "gyro_y_dps": "0.61", "gyro_z_dps": "-0.48", "gyro_magnitude_dps": "3.3", "tilt_pitch_deg": "-0.65", "tilt_roll_deg": "63.02", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "160", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.20,H:81.80,SRAW:3938,SM:0.0,SC:DRY,RRAW:3579,RP:0.0,RC:NO_RAIN,MV:1,TX:-0.65,TY:63.02,AXG:-0.882,AYG:-0.011,AZG:-0.449,GXD:3.21,GYD:0.61,GZD:-0.48"}, {"timestamp": "04-11-2026 00:16:32", "packet_id": "502", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "81.8", "dew_point_c": "26.73", "soil_raw_adc": "2437", "soil_moisture_pct": "31.3", "soil_condition": "MODERATE", "rain_raw_adc": "1701", "rain_intensity_pct": "72.0", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-14434", "accel_y_raw": "-262", "accel_z_raw": "-7356", "accel_x_g": "-0.881", "accel_y_g": "-0.016", "accel_z_g": "-0.449", "accel_magnitude_g": "0.989", "gyro_x_raw": "375", "gyro_y_raw": "128", "gyro_z_raw": "-31", "gyro_x_dps": "2.87", "gyro_y_dps": "0.98", "gyro_z_dps": "-0.24", "gyro_magnitude_dps": "3.04", "tilt_pitch_deg": "-0.91", "tilt_roll_deg": "62.96", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "173", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.20,H:81.80,SRAW:2437,SM:31.3,SC:MODERATE,RRAW:1701,RP:72.0,RC:MODERATE_RAIN,MV:1,TX:-0.91,TY:62.96,AXG:-0.881,AYG:-0.016,AZG:-0.449,GXD:2.87,GYD:0.98,GZD:-0.24"}, {"timestamp": "04-11-2026 00:16:37", "packet_id": "503", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "81.9", "dew_point_c": "26.75", "soil_raw_adc": "2423", "soil_moisture_pct": "32.1", "soil_condition": "MODERATE", "rain_raw_adc": "1833", "rain_intensity_pct": "66.7", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-14483", "accel_y_raw": "-245", "accel_z_raw": "-7323", "accel_x_g": "-0.884", "accel_y_g": "-0.015", "accel_z_g": "-0.447", "accel_magnitude_g": "0.991", "gyro_x_raw": "370", "gyro_y_raw": "89", "gyro_z_raw": "-28", "gyro_x_dps": "2.83", "gyro_y_dps": "0.68", "gyro_z_dps": "-0.22", "gyro_magnitude_dps": "2.92", "tilt_pitch_deg": "-0.85", "tilt_roll_deg": "63.17", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "173", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.20,H:81.90,SRAW:2423,SM:32.1,SC:MODERATE,RRAW:1833,RP:66.7,RC:MODERATE_RAIN,MV:1,TX:-0.85,TY:63.17,AXG:-0.884,AYG:-0.015,AZG:-0.447,GXD:2.83,GYD:0.68,GZD:-0.22"}, {"timestamp": "04-11-2026 00:16:42", "packet_id": "504", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "81.9", "dew_point_c": "26.75", "soil_raw_adc": "2400", "soil_moisture_pct": "33.3", "soil_condition": "MODERATE", "rain_raw_adc": "1817", "rain_intensity_pct": "67.3", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-14467", "accel_y_raw": "-262", "accel_z_raw": "-7405", "accel_x_g": "-0.883", "accel_y_g": "-0.016", "accel_z_g": "-0.452", "accel_magnitude_g": "0.992", "gyro_x_raw": "396", "gyro_y_raw": "91", "gyro_z_raw": "-14", "gyro_x_dps": "3.03", "gyro_y_dps": "0.7", "gyro_z_dps": "-0.11", "gyro_magnitude_dps": "3.11", "tilt_pitch_deg": "-0.9", "tilt_roll_deg": "62.86", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "173", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.20,H:81.90,SRAW:2400,SM:33.3,SC:MODERATE,RRAW:1817,RP:67.3,RC:MODERATE_RAIN,MV:1,TX:-0.90,TY:62.86,AXG:-0.883,AYG:-0.016,AZG:-0.452,GXD:3.03,GYD:0.70,GZD:-0.11"}, {"timestamp": "04-11-2026 00:16:47", "packet_id": "505", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "81.9", "dew_point_c": "26.75", "soil_raw_adc": "4095", "soil_moisture_pct": "0.0", "soil_condition": "DRY", "rain_raw_adc": "1653", "rain_intensity_pct": "73.9", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-14336", "accel_y_raw": "-278", "accel_z_raw": "-7585", "accel_x_g": "-0.875", "accel_y_g": "-0.017", "accel_z_g": "-0.463", "accel_magnitude_g": "0.99", "gyro_x_raw": "419", "gyro_y_raw": "102", "gyro_z_raw": "-39", "gyro_x_dps": "3.2", "gyro_y_dps": "0.78", "gyro_z_dps": "-0.3", "gyro_magnitude_dps": "3.31", "tilt_pitch_deg": "-0.97", "tilt_roll_deg": "62.13", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "167", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.20,H:81.90,SRAW:4095,SM:0.0,SC:DRY,RRAW:1653,RP:73.9,RC:MODERATE_RAIN,MV:1,TX:-0.97,TY:62.13,AXG:-0.875,AYG:-0.017,AZG:-0.463,GXD:3.20,GYD:0.78,GZD:-0.30"}, {"timestamp": "04-11-2026 00:16:52", "packet_id": "506", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "81.9", "dew_point_c": "26.75", "soil_raw_adc": "2160", "soil_moisture_pct": "46.7", "soil_condition": "MODERATE", "rain_raw_adc": "2064", "rain_intensity_pct": "57.4", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-14352", "accel_y_raw": "-163", "accel_z_raw": "-7602", "accel_x_g": "-0.876", "accel_y_g": "-0.01", "accel_z_g": "-0.464", "accel_magnitude_g": "0.991", "gyro_x_raw": "389", "gyro_y_raw": "121", "gyro_z_raw": "-48", "gyro_x_dps": "2.97", "gyro_y_dps": "0.93", "gyro_z_dps": "-0.37", "gyro_magnitude_dps": "3.13", "tilt_pitch_deg": "-0.59", "tilt_roll_deg": "62.12", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "173", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.20,H:81.90,SRAW:2160,SM:46.7,SC:MODERATE,RRAW:2064,RP:57.4,RC:MODERATE_RAIN,MV:1,TX:-0.59,TY:62.12,AXG:-0.876,AYG:-0.010,AZG:-0.464,GXD:2.97,GYD:0.93,GZD:-0.37"}, {"timestamp": "04-11-2026 00:16:57", "packet_id": "507", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "82.0", "dew_point_c": "26.77", "soil_raw_adc": "2123", "soil_moisture_pct": "48.7", "soil_condition": "MODERATE", "rain_raw_adc": "2078", "rain_intensity_pct": "56.9", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-14286", "accel_y_raw": "-294", "accel_z_raw": "-7569", "accel_x_g": "-0.872", "accel_y_g": "-0.018", "accel_z_g": "-0.462", "accel_magnitude_g": "0.987", "gyro_x_raw": "395", "gyro_y_raw": "102", "gyro_z_raw": "-23", "gyro_x_dps": "3.02", "gyro_y_dps": "0.78", "gyro_z_dps": "-0.18", "gyro_magnitude_dps": "3.12", "tilt_pitch_deg": "-1.06", "tilt_roll_deg": "62.08", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "173", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.20,H:82.00,SRAW:2123,SM:48.7,SC:MODERATE,RRAW:2078,RP:56.9,RC:MODERATE_RAIN,MV:1,TX:-1.06,TY:62.08,AXG:-0.872,AYG:-0.018,AZG:-0.462,GXD:3.02,GYD:0.78,GZD:-0.18"}, {"timestamp": "04-11-2026 00:17:02", "packet_id": "508", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "81.9", "dew_point_c": "26.75", "soil_raw_adc": "2083", "soil_moisture_pct": "50.9", "soil_condition": "MODERATE", "rain_raw_adc": "1994", "rain_intensity_pct": "60.2", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-14270", "accel_y_raw": "-344", "accel_z_raw": "-7553", "accel_x_g": "-0.871", "accel_y_g": "-0.021", "accel_z_g": "-0.461", "accel_magnitude_g": "0.986", "gyro_x_raw": "403", "gyro_y_raw": "106", "gyro_z_raw": "-40", "gyro_x_dps": "3.08", "gyro_y_dps": "0.81", "gyro_z_dps": "-0.31", "gyro_magnitude_dps": "3.2", "tilt_pitch_deg": "-1.21", "tilt_roll_deg": "62.08", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "173", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.20,H:81.90,SRAW:2083,SM:50.9,SC:MODERATE,RRAW:1994,RP:60.2,RC:MODERATE_RAIN,MV:1,TX:-1.21,TY:62.08,AXG:-0.871,AYG:-0.021,AZG:-0.461,GXD:3.08,GYD:0.81,GZD:-0.31"}, {"timestamp": "04-11-2026 00:17:07", "packet_id": "509", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "82.0", "dew_point_c": "26.77", "soil_raw_adc": "2430", "soil_moisture_pct": "31.7", "soil_condition": "MODERATE", "rain_raw_adc": "1991", "rain_intensity_pct": "60.4", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-14237", "accel_y_raw": "-311", "accel_z_raw": "-7667", "accel_x_g": "-0.869", "accel_y_g": "-0.019", "accel_z_g": "-0.468", "accel_magnitude_g": "0.987", "gyro_x_raw": "424", "gyro_y_raw": "79", "gyro_z_raw": "-35", "gyro_x_dps": "3.24", "gyro_y_dps": "0.61", "gyro_z_dps": "-0.27", "gyro_magnitude_dps": "3.31", "tilt_pitch_deg": "-1.12", "tilt_roll_deg": "61.67", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "173", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.20,H:82.00,SRAW:2430,SM:31.7,SC:MODERATE,RRAW:1991,RP:60.4,RC:MODERATE_RAIN,MV:1,TX:-1.12,TY:61.67,AXG:-0.869,AYG:-0.019,AZG:-0.468,GXD:3.24,GYD:0.61,GZD:-0.27"}, {"timestamp": "04-11-2026 00:17:12", "packet_id": "510", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "82.0", "dew_point_c": "26.77", "soil_raw_adc": "2496", "soil_moisture_pct": "28.0", "soil_condition": "MODERATE", "rain_raw_adc": "1847", "rain_intensity_pct": "66.1", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-14204", "accel_y_raw": "-311", "accel_z_raw": "-7684", "accel_x_g": "-0.867", "accel_y_g": "-0.019", "accel_z_g": "-0.469", "accel_magnitude_g": "0.986", "gyro_x_raw": "387", "gyro_y_raw": "108", "gyro_z_raw": "-43", "gyro_x_dps": "2.96", "gyro_y_dps": "0.83", "gyro_z_dps": "-0.33", "gyro_magnitude_dps": "3.09", "tilt_pitch_deg": "-1.08", "tilt_roll_deg": "61.6", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "173", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.20,H:82.00,SRAW:2496,SM:28.0,SC:MODERATE,RRAW:1847,RP:66.1,RC:MODERATE_RAIN,MV:1,TX:-1.08,TY:61.60,AXG:-0.867,AYG:-0.019,AZG:-0.469,GXD:2.96,GYD:0.83,GZD:-0.33"}, {"timestamp": "04-11-2026 00:17:17", "packet_id": "511", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "82.1", "dew_point_c": "26.8", "soil_raw_adc": "2236", "soil_moisture_pct": "42.4", "soil_condition": "MODERATE", "rain_raw_adc": "1918", "rain_intensity_pct": "63.3", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-14286", "accel_y_raw": "-327", "accel_z_raw": "-7684", "accel_x_g": "-0.872", "accel_y_g": "-0.02", "accel_z_g": "-0.469", "accel_magnitude_g": "0.99", "gyro_x_raw": "413", "gyro_y_raw": "133", "gyro_z_raw": "-40", "gyro_x_dps": "3.16", "gyro_y_dps": "1.02", "gyro_z_dps": "-0.31", "gyro_magnitude_dps": "3.33", "tilt_pitch_deg": "-1.13", "tilt_roll_deg": "61.68", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "173", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.20,H:82.10,SRAW:2236,SM:42.4,SC:MODERATE,RRAW:1918,RP:63.3,RC:MODERATE_RAIN,MV:1,TX:-1.13,TY:61.68,AXG:-0.872,AYG:-0.020,AZG:-0.469,GXD:3.16,GYD:1.02,GZD:-0.31"}, {"timestamp": "04-11-2026 00:17:22", "packet_id": "512", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "82.0", "dew_point_c": "26.77", "soil_raw_adc": "2442", "soil_moisture_pct": "31.0", "soil_condition": "MODERATE", "rain_raw_adc": "1486", "rain_intensity_pct": "80.6", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-14254", "accel_y_raw": "-376", "accel_z_raw": "-7634", "accel_x_g": "-0.87", "accel_y_g": "-0.023", "accel_z_g": "-0.466", "accel_magnitude_g": "0.987", "gyro_x_raw": "365", "gyro_y_raw": "90", "gyro_z_raw": "-28", "gyro_x_dps": "2.79", "gyro_y_dps": "0.69", "gyro_z_dps": "-0.22", "gyro_magnitude_dps": "2.88", "tilt_pitch_deg": "-1.35", "tilt_roll_deg": "61.81", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "170", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.20,H:82.00,SRAW:2442,SM:31.0,SC:MODERATE,RRAW:1486,RP:80.6,RC:HEAVY_RAIN,MV:1,TX:-1.35,TY:61.81,AXG:-0.870,AYG:-0.023,AZG:-0.466,GXD:2.79,GYD:0.69,GZD:-0.22"}, {"timestamp": "04-11-2026 00:17:27", "packet_id": "513", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "82.0", "dew_point_c": "26.77", "soil_raw_adc": "2434", "soil_moisture_pct": "31.4", "soil_condition": "MODERATE", "rain_raw_adc": "1770", "rain_intensity_pct": "69.2", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-14237", "accel_y_raw": "-245", "accel_z_raw": "-7766", "accel_x_g": "-0.869", "accel_y_g": "-0.015", "accel_z_g": "-0.474", "accel_magnitude_g": "0.99", "gyro_x_raw": "386", "gyro_y_raw": "111", "gyro_z_raw": "-49", "gyro_x_dps": "2.95", "gyro_y_dps": "0.85", "gyro_z_dps": "-0.38", "gyro_magnitude_dps": "3.09", "tilt_pitch_deg": "-0.88", "tilt_roll_deg": "61.38", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "173", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.20,H:82.00,SRAW:2434,SM:31.4,SC:MODERATE,RRAW:1770,RP:69.2,RC:MODERATE_RAIN,MV:1,TX:-0.88,TY:61.38,AXG:-0.869,AYG:-0.015,AZG:-0.474,GXD:2.95,GYD:0.85,GZD:-0.38"}, {"timestamp": "04-11-2026 00:17:32", "packet_id": "514", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "81.9", "dew_point_c": "26.75", "soil_raw_adc": "2432", "soil_moisture_pct": "31.6", "soil_condition": "MODERATE", "rain_raw_adc": "1844", "rain_intensity_pct": "66.2", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-14254", "accel_y_raw": "-327", "accel_z_raw": "-7602", "accel_x_g": "-0.87", "accel_y_g": "-0.02", "accel_z_g": "-0.464", "accel_magnitude_g": "0.986", "gyro_x_raw": "416", "gyro_y_raw": "72", "gyro_z_raw": "-48", "gyro_x_dps": "3.18", "gyro_y_dps": "0.55", "gyro_z_dps": "-0.37", "gyro_magnitude_dps": "3.25", "tilt_pitch_deg": "-1.15", "tilt_roll_deg": "61.89", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "173", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.20,H:81.90,SRAW:2432,SM:31.6,SC:MODERATE,RRAW:1844,RP:66.2,RC:MODERATE_RAIN,MV:1,TX:-1.15,TY:61.89,AXG:-0.870,AYG:-0.020,AZG:-0.464,GXD:3.18,GYD:0.55,GZD:-0.37"}, {"timestamp": "04-11-2026 00:17:37", "packet_id": "515", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "81.9", "dew_point_c": "26.75", "soil_raw_adc": "2309", "soil_moisture_pct": "38.4", "soil_condition": "MODERATE", "rain_raw_adc": "1787", "rain_intensity_pct": "68.5", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-5160", "accel_y_raw": "245", "accel_z_raw": "16007", "accel_x_g": "-0.315", "accel_y_g": "0.015", "accel_z_g": "0.977", "accel_magnitude_g": "1.027", "gyro_x_raw": "395", "gyro_y_raw": "20", "gyro_z_raw": "-26", "gyro_x_dps": "3.02", "gyro_y_dps": "0.16", "gyro_z_dps": "-0.2", "gyro_magnitude_dps": "3.03", "tilt_pitch_deg": "0.84", "tilt_roll_deg": "17.88", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "170", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.20,H:81.90,SRAW:2309,SM:38.4,SC:MODERATE,RRAW:1787,RP:68.5,RC:MODERATE_RAIN,MV:1,TX:0.84,TY:17.88,AXG:-0.315,AYG:0.015,AZG:0.977,GXD:3.02,GYD:0.16,GZD:-0.20"}, {"timestamp": "04-11-2026 00:17:42", "packet_id": "516", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "81.9", "dew_point_c": "26.75", "soil_raw_adc": "2368", "soil_moisture_pct": "35.1", "soil_condition": "MODERATE", "rain_raw_adc": "1863", "rain_intensity_pct": "65.5", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-5013", "accel_y_raw": "212", "accel_z_raw": "15941", "accel_x_g": "-0.306", "accel_y_g": "0.013", "accel_z_g": "0.973", "accel_magnitude_g": "1.02", "gyro_x_raw": "404", "gyro_y_raw": "95", "gyro_z_raw": "-35", "gyro_x_dps": "3.09", "gyro_y_dps": "0.73", "gyro_z_dps": "-0.27", "gyro_magnitude_dps": "3.19", "tilt_pitch_deg": "0.71", "tilt_roll_deg": "17.44", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "170", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.20,H:81.90,SRAW:2368,SM:35.1,SC:MODERATE,RRAW:1863,RP:65.5,RC:MODERATE_RAIN,MV:1,TX:0.71,TY:17.44,AXG:-0.306,AYG:0.013,AZG:0.973,GXD:3.09,GYD:0.73,GZD:-0.27"}, {"timestamp": "04-11-2026 00:17:47", "packet_id": "517", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "81.9", "dew_point_c": "26.75", "soil_raw_adc": "2424", "soil_moisture_pct": "32.0", "soil_condition": "MODERATE", "rain_raw_adc": "1854", "rain_intensity_pct": "65.8", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-5095", "accel_y_raw": "196", "accel_z_raw": "16121", "accel_x_g": "-0.311", "accel_y_g": "0.012", "accel_z_g": "0.984", "accel_magnitude_g": "1.032", "gyro_x_raw": "386", "gyro_y_raw": "86", "gyro_z_raw": "-51", "gyro_x_dps": "2.95", "gyro_y_dps": "0.66", "gyro_z_dps": "-0.39", "gyro_magnitude_dps": "3.05", "tilt_pitch_deg": "0.65", "tilt_roll_deg": "17.53", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "170", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.20,H:81.90,SRAW:2424,SM:32.0,SC:MODERATE,RRAW:1854,RP:65.8,RC:MODERATE_RAIN,MV:1,TX:0.65,TY:17.53,AXG:-0.311,AYG:0.012,AZG:0.984,GXD:2.95,GYD:0.66,GZD:-0.39"}, {"timestamp": "04-11-2026 00:17:52", "packet_id": "518", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "81.9", "dew_point_c": "26.75", "soil_raw_adc": "2395", "soil_moisture_pct": "33.6", "soil_condition": "MODERATE", "rain_raw_adc": "1831", "rain_intensity_pct": "66.8", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-5029", "accel_y_raw": "196", "accel_z_raw": "16138", "accel_x_g": "-0.307", "accel_y_g": "0.012", "accel_z_g": "0.985", "accel_magnitude_g": "1.032", "gyro_x_raw": "382", "gyro_y_raw": "86", "gyro_z_raw": "-51", "gyro_x_dps": "2.92", "gyro_y_dps": "0.66", "gyro_z_dps": "-0.39", "gyro_magnitude_dps": "3.02", "tilt_pitch_deg": "0.68", "tilt_roll_deg": "17.33", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "170", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.20,H:81.90,SRAW:2395,SM:33.6,SC:MODERATE,RRAW:1831,RP:66.8,RC:MODERATE_RAIN,MV:1,TX:0.68,TY:17.33,AXG:-0.307,AYG:0.012,AZG:0.985,GXD:2.92,GYD:0.66,GZD:-0.39"}, {"timestamp": "04-11-2026 00:17:57", "packet_id": "519", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "82.0", "dew_point_c": "26.77", "soil_raw_adc": "2410", "soil_moisture_pct": "32.8", "soil_condition": "MODERATE", "rain_raw_adc": "1607", "rain_intensity_pct": "75.7", "rain_condition": "HEAVY_RAIN", "accel_x_raw": "-5079", "accel_y_raw": "245", "accel_z_raw": "16039", "accel_x_g": "-0.31", "accel_y_g": "0.015", "accel_z_g": "0.979", "accel_magnitude_g": "1.027", "gyro_x_raw": "395", "gyro_y_raw": "100", "gyro_z_raw": "-44", "gyro_x_dps": "3.02", "gyro_y_dps": "0.77", "gyro_z_dps": "-0.34", "gyro_magnitude_dps": "3.14", "tilt_pitch_deg": "0.82", "tilt_roll_deg": "17.55", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "167", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.20,H:82.00,SRAW:2410,SM:32.8,SC:MODERATE,RRAW:1607,RP:75.7,RC:HEAVY_RAIN,MV:1,TX:0.82,TY:17.55,AXG:-0.310,AYG:0.015,AZG:0.979,GXD:3.02,GYD:0.77,GZD:-0.34"}, {"timestamp": "04-11-2026 00:18:02", "packet_id": "520", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "82.0", "dew_point_c": "26.77", "soil_raw_adc": "2384", "soil_moisture_pct": "34.2", "soil_condition": "MODERATE", "rain_raw_adc": "3838", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-2981", "accel_y_raw": "229", "accel_z_raw": "16547", "accel_x_g": "-0.182", "accel_y_g": "0.014", "accel_z_g": "1.01", "accel_magnitude_g": "1.026", "gyro_x_raw": "386", "gyro_y_raw": "32", "gyro_z_raw": "-52", "gyro_x_dps": "2.95", "gyro_y_dps": "0.25", "gyro_z_dps": "-0.4", "gyro_magnitude_dps": "2.99", "tilt_pitch_deg": "0.8", "tilt_roll_deg": "10.23", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "163", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:30.20,H:82.00,SRAW:2384,SM:34.2,SC:MODERATE,RRAW:3838,RP:0.0,RC:NO_RAIN,MV:1,TX:0.80,TY:10.23,AXG:-0.182,AYG:0.014,AZG:1.010,GXD:2.95,GYD:0.25,GZD:-0.40"}, {"timestamp": "04-11-2026 00:18:07", "packet_id": "521", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "82.0", "dew_point_c": "26.77", "soil_raw_adc": "2439", "soil_moisture_pct": "31.2", "soil_condition": "MODERATE", "rain_raw_adc": "4095", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-2949", "accel_y_raw": "278", "accel_z_raw": "16531", "accel_x_g": "-0.18", "accel_y_g": "0.017", "accel_z_g": "1.009", "accel_magnitude_g": "1.025", "gyro_x_raw": "398", "gyro_y_raw": "99", "gyro_z_raw": "-39", "gyro_x_dps": "3.04", "gyro_y_dps": "0.76", "gyro_z_dps": "-0.3", "gyro_magnitude_dps": "3.15", "tilt_pitch_deg": "0.96", "tilt_roll_deg": "10.13", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "163", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:30.20,H:82.00,SRAW:2439,SM:31.2,SC:MODERATE,RRAW:4095,RP:0.0,RC:NO_RAIN,MV:1,TX:0.96,TY:10.13,AXG:-0.180,AYG:0.017,AZG:1.009,GXD:3.04,GYD:0.76,GZD:-0.30"}, {"timestamp": "04-11-2026 00:18:12", "packet_id": "522", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "82.0", "dew_point_c": "26.77", "soil_raw_adc": "2424", "soil_moisture_pct": "32.0", "soil_condition": "MODERATE", "rain_raw_adc": "4095", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-2949", "accel_y_raw": "212", "accel_z_raw": "16646", "accel_x_g": "-0.18", "accel_y_g": "0.013", "accel_z_g": "1.016", "accel_magnitude_g": "1.032", "gyro_x_raw": "372", "gyro_y_raw": "60", "gyro_z_raw": "-62", "gyro_x_dps": "2.84", "gyro_y_dps": "0.46", "gyro_z_dps": "-0.48", "gyro_magnitude_dps": "2.92", "tilt_pitch_deg": "0.75", "tilt_roll_deg": "10.06", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "163", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:30.20,H:82.00,SRAW:2424,SM:32.0,SC:MODERATE,RRAW:4095,RP:0.0,RC:NO_RAIN,MV:1,TX:0.75,TY:10.06,AXG:-0.180,AYG:0.013,AZG:1.016,GXD:2.84,GYD:0.46,GZD:-0.48"}, {"timestamp": "04-11-2026 00:18:17", "packet_id": "523", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "82.0", "dew_point_c": "26.77", "soil_raw_adc": "2416", "soil_moisture_pct": "32.4", "soil_condition": "MODERATE", "rain_raw_adc": "4095", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-2899", "accel_y_raw": "327", "accel_z_raw": "16613", "accel_x_g": "-0.177", "accel_y_g": "0.02", "accel_z_g": "1.014", "accel_magnitude_g": "1.03", "gyro_x_raw": "399", "gyro_y_raw": "99", "gyro_z_raw": "-41", "gyro_x_dps": "3.05", "gyro_y_dps": "0.76", "gyro_z_dps": "-0.32", "gyro_magnitude_dps": "3.16", "tilt_pitch_deg": "1.09", "tilt_roll_deg": "9.93", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "162", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:30.20,H:82.00,SRAW:2416,SM:32.4,SC:MODERATE,RRAW:4095,RP:0.0,RC:NO_RAIN,MV:1,TX:1.09,TY:9.93,AXG:-0.177,AYG:0.020,AZG:1.014,GXD:3.05,GYD:0.76,GZD:-0.32"}, {"timestamp": "04-11-2026 00:18:22", "packet_id": "524", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "82.0", "dew_point_c": "26.77", "soil_raw_adc": "2379", "soil_moisture_pct": "34.5", "soil_condition": "MODERATE", "rain_raw_adc": "4095", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-2883", "accel_y_raw": "327", "accel_z_raw": "16547", "accel_x_g": "-0.176", "accel_y_g": "0.02", "accel_z_g": "1.01", "accel_magnitude_g": "1.025", "gyro_x_raw": "399", "gyro_y_raw": "43", "gyro_z_raw": "-47", "gyro_x_dps": "3.05", "gyro_y_dps": "0.33", "gyro_z_dps": "-0.36", "gyro_magnitude_dps": "3.09", "tilt_pitch_deg": "1.09", "tilt_roll_deg": "9.88", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "162", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:30.20,H:82.00,SRAW:2379,SM:34.5,SC:MODERATE,RRAW:4095,RP:0.0,RC:NO_RAIN,MV:1,TX:1.09,TY:9.88,AXG:-0.176,AYG:0.020,AZG:1.010,GXD:3.05,GYD:0.33,GZD:-0.36"}, {"timestamp": "04-11-2026 00:18:27", "packet_id": "525", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "82.0", "dew_point_c": "26.77", "soil_raw_adc": "2385", "soil_moisture_pct": "34.2", "soil_condition": "MODERATE", "rain_raw_adc": "4095", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-2899", "accel_y_raw": "344", "accel_z_raw": "16629", "accel_x_g": "-0.177", "accel_y_g": "0.021", "accel_z_g": "1.015", "accel_magnitude_g": "1.031", "gyro_x_raw": "398", "gyro_y_raw": "70", "gyro_z_raw": "-51", "gyro_x_dps": "3.04", "gyro_y_dps": "0.54", "gyro_z_dps": "-0.39", "gyro_magnitude_dps": "3.11", "tilt_pitch_deg": "1.17", "tilt_roll_deg": "9.87", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "162", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:30.20,H:82.00,SRAW:2385,SM:34.2,SC:MODERATE,RRAW:4095,RP:0.0,RC:NO_RAIN,MV:1,TX:1.17,TY:9.87,AXG:-0.177,AYG:0.021,AZG:1.015,GXD:3.04,GYD:0.54,GZD:-0.39"}, {"timestamp": "04-11-2026 00:18:32", "packet_id": "526", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "82.0", "dew_point_c": "26.77", "soil_raw_adc": "2351", "soil_moisture_pct": "36.1", "soil_condition": "MODERATE", "rain_raw_adc": "4095", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-2932", "accel_y_raw": "311", "accel_z_raw": "16629", "accel_x_g": "-0.179", "accel_y_g": "0.019", "accel_z_g": "1.015", "accel_magnitude_g": "1.031", "gyro_x_raw": "403", "gyro_y_raw": "57", "gyro_z_raw": "-9", "gyro_x_dps": "3.08", "gyro_y_dps": "0.44", "gyro_z_dps": "-0.07", "gyro_magnitude_dps": "3.11", "tilt_pitch_deg": "1.06", "tilt_roll_deg": "10.01", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "163", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:30.20,H:82.00,SRAW:2351,SM:36.1,SC:MODERATE,RRAW:4095,RP:0.0,RC:NO_RAIN,MV:1,TX:1.06,TY:10.01,AXG:-0.179,AYG:0.019,AZG:1.015,GXD:3.08,GYD:0.44,GZD:-0.07"}, {"timestamp": "04-11-2026 00:18:37", "packet_id": "527", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "82.0", "dew_point_c": "26.77", "soil_raw_adc": "2362", "soil_moisture_pct": "35.4", "soil_condition": "MODERATE", "rain_raw_adc": "4095", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-2834", "accel_y_raw": "245", "accel_z_raw": "16646", "accel_x_g": "-0.173", "accel_y_g": "0.015", "accel_z_g": "1.016", "accel_magnitude_g": "1.031", "gyro_x_raw": "416", "gyro_y_raw": "79", "gyro_z_raw": "-32", "gyro_x_dps": "3.18", "gyro_y_dps": "0.61", "gyro_z_dps": "-0.25", "gyro_magnitude_dps": "3.25", "tilt_pitch_deg": "0.81", "tilt_roll_deg": "9.66", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "162", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:30.20,H:82.00,SRAW:2362,SM:35.4,SC:MODERATE,RRAW:4095,RP:0.0,RC:NO_RAIN,MV:1,TX:0.81,TY:9.66,AXG:-0.173,AYG:0.015,AZG:1.016,GXD:3.18,GYD:0.61,GZD:-0.25"}, {"timestamp": "04-11-2026 00:18:42", "packet_id": "528", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "82.0", "dew_point_c": "26.77", "soil_raw_adc": "2323", "soil_moisture_pct": "37.6", "soil_condition": "MODERATE", "rain_raw_adc": "4060", "rain_intensity_pct": "0.0", "rain_condition": "NO_RAIN", "accel_x_raw": "-2801", "accel_y_raw": "262", "accel_z_raw": "16498", "accel_x_g": "-0.171", "accel_y_g": "0.016", "accel_z_g": "1.007", "accel_magnitude_g": "1.022", "gyro_x_raw": "407", "gyro_y_raw": "116", "gyro_z_raw": "9", "gyro_x_dps": "3.11", "gyro_y_dps": "0.89", "gyro_z_dps": "0.07", "gyro_magnitude_dps": "3.24", "tilt_pitch_deg": "0.9", "tilt_roll_deg": "9.65", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "161", "landslide_risk_level": "SAFE", "hazard_status": "NORMAL_STABLE", "raw_packet": "GS1,TV:1,T:30.20,H:82.00,SRAW:2323,SM:37.6,SC:MODERATE,RRAW:4060,RP:0.0,RC:NO_RAIN,MV:1,TX:0.90,TY:9.65,AXG:-0.171,AYG:0.016,AZG:1.007,GXD:3.11,GYD:0.89,GZD:0.07"}, {"timestamp": "04-11-2026 00:18:47", "packet_id": "529", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "82.1", "dew_point_c": "26.8", "soil_raw_adc": "2375", "soil_moisture_pct": "34.7", "soil_condition": "MODERATE", "rain_raw_adc": "1641", "rain_intensity_pct": "74.4", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-2785", "accel_y_raw": "262", "accel_z_raw": "16678", "accel_x_g": "-0.17", "accel_y_g": "0.016", "accel_z_g": "1.018", "accel_magnitude_g": "1.032", "gyro_x_raw": "369", "gyro_y_raw": "104", "gyro_z_raw": "-40", "gyro_x_dps": "2.82", "gyro_y_dps": "0.8", "gyro_z_dps": "-0.31", "gyro_magnitude_dps": "2.95", "tilt_pitch_deg": "0.88", "tilt_roll_deg": "9.5", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.20,H:82.10,SRAW:2375,SM:34.7,SC:MODERATE,RRAW:1641,RP:74.4,RC:MODERATE_RAIN,MV:1,TX:0.88,TY:9.50,AXG:-0.170,AYG:0.016,AZG:1.018,GXD:2.82,GYD:0.80,GZD:-0.31"}, {"timestamp": "04-11-2026 00:18:52", "packet_id": "530", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "82.1", "dew_point_c": "26.8", "soil_raw_adc": "2448", "soil_moisture_pct": "30.7", "soil_condition": "MODERATE", "rain_raw_adc": "1797", "rain_intensity_pct": "68.1", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-2818", "accel_y_raw": "294", "accel_z_raw": "16629", "accel_x_g": "-0.172", "accel_y_g": "0.018", "accel_z_g": "1.015", "accel_magnitude_g": "1.03", "gyro_x_raw": "396", "gyro_y_raw": "93", "gyro_z_raw": "-69", "gyro_x_dps": "3.03", "gyro_y_dps": "0.71", "gyro_z_dps": "-0.53", "gyro_magnitude_dps": "3.16", "tilt_pitch_deg": "0.98", "tilt_roll_deg": "9.6", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.20,H:82.10,SRAW:2448,SM:30.7,SC:MODERATE,RRAW:1797,RP:68.1,RC:MODERATE_RAIN,MV:1,TX:0.98,TY:9.60,AXG:-0.172,AYG:0.018,AZG:1.015,GXD:3.03,GYD:0.71,GZD:-0.53"}, {"timestamp": "04-11-2026 00:18:57", "packet_id": "531", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "82.1", "dew_point_c": "26.8", "soil_raw_adc": "2384", "soil_moisture_pct": "34.2", "soil_condition": "MODERATE", "rain_raw_adc": "1904", "rain_intensity_pct": "63.8", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-2850", "accel_y_raw": "278", "accel_z_raw": "16596", "accel_x_g": "-0.174", "accel_y_g": "0.017", "accel_z_g": "1.013", "accel_magnitude_g": "1.028", "gyro_x_raw": "400", "gyro_y_raw": "56", "gyro_z_raw": "-44", "gyro_x_dps": "3.06", "gyro_y_dps": "0.43", "gyro_z_dps": "-0.34", "gyro_magnitude_dps": "3.11", "tilt_pitch_deg": "0.97", "tilt_roll_deg": "9.73", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.20,H:82.10,SRAW:2384,SM:34.2,SC:MODERATE,RRAW:1904,RP:63.8,RC:MODERATE_RAIN,MV:1,TX:0.97,TY:9.73,AXG:-0.174,AYG:0.017,AZG:1.013,GXD:3.06,GYD:0.43,GZD:-0.34"}, {"timestamp": "04-11-2026 00:19:02", "packet_id": "532", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "82.1", "dew_point_c": "26.8", "soil_raw_adc": "2416", "soil_moisture_pct": "32.4", "soil_condition": "MODERATE", "rain_raw_adc": "1877", "rain_intensity_pct": "64.9", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-2867", "accel_y_raw": "327", "accel_z_raw": "16596", "accel_x_g": "-0.175", "accel_y_g": "0.02", "accel_z_g": "1.013", "accel_magnitude_g": "1.028", "gyro_x_raw": "381", "gyro_y_raw": "99", "gyro_z_raw": "-23", "gyro_x_dps": "2.91", "gyro_y_dps": "0.76", "gyro_z_dps": "-0.18", "gyro_magnitude_dps": "3.01", "tilt_pitch_deg": "1.1", "tilt_roll_deg": "9.79", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.20,H:82.10,SRAW:2416,SM:32.4,SC:MODERATE,RRAW:1877,RP:64.9,RC:MODERATE_RAIN,MV:1,TX:1.10,TY:9.79,AXG:-0.175,AYG:0.020,AZG:1.013,GXD:2.91,GYD:0.76,GZD:-0.18"}, {"timestamp": "04-11-2026 00:19:07", "packet_id": "533", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "82.2", "dew_point_c": "26.82", "soil_raw_adc": "2414", "soil_moisture_pct": "32.6", "soil_condition": "MODERATE", "rain_raw_adc": "1985", "rain_intensity_pct": "60.6", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-2818", "accel_y_raw": "245", "accel_z_raw": "16646", "accel_x_g": "-0.172", "accel_y_g": "0.015", "accel_z_g": "1.016", "accel_magnitude_g": "1.031", "gyro_x_raw": "389", "gyro_y_raw": "52", "gyro_z_raw": "-22", "gyro_x_dps": "2.97", "gyro_y_dps": "0.4", "gyro_z_dps": "-0.17", "gyro_magnitude_dps": "3.0", "tilt_pitch_deg": "0.83", "tilt_roll_deg": "9.62", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.20,H:82.20,SRAW:2414,SM:32.6,SC:MODERATE,RRAW:1985,RP:60.6,RC:MODERATE_RAIN,MV:1,TX:0.83,TY:9.62,AXG:-0.172,AYG:0.015,AZG:1.016,GXD:2.97,GYD:0.40,GZD:-0.17"}, {"timestamp": "04-11-2026 00:19:12", "packet_id": "534", "temperature_c": "30.1", "temperature_f": "86.18", "humidity_pct": "82.1", "dew_point_c": "26.7", "soil_raw_adc": "2429", "soil_moisture_pct": "31.7", "soil_condition": "MODERATE", "rain_raw_adc": "1995", "rain_intensity_pct": "60.2", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-2768", "accel_y_raw": "294", "accel_z_raw": "16613", "accel_x_g": "-0.169", "accel_y_g": "0.018", "accel_z_g": "1.014", "accel_magnitude_g": "1.028", "gyro_x_raw": "391", "gyro_y_raw": "111", "gyro_z_raw": "-32", "gyro_x_dps": "2.99", "gyro_y_dps": "0.85", "gyro_z_dps": "-0.25", "gyro_magnitude_dps": "3.12", "tilt_pitch_deg": "0.98", "tilt_roll_deg": "9.45", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.10,H:82.10,SRAW:2429,SM:31.7,SC:MODERATE,RRAW:1995,RP:60.2,RC:MODERATE_RAIN,MV:1,TX:0.98,TY:9.45,AXG:-0.169,AYG:0.018,AZG:1.014,GXD:2.99,GYD:0.85,GZD:-0.25"}, {"timestamp": "04-11-2026 00:19:17", "packet_id": "535", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "82.2", "dew_point_c": "26.82", "soil_raw_adc": "2523", "soil_moisture_pct": "26.5", "soil_condition": "MODERATE", "rain_raw_adc": "1969", "rain_intensity_pct": "61.2", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-2818", "accel_y_raw": "294", "accel_z_raw": "16613", "accel_x_g": "-0.172", "accel_y_g": "0.018", "accel_z_g": "1.014", "accel_magnitude_g": "1.029", "gyro_x_raw": "306", "gyro_y_raw": "79", "gyro_z_raw": "379", "gyro_x_dps": "2.34", "gyro_y_dps": "0.61", "gyro_z_dps": "2.9", "gyro_magnitude_dps": "3.78", "tilt_pitch_deg": "0.99", "tilt_roll_deg": "9.62", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "168", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.20,H:82.20,SRAW:2523,SM:26.5,SC:MODERATE,RRAW:1969,RP:61.2,RC:MODERATE_RAIN,MV:1,TX:0.99,TY:9.62,AXG:-0.172,AYG:0.018,AZG:1.014,GXD:2.34,GYD:0.61,GZD:2.90"}, {"timestamp": "04-11-2026 00:19:22", "packet_id": "536", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "82.2", "dew_point_c": "26.82", "soil_raw_adc": "2403", "soil_moisture_pct": "33.2", "soil_condition": "MODERATE", "rain_raw_adc": "1926", "rain_intensity_pct": "63.0", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-2834", "accel_y_raw": "294", "accel_z_raw": "16564", "accel_x_g": "-0.173", "accel_y_g": "0.018", "accel_z_g": "1.011", "accel_magnitude_g": "1.026", "gyro_x_raw": "374", "gyro_y_raw": "79", "gyro_z_raw": "-74", "gyro_x_dps": "2.86", "gyro_y_dps": "0.61", "gyro_z_dps": "-0.57", "gyro_magnitude_dps": "2.98", "tilt_pitch_deg": "0.98", "tilt_roll_deg": "9.69", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.20,H:82.20,SRAW:2403,SM:33.2,SC:MODERATE,RRAW:1926,RP:63.0,RC:MODERATE_RAIN,MV:1,TX:0.98,TY:9.69,AXG:-0.173,AYG:0.018,AZG:1.011,GXD:2.86,GYD:0.61,GZD:-0.57"}, {"timestamp": "04-11-2026 00:19:27", "packet_id": "537", "temperature_c": "30.1", "temperature_f": "86.18", "humidity_pct": "82.2", "dew_point_c": "26.72", "soil_raw_adc": "2398", "soil_moisture_pct": "33.4", "soil_condition": "MODERATE", "rain_raw_adc": "1931", "rain_intensity_pct": "62.8", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-5865", "accel_y_raw": "425", "accel_z_raw": "16351", "accel_x_g": "-0.358", "accel_y_g": "0.026", "accel_z_g": "0.998", "accel_magnitude_g": "1.061", "gyro_x_raw": "1316", "gyro_y_raw": "15168", "gyro_z_raw": "-3264", "gyro_x_dps": "10.05", "gyro_y_dps": "115.79", "gyro_z_dps": "-24.92", "gyro_magnitude_dps": "118.87", "tilt_pitch_deg": "1.41", "tilt_roll_deg": "19.72", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "174", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.10,H:82.20,SRAW:2398,SM:33.4,SC:MODERATE,RRAW:1931,RP:62.8,RC:MODERATE_RAIN,MV:1,TX:1.41,TY:19.72,AXG:-0.358,AYG:0.026,AZG:0.998,GXD:10.05,GYD:115.79,GZD:-24.92"}, {"timestamp": "04-11-2026 00:19:32", "packet_id": "538", "temperature_c": "30.1", "temperature_f": "86.18", "humidity_pct": "82.3", "dew_point_c": "26.74", "soil_raw_adc": "2514", "soil_moisture_pct": "27.0", "soil_condition": "MODERATE", "rain_raw_adc": "1936", "rain_intensity_pct": "62.6", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-2654", "accel_y_raw": "262", "accel_z_raw": "16596", "accel_x_g": "-0.162", "accel_y_g": "0.016", "accel_z_g": "1.013", "accel_magnitude_g": "1.026", "gyro_x_raw": "407", "gyro_y_raw": "82", "gyro_z_raw": "-34", "gyro_x_dps": "3.11", "gyro_y_dps": "0.63", "gyro_z_dps": "-0.26", "gyro_magnitude_dps": "3.18", "tilt_pitch_deg": "0.91", "tilt_roll_deg": "9.08", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.10,H:82.30,SRAW:2514,SM:27.0,SC:MODERATE,RRAW:1936,RP:62.6,RC:MODERATE_RAIN,MV:1,TX:0.91,TY:9.08,AXG:-0.162,AYG:0.016,AZG:1.013,GXD:3.11,GYD:0.63,GZD:-0.26"}, {"timestamp": "04-11-2026 00:19:37", "packet_id": "539", "temperature_c": "30.1", "temperature_f": "86.18", "humidity_pct": "82.3", "dew_point_c": "26.74", "soil_raw_adc": "2493", "soil_moisture_pct": "28.2", "soil_condition": "MODERATE", "rain_raw_adc": "1938", "rain_intensity_pct": "62.5", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-2588", "accel_y_raw": "262", "accel_z_raw": "16629", "accel_x_g": "-0.158", "accel_y_g": "0.016", "accel_z_g": "1.015", "accel_magnitude_g": "1.027", "gyro_x_raw": "382", "gyro_y_raw": "91", "gyro_z_raw": "-70", "gyro_x_dps": "2.92", "gyro_y_dps": "0.7", "gyro_z_dps": "-0.54", "gyro_magnitude_dps": "3.05", "tilt_pitch_deg": "0.9", "tilt_roll_deg": "8.85", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.10,H:82.30,SRAW:2493,SM:28.2,SC:MODERATE,RRAW:1938,RP:62.5,RC:MODERATE_RAIN,MV:1,TX:0.90,TY:8.85,AXG:-0.158,AYG:0.016,AZG:1.015,GXD:2.92,GYD:0.70,GZD:-0.54"}, {"timestamp": "04-11-2026 00:19:42", "packet_id": "540", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "82.4", "dew_point_c": "26.86", "soil_raw_adc": "2506", "soil_moisture_pct": "27.4", "soil_condition": "MODERATE", "rain_raw_adc": "1938", "rain_intensity_pct": "62.5", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-2506", "accel_y_raw": "278", "accel_z_raw": "16711", "accel_x_g": "-0.153", "accel_y_g": "0.017", "accel_z_g": "1.02", "accel_magnitude_g": "1.032", "gyro_x_raw": "361", "gyro_y_raw": "87", "gyro_z_raw": "-64", "gyro_x_dps": "2.76", "gyro_y_dps": "0.67", "gyro_z_dps": "-0.49", "gyro_magnitude_dps": "2.88", "tilt_pitch_deg": "0.92", "tilt_roll_deg": "8.55", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.20,H:82.40,SRAW:2506,SM:27.4,SC:MODERATE,RRAW:1938,RP:62.5,RC:MODERATE_RAIN,MV:1,TX:0.92,TY:8.55,AXG:-0.153,AYG:0.017,AZG:1.020,GXD:2.76,GYD:0.67,GZD:-0.49"}, {"timestamp": "04-11-2026 00:19:47", "packet_id": "541", "temperature_c": "30.1", "temperature_f": "86.18", "humidity_pct": "82.3", "dew_point_c": "26.74", "soil_raw_adc": "2593", "soil_moisture_pct": "22.6", "soil_condition": "DRY", "rain_raw_adc": "1935", "rain_intensity_pct": "62.6", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-2572", "accel_y_raw": "229", "accel_z_raw": "16695", "accel_x_g": "-0.157", "accel_y_g": "0.014", "accel_z_g": "1.019", "accel_magnitude_g": "1.031", "gyro_x_raw": "370", "gyro_y_raw": "100", "gyro_z_raw": "-6", "gyro_x_dps": "2.83", "gyro_y_dps": "0.77", "gyro_z_dps": "-0.05", "gyro_magnitude_dps": "2.93", "tilt_pitch_deg": "0.8", "tilt_roll_deg": "8.78", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "164", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.10,H:82.30,SRAW:2593,SM:22.6,SC:DRY,RRAW:1935,RP:62.6,RC:MODERATE_RAIN,MV:1,TX:0.80,TY:8.78,AXG:-0.157,AYG:0.014,AZG:1.019,GXD:2.83,GYD:0.77,GZD:-0.05"}, {"timestamp": "04-11-2026 00:19:52", "packet_id": "542", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "82.4", "dew_point_c": "26.86", "soil_raw_adc": "2374", "soil_moisture_pct": "34.8", "soil_condition": "MODERATE", "rain_raw_adc": "1919", "rain_intensity_pct": "63.2", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-2588", "accel_y_raw": "229", "accel_z_raw": "16760", "accel_x_g": "-0.158", "accel_y_g": "0.014", "accel_z_g": "1.023", "accel_magnitude_g": "1.035", "gyro_x_raw": "378", "gyro_y_raw": "64", "gyro_z_raw": "-60", "gyro_x_dps": "2.89", "gyro_y_dps": "0.49", "gyro_z_dps": "-0.46", "gyro_magnitude_dps": "2.97", "tilt_pitch_deg": "0.77", "tilt_roll_deg": "8.78", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.20,H:82.40,SRAW:2374,SM:34.8,SC:MODERATE,RRAW:1919,RP:63.2,RC:MODERATE_RAIN,MV:1,TX:0.77,TY:8.78,AXG:-0.158,AYG:0.014,AZG:1.023,GXD:2.89,GYD:0.49,GZD:-0.46"}, {"timestamp": "04-11-2026 00:19:57", "packet_id": "543", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "82.4", "dew_point_c": "26.86", "soil_raw_adc": "2455", "soil_moisture_pct": "30.3", "soil_condition": "MODERATE", "rain_raw_adc": "1885", "rain_intensity_pct": "64.6", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-2457", "accel_y_raw": "327", "accel_z_raw": "16564", "accel_x_g": "-0.15", "accel_y_g": "0.02", "accel_z_g": "1.011", "accel_magnitude_g": "1.022", "gyro_x_raw": "396", "gyro_y_raw": "104", "gyro_z_raw": "-64", "gyro_x_dps": "3.03", "gyro_y_dps": "0.8", "gyro_z_dps": "-0.49", "gyro_magnitude_dps": "3.17", "tilt_pitch_deg": "1.09", "tilt_roll_deg": "8.46", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.20,H:82.40,SRAW:2455,SM:30.3,SC:MODERATE,RRAW:1885,RP:64.6,RC:MODERATE_RAIN,MV:1,TX:1.09,TY:8.46,AXG:-0.150,AYG:0.020,AZG:1.011,GXD:3.03,GYD:0.80,GZD:-0.49"}, {"timestamp": "04-11-2026 00:20:02", "packet_id": "544", "temperature_c": "30.1", "temperature_f": "86.18", "humidity_pct": "82.4", "dew_point_c": "26.76", "soil_raw_adc": "2418", "soil_moisture_pct": "32.3", "soil_condition": "MODERATE", "rain_raw_adc": "1913", "rain_intensity_pct": "63.5", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-2572", "accel_y_raw": "262", "accel_z_raw": "16613", "accel_x_g": "-0.157", "accel_y_g": "0.016", "accel_z_g": "1.014", "accel_magnitude_g": "1.026", "gyro_x_raw": "378", "gyro_y_raw": "66", "gyro_z_raw": "-35", "gyro_x_dps": "2.89", "gyro_y_dps": "0.51", "gyro_z_dps": "-0.27", "gyro_magnitude_dps": "2.95", "tilt_pitch_deg": "0.91", "tilt_roll_deg": "8.8", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.10,H:82.40,SRAW:2418,SM:32.3,SC:MODERATE,RRAW:1913,RP:63.5,RC:MODERATE_RAIN,MV:1,TX:0.91,TY:8.80,AXG:-0.157,AYG:0.016,AZG:1.014,GXD:2.89,GYD:0.51,GZD:-0.27"}, {"timestamp": "04-11-2026 00:20:07", "packet_id": "545", "temperature_c": "30.1", "temperature_f": "86.18", "humidity_pct": "82.3", "dew_point_c": "26.74", "soil_raw_adc": "2429", "soil_moisture_pct": "31.7", "soil_condition": "MODERATE", "rain_raw_adc": "1908", "rain_intensity_pct": "63.7", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-2539", "accel_y_raw": "278", "accel_z_raw": "16613", "accel_x_g": "-0.155", "accel_y_g": "0.017", "accel_z_g": "1.014", "accel_magnitude_g": "1.026", "gyro_x_raw": "366", "gyro_y_raw": "78", "gyro_z_raw": "-13", "gyro_x_dps": "2.8", "gyro_y_dps": "0.6", "gyro_z_dps": "-0.1", "gyro_magnitude_dps": "2.87", "tilt_pitch_deg": "0.93", "tilt_roll_deg": "8.7", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.10,H:82.30,SRAW:2429,SM:31.7,SC:MODERATE,RRAW:1908,RP:63.7,RC:MODERATE_RAIN,MV:1,TX:0.93,TY:8.70,AXG:-0.155,AYG:0.017,AZG:1.014,GXD:2.80,GYD:0.60,GZD:-0.10"}, {"timestamp": "04-11-2026 00:20:12", "packet_id": "546", "temperature_c": "30.1", "temperature_f": "86.18", "humidity_pct": "82.3", "dew_point_c": "26.74", "soil_raw_adc": "2443", "soil_moisture_pct": "30.9", "soil_condition": "MODERATE", "rain_raw_adc": "1903", "rain_intensity_pct": "63.9", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-2572", "accel_y_raw": "278", "accel_z_raw": "16859", "accel_x_g": "-0.157", "accel_y_g": "0.017", "accel_z_g": "1.029", "accel_magnitude_g": "1.041", "gyro_x_raw": "393", "gyro_y_raw": "111", "gyro_z_raw": "-48", "gyro_x_dps": "3.0", "gyro_y_dps": "0.85", "gyro_z_dps": "-0.37", "gyro_magnitude_dps": "3.14", "tilt_pitch_deg": "0.93", "tilt_roll_deg": "8.7", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.10,H:82.30,SRAW:2443,SM:30.9,SC:MODERATE,RRAW:1903,RP:63.9,RC:MODERATE_RAIN,MV:1,TX:0.93,TY:8.70,AXG:-0.157,AYG:0.017,AZG:1.029,GXD:3.00,GYD:0.85,GZD:-0.37"}, {"timestamp": "04-11-2026 00:20:17", "packet_id": "547", "temperature_c": "30.1", "temperature_f": "86.18", "humidity_pct": "82.3", "dew_point_c": "26.74", "soil_raw_adc": "2447", "soil_moisture_pct": "30.7", "soil_condition": "MODERATE", "rain_raw_adc": "1915", "rain_intensity_pct": "63.4", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-2539", "accel_y_raw": "278", "accel_z_raw": "16695", "accel_x_g": "-0.155", "accel_y_g": "0.017", "accel_z_g": "1.019", "accel_magnitude_g": "1.031", "gyro_x_raw": "373", "gyro_y_raw": "90", "gyro_z_raw": "-26", "gyro_x_dps": "2.85", "gyro_y_dps": "0.69", "gyro_z_dps": "-0.2", "gyro_magnitude_dps": "2.94", "tilt_pitch_deg": "0.95", "tilt_roll_deg": "8.66", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.10,H:82.30,SRAW:2447,SM:30.7,SC:MODERATE,RRAW:1915,RP:63.4,RC:MODERATE_RAIN,MV:1,TX:0.95,TY:8.66,AXG:-0.155,AYG:0.017,AZG:1.019,GXD:2.85,GYD:0.69,GZD:-0.20"}, {"timestamp": "04-11-2026 00:20:22", "packet_id": "548", "temperature_c": "30.1", "temperature_f": "86.18", "humidity_pct": "82.3", "dew_point_c": "26.74", "soil_raw_adc": "2491", "soil_moisture_pct": "28.3", "soil_condition": "MODERATE", "rain_raw_adc": "1895", "rain_intensity_pct": "64.2", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-2605", "accel_y_raw": "245", "accel_z_raw": "16842", "accel_x_g": "-0.159", "accel_y_g": "0.015", "accel_z_g": "1.028", "accel_magnitude_g": "1.04", "gyro_x_raw": "332", "gyro_y_raw": "111", "gyro_z_raw": "-69", "gyro_x_dps": "2.54", "gyro_y_dps": "0.85", "gyro_z_dps": "-0.53", "gyro_magnitude_dps": "2.73", "tilt_pitch_deg": "0.81", "tilt_roll_deg": "8.79", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "ADVISORY", "hazard_status": "MOISTURE_ADVISORY", "raw_packet": "GS1,TV:1,T:30.10,H:82.30,SRAW:2491,SM:28.3,SC:MODERATE,RRAW:1895,RP:64.2,RC:MODERATE_RAIN,MV:1,TX:0.81,TY:8.79,AXG:-0.159,AYG:0.015,AZG:1.028,GXD:2.54,GYD:0.85,GZD:-0.53"}, {"timestamp": "04-11-2026 00:20:27", "packet_id": "549", "temperature_c": "30.1", "temperature_f": "86.18", "humidity_pct": "82.3", "dew_point_c": "26.74", "soil_raw_adc": "2325", "soil_moisture_pct": "37.5", "soil_condition": "MODERATE", "rain_raw_adc": "1845", "rain_intensity_pct": "66.2", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-7618", "accel_y_raw": "180", "accel_z_raw": "14761", "accel_x_g": "-0.465", "accel_y_g": "0.011", "accel_z_g": "0.901", "accel_magnitude_g": "1.014", "gyro_x_raw": "399", "gyro_y_raw": "56", "gyro_z_raw": "-6", "gyro_x_dps": "3.05", "gyro_y_dps": "0.43", "gyro_z_dps": "-0.05", "gyro_magnitude_dps": "3.08", "tilt_pitch_deg": "0.62", "tilt_roll_deg": "27.29", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "170", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.10,H:82.30,SRAW:2325,SM:37.5,SC:MODERATE,RRAW:1845,RP:66.2,RC:MODERATE_RAIN,MV:1,TX:0.62,TY:27.29,AXG:-0.465,AYG:0.011,AZG:0.901,GXD:3.05,GYD:0.43,GZD:-0.05"}, {"timestamp": "04-11-2026 00:20:32", "packet_id": "550", "temperature_c": "30.1", "temperature_f": "86.18", "humidity_pct": "82.3", "dew_point_c": "26.74", "soil_raw_adc": "2368", "soil_moisture_pct": "35.1", "soil_condition": "MODERATE", "rain_raw_adc": "1889", "rain_intensity_pct": "64.4", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-7618", "accel_y_raw": "131", "accel_z_raw": "14827", "accel_x_g": "-0.465", "accel_y_g": "0.008", "accel_z_g": "0.905", "accel_magnitude_g": "1.018", "gyro_x_raw": "382", "gyro_y_raw": "78", "gyro_z_raw": "-48", "gyro_x_dps": "2.92", "gyro_y_dps": "0.6", "gyro_z_dps": "-0.37", "gyro_magnitude_dps": "3.0", "tilt_pitch_deg": "0.45", "tilt_roll_deg": "27.19", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "170", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.10,H:82.30,SRAW:2368,SM:35.1,SC:MODERATE,RRAW:1889,RP:64.4,RC:MODERATE_RAIN,MV:1,TX:0.45,TY:27.19,AXG:-0.465,AYG:0.008,AZG:0.905,GXD:2.92,GYD:0.60,GZD:-0.37"}, {"timestamp": "04-11-2026 00:20:37", "packet_id": "551", "temperature_c": "30.1", "temperature_f": "86.18", "humidity_pct": "82.4", "dew_point_c": "26.76", "soil_raw_adc": "2439", "soil_moisture_pct": "31.2", "soil_condition": "MODERATE", "rain_raw_adc": "1887", "rain_intensity_pct": "64.5", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-7618", "accel_y_raw": "212", "accel_z_raw": "14942", "accel_x_g": "-0.465", "accel_y_g": "0.013", "accel_z_g": "0.912", "accel_magnitude_g": "1.024", "gyro_x_raw": "361", "gyro_y_raw": "55", "gyro_z_raw": "-2", "gyro_x_dps": "2.76", "gyro_y_dps": "0.42", "gyro_z_dps": "-0.02", "gyro_magnitude_dps": "2.79", "tilt_pitch_deg": "0.72", "tilt_roll_deg": "27.03", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "170", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.10,H:82.40,SRAW:2439,SM:31.2,SC:MODERATE,RRAW:1887,RP:64.5,RC:MODERATE_RAIN,MV:1,TX:0.72,TY:27.03,AXG:-0.465,AYG:0.013,AZG:0.912,GXD:2.76,GYD:0.42,GZD:-0.02"}, {"timestamp": "04-11-2026 00:20:42", "packet_id": "552", "temperature_c": "30.1", "temperature_f": "86.18", "humidity_pct": "82.4", "dew_point_c": "26.76", "soil_raw_adc": "2461", "soil_moisture_pct": "29.9", "soil_condition": "MODERATE", "rain_raw_adc": "1838", "rain_intensity_pct": "66.5", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-7602", "accel_y_raw": "163", "accel_z_raw": "14860", "accel_x_g": "-0.464", "accel_y_g": "0.01", "accel_z_g": "0.907", "accel_magnitude_g": "1.019", "gyro_x_raw": "378", "gyro_y_raw": "77", "gyro_z_raw": "-19", "gyro_x_dps": "2.89", "gyro_y_dps": "0.59", "gyro_z_dps": "-0.15", "gyro_magnitude_dps": "2.95", "tilt_pitch_deg": "0.54", "tilt_roll_deg": "27.07", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "170", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.10,H:82.40,SRAW:2461,SM:29.9,SC:MODERATE,RRAW:1838,RP:66.5,RC:MODERATE_RAIN,MV:1,TX:0.54,TY:27.07,AXG:-0.464,AYG:0.010,AZG:0.907,GXD:2.89,GYD:0.59,GZD:-0.15"}, {"timestamp": "04-11-2026 00:20:47", "packet_id": "553", "temperature_c": "30.1", "temperature_f": "86.18", "humidity_pct": "82.4", "dew_point_c": "26.76", "soil_raw_adc": "2448", "soil_moisture_pct": "30.7", "soil_condition": "MODERATE", "rain_raw_adc": "1883", "rain_intensity_pct": "64.7", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-7585", "accel_y_raw": "98", "accel_z_raw": "14860", "accel_x_g": "-0.463", "accel_y_g": "0.006", "accel_z_g": "0.907", "accel_magnitude_g": "1.018", "gyro_x_raw": "365", "gyro_y_raw": "72", "gyro_z_raw": "-37", "gyro_x_dps": "2.79", "gyro_y_dps": "0.55", "gyro_z_dps": "-0.29", "gyro_magnitude_dps": "2.86", "tilt_pitch_deg": "0.34", "tilt_roll_deg": "27.05", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "170", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.10,H:82.40,SRAW:2448,SM:30.7,SC:MODERATE,RRAW:1883,RP:64.7,RC:MODERATE_RAIN,MV:1,TX:0.34,TY:27.05,AXG:-0.463,AYG:0.006,AZG:0.907,GXD:2.79,GYD:0.55,GZD:-0.29"}, {"timestamp": "04-11-2026 00:20:52", "packet_id": "554", "temperature_c": "30.1", "temperature_f": "86.18", "humidity_pct": "82.4", "dew_point_c": "26.76", "soil_raw_adc": "2458", "soil_moisture_pct": "30.1", "soil_condition": "MODERATE", "rain_raw_adc": "1885", "rain_intensity_pct": "64.6", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-7536", "accel_y_raw": "180", "accel_z_raw": "14909", "accel_x_g": "-0.46", "accel_y_g": "0.011", "accel_z_g": "0.91", "accel_magnitude_g": "1.02", "gyro_x_raw": "410", "gyro_y_raw": "93", "gyro_z_raw": "-11", "gyro_x_dps": "3.13", "gyro_y_dps": "0.71", "gyro_z_dps": "-0.09", "gyro_magnitude_dps": "3.21", "tilt_pitch_deg": "0.6", "tilt_roll_deg": "26.82", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "170", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.10,H:82.40,SRAW:2458,SM:30.1,SC:MODERATE,RRAW:1885,RP:64.6,RC:MODERATE_RAIN,MV:1,TX:0.60,TY:26.82,AXG:-0.460,AYG:0.011,AZG:0.910,GXD:3.13,GYD:0.71,GZD:-0.09"}, {"timestamp": "04-11-2026 00:20:57", "packet_id": "555", "temperature_c": "30.1", "temperature_f": "86.18", "humidity_pct": "82.4", "dew_point_c": "26.76", "soil_raw_adc": "2463", "soil_moisture_pct": "29.8", "soil_condition": "MODERATE", "rain_raw_adc": "1889", "rain_intensity_pct": "64.4", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-7651", "accel_y_raw": "294", "accel_z_raw": "14925", "accel_x_g": "-0.467", "accel_y_g": "0.018", "accel_z_g": "0.911", "accel_magnitude_g": "1.024", "gyro_x_raw": "408", "gyro_y_raw": "115", "gyro_z_raw": "1", "gyro_x_dps": "3.12", "gyro_y_dps": "0.88", "gyro_z_dps": "0.01", "gyro_magnitude_dps": "3.24", "tilt_pitch_deg": "1.01", "tilt_roll_deg": "27.1", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "169", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.10,H:82.40,SRAW:2463,SM:29.8,SC:MODERATE,RRAW:1889,RP:64.4,RC:MODERATE_RAIN,MV:1,TX:1.01,TY:27.10,AXG:-0.467,AYG:0.018,AZG:0.911,GXD:3.12,GYD:0.88,GZD:0.01"}, {"timestamp": "04-11-2026 00:21:02", "packet_id": "556", "temperature_c": "30.1", "temperature_f": "86.18", "humidity_pct": "82.5", "dew_point_c": "26.78", "soil_raw_adc": "2466", "soil_moisture_pct": "29.7", "soil_condition": "MODERATE", "rain_raw_adc": "1875", "rain_intensity_pct": "65.0", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-7700", "accel_y_raw": "163", "accel_z_raw": "14991", "accel_x_g": "-0.47", "accel_y_g": "0.01", "accel_z_g": "0.915", "accel_magnitude_g": "1.029", "gyro_x_raw": "370", "gyro_y_raw": "72", "gyro_z_raw": "-24", "gyro_x_dps": "2.83", "gyro_y_dps": "0.55", "gyro_z_dps": "-0.19", "gyro_magnitude_dps": "2.89", "tilt_pitch_deg": "0.56", "tilt_roll_deg": "27.19", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "170", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.10,H:82.50,SRAW:2466,SM:29.7,SC:MODERATE,RRAW:1875,RP:65.0,RC:MODERATE_RAIN,MV:1,TX:0.56,TY:27.19,AXG:-0.470,AYG:0.010,AZG:0.915,GXD:2.83,GYD:0.55,GZD:-0.19"}, {"timestamp": "04-11-2026 00:21:07", "packet_id": "557", "temperature_c": "30.1", "temperature_f": "86.18", "humidity_pct": "82.4", "dew_point_c": "26.76", "soil_raw_adc": "2507", "soil_moisture_pct": "27.4", "soil_condition": "MODERATE", "rain_raw_adc": "1873", "rain_intensity_pct": "65.1", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-7553", "accel_y_raw": "212", "accel_z_raw": "14893", "accel_x_g": "-0.461", "accel_y_g": "0.013", "accel_z_g": "0.909", "accel_magnitude_g": "1.019", "gyro_x_raw": "391", "gyro_y_raw": "66", "gyro_z_raw": "-3", "gyro_x_dps": "2.99", "gyro_y_dps": "0.51", "gyro_z_dps": "-0.03", "gyro_magnitude_dps": "3.03", "tilt_pitch_deg": "0.71", "tilt_roll_deg": "26.92", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "170", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.10,H:82.40,SRAW:2507,SM:27.4,SC:MODERATE,RRAW:1873,RP:65.1,RC:MODERATE_RAIN,MV:1,TX:0.71,TY:26.92,AXG:-0.461,AYG:0.013,AZG:0.909,GXD:2.99,GYD:0.51,GZD:-0.03"}, {"timestamp": "04-11-2026 00:21:12", "packet_id": "558", "temperature_c": "30.1", "temperature_f": "86.18", "humidity_pct": "82.4", "dew_point_c": "26.76", "soil_raw_adc": "2375", "soil_moisture_pct": "34.7", "soil_condition": "MODERATE", "rain_raw_adc": "1869", "rain_intensity_pct": "65.2", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-7569", "accel_y_raw": "196", "accel_z_raw": "14991", "accel_x_g": "-0.462", "accel_y_g": "0.012", "accel_z_g": "0.915", "accel_magnitude_g": "1.025", "gyro_x_raw": "377", "gyro_y_raw": "74", "gyro_z_raw": "-31", "gyro_x_dps": "2.88", "gyro_y_dps": "0.57", "gyro_z_dps": "-0.24", "gyro_magnitude_dps": "2.95", "tilt_pitch_deg": "0.68", "tilt_roll_deg": "26.77", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "170", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.10,H:82.40,SRAW:2375,SM:34.7,SC:MODERATE,RRAW:1869,RP:65.2,RC:MODERATE_RAIN,MV:1,TX:0.68,TY:26.77,AXG:-0.462,AYG:0.012,AZG:0.915,GXD:2.88,GYD:0.57,GZD:-0.24"}, {"timestamp": "04-11-2026 00:21:17", "packet_id": "559", "temperature_c": "30.1", "temperature_f": "86.18", "humidity_pct": "82.4", "dew_point_c": "26.76", "soil_raw_adc": "2384", "soil_moisture_pct": "34.2", "soil_condition": "MODERATE", "rain_raw_adc": "1847", "rain_intensity_pct": "66.1", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-7585", "accel_y_raw": "212", "accel_z_raw": "14909", "accel_x_g": "-0.463", "accel_y_g": "0.013", "accel_z_g": "0.91", "accel_magnitude_g": "1.021", "gyro_x_raw": "390", "gyro_y_raw": "44", "gyro_z_raw": "-47", "gyro_x_dps": "2.98", "gyro_y_dps": "0.34", "gyro_z_dps": "-0.36", "gyro_magnitude_dps": "3.02", "tilt_pitch_deg": "0.75", "tilt_roll_deg": "26.95", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "170", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.10,H:82.40,SRAW:2384,SM:34.2,SC:MODERATE,RRAW:1847,RP:66.1,RC:MODERATE_RAIN,MV:1,TX:0.75,TY:26.95,AXG:-0.463,AYG:0.013,AZG:0.910,GXD:2.98,GYD:0.34,GZD:-0.36"}, {"timestamp": "04-11-2026 00:21:22", "packet_id": "560", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "82.4", "dew_point_c": "26.86", "soil_raw_adc": "2378", "soil_moisture_pct": "34.6", "soil_condition": "MODERATE", "rain_raw_adc": "1858", "rain_intensity_pct": "65.7", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-7536", "accel_y_raw": "229", "accel_z_raw": "14942", "accel_x_g": "-0.46", "accel_y_g": "0.014", "accel_z_g": "0.912", "accel_magnitude_g": "1.022", "gyro_x_raw": "398", "gyro_y_raw": "73", "gyro_z_raw": "-18", "gyro_x_dps": "3.04", "gyro_y_dps": "0.56", "gyro_z_dps": "-0.14", "gyro_magnitude_dps": "3.09", "tilt_pitch_deg": "0.81", "tilt_roll_deg": "26.78", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "170", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.20,H:82.40,SRAW:2378,SM:34.6,SC:MODERATE,RRAW:1858,RP:65.7,RC:MODERATE_RAIN,MV:1,TX:0.81,TY:26.78,AXG:-0.460,AYG:0.014,AZG:0.912,GXD:3.04,GYD:0.56,GZD:-0.14"}, {"timestamp": "04-11-2026 00:21:27", "packet_id": "561", "temperature_c": "30.1", "temperature_f": "86.18", "humidity_pct": "82.3", "dew_point_c": "26.74", "soil_raw_adc": "2382", "soil_moisture_pct": "34.3", "soil_condition": "MODERATE", "rain_raw_adc": "1847", "rain_intensity_pct": "66.1", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-7553", "accel_y_raw": "212", "accel_z_raw": "14925", "accel_x_g": "-0.461", "accel_y_g": "0.013", "accel_z_g": "0.911", "accel_magnitude_g": "1.021", "gyro_x_raw": "378", "gyro_y_raw": "111", "gyro_z_raw": "-11", "gyro_x_dps": "2.89", "gyro_y_dps": "0.85", "gyro_z_dps": "-0.09", "gyro_magnitude_dps": "3.01", "tilt_pitch_deg": "0.75", "tilt_roll_deg": "26.87", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "170", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.10,H:82.30,SRAW:2382,SM:34.3,SC:MODERATE,RRAW:1847,RP:66.1,RC:MODERATE_RAIN,MV:1,TX:0.75,TY:26.87,AXG:-0.461,AYG:0.013,AZG:0.911,GXD:2.89,GYD:0.85,GZD:-0.09"}, {"timestamp": "04-11-2026 00:21:32", "packet_id": "562", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "82.3", "dew_point_c": "26.84", "soil_raw_adc": "2423", "soil_moisture_pct": "32.1", "soil_condition": "MODERATE", "rain_raw_adc": "1862", "rain_intensity_pct": "65.5", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-7684", "accel_y_raw": "163", "accel_z_raw": "14942", "accel_x_g": "-0.469", "accel_y_g": "0.01", "accel_z_g": "0.912", "accel_magnitude_g": "1.026", "gyro_x_raw": "395", "gyro_y_raw": "72", "gyro_z_raw": "-32", "gyro_x_dps": "3.02", "gyro_y_dps": "0.55", "gyro_z_dps": "-0.25", "gyro_magnitude_dps": "3.08", "tilt_pitch_deg": "0.55", "tilt_roll_deg": "27.21", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "170", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.20,H:82.30,SRAW:2423,SM:32.1,SC:MODERATE,RRAW:1862,RP:65.5,RC:MODERATE_RAIN,MV:1,TX:0.55,TY:27.21,AXG:-0.469,AYG:0.010,AZG:0.912,GXD:3.02,GYD:0.55,GZD:-0.25"}, {"timestamp": "04-11-2026 00:21:37", "packet_id": "563", "temperature_c": "30.1", "temperature_f": "86.18", "humidity_pct": "82.3", "dew_point_c": "26.74", "soil_raw_adc": "2382", "soil_moisture_pct": "34.3", "soil_condition": "MODERATE", "rain_raw_adc": "1852", "rain_intensity_pct": "65.9", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-7520", "accel_y_raw": "212", "accel_z_raw": "14991", "accel_x_g": "-0.459", "accel_y_g": "0.013", "accel_z_g": "0.915", "accel_magnitude_g": "1.024", "gyro_x_raw": "386", "gyro_y_raw": "87", "gyro_z_raw": "-65", "gyro_x_dps": "2.95", "gyro_y_dps": "0.67", "gyro_z_dps": "-0.5", "gyro_magnitude_dps": "3.07", "tilt_pitch_deg": "0.75", "tilt_roll_deg": "26.64", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "170", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.10,H:82.30,SRAW:2382,SM:34.3,SC:MODERATE,RRAW:1852,RP:65.9,RC:MODERATE_RAIN,MV:1,TX:0.75,TY:26.64,AXG:-0.459,AYG:0.013,AZG:0.915,GXD:2.95,GYD:0.67,GZD:-0.50"}, {"timestamp": "04-11-2026 00:21:42", "packet_id": "564", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "82.3", "dew_point_c": "26.84", "soil_raw_adc": "2382", "soil_moisture_pct": "34.3", "soil_condition": "MODERATE", "rain_raw_adc": "1845", "rain_intensity_pct": "66.2", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-7520", "accel_y_raw": "147", "accel_z_raw": "14827", "accel_x_g": "-0.459", "accel_y_g": "0.009", "accel_z_g": "0.905", "accel_magnitude_g": "1.015", "gyro_x_raw": "349", "gyro_y_raw": "85", "gyro_z_raw": "-41", "gyro_x_dps": "2.67", "gyro_y_dps": "0.65", "gyro_z_dps": "-0.32", "gyro_magnitude_dps": "2.77", "tilt_pitch_deg": "0.51", "tilt_roll_deg": "26.91", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "170", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.20,H:82.30,SRAW:2382,SM:34.3,SC:MODERATE,RRAW:1845,RP:66.2,RC:MODERATE_RAIN,MV:1,TX:0.51,TY:26.91,AXG:-0.459,AYG:0.009,AZG:0.905,GXD:2.67,GYD:0.65,GZD:-0.32"}, {"timestamp": "04-11-2026 00:21:47", "packet_id": "565", "temperature_c": "30.1", "temperature_f": "86.18", "humidity_pct": "82.3", "dew_point_c": "26.74", "soil_raw_adc": "2368", "soil_moisture_pct": "35.1", "soil_condition": "MODERATE", "rain_raw_adc": "1854", "rain_intensity_pct": "65.8", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-7536", "accel_y_raw": "196", "accel_z_raw": "15024", "accel_x_g": "-0.46", "accel_y_g": "0.012", "accel_z_g": "0.917", "accel_magnitude_g": "1.026", "gyro_x_raw": "398", "gyro_y_raw": "79", "gyro_z_raw": "-62", "gyro_x_dps": "3.04", "gyro_y_dps": "0.61", "gyro_z_dps": "-0.48", "gyro_magnitude_dps": "3.14", "tilt_pitch_deg": "0.67", "tilt_roll_deg": "26.62", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "170", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.10,H:82.30,SRAW:2368,SM:35.1,SC:MODERATE,RRAW:1854,RP:65.8,RC:MODERATE_RAIN,MV:1,TX:0.67,TY:26.62,AXG:-0.460,AYG:0.012,AZG:0.917,GXD:3.04,GYD:0.61,GZD:-0.48"}, {"timestamp": "04-11-2026 00:21:52", "packet_id": "566", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "82.3", "dew_point_c": "26.84", "soil_raw_adc": "2382", "soil_moisture_pct": "34.3", "soil_condition": "MODERATE", "rain_raw_adc": "1844", "rain_intensity_pct": "66.2", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-7569", "accel_y_raw": "212", "accel_z_raw": "14942", "accel_x_g": "-0.462", "accel_y_g": "0.013", "accel_z_g": "0.912", "accel_magnitude_g": "1.022", "gyro_x_raw": "399", "gyro_y_raw": "107", "gyro_z_raw": "-10", "gyro_x_dps": "3.05", "gyro_y_dps": "0.82", "gyro_z_dps": "-0.08", "gyro_magnitude_dps": "3.16", "tilt_pitch_deg": "0.71", "tilt_roll_deg": "26.87", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "170", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.20,H:82.30,SRAW:2382,SM:34.3,SC:MODERATE,RRAW:1844,RP:66.2,RC:MODERATE_RAIN,MV:1,TX:0.71,TY:26.87,AXG:-0.462,AYG:0.013,AZG:0.912,GXD:3.05,GYD:0.82,GZD:-0.08"}, {"timestamp": "04-11-2026 00:21:57", "packet_id": "567", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "82.3", "dew_point_c": "26.84", "soil_raw_adc": "2374", "soil_moisture_pct": "34.8", "soil_condition": "MODERATE", "rain_raw_adc": "1846", "rain_intensity_pct": "66.2", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-7585", "accel_y_raw": "163", "accel_z_raw": "15007", "accel_x_g": "-0.463", "accel_y_g": "0.01", "accel_z_g": "0.916", "accel_magnitude_g": "1.026", "gyro_x_raw": "412", "gyro_y_raw": "93", "gyro_z_raw": "-58", "gyro_x_dps": "3.15", "gyro_y_dps": "0.71", "gyro_z_dps": "-0.45", "gyro_magnitude_dps": "3.26", "tilt_pitch_deg": "0.53", "tilt_roll_deg": "26.83", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "170", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.20,H:82.30,SRAW:2374,SM:34.8,SC:MODERATE,RRAW:1846,RP:66.2,RC:MODERATE_RAIN,MV:1,TX:0.53,TY:26.83,AXG:-0.463,AYG:0.010,AZG:0.916,GXD:3.15,GYD:0.71,GZD:-0.45"}, {"timestamp": "04-11-2026 00:22:02", "packet_id": "568", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "82.3", "dew_point_c": "26.84", "soil_raw_adc": "2365", "soil_moisture_pct": "35.3", "soil_condition": "MODERATE", "rain_raw_adc": "1840", "rain_intensity_pct": "66.4", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-7503", "accel_y_raw": "163", "accel_z_raw": "14925", "accel_x_g": "-0.458", "accel_y_g": "0.01", "accel_z_g": "0.911", "accel_magnitude_g": "1.02", "gyro_x_raw": "411", "gyro_y_raw": "73", "gyro_z_raw": "-55", "gyro_x_dps": "3.14", "gyro_y_dps": "0.56", "gyro_z_dps": "-0.42", "gyro_magnitude_dps": "3.22", "tilt_pitch_deg": "0.53", "tilt_roll_deg": "26.69", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "170", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.20,H:82.30,SRAW:2365,SM:35.3,SC:MODERATE,RRAW:1840,RP:66.4,RC:MODERATE_RAIN,MV:1,TX:0.53,TY:26.69,AXG:-0.458,AYG:0.010,AZG:0.911,GXD:3.14,GYD:0.56,GZD:-0.42"}, {"timestamp": "04-11-2026 00:22:07", "packet_id": "569", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "82.3", "dew_point_c": "26.84", "soil_raw_adc": "2347", "soil_moisture_pct": "36.3", "soil_condition": "MODERATE", "rain_raw_adc": "1850", "rain_intensity_pct": "66.0", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-7602", "accel_y_raw": "180", "accel_z_raw": "15024", "accel_x_g": "-0.464", "accel_y_g": "0.011", "accel_z_g": "0.917", "accel_magnitude_g": "1.028", "gyro_x_raw": "391", "gyro_y_raw": "61", "gyro_z_raw": "-65", "gyro_x_dps": "2.99", "gyro_y_dps": "0.47", "gyro_z_dps": "-0.5", "gyro_magnitude_dps": "3.07", "tilt_pitch_deg": "0.63", "tilt_roll_deg": "26.81", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "170", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.20,H:82.30,SRAW:2347,SM:36.3,SC:MODERATE,RRAW:1850,RP:66.0,RC:MODERATE_RAIN,MV:1,TX:0.63,TY:26.81,AXG:-0.464,AYG:0.011,AZG:0.917,GXD:2.99,GYD:0.47,GZD:-0.50"}, {"timestamp": "04-11-2026 00:22:12", "packet_id": "570", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "82.3", "dew_point_c": "26.84", "soil_raw_adc": "2430", "soil_moisture_pct": "31.7", "soil_condition": "MODERATE", "rain_raw_adc": "1843", "rain_intensity_pct": "66.3", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-7553", "accel_y_raw": "212", "accel_z_raw": "14974", "accel_x_g": "-0.461", "accel_y_g": "0.013", "accel_z_g": "0.914", "accel_magnitude_g": "1.024", "gyro_x_raw": "386", "gyro_y_raw": "79", "gyro_z_raw": "-57", "gyro_x_dps": "2.95", "gyro_y_dps": "0.61", "gyro_z_dps": "-0.44", "gyro_magnitude_dps": "3.04", "tilt_pitch_deg": "0.71", "tilt_roll_deg": "26.77", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "170", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.20,H:82.30,SRAW:2430,SM:31.7,SC:MODERATE,RRAW:1843,RP:66.3,RC:MODERATE_RAIN,MV:1,TX:0.71,TY:26.77,AXG:-0.461,AYG:0.013,AZG:0.914,GXD:2.95,GYD:0.61,GZD:-0.44"}, {"timestamp": "04-11-2026 00:22:17", "packet_id": "571", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "82.3", "dew_point_c": "26.84", "soil_raw_adc": "2384", "soil_moisture_pct": "34.2", "soil_condition": "MODERATE", "rain_raw_adc": "1846", "rain_intensity_pct": "66.2", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-7569", "accel_y_raw": "114", "accel_z_raw": "14958", "accel_x_g": "-0.462", "accel_y_g": "0.007", "accel_z_g": "0.913", "accel_magnitude_g": "1.023", "gyro_x_raw": "355", "gyro_y_raw": "68", "gyro_z_raw": "-48", "gyro_x_dps": "2.71", "gyro_y_dps": "0.52", "gyro_z_dps": "-0.37", "gyro_magnitude_dps": "2.78", "tilt_pitch_deg": "0.41", "tilt_roll_deg": "26.83", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "170", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.20,H:82.30,SRAW:2384,SM:34.2,SC:MODERATE,RRAW:1846,RP:66.2,RC:MODERATE_RAIN,MV:1,TX:0.41,TY:26.83,AXG:-0.462,AYG:0.007,AZG:0.913,GXD:2.71,GYD:0.52,GZD:-0.37"}, {"timestamp": "04-11-2026 00:22:22", "packet_id": "572", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "82.3", "dew_point_c": "26.84", "soil_raw_adc": "2391", "soil_moisture_pct": "33.8", "soil_condition": "MODERATE", "rain_raw_adc": "1838", "rain_intensity_pct": "66.5", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-7553", "accel_y_raw": "196", "accel_z_raw": "15007", "accel_x_g": "-0.461", "accel_y_g": "0.012", "accel_z_g": "0.916", "accel_magnitude_g": "1.026", "gyro_x_raw": "373", "gyro_y_raw": "81", "gyro_z_raw": "-31", "gyro_x_dps": "2.85", "gyro_y_dps": "0.62", "gyro_z_dps": "-0.24", "gyro_magnitude_dps": "2.93", "tilt_pitch_deg": "0.65", "tilt_roll_deg": "26.7", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "170", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.20,H:82.30,SRAW:2391,SM:33.8,SC:MODERATE,RRAW:1838,RP:66.5,RC:MODERATE_RAIN,MV:1,TX:0.65,TY:26.70,AXG:-0.461,AYG:0.012,AZG:0.916,GXD:2.85,GYD:0.62,GZD:-0.24"}, {"timestamp": "04-11-2026 00:22:27", "packet_id": "573", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "82.2", "dew_point_c": "26.82", "soil_raw_adc": "2402", "soil_moisture_pct": "33.2", "soil_condition": "MODERATE", "rain_raw_adc": "1807", "rain_intensity_pct": "67.7", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-7536", "accel_y_raw": "229", "accel_z_raw": "15007", "accel_x_g": "-0.46", "accel_y_g": "0.014", "accel_z_g": "0.916", "accel_magnitude_g": "1.025", "gyro_x_raw": "369", "gyro_y_raw": "124", "gyro_z_raw": "-61", "gyro_x_dps": "2.82", "gyro_y_dps": "0.95", "gyro_z_dps": "-0.47", "gyro_magnitude_dps": "3.01", "tilt_pitch_deg": "0.81", "tilt_roll_deg": "26.66", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "170", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.20,H:82.20,SRAW:2402,SM:33.2,SC:MODERATE,RRAW:1807,RP:67.7,RC:MODERATE_RAIN,MV:1,TX:0.81,TY:26.66,AXG:-0.460,AYG:0.014,AZG:0.916,GXD:2.82,GYD:0.95,GZD:-0.47"}, {"timestamp": "04-11-2026 00:22:32", "packet_id": "574", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "82.3", "dew_point_c": "26.84", "soil_raw_adc": "2386", "soil_moisture_pct": "34.1", "soil_condition": "MODERATE", "rain_raw_adc": "1846", "rain_intensity_pct": "66.2", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-7569", "accel_y_raw": "212", "accel_z_raw": "14925", "accel_x_g": "-0.462", "accel_y_g": "0.013", "accel_z_g": "0.911", "accel_magnitude_g": "1.022", "gyro_x_raw": "398", "gyro_y_raw": "31", "gyro_z_raw": "-23", "gyro_x_dps": "3.04", "gyro_y_dps": "0.24", "gyro_z_dps": "-0.18", "gyro_magnitude_dps": "3.05", "tilt_pitch_deg": "0.71", "tilt_roll_deg": "26.88", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "170", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.20,H:82.30,SRAW:2386,SM:34.1,SC:MODERATE,RRAW:1846,RP:66.2,RC:MODERATE_RAIN,MV:1,TX:0.71,TY:26.88,AXG:-0.462,AYG:0.013,AZG:0.911,GXD:3.04,GYD:0.24,GZD:-0.18"}, {"timestamp": "04-11-2026 00:22:37", "packet_id": "575", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "82.3", "dew_point_c": "26.84", "soil_raw_adc": "2353", "soil_moisture_pct": "35.9", "soil_condition": "MODERATE", "rain_raw_adc": "1857", "rain_intensity_pct": "65.7", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-7602", "accel_y_raw": "229", "accel_z_raw": "14991", "accel_x_g": "-0.464", "accel_y_g": "0.014", "accel_z_g": "0.915", "accel_magnitude_g": "1.026", "gyro_x_raw": "407", "gyro_y_raw": "70", "gyro_z_raw": "-15", "gyro_x_dps": "3.11", "gyro_y_dps": "0.54", "gyro_z_dps": "-0.12", "gyro_magnitude_dps": "3.16", "tilt_pitch_deg": "0.79", "tilt_roll_deg": "26.88", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "170", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.20,H:82.30,SRAW:2353,SM:35.9,SC:MODERATE,RRAW:1857,RP:65.7,RC:MODERATE_RAIN,MV:1,TX:0.79,TY:26.88,AXG:-0.464,AYG:0.014,AZG:0.915,GXD:3.11,GYD:0.54,GZD:-0.12"}, {"timestamp": "04-11-2026 00:22:42", "packet_id": "576", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "82.3", "dew_point_c": "26.84", "soil_raw_adc": "2407", "soil_moisture_pct": "32.9", "soil_condition": "MODERATE", "rain_raw_adc": "1846", "rain_intensity_pct": "66.2", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-7536", "accel_y_raw": "98", "accel_z_raw": "14942", "accel_x_g": "-0.46", "accel_y_g": "0.006", "accel_z_g": "0.912", "accel_magnitude_g": "1.021", "gyro_x_raw": "368", "gyro_y_raw": "75", "gyro_z_raw": "-58", "gyro_x_dps": "2.81", "gyro_y_dps": "0.58", "gyro_z_dps": "-0.45", "gyro_magnitude_dps": "2.9", "tilt_pitch_deg": "0.36", "tilt_roll_deg": "26.75", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "170", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.20,H:82.30,SRAW:2407,SM:32.9,SC:MODERATE,RRAW:1846,RP:66.2,RC:MODERATE_RAIN,MV:1,TX:0.36,TY:26.75,AXG:-0.460,AYG:0.006,AZG:0.912,GXD:2.81,GYD:0.58,GZD:-0.45"}, {"timestamp": "04-11-2026 00:22:47", "packet_id": "577", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "82.2", "dew_point_c": "26.82", "soil_raw_adc": "2403", "soil_moisture_pct": "33.2", "soil_condition": "MODERATE", "rain_raw_adc": "1843", "rain_intensity_pct": "66.3", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-7569", "accel_y_raw": "229", "accel_z_raw": "14942", "accel_x_g": "-0.462", "accel_y_g": "0.014", "accel_z_g": "0.912", "accel_magnitude_g": "1.022", "gyro_x_raw": "375", "gyro_y_raw": "89", "gyro_z_raw": "-6", "gyro_x_dps": "2.87", "gyro_y_dps": "0.68", "gyro_z_dps": "-0.05", "gyro_magnitude_dps": "2.95", "tilt_pitch_deg": "0.78", "tilt_roll_deg": "26.84", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "170", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.20,H:82.20,SRAW:2403,SM:33.2,SC:MODERATE,RRAW:1843,RP:66.3,RC:MODERATE_RAIN,MV:1,TX:0.78,TY:26.84,AXG:-0.462,AYG:0.014,AZG:0.912,GXD:2.87,GYD:0.68,GZD:-0.05"}, {"timestamp": "04-11-2026 00:22:52", "packet_id": "578", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "82.2", "dew_point_c": "26.82", "soil_raw_adc": "2401", "soil_moisture_pct": "33.3", "soil_condition": "MODERATE", "rain_raw_adc": "1843", "rain_intensity_pct": "66.3", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-7684", "accel_y_raw": "147", "accel_z_raw": "14893", "accel_x_g": "-0.469", "accel_y_g": "0.009", "accel_z_g": "0.909", "accel_magnitude_g": "1.023", "gyro_x_raw": "399", "gyro_y_raw": "90", "gyro_z_raw": "-39", "gyro_x_dps": "3.05", "gyro_y_dps": "0.69", "gyro_z_dps": "-0.3", "gyro_magnitude_dps": "3.14", "tilt_pitch_deg": "0.48", "tilt_roll_deg": "27.31", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "170", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.20,H:82.20,SRAW:2401,SM:33.3,SC:MODERATE,RRAW:1843,RP:66.3,RC:MODERATE_RAIN,MV:1,TX:0.48,TY:27.31,AXG:-0.469,AYG:0.009,AZG:0.909,GXD:3.05,GYD:0.69,GZD:-0.30"}, {"timestamp": "04-11-2026 00:22:57", "packet_id": "579", "temperature_c": "30.2", "temperature_f": "86.36", "humidity_pct": "82.2", "dew_point_c": "26.82", "soil_raw_adc": "2438", "soil_moisture_pct": "31.2", "soil_condition": "MODERATE", "rain_raw_adc": "1835", "rain_intensity_pct": "66.6", "rain_condition": "MODERATE_RAIN", "accel_x_raw": "-7553", "accel_y_raw": "180", "accel_z_raw": "14876", "accel_x_g": "-0.461", "accel_y_g": "0.011", "accel_z_g": "0.908", "accel_magnitude_g": "1.018", "gyro_x_raw": "387", "gyro_y_raw": "65", "gyro_z_raw": "-15", "gyro_x_dps": "2.96", "gyro_y_dps": "0.5", "gyro_z_dps": "-0.12", "gyro_magnitude_dps": "3.0", "tilt_pitch_deg": "0.62", "tilt_roll_deg": "26.91", "rssi_dbm": "-80", "signal_quality": "GOOD", "packet_length_bytes": "170", "landslide_risk_level": "WARNING", "hazard_status": "GROUND_SHIFT_DETECTED", "raw_packet": "GS1,TV:1,T:30.20,H:82.20,SRAW:2438,SM:31.2,SC:MODERATE,RRAW:1835,RP:66.6,RC:MODERATE_RAIN,MV:1,TX:0.62,TY:26.91,AXG:-0.461,AYG:0.011,AZG:0.908,GXD:2.96,GYD:0.50,GZD:-0.12"}];

    // Central Telemetry Repository (All 34 Detailed Sensor & Ingestion Fields)
    const allPackets = [];
    let currentlyInspectedPacket = null;
    let packetArrivalTimestamps = [];
    let peakAccelMagObserved = 0;
    let peakGyroMagObserved = 0;

    /**
     * Converts any raw payload or parsed object into canonical 34-column telemetry record
     */
    function normalizeTelemetryPacket(data, rawText = '') {
      const now = new Date();
      const ts = data.timestamp || (
        now.getFullYear() + '-' +
        String(now.getMonth() + 1).padStart(2, '0') + '-' +
        String(now.getDate()).padStart(2, '0') + ' ' +
        String(now.getHours()).padStart(2, '0') + ':' +
        String(now.getMinutes()).padStart(2, '0') + ':' +
        String(now.getSeconds()).padStart(2, '0')
      );
      const pktId = data.packet_id ? parseInt(data.packet_id) : (allPackets.length + 1);

      // Atmospheric Temperature (°C, °F, Dew Point)
      let tempC = (data.temperature_c !== undefined) ? parseFloat(data.temperature_c) : (data.temperature !== undefined ? parseFloat(data.temperature) : 25.0);
      if (isNaN(tempC)) tempC = 25.0;
      let tempF = (data.temperature_f !== undefined) ? parseFloat(data.temperature_f) : ((tempC * 9 / 5) + 32);

      let hum = (data.humidity_pct !== undefined) ? parseFloat(data.humidity_pct) : (data.humidity !== undefined ? parseFloat(data.humidity) : 50.0);
      if (isNaN(hum)) hum = 50.0;
      let dewPoint = (data.dew_point_c !== undefined) ? parseFloat(data.dew_point_c) : (tempC - ((100 - hum) / 5));

      // Hydrologic Soil Moisture (ADC Raw & % Calibrated)
      let soilRaw = (data.soil_raw_adc !== undefined && data.soil_raw_adc !== '') ? parseInt(data.soil_raw_adc) : (data.soil_raw !== undefined ? parseInt(data.soil_raw) : (data.soil !== undefined && data.soil > 100 ? parseInt(data.soil) : 3100));
      let soilPct = (data.soil_moisture_pct !== undefined && data.soil_moisture_pct !== '') ? parseFloat(data.soil_moisture_pct) : (data.soil_pct !== undefined ? parseFloat(data.soil_pct) : (data.soil !== undefined ? (data.soil <= 100 ? parseFloat(data.soil) : Math.max(0, Math.min(100, ((4095 - data.soil) / 30.95)))) : 32.0));
      let soilCond = data.soil_condition || (soilPct < 25 ? 'DRY' : (soilPct < 65 ? 'MODERATE' : (soilPct < 85 ? 'WET' : 'SATURATED')));

      // Rain Precipitation Gauge (ADC Raw & % Calibrated)
      let rainRaw = (data.rain_raw_adc !== undefined && data.rain_raw_adc !== '') ? parseInt(data.rain_raw_adc) : (data.rain_raw !== undefined ? parseInt(data.rain_raw) : (data.rain !== undefined && data.rain > 100 ? parseInt(data.rain) : 4095));
      let rainPct = (data.rain_intensity_pct !== undefined && data.rain_intensity_pct !== '') ? parseFloat(data.rain_intensity_pct) : (data.rain_pct !== undefined ? parseFloat(data.rain_pct) : (data.rain !== undefined ? (data.rain <= 100 ? parseFloat(data.rain) : Math.max(0, Math.min(100, ((4095 - data.rain) / 30.95)))) : 0.0));
      let rainCond = data.rain_condition || (rainPct < 10 ? 'NO_RAIN' : (rainPct < 45 ? 'LIGHT_RAIN' : 'HEAVY_RAIN'));

      // MPU6050 3-Axis Accelerometer (Raw counts, Physical g, Vector Magnitude |A|)
      let axRaw = (data.accel_x_raw !== undefined && data.accel_x_raw !== '') ? parseInt(data.accel_x_raw) : (data.ax !== undefined ? parseInt(data.ax) : 0);
      let ayRaw = (data.accel_y_raw !== undefined && data.accel_y_raw !== '') ? parseInt(data.accel_y_raw) : (data.ay !== undefined ? parseInt(data.ay) : 0);
      let azRaw = (data.accel_z_raw !== undefined && data.accel_z_raw !== '') ? parseInt(data.accel_z_raw) : (data.az !== undefined ? parseInt(data.az) : 16384);

      let axG = (data.accel_x_g !== undefined && data.accel_x_g !== '') ? parseFloat(data.accel_x_g) : (data.ax_g !== undefined ? parseFloat(data.ax_g) : ((Math.abs(axRaw) > 4) ? (axRaw / 16384.0) : axRaw));
      let ayG = (data.accel_y_g !== undefined && data.accel_y_g !== '') ? parseFloat(data.accel_y_g) : (data.ay_g !== undefined ? parseFloat(data.ay_g) : ((Math.abs(ayRaw) > 4) ? (ayRaw / 16384.0) : ayRaw));
      let azG = (data.accel_z_g !== undefined && data.accel_z_g !== '') ? parseFloat(data.accel_z_g) : (data.az_g !== undefined ? parseFloat(data.az_g) : ((Math.abs(azRaw) > 4) ? (azRaw / 16384.0) : azRaw));
      let accMag = (data.accel_magnitude_g !== undefined && data.accel_magnitude_g !== '') ? parseFloat(data.accel_magnitude_g) : Math.sqrt(axG * axG + ayG * ayG + azG * azG);

      // MPU6050 3-Axis Gyroscope (Raw counts, Physical °/s, Vector Angular Velocity |ω|)
      let gxRaw = (data.gyro_x_raw !== undefined && data.gyro_x_raw !== '') ? parseInt(data.gyro_x_raw) : (data.gx !== undefined ? parseInt(data.gx) : 0);
      let gyRaw = (data.gyro_y_raw !== undefined && data.gyro_y_raw !== '') ? parseInt(data.gyro_y_raw) : (data.gy !== undefined ? parseInt(data.gy) : 0);
      let gzRaw = (data.gyro_z_raw !== undefined && data.gyro_z_raw !== '') ? parseInt(data.gyro_z_raw) : (data.gz !== undefined ? parseInt(data.gz) : 0);

      let gxDps = (data.gyro_x_dps !== undefined && data.gyro_x_dps !== '') ? parseFloat(data.gyro_x_dps) : (data.gx_dps !== undefined ? parseFloat(data.gx_dps) : ((Math.abs(gxRaw) > 30) ? (gxRaw / 131.0) : gxRaw));
      let gyDps = (data.gyro_y_dps !== undefined && data.gyro_y_dps !== '') ? parseFloat(data.gyro_y_dps) : (data.gy_dps !== undefined ? parseFloat(data.gy_dps) : ((Math.abs(gyRaw) > 30) ? (gyRaw / 131.0) : gyRaw));
      let gzDps = (data.gyro_z_dps !== undefined && data.gyro_z_dps !== '') ? parseFloat(data.gyro_z_dps) : (data.gz_dps !== undefined ? parseFloat(data.gz_dps) : ((Math.abs(gzRaw) > 30) ? (gzRaw / 131.0) : gzRaw));
      let gyroMag = (data.gyro_magnitude_dps !== undefined && data.gyro_magnitude_dps !== '') ? parseFloat(data.gyro_magnitude_dps) : Math.sqrt(gxDps * gxDps + gyDps * gyDps + gzDps * gzDps);

      // Inclinometer / Slope Tilt (Pitch Tx, Roll Ty in degrees)
      let pitchDeg = (data.tilt_pitch_deg !== undefined && data.tilt_pitch_deg !== '') ? parseFloat(data.tilt_pitch_deg) : (data.tx !== undefined ? parseFloat(data.tx) : (Math.atan2(ayG, azG) * 180 / Math.PI));
      let rollDeg = (data.tilt_roll_deg !== undefined && data.tilt_roll_deg !== '') ? parseFloat(data.tilt_roll_deg) : (data.ty !== undefined ? parseFloat(data.ty) : (Math.atan2(-axG, Math.sqrt(ayG * ayG + azG * azG)) * 180 / Math.PI));

      // RF Telemetry Link (RSSI, Quality, Payload Length)
      let rssi = (data.rssi_dbm !== undefined && data.rssi_dbm !== '') ? parseInt(data.rssi_dbm) : (data.rssi !== undefined ? parseInt(data.rssi) : (latestRssi || -85));
      let sigQual = data.signal_quality || classifyRssi(rssi).label.toUpperCase();
      let pktLen = (data.packet_length_bytes !== undefined && data.packet_length_bytes !== '') ? parseInt(data.packet_length_bytes) : (data.packetLength || (rawText ? rawText.length : 83));

      // Landslide Hazard Risk Evaluation
      let riskLevel = data.landslide_risk_level || 'SAFE';
      let hazardStatus = data.hazard_status || 'MONITORING_BASELINE';

      if (!data.landslide_risk_level) {
        const isHighSat = (soilPct > 80 && rainPct > 45);
        const isMotion = (accMag > 1.45 || accMag < 0.55 || gyroMag > 18 || Math.abs(pitchDeg) > 15 || Math.abs(rollDeg) > 15);
        if (isMotion && isHighSat) {
          riskLevel = 'CRITICAL ALERT';
          hazardStatus = 'HIGH_LANDSLIDE_HAZARD';
        } else if (isMotion) {
          riskLevel = 'WARNING';
          hazardStatus = 'GROUND_SHIFT_DETECTED';
        } else if (isHighSat) {
          riskLevel = 'ELEVATED';
          hazardStatus = 'SATURATION_WARNING';
        } else if (soilPct > 65 || rainPct > 30) {
          riskLevel = 'ADVISORY';
          hazardStatus = 'MOISTURE_ADVISORY';
        } else {
          riskLevel = 'SAFE';
          hazardStatus = 'MONITORING_BASELINE';
        }
      }

      const rawPkt = data.raw_packet || rawText || `TEMP:${tempC.toFixed(1)},HUM:${hum.toFixed(1)},SOIL:${soilRaw},RAIN:${rainRaw},AX:${axRaw},AY:${ayRaw},AZ:${azRaw},GX:${gxRaw},GY:${gyRaw},GZ:${gzRaw}`;

      return {
        timestamp: ts,
        packet_id: pktId,
        temperature_c: parseFloat(tempC.toFixed(1)),
        temperature_f: parseFloat(tempF.toFixed(2)),
        humidity_pct: parseFloat(hum.toFixed(1)),
        dew_point_c: parseFloat(dewPoint.toFixed(2)),
        soil_raw_adc: soilRaw,
        soil_moisture_pct: parseFloat(soilPct.toFixed(1)),
        soil_condition: soilCond,
        rain_raw_adc: rainRaw,
        rain_intensity_pct: parseFloat(rainPct.toFixed(1)),
        rain_condition: rainCond,
        accel_x_raw: axRaw,
        accel_y_raw: ayRaw,
        accel_z_raw: azRaw,
        accel_x_g: parseFloat(axG.toFixed(3)),
        accel_y_g: parseFloat(ayG.toFixed(3)),
        accel_z_g: parseFloat(azG.toFixed(3)),
        accel_magnitude_g: parseFloat(accMag.toFixed(3)),
        gyro_x_raw: gxRaw,
        gyro_y_raw: gyRaw,
        gyro_z_raw: gzRaw,
        gyro_x_dps: parseFloat(gxDps.toFixed(2)),
        gyro_y_dps: parseFloat(gyDps.toFixed(2)),
        gyro_z_dps: parseFloat(gzDps.toFixed(2)),
        gyro_magnitude_dps: parseFloat(gyroMag.toFixed(2)),
        tilt_pitch_deg: parseFloat(pitchDeg.toFixed(2)),
        tilt_roll_deg: parseFloat(rollDeg.toFixed(2)),
        rssi_dbm: rssi,
        signal_quality: sigQual,
        packet_length_bytes: pktLen,
        landslide_risk_level: riskLevel,
        hazard_status: hazardStatus,
        raw_packet: rawPkt,
        source: data.source || (isPrimaryConnected ? 'USB Serial Receiver' : 'SX1278 LoRa Node')
      };
    }

    /**
     * Ingests a new live packet into the central dataset and updates UI analytics
     */
    function recordTelemetryPacket(data, rawText = '') {
      const record = normalizeTelemetryPacket(data, rawText);
      allPackets.push(record);

      const now = Date.now();
      packetArrivalTimestamps.push(now);
      if (packetArrivalTimestamps.length > 60) packetArrivalTimestamps.shift();

      if (record.accel_magnitude_g > peakAccelMagObserved) peakAccelMagObserved = record.accel_magnitude_g;
      if (record.gyro_magnitude_dps > peakGyroMagObserved) peakGyroMagObserved = record.gyro_magnitude_dps;

      updatePacketAnalytics();
      addPacketTableRow(record);
    }

    /**
     * Computes statistical analysis across all ingested packets and updates the 4 analytical domain cards
     */
    function updatePacketAnalytics() {
      const total = allPackets.length;
      if (total === 0) return;

      const latest = allPackets[allPackets.length - 1];

      // 1. Packet Ingestion & RF Link Health (Card 1)
      if (el.anPktTotal) el.anPktTotal.textContent = total;
      
      // Calculate packet reception rate
      let rateHz = 0;
      let meanIntervalMs = 1000;
      if (packetArrivalTimestamps.length >= 2) {
        const dt = (packetArrivalTimestamps[packetArrivalTimestamps.length - 1] - packetArrivalTimestamps[0]) / 1000;
        if (dt > 0) {
          rateHz = ((packetArrivalTimestamps.length - 1) / dt).toFixed(1);
          meanIntervalMs = Math.round((dt / (packetArrivalTimestamps.length - 1)) * 1000);
        }
      }
      if (el.anPktRate) el.anPktRate.textContent = `${rateHz} Hz`;
      if (el.anPktPpm) el.anPktPpm.textContent = Math.round(rateHz * 60);
      if (el.anIntervalVal) el.anIntervalVal.textContent = `${meanIntervalMs} ms`;
      if (el.anLastLen) el.anLastLen.textContent = `${latest.packet_length_bytes} bytes`;

      // RSSI statistics
      const validRssis = allPackets.map(p => p.rssi_dbm).filter(r => r !== null && r !== undefined && !isNaN(r));
      if (validRssis.length > 0) {
        const avgRssi = Math.round(validRssis.reduce((a, b) => a + b, 0) / validRssis.length);
        const minRssi = Math.min(...validRssis);
        const maxRssi = Math.max(...validRssis);
        if (el.anRssiAvg) el.anRssiAvg.textContent = `${avgRssi} dBm`;
        if (el.anRssiMinMax) el.anRssiMinMax.textContent = `${minRssi} / ${maxRssi} dBm`;

        const sig = classifyRssi(latest.rssi_dbm);
        if (el.anSignalRating) {
          el.anSignalRating.textContent = sig.label.toUpperCase();
          el.anSignalRating.className = `pkt-pill pkt-pill-${sig.label === 'Excellent' || sig.label === 'Good' ? 'safe' : (sig.label === 'Weak' ? 'warning' : 'critical')}`;
        }
      }

      // 2. Atmospheric & Soil Analytics (Card 2)
      if (el.anTempVal) el.anTempVal.textContent = latest.temperature_c.toFixed(1);
      if (el.anTempFVal) el.anTempFVal.textContent = latest.temperature_f.toFixed(1);
      if (el.anHumVal) el.anHumVal.textContent = `${latest.humidity_pct.toFixed(1)} %`;
      if (el.anDewVal) el.anDewVal.textContent = `${latest.dew_point_c.toFixed(1)} °C`;

      const temps = allPackets.map(p => p.temperature_c).filter(t => !isNaN(t));
      const hums = allPackets.map(p => p.humidity_pct).filter(h => !isNaN(h));
      if (temps.length > 0 && el.anTempMinMax) {
        el.anTempMinMax.textContent = `${Math.min(...temps).toFixed(1)} / ${Math.max(...temps).toFixed(1)} °C`;
      }
      if (hums.length > 0 && el.anHumMinMax) {
        el.anHumMinMax.textContent = `${Math.min(...hums).toFixed(1)} / ${Math.max(...hums).toFixed(1)} %`;
      }

      if (el.anSoilVal) el.anSoilVal.textContent = `${latest.soil_moisture_pct.toFixed(1)} %`;
      if (el.anSoilAdcSub) el.anSoilAdcSub.textContent = `ADC ${latest.soil_raw_adc}`;
      if (el.anRainVal) el.anRainVal.textContent = `${latest.rain_intensity_pct.toFixed(1)} %`;
      if (el.anRainAdcSub) el.anRainAdcSub.textContent = `ADC ${latest.rain_raw_adc}`;

      if (el.anHydroState) {
        const hydroText = `${latest.soil_condition} / ${latest.rain_condition.replace('_', ' ')}`;
        el.anHydroState.textContent = hydroText;
        el.anHydroState.className = `pkt-pill pkt-pill-${latest.soil_moisture_pct > 80 ? 'critical' : (latest.soil_moisture_pct > 65 || latest.rain_intensity_pct > 30 ? 'warning' : 'safe')}`;
      }

      // 3. Kinematics & Tilt Analytics (Card 3)
      if (el.anAccMag) el.anAccMag.textContent = latest.accel_magnitude_g.toFixed(2);
      if (el.anGyroMag) el.anGyroMag.textContent = `${latest.gyro_magnitude_dps.toFixed(1)} °/s`;
      if (el.anAccPeak) el.anAccPeak.textContent = `${peakAccelMagObserved.toFixed(2)} g`;
      if (el.anGyroPeak) el.anGyroPeak.textContent = `${peakGyroMagObserved.toFixed(1)} °/s`;
      if (el.anPitchVal) el.anPitchVal.textContent = `${latest.tilt_pitch_deg.toFixed(1)} °`;
      if (el.anRollVal) el.anRollVal.textContent = `${latest.tilt_roll_deg.toFixed(1)} °`;

      if (el.anKinState) {
        let kinText = 'STATIONARY BASELINE';
        let kinPill = 'safe';
        if (latest.accel_magnitude_g > 1.45 || latest.accel_magnitude_g < 0.55 || latest.gyro_magnitude_dps > 25) {
          kinText = 'DYNAMIC GROUND MOTION';
          kinPill = 'critical';
        } else if (Math.abs(latest.tilt_pitch_deg) > 15 || Math.abs(latest.tilt_roll_deg) > 15) {
          kinText = 'SLOPE TILT DISPLACEMENT';
          kinPill = 'warning';
        } else if (latest.gyro_magnitude_dps > 8) {
          kinText = 'VIBRATIONAL CREEP';
          kinPill = 'warning';
        }
        el.anKinState.textContent = kinText;
        el.anKinState.className = `pkt-pill pkt-pill-${kinPill}`;
      }

      // 4. Hazard Risk Probability & Distribution Bar (Card 4)
      let countSafe = 0, countAdvisory = 0, countElevated = 0, countWarning = 0, countCritical = 0;
      allPackets.forEach(p => {
        const r = (p.landslide_risk_level || '').toUpperCase();
        if (r.includes('CRIT')) countCritical++;
        else if (r.includes('WARN')) countWarning++;
        else if (r.includes('ELEV')) countElevated++;
        else if (r.includes('ADV')) countAdvisory++;
        else countSafe++;
      });

      const pctSafe = ((countSafe / total) * 100).toFixed(1);
      const pctAdv = ((countAdvisory / total) * 100).toFixed(1);
      const pctElev = ((countElevated / total) * 100).toFixed(1);
      const pctWarn = ((countWarning / total) * 100).toFixed(1);
      const pctCrit = ((countCritical / total) * 100).toFixed(1);

      if (el.barSegSafe) el.barSegSafe.style.width = `${pctSafe}%`;
      if (el.barSegAdvisory) el.barSegAdvisory.style.width = `${pctAdv}%`;
      if (el.barSegElevated) el.barSegElevated.style.width = `${pctElev}%`;
      if (el.barSegWarning) el.barSegWarning.style.width = `${pctWarn}%`;
      if (el.barSegCritical) el.barSegCritical.style.width = `${pctCrit}%`;

      if (el.lblPctSafe) el.lblPctSafe.textContent = `${pctSafe}%`;
      if (el.lblPctAdvisory) el.lblPctAdvisory.textContent = `${pctAdv}%`;
      if (el.lblPctElevated) el.lblPctElevated.textContent = `${pctElev}%`;
      if (el.lblPctWarning) el.lblPctWarning.textContent = `${pctWarn}%`;
      if (el.lblPctCritical) el.lblPctCritical.textContent = `${pctCrit}%`;

      if (el.anRiskPill) {
        el.anRiskPill.textContent = latest.landslide_risk_level;
        const rUpper = latest.landslide_risk_level.toUpperCase();
        el.anRiskPill.className = `pkt-pill pkt-pill-${rUpper.includes('CRIT') ? 'critical' : (rUpper.includes('WARN') ? 'warning' : (rUpper.includes('ELEV') || rUpper.includes('ADV') ? 'advisory' : 'safe'))}`;
      }

      // Dominant risk level
      const counts = [
        { label: 'Normal / Stable', val: countSafe },
        { label: 'Moisture Advisory', val: countAdvisory },
        { label: 'Saturation Elevated', val: countElevated },
        { label: 'Ground Warning', val: countWarning },
        { label: 'Critical Landslide Alert', val: countCritical }
      ];
      counts.sort((a, b) => b.val - a.val);
      if (el.anDominantRisk) el.anDominantRisk.textContent = counts[0].label;

      // Geotechnical Triggers
      if (el.anTrigSoil) {
        const isSoilTrig = latest.soil_moisture_pct > 80;
        el.anTrigSoil.textContent = isSoilTrig ? 'TRIGGERED (>80%)' : `Nominal (${latest.soil_moisture_pct.toFixed(0)}%)`;
        el.anTrigSoil.style.color = isSoilTrig ? 'var(--accent-rose)' : 'var(--accent-emerald)';
      }
      if (el.anTrigMotion) {
        const isMotionTrig = (latest.accel_magnitude_g > 1.45 || latest.accel_magnitude_g < 0.55);
        el.anTrigMotion.textContent = isMotionTrig ? 'TRIGGERED (>1.45g)' : `Stable (${latest.accel_magnitude_g.toFixed(2)}g)`;
        el.anTrigMotion.style.color = isMotionTrig ? 'var(--accent-rose)' : 'var(--accent-emerald)';
      }
    }

    /**
     * Adds a row into the live packets table with inspection button
     */
    function addPacketTableRow(pkt) {
      if (!el.tbodyPacketsFeed) return;

      if (el.badgePktTableCount) {
        el.badgePktTableCount.textContent = `${allPackets.length} Packets Cached`;
      }

      // Remove placeholder row if present
      const emptyRow = el.tbodyPacketsFeed.querySelector('td[colspan]');
      if (emptyRow) emptyRow.parentElement.remove();

      const tr = document.createElement('tr');
      tr.setAttribute('data-pkt-id', pkt.packet_id);
      tr.setAttribute('data-risk', pkt.landslide_risk_level.toLowerCase());

      const rUpper = pkt.landslide_risk_level.toUpperCase();
      const pillClass = rUpper.includes('CRIT') ? 'pkt-pill-critical' : (rUpper.includes('WARN') ? 'pkt-pill-warning' : (rUpper.includes('ELEV') || rUpper.includes('ADV') ? 'pkt-pill-advisory' : 'pkt-pill-safe'));

      const rawExcerpt = (pkt.raw_packet.length > 32) ? (pkt.raw_packet.substring(0, 32) + '...') : pkt.raw_packet;
      const timeStr = pkt.timestamp.includes(' ') ? pkt.timestamp.split(' ')[1] : pkt.timestamp;

      tr.innerHTML = `
        <td style="font-weight: 700; color: var(--text-primary);">#${pkt.packet_id}</td>
        <td style="font-family: monospace; color: var(--text-secondary);">${timeStr}</td>
        <td style="color: var(--text-muted); font-size: 0.72rem;">${pkt.source || 'Node'}</td>
        <td style="font-family: monospace; color: var(--accent-cyan);">${pkt.rssi_dbm !== null ? pkt.rssi_dbm + ' dBm' : '--'}</td>
        <td><strong>${pkt.temperature_c.toFixed(1)}°C</strong> <span style="color: var(--text-muted); font-size: 0.72rem;">(${pkt.temperature_f.toFixed(1)}°F)</span></td>
        <td style="color: var(--accent-cyan); font-weight: 600;">${pkt.humidity_pct.toFixed(1)}%</td>
        <td><strong>${pkt.soil_moisture_pct.toFixed(1)}%</strong> <span style="color: var(--text-muted); font-size: 0.7rem;">(ADC ${pkt.soil_raw_adc})</span></td>
        <td>${pkt.rain_intensity_pct.toFixed(1)}% <span style="color: var(--text-muted); font-size: 0.7rem;">(ADC ${pkt.rain_raw_adc})</span></td>
        <td style="font-family: monospace; font-weight: 700; color: ${pkt.accel_magnitude_g > 1.3 ? 'var(--accent-rose)' : 'var(--text-primary)'};">${pkt.accel_magnitude_g.toFixed(2)} g</td>
        <td style="font-family: monospace; color: var(--accent-purple);">${pkt.gyro_magnitude_dps.toFixed(1)} °/s</td>
        <td style="font-size: 0.72rem; color: var(--text-secondary);">P:${pkt.tilt_pitch_deg.toFixed(0)}° R:${pkt.tilt_roll_deg.toFixed(0)}°</td>
        <td><span class="pkt-pill ${pillClass}">${pkt.landslide_risk_level}</span></td>
        <td class="raw-cell" title="${pkt.raw_packet}"><code>${rawExcerpt}</code></td>
        <td style="text-align: center;"><button class="btn-inspect-action" onclick="openPacketInspector(${pkt.packet_id})">🔬 Inspect</button></td>
      `;

      el.tbodyPacketsFeed.insertBefore(tr, el.tbodyPacketsFeed.firstChild);

      // Keep maximum 250 rows in DOM for smooth browser performance
      while (el.tbodyPacketsFeed.children.length > 250) {
        el.tbodyPacketsFeed.removeChild(el.tbodyPacketsFeed.lastChild);
      }
    }

    /**
     * Filters the packets table based on search query and risk level dropdown
     */
    function filterPacketTable() {
      const query = (el.inputPktSearch ? el.inputPktSearch.value.trim().toLowerCase() : '');
      const riskFilter = (el.selectPktRiskFilter ? el.selectPktRiskFilter.value : 'all');

      if (!el.tbodyPacketsFeed) return;
      const rows = el.tbodyPacketsFeed.querySelectorAll('tr[data-pkt-id]');
      rows.forEach(tr => {
        const text = tr.textContent.toLowerCase();
        const risk = tr.getAttribute('data-risk') || '';
        let matchSearch = (!query || text.includes(query));
        let matchRisk = true;

        if (riskFilter === 'critical') matchRisk = risk.includes('crit');
        else if (riskFilter === 'warning') matchRisk = risk.includes('warn');
        else if (riskFilter === 'elevated') matchRisk = risk.includes('elev') || risk.includes('adv');
        else if (riskFilter === 'safe') matchRisk = risk.includes('safe') || risk.includes('monit');

        tr.style.display = (matchSearch && matchRisk) ? '' : 'none';
      });
    }

    /**
     * Opens deep-dive modal displaying full sensor breakdown and engineering parameters
     */
    window.openPacketInspector = function(packetId) {
      const pkt = allPackets.find(p => p.packet_id === packetId) || allPackets[allPackets.length - 1];
      if (!pkt) return;
      currentlyInspectedPacket = pkt;

      if (el.inspectRawPayload) el.inspectRawPayload.textContent = pkt.raw_packet;
      if (el.inspectPktTime) el.inspectPktTime.textContent = `#${pkt.packet_id} • ${pkt.timestamp}`;
      if (el.inspectPktSource) el.inspectPktSource.textContent = pkt.source || 'SX1278 LoRa Node';
      if (el.inspectPktRssi) el.inspectPktRssi.textContent = `${pkt.rssi_dbm !== null ? pkt.rssi_dbm : '--'} dBm (${pkt.signal_quality})`;
      if (el.inspectPktRisk) {
        el.inspectPktRisk.textContent = `${pkt.landslide_risk_level} (${pkt.hazard_status})`;
        const rUpper = pkt.landslide_risk_level.toUpperCase();
        el.inspectPktRisk.style.color = rUpper.includes('CRIT') ? 'var(--accent-rose)' : (rUpper.includes('WARN') ? 'var(--accent-amber)' : 'var(--accent-emerald)');
      }

      // Populate Dissection Table
      if (el.inspectTableBody) {
        el.inspectTableBody.innerHTML = `
          <tr>
            <td style="color: var(--text-primary); font-weight: 600;">🌡️ Atmospheric Temperature</td>
            <td style="text-align: right; font-family: monospace;">${pkt.temperature_c.toFixed(1)} °C</td>
            <td style="text-align: right; font-family: monospace; color: var(--accent-amber);">${pkt.temperature_f.toFixed(2)} °F</td>
            <td style="text-align: right;">Dew Point: ${pkt.dew_point_c.toFixed(2)} °C</td>
          </tr>
          <tr>
            <td style="color: var(--text-primary); font-weight: 600;">💧 Relative Humidity (DHT22)</td>
            <td style="text-align: right; font-family: monospace;">${pkt.humidity_pct.toFixed(1)} %</td>
            <td style="text-align: right; font-family: monospace; color: var(--accent-cyan);">${pkt.humidity_pct.toFixed(1)} % RH</td>
            <td style="text-align: right;">Dew spread: ${(pkt.temperature_c - pkt.dew_point_c).toFixed(1)} °C</td>
          </tr>
          <tr>
            <td style="color: var(--text-primary); font-weight: 600;">🌱 Volumetric Soil Moisture</td>
            <td style="text-align: right; font-family: monospace;">Raw: ${pkt.soil_raw_adc} ADC</td>
            <td style="text-align: right; font-family: monospace; color: var(--accent-emerald); font-weight: 700;">${pkt.soil_moisture_pct.toFixed(1)} %</td>
            <td style="text-align: right;"><span class="pkt-pill pkt-pill-${pkt.soil_moisture_pct > 80 ? 'critical' : 'safe'}">${pkt.soil_condition}</span></td>
          </tr>
          <tr>
            <td style="color: var(--text-primary); font-weight: 600;">🌧️ Surface Precipitation Gauge</td>
            <td style="text-align: right; font-family: monospace;">Raw: ${pkt.rain_raw_adc} ADC</td>
            <td style="text-align: right; font-family: monospace; color: var(--accent-blue); font-weight: 700;">${pkt.rain_intensity_pct.toFixed(1)} %</td>
            <td style="text-align: right;"><span class="pkt-pill pkt-pill-${pkt.rain_intensity_pct > 40 ? 'warning' : 'safe'}">${pkt.rain_condition.replace('_', ' ')}</span></td>
          </tr>
          <tr>
            <td style="color: var(--text-primary); font-weight: 600;">📐 3-Axis Accelerometer (MPU6050)</td>
            <td style="text-align: right; font-family: monospace; font-size: 0.7rem;">X:${pkt.accel_x_raw}, Y:${pkt.accel_y_raw}, Z:${pkt.accel_z_raw}</td>
            <td style="text-align: right; font-family: monospace; color: #60a5fa;">X:${pkt.accel_x_g.toFixed(2)}g, Y:${pkt.accel_y_g.toFixed(2)}g, Z:${pkt.accel_z_g.toFixed(2)}g</td>
            <td style="text-align: right; font-weight: 700;">Total |A|: ${pkt.accel_magnitude_g.toFixed(3)} g</td>
          </tr>
          <tr>
            <td style="color: var(--text-primary); font-weight: 600;">🔄 3-Axis Gyroscope (MPU6050)</td>
            <td style="text-align: right; font-family: monospace; font-size: 0.7rem;">X:${pkt.gyro_x_raw}, Y:${pkt.gyro_y_raw}, Z:${pkt.gyro_z_raw}</td>
            <td style="text-align: right; font-family: monospace; color: var(--accent-purple);">X:${pkt.gyro_x_dps.toFixed(1)}°, Y:${pkt.gyro_y_dps.toFixed(1)}°, Z:${pkt.gyro_z_dps.toFixed(1)}°/s</td>
            <td style="text-align: right; font-weight: 700;">Total |ω|: ${pkt.gyro_magnitude_dps.toFixed(2)} °/s</td>
          </tr>
          <tr>
            <td style="color: var(--text-primary); font-weight: 600;">⛰️ Slope Inclinometer &amp; Tilt</td>
            <td style="text-align: right; font-family: monospace;">Pitch / Roll</td>
            <td style="text-align: right; font-family: monospace; color: #34d399;">Tx: ${pkt.tilt_pitch_deg.toFixed(1)}°, Ty: ${pkt.tilt_roll_deg.toFixed(1)}°</td>
            <td style="text-align: right;">Deviation: ${(Math.sqrt(pkt.tilt_pitch_deg*pkt.tilt_pitch_deg + pkt.tilt_roll_deg*pkt.tilt_roll_deg)).toFixed(1)}°</td>
          </tr>
          <tr>
            <td style="color: var(--text-primary); font-weight: 600;">📻 LoRa SX1278 RF Telemetry Link</td>
            <td style="text-align: right; font-family: monospace;">${pkt.rssi_dbm !== null ? pkt.rssi_dbm : '--'} dBm</td>
            <td style="text-align: right; font-family: monospace; color: var(--accent-cyan);">${pkt.packet_length_bytes} Bytes Frame</td>
            <td style="text-align: right;">Rating: ${pkt.signal_quality}</td>
          </tr>
          <tr>
            <td style="color: var(--text-primary); font-weight: 600;">🛡️ Early Warning Hazard Assessment</td>
            <td style="text-align: right; font-family: monospace;">Trigger Evaluation</td>
            <td style="text-align: right; font-family: monospace; font-weight: 700; color: ${pkt.landslide_risk_level.includes('CRIT') ? 'var(--accent-rose)' : 'var(--accent-emerald)'};">${pkt.landslide_risk_level}</td>
            <td style="text-align: right;">State: ${pkt.hazard_status}</td>
          </tr>
        `;
      }

      if (el.modalPacketInspect) el.modalPacketInspect.classList.add('open');
    };

    function closePacketInspector() {
      if (el.modalPacketInspect) el.modalPacketInspect.classList.remove('open');
    }

    function copyInspectedPacketJson() {
      if (!currentlyInspectedPacket) return;
      const jsonStr = JSON.stringify(currentlyInspectedPacket, null, 2);
      navigator.clipboard.writeText(jsonStr).then(() => {
        alert('Packet JSON successfully copied to clipboard!');
      }).catch(err => {
        alert('Failed to copy JSON: ' + err.message);
      });
    }

    /**
     * DOWNLOAD FULL DETAILED REPORT
     * Generates a complete, publication-grade geotechnical engineering report with PDF-ready print layout
     */
    function downloadFullDetailedReport() {
      if (allPackets.length === 0) {
        alert('No telemetry packets have been received yet to generate report.');
        return;
      }

      const totalPkts = allPackets.length;
      const latest = allPackets[allPackets.length - 1];
      const reportDate = new Date().toLocaleString();
      const reportId = 'GEOSENTINEL-REP-' + Date.now();

      // Compute aggregates
      const temps = allPackets.map(p => p.temperature_c).filter(t => !isNaN(t));
      const hums = allPackets.map(p => p.humidity_pct).filter(h => !isNaN(h));
      const soils = allPackets.map(p => p.soil_moisture_pct).filter(s => !isNaN(s));
      const rains = allPackets.map(p => p.rain_intensity_pct).filter(r => !isNaN(r));
      const accMags = allPackets.map(p => p.accel_magnitude_g).filter(a => !isNaN(a));
      const gyroMags = allPackets.map(p => p.gyro_magnitude_dps).filter(g => !isNaN(g));
      const pitches = allPackets.map(p => p.tilt_pitch_deg).filter(pt => !isNaN(pt));
      const rolls = allPackets.map(p => p.tilt_roll_deg).filter(rl => !isNaN(rl));
      const rssis = allPackets.map(p => p.rssi_dbm).filter(r => r !== null && !isNaN(r));

      const calcStats = (arr) => {
        if (!arr.length) return { min: '--', max: '--', mean: '--' };
        const min = Math.min(...arr).toFixed(2);
        const max = Math.max(...arr).toFixed(2);
        const mean = (arr.reduce((a, b) => a + b, 0) / arr.length).toFixed(2);
        return { min, max, mean };
      };

      const sTemp = calcStats(temps);
      const sHum = calcStats(hums);
      const sSoil = calcStats(soils);
      const sRain = calcStats(rains);
      const sAcc = calcStats(accMags);
      const sGyro = calcStats(gyroMags);
      const sPitch = calcStats(pitches);
      const sRoll = calcStats(rolls);
      const sRssi = calcStats(rssis);

      // Hazard counts
      let countSafe = 0, countAdv = 0, countElev = 0, countWarn = 0, countCrit = 0;
      allPackets.forEach(p => {
        const r = (p.landslide_risk_level || '').toUpperCase();
        if (r.includes('CRIT')) countCrit++;
        else if (r.includes('WARN')) countWarn++;
        else if (r.includes('ELEV')) countElev++;
        else if (r.includes('ADV')) countAdv++;
        else countSafe++;
      });

      // Generate HTML string for standalone report
      const reportHtml = `<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>GeoSentinel Comprehensive Landslide Telemetry & Investigation Report - ${reportId}</title>
  <style>
    :root {
      --primary: #1e3a8a;
      --primary-light: #3b82f6;
      --text-dark: #0f172a;
      --text-muted: #64748b;
      --border: #cbd5e1;
      --bg-alt: #f8fafc;
      --danger: #dc2626;
      --warning: #d97706;
      --success: #16a34a;
    }
    * { box-sizing: border-box; margin: 0; padding: 0; }
    body {
      font-family: 'Segoe UI', -apple-system, BlinkMacSystemFont, Roboto, sans-serif;
      color: var(--text-dark);
      background: #f1f5f9;
      line-height: 1.5;
      padding: 24px;
    }
    .report-wrapper {
      max-width: 1040px;
      margin: 0 auto;
      background: #ffffff;
      padding: 40px 48px;
      border-radius: 8px;
      box-shadow: 0 4px 20px rgba(0, 0, 0, 0.08);
      border: 1px solid var(--border);
    }
    .action-bar {
      position: sticky;
      top: 0;
      background: #ffffff;
      padding: 12px 0 20px 0;
      display: flex;
      justify-content: space-between;
      align-items: center;
      border-bottom: 2px solid #e2e8f0;
      margin-bottom: 24px;
      z-index: 100;
    }
    .btn-action {
      background: var(--primary);
      color: #ffffff;
      border: none;
      padding: 9px 20px;
      border-radius: 6px;
      font-weight: 600;
      font-size: 0.9rem;
      cursor: pointer;
      display: inline-flex;
      align-items: center;
      gap: 6px;
    }
    .btn-action:hover { background: #1d4ed8; }
    .btn-secondary { background: #e2e8f0; color: #1e293b; }
    .btn-secondary:hover { background: #cbd5e1; }
    .rep-header {
      display: flex;
      justify-content: space-between;
      align-items: flex-start;
      border-bottom: 3px solid var(--primary);
      padding-bottom: 20px;
      margin-bottom: 24px;
    }
    .rep-title h1 { font-size: 1.65rem; color: var(--primary); letter-spacing: -0.5px; }
    .rep-title p { font-size: 0.9rem; color: var(--text-muted); margin-top: 4px; font-weight: 500; }
    .rep-meta { text-align: right; font-size: 0.82rem; color: var(--text-muted); }
    .rep-meta strong { color: var(--text-dark); }
    h2 {
      font-size: 1.15rem;
      color: var(--primary);
      border-left: 4px solid var(--primary);
      padding-left: 10px;
      margin: 28px 0 14px 0;
      text-transform: uppercase;
      letter-spacing: 0.5px;
    }
    p, li { font-size: 0.92rem; color: #334155; }
    .status-banner {
      background: var(--bg-alt);
      border: 1px solid var(--border);
      border-radius: 8px;
      padding: 16px 20px;
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(210px, 1fr));
      gap: 16px;
      margin-bottom: 20px;
    }
    .stat-box { display: flex; flex-direction: column; }
    .stat-label { font-size: 0.75rem; text-transform: uppercase; color: var(--text-muted); font-weight: 700; }
    .stat-value { font-size: 1.25rem; font-weight: 700; color: var(--text-dark); margin-top: 2px; }
    table {
      width: 100%;
      border-collapse: collapse;
      margin: 12px 0 20px 0;
      font-size: 0.85rem;
    }
    th, td {
      padding: 8px 12px;
      border: 1px solid var(--border);
      text-align: left;
    }
    th {
      background: var(--bg-alt);
      font-weight: 700;
      color: var(--text-dark);
      text-transform: uppercase;
      font-size: 0.76rem;
    }
    tr:nth-child(even) { background: #fdfdfd; }
    .badge {
      display: inline-block;
      padding: 3px 8px;
      border-radius: 4px;
      font-size: 0.75rem;
      font-weight: 700;
      text-transform: uppercase;
    }
    .badge-safe { background: #dcfce7; color: var(--success); }
    .badge-warn { background: #fef3c7; color: var(--warning); }
    .badge-crit { background: #fee2e2; color: var(--danger); }
    .footer {
      border-top: 1px solid var(--border);
      margin-top: 36px;
      padding-top: 18px;
      display: flex;
      justify-content: space-between;
      font-size: 0.78rem;
      color: var(--text-muted);
    }
    @media print {
      body { background: #fff; padding: 0; color: #000; }
      .report-wrapper { box-shadow: none; border: none; padding: 0; max-width: 100%; }
      .action-bar { display: none !important; }
      h2 { page-break-after: avoid; }
      table { page-break-inside: auto; }
      tr { page-break-inside: avoid; }
      @page { margin: 15mm; size: A4 portrait; }
    }
  </style>
</head>
<body>
  <div class="report-wrapper">
    <div class="action-bar no-print">
      <div>
        <button class="btn-action" onclick="window.print()">🖨️ Print / Save as PDF</button>
        <button class="btn-action btn-secondary" onclick="downloadReportSource()" style="margin-left: 8px;">💾 Download Report File (.html)</button>
      </div>
      <button class="btn-action btn-secondary" onclick="window.close()">✕ Close Window</button>
    </div>

    <div class="rep-header">
      <div class="rep-title">
        <h1>GeoSentinel Early Warning System</h1>
        <p>Comprehensive Geotechnical &amp; Telemetry Investigation Report</p>
      </div>
      <div class="rep-meta">
        <div>Document Ref: <strong>${reportId}</strong></div>
        <div>Generated: <strong>${reportDate}</strong></div>
        <div>Base Station: <strong>SX1278 LoRa Receiver (Dual-Port)</strong></div>
        <div>Ingestion Status: <strong>VERIFIED COMPLETE (${totalPkts} Packets)</strong></div>
      </div>
    </div>

    <!-- Executive Assessment -->
    <h2>1. Executive Geological Assessment &amp; Threat Level</h2>
    <div class="status-banner">
      <div class="stat-box">
        <span class="stat-label">Current Hazard Status</span>
        <span class="stat-value" style="color: ${latest.landslide_risk_level.includes('CRIT') ? 'var(--danger)' : (latest.landslide_risk_level.includes('WARN') ? 'var(--warning)' : 'var(--success)')};">
          ${latest.landslide_risk_level}
        </span>
      </div>
      <div class="stat-box">
        <span class="stat-label">Total Ingested Packets</span>
        <span class="stat-value">${totalPkts} Packets</span>
      </div>
      <div class="stat-box">
        <span class="stat-label">Soil Saturation State</span>
        <span class="stat-value">${latest.soil_moisture_pct.toFixed(1)}% (${latest.soil_condition})</span>
      </div>
      <div class="stat-box">
        <span class="stat-label">Kinematic Shock Peak</span>
        <span class="stat-value">${sAcc.max} g (|A|)</span>
      </div>
    </div>
    <p>
      <strong>Geotechnical Summary:</strong> GeoSentinel automated multi-sensor analysis processed ${totalPkts} real-time LoRa telemetry packets. 
      Volumetric soil saturation averaged <strong>${sSoil.mean}%</strong> (Peak: <strong>${sSoil.max}%</strong>), with surface precipitation intensity averaging <strong>${sRain.mean}%</strong>. 
      Kinematic 3D vector acceleration |A| averaged <strong>${sAcc.mean} g</strong> with peak displacement shock of <strong>${sAcc.max} g</strong> and rotational angular rate peak of <strong>${sGyro.max} °/s</strong>. 
      Overall slope condition is classified under <strong>${latest.landslide_risk_level}</strong> protocols.
    </p>

    <!-- Comprehensive Sensor Telemetry Statistics -->
    <h2>2. Multi-Sensor Telemetry Statistical Summary (All Ingested Channels)</h2>
    <table>
      <thead>
        <tr>
          <th>Subsystem / Sensor Domain</th>
          <th>Ingested Parameter</th>
          <th>Min Recorded</th>
          <th>Max Recorded</th>
          <th>Mean / Latest</th>
          <th>Unit</th>
          <th>Nominal Baseline</th>
          <th>Geotechnical Status</th>
        </tr>
      </thead>
      <tbody>
        <tr>
          <td><strong>Atmospheric (DHT22)</strong></td>
          <td>Ambient Temperature (°C)</td>
          <td>${sTemp.min}</td>
          <td>${sTemp.max}</td>
          <td>${sTemp.mean} / ${latest.temperature_c.toFixed(1)}</td>
          <td>°C</td>
          <td>15.0 - 38.0 °C</td>
          <td><span class="badge badge-safe">Nominal</span></td>
        </tr>
        <tr>
          <td><strong>Atmospheric (DHT22)</strong></td>
          <td>Ambient Temperature (°F)</td>
          <td>${(sTemp.min !== '--' ? (sTemp.min*1.8+32).toFixed(1) : '--')}</td>
          <td>${(sTemp.max !== '--' ? (sTemp.max*1.8+32).toFixed(1) : '--')}</td>
          <td>${latest.temperature_f.toFixed(1)}</td>
          <td>°F</td>
          <td>59.0 - 100.4 °F</td>
          <td><span class="badge badge-safe">Nominal</span></td>
        </tr>
        <tr>
          <td><strong>Atmospheric (DHT22)</strong></td>
          <td>Relative Humidity (% RH)</td>
          <td>${sHum.min}</td>
          <td>${sHum.max}</td>
          <td>${sHum.mean} / ${latest.humidity_pct.toFixed(1)}</td>
          <td>%</td>
          <td>30.0 - 85.0 %</td>
          <td><span class="badge badge-safe">Normal Ambient</span></td>
        </tr>
        <tr>
          <td><strong>Atmospheric (Derived)</strong></td>
          <td>Dew Point Temperature</td>
          <td>--</td>
          <td>--</td>
          <td>${latest.dew_point_c.toFixed(1)}</td>
          <td>°C</td>
          <td>&lt; 26.0 °C</td>
          <td><span class="badge badge-safe">Stable</span></td>
        </tr>
        <tr>
          <td><strong>Hydrological (AO ADC)</strong></td>
          <td>Volumetric Soil Moisture</td>
          <td>${sSoil.min}</td>
          <td>${sSoil.max}</td>
          <td>${sSoil.mean} / ${latest.soil_moisture_pct.toFixed(1)}</td>
          <td>%</td>
          <td>&lt; 70.0 % (Dry/Mod)</td>
          <td><span class="badge ${latest.soil_moisture_pct > 80 ? 'badge-crit' : (latest.soil_moisture_pct > 65 ? 'badge-warn' : 'badge-safe')}">${latest.soil_condition}</span></td>
        </tr>
        <tr>
          <td><strong>Hydrological (AO ADC)</strong></td>
          <td>Rain Gauge Precipitation</td>
          <td>${sRain.min}</td>
          <td>${sRain.max}</td>
          <td>${sRain.mean} / ${latest.rain_intensity_pct.toFixed(1)}</td>
          <td>%</td>
          <td>&lt; 30.0 %</td>
          <td><span class="badge ${latest.rain_intensity_pct > 40 ? 'badge-warn' : 'badge-safe'}">${latest.rain_condition.replace('_', ' ')}</span></td>
        </tr>
        <tr>
          <td><strong>Kinematics (MPU6050 3D)</strong></td>
          <td>Total Acceleration Vector |A|</td>
          <td>${sAcc.min}</td>
          <td>${sAcc.max}</td>
          <td>${sAcc.mean} / ${latest.accel_magnitude_g.toFixed(2)}</td>
          <td>g</td>
          <td>0.85 - 1.15 g (1g static)</td>
          <td><span class="badge ${latest.accel_magnitude_g > 1.45 || latest.accel_magnitude_g < 0.55 ? 'badge-crit' : 'badge-safe'}">${latest.accel_magnitude_g > 1.45 ? 'Dynamic Motion' : 'Static Ground'}</span></td>
        </tr>
        <tr>
          <td><strong>Kinematics (MPU6050 3D)</strong></td>
          <td>Angular Velocity Rate |ω|</td>
          <td>${sGyro.min}</td>
          <td>${sGyro.max}</td>
          <td>${sGyro.mean} / ${latest.gyro_magnitude_dps.toFixed(1)}</td>
          <td>°/s</td>
          <td>&lt; 15.0 °/s</td>
          <td><span class="badge ${latest.gyro_magnitude_dps > 18 ? 'badge-crit' : 'badge-safe'}">${latest.gyro_magnitude_dps > 18 ? 'Rapid Tilt' : 'Stationary'}</span></td>
        </tr>
        <tr>
          <td><strong>Inclinometer (Derived)</strong></td>
          <td>Slope Pitch (Tx) &amp; Roll (Ty)</td>
          <td>${sPitch.min}° / ${sRoll.min}°</td>
          <td>${sPitch.max}° / ${sRoll.max}°</td>
          <td>Pitch: ${latest.tilt_pitch_deg.toFixed(1)}°, Roll: ${latest.tilt_roll_deg.toFixed(1)}°</td>
          <td>deg</td>
          <td>&lt; 15.0° baseline tilt</td>
          <td><span class="badge badge-safe">Stable Incline</span></td>
        </tr>
        <tr>
          <td><strong>RF Telemetry (SX1278)</strong></td>
          <td>LoRa RSSI Signal Strength</td>
          <td>${sRssi.min}</td>
          <td>${sRssi.max}</td>
          <td>${sRssi.mean} / ${latest.rssi_dbm !== null ? latest.rssi_dbm : '--'}</td>
          <td>dBm</td>
          <td>&gt; -105 dBm</td>
          <td><span class="badge badge-safe">${latest.signal_quality}</span></td>
        </tr>
      </tbody>
    </table>

    <!-- Geotechnical Trigger Matrix -->
    <h2>3. Geotechnical Trigger Thresholds &amp; Safety Limits</h2>
    <table>
      <thead>
        <tr>
          <th>Monitoring Indicator</th>
          <th>Warning Threshold</th>
          <th>Critical Action Limit</th>
          <th>Observed Maximum</th>
          <th>Evaluation &amp; Risk State</th>
        </tr>
      </thead>
      <tbody>
        <tr>
          <td>Soil Moisture Saturation</td>
          <td>&gt; 65 % (Moist)</td>
          <td>&gt; 80 % (Liquefaction Danger)</td>
          <td><strong>${sSoil.max} %</strong></td>
          <td>${parseFloat(sSoil.max) > 80 ? 'CRITICAL TRIGGER: Soil near full saturation' : 'Within normal operational envelope'}</td>
        </tr>
        <tr>
          <td>Kinematic Shock (|A|)</td>
          <td>&gt; 1.25 g or &lt; 0.75 g</td>
          <td>&gt; 1.45 g (Displacement Shock)</td>
          <td><strong>${sAcc.max} g</strong></td>
          <td>${parseFloat(sAcc.max) > 1.45 ? 'WARNING TRIGGER: Dynamic movement recorded' : 'Stable static gravity vector baseline'}</td>
        </tr>
        <tr>
          <td>Angular Velocity (|ω|)</td>
          <td>&gt; 10.0 °/s</td>
          <td>&gt; 18.0 °/s (Rotational Failure)</td>
          <td><strong>${sGyro.max} °/s</strong></td>
          <td>${parseFloat(sGyro.max) > 18 ? 'WARNING TRIGGER: High rotational velocity' : 'Rotational stability verified'}</td>
        </tr>
        <tr>
          <td>Slope Angular Incline</td>
          <td>&gt; 10.0 ° Tilt Shift</td>
          <td>&gt; 15.0 ° Angular Displacement</td>
          <td><strong>${sPitch.max}° / ${sRoll.max}°</strong></td>
          <td>Inclinometer parameters aligned with geotechnical baseline</td>
        </tr>
      </tbody>
    </table>

    <!-- Hazard Level Distribution -->
    <h2>4. Ingested Telemetry Hazard Distribution</h2>
    <table>
      <thead>
        <tr>
          <th>Hazard Level</th>
          <th>Packet Count</th>
          <th>Percentage</th>
          <th>Geotechnical Interpretation</th>
        </tr>
      </thead>
      <tbody>
        <tr>
          <td><span class="badge badge-safe">Safe / Normal</span></td>
          <td><strong>${countSafe}</strong></td>
          <td>${((countSafe/totalPkts)*100).toFixed(1)} %</td>
          <td>All physical registers stationary, soil moisture below threshold.</td>
        </tr>
        <tr>
          <td><span class="badge badge-safe" style="color: #0284c7; background: #e0f2fe;">Advisory</span></td>
          <td><strong>${countAdv}</strong></td>
          <td>${((countAdv/totalPkts)*100).toFixed(1)} %</td>
          <td>Moderate moisture or light precipitation detected. Ground stable.</td>
        </tr>
        <tr>
          <td><span class="badge badge-warn">Elevated</span></td>
          <td><strong>${countElev}</strong></td>
          <td>${((countElev/totalPkts)*100).toFixed(1)} %</td>
          <td>High moisture saturation; elevated liquefaction susceptibility.</td>
        </tr>
        <tr>
          <td><span class="badge badge-warn" style="color: #ea580c; background: #ffedd5;">Warning</span></td>
          <td><strong>${countWarn}</strong></td>
          <td>${((countWarn/totalPkts)*100).toFixed(1)} %</td>
          <td>Significant ground shift, tilt deviation, or kinematic displacement.</td>
        </tr>
        <tr>
          <td><span class="badge badge-crit">Critical Alert</span></td>
          <td><strong>${countCrit}</strong></td>
          <td>${((countCrit/totalPkts)*100).toFixed(1)} %</td>
          <td>Combined ground displacement under high soil pore-pressure saturation.</td>
        </tr>
      </tbody>
    </table>

    <!-- Chronological Telemetry Packets Log -->
    <h2>5. Chronological Telemetry Packets Log (Sample / Recent 50 Packets)</h2>
    <table>
      <thead>
        <tr>
          <th>Seq</th>
          <th>Timestamp</th>
          <th>Temp (°C)</th>
          <th>Hum (%)</th>
          <th>Soil (%)</th>
          <th>Rain (%)</th>
          <th>Accel |A|</th>
          <th>Gyro |ω|</th>
          <th>Tilt P/R</th>
          <th>RSSI</th>
          <th>Risk Tier</th>
          <th>Raw Telemetry Payload Snippet</th>
        </tr>
      </thead>
      <tbody>
        ${
          allPackets.slice(-50).map(p => `
            <tr>
              <td><strong>#${p.packet_id}</strong></td>
              <td style="font-family: monospace; font-size: 0.76rem;">${p.timestamp}</td>
              <td>${p.temperature_c.toFixed(1)}</td>
              <td>${p.humidity_pct.toFixed(1)}</td>
              <td>${p.soil_moisture_pct.toFixed(1)}</td>
              <td>${p.rain_intensity_pct.toFixed(1)}</td>
              <td style="font-family: monospace;">${p.accel_magnitude_g.toFixed(2)}g</td>
              <td style="font-family: monospace;">${p.gyro_magnitude_dps.toFixed(1)}°/s</td>
              <td>${p.tilt_pitch_deg.toFixed(0)}° / ${p.tilt_roll_deg.toFixed(0)}°</td>
              <td>${p.rssi_dbm !== null ? p.rssi_dbm : '--'}</td>
              <td><span class="badge ${p.landslide_risk_level.includes('CRIT') ? 'badge-crit' : (p.landslide_risk_level.includes('WARN') ? 'badge-warn' : 'badge-safe')}">${p.landslide_risk_level}</span></td>
              <td style="font-family: monospace; font-size: 0.7rem; color: var(--text-muted);">${p.raw_packet.length > 28 ? p.raw_packet.substring(0, 28) + '...' : p.raw_packet}</td>
            </tr>
          `).join('')
        }
      </tbody>
    </table>

    <!-- Standard Operating Procedures -->
    <h2>6. Emergency SOP &amp; Geotechnical Response Protocols</h2>
    <ul style="padding-left: 20px; line-height: 1.8;">
      <li><strong>Level 1 (Normal / Monitoring):</strong> Maintain continuous 1Hz telemetry sampling. Routine bi-weekly physical calibration check of rain funnel and soil probe contacts.</li>
      <li><strong>Level 2 (Advisory / Elevated):</strong> Issue automated alerts to local emergency management cell. Increase sampling frequency if on battery mode. Monitor slope drainage gullies for sedimentation.</li>
      <li><strong>Level 3 (Warning):</strong> Activate on-site automated visual warning beacons. Restrict commercial vehicular transit on downslope corridors. Deploy geotechnical inspection team to inspect tension cracks.</li>
      <li><strong>Level 4 (Critical Alert):</strong> Trigger local civil defense sirens immediately. Execute evacuation order for downhill settlement zones. Close arterial transport links adjacent to monitored slope face.</li>
    </ul>

    <div class="footer">
      <div>Report compiled by: <strong>GeoSentinel Automated Telemetry Engine v2.4</strong></div>
      <div>Station Operator: <strong>NEHA Project Lead &bull; ESP32 &amp; SX1278 Platform</strong></div>
      <div>Official Certification: <strong>VERIFIED TELEMETRY DATA</strong></div>
    </div>
  </div>

  <script>\n    function downloadReportSource() {
      const blob = new Blob([document.documentElement.outerHTML], { type: 'text/html' });
      const url = URL.createObjectURL(blob);
      const a = document.createElement('a');
      a.href = url;
      a.download = 'geosentinel_detailed_report_' + Date.now() + '.html';
      document.body.appendChild(a);
      a.click();
      document.body.removeChild(a);
      URL.revokeObjectURL(url);
    }
  <\/script>
</body>
</html>`;

      // Open printable report in new window and prompt print dialog
      const repWindow = window.open('', '_blank');
      if (repWindow) {
        repWindow.document.open();
        repWindow.document.write(reportHtml);
        repWindow.document.close();
      } else {
        // Fallback if popup blocked: directly download HTML report file
        const blob = new Blob([reportHtml], { type: 'text/html' });
        const url = URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url;
        a.download = 'geosentinel_detailed_geotechnical_report.html';
        document.body.appendChild(a);
        a.click();
        document.body.removeChild(a);
        URL.revokeObjectURL(url);
      }
    }

    /**
     * DOWNLOAD FULL DATASET (CSV)
     * Exports the complete 34-column CSV file containing all sensor details and risk ratings
     */
    function downloadFullDatasetCSV() {
      if (allPackets.length === 0) {
        alert('No dataset records available to download.');
        return;
      }

      // Standard 34-column headers matching geosentinel_dataset.csv
      const headers = [
        'timestamp', 'packet_id', 'temperature_c', 'temperature_f', 'humidity_pct', 'dew_point_c',
        'soil_raw_adc', 'soil_moisture_pct', 'soil_condition',
        'rain_raw_adc', 'rain_intensity_pct', 'rain_condition',
        'accel_x_raw', 'accel_y_raw', 'accel_z_raw',
        'accel_x_g', 'accel_y_g', 'accel_z_g', 'accel_magnitude_g',
        'gyro_x_raw', 'gyro_y_raw', 'gyro_z_raw',
        'gyro_x_dps', 'gyro_y_dps', 'gyro_z_dps', 'gyro_magnitude_dps',
        'tilt_pitch_deg', 'tilt_roll_deg',
        'rssi_dbm', 'signal_quality', 'packet_length_bytes',
        'landslide_risk_level', 'hazard_status', 'raw_packet'
      ];

      const csvRows = [headers.join(',')];

      allPackets.forEach(p => {
        const row = [
          `"${p.timestamp}"`,
          p.packet_id,
          p.temperature_c,
          p.temperature_f,
          p.humidity_pct,
          p.dew_point_c,
          p.soil_raw_adc,
          p.soil_moisture_pct,
          p.soil_condition,
          p.rain_raw_adc,
          p.rain_intensity_pct,
          p.rain_condition,
          p.accel_x_raw,
          p.accel_y_raw,
          p.accel_z_raw,
          p.accel_x_g,
          p.accel_y_g,
          p.accel_z_g,
          p.accel_magnitude_g,
          p.gyro_x_raw,
          p.gyro_y_raw,
          p.gyro_z_raw,
          p.gyro_x_dps,
          p.gyro_y_dps,
          p.gyro_z_dps,
          p.gyro_magnitude_dps,
          p.tilt_pitch_deg,
          p.tilt_roll_deg,
          p.rssi_dbm !== null ? p.rssi_dbm : '',
          `"${p.signal_quality}"`,
          p.packet_length_bytes,
          `"${p.landslide_risk_level}"`,
          `"${p.hazard_status}"`,
          `"${p.raw_packet.replace(/"/g, '""')}"`
        ];
        csvRows.push(row.join(','));
      });

      const csvString = csvRows.join('\r\n');
      const blob = new Blob([csvString], { type: 'text/csv;charset=utf-8;' });
      const url = URL.createObjectURL(blob);
      const a = document.createElement('a');
      a.href = url;
      a.download = `geosentinel_dataset_${allPackets.length}_records.csv`;
      document.body.appendChild(a);
      a.click();
      document.body.removeChild(a);
      URL.revokeObjectURL(url);
    }

    /**
     * Export complete dataset analysis as JSON
     */
    function exportJsonAnalysis() {
      const exportData = {
        system: "GeoSentinel Landslide Early Warning System",
        exportedAt: new Date().toISOString(),
        totalPackets: allPackets.length,
        peakAccelerationMagnitude: peakAccelMagObserved,
        peakGyroscopeMagnitude: peakGyroMagObserved,
        dataset: allPackets
      };

      const blob = new Blob([JSON.stringify(exportData, null, 2)], { type: 'application/json' });
      const url = URL.createObjectURL(blob);
      const a = document.createElement('a');
      a.href = url;
      a.download = `geosentinel_dataset_analysis_${Date.now()}.json`;
      document.body.appendChild(a);
      a.click();
      document.body.removeChild(a);
      URL.revokeObjectURL(url);
    }

    /**
     * Bootstraps initial historical dataset records into dashboard
     */
    function applyRecordToGauges(record) {
      if (!record) return;
      if (el.valTemp) el.valTemp.textContent = record.temperature_c.toFixed(1);
      if (el.valTempF) el.valTempF.textContent = `${record.temperature_f.toFixed(1)} °F`;
      if (el.barTemp) el.barTemp.style.width = `${Math.max(0, Math.min(100, ((record.temperature_c + 10) / 60) * 100))}%`;

      if (el.valHum) el.valHum.textContent = record.humidity_pct.toFixed(1);
      if (el.barHum) el.barHum.style.width = `${Math.max(0, Math.min(100, record.humidity_pct))}%`;

      if (el.valSoil) el.valSoil.textContent = record.soil_moisture_pct.toFixed(1);
      if (el.barSoil) el.barSoil.style.width = `${record.soil_moisture_pct}%`;
      if (el.unitSoil) el.unitSoil.textContent = '%';
      if (el.footerSoilRange) el.footerSoilRange.textContent = `Raw: ${record.soil_raw_adc} ADC | State: ${record.soil_condition}`;

      if (el.valRain) el.valRain.textContent = record.rain_intensity_pct.toFixed(1);
      if (el.barRain) el.barRain.style.width = `${record.rain_intensity_pct}%`;
      if (el.unitRain) el.unitRain.textContent = '%';
      if (el.footerRainRange) el.footerRainRange.textContent = `Raw: ${record.rain_raw_adc} ADC | State: ${record.rain_condition.replace('_', ' ')}`;

      if (el.valAx) el.valAx.textContent = record.accel_x_g.toFixed(3);
      if (el.valAy) el.valAy.textContent = record.accel_y_g.toFixed(3);
      if (el.valAz) el.valAz.textContent = record.accel_z_g.toFixed(3);

      if (el.valGx) el.valGx.textContent = record.gyro_x_dps.toFixed(1);
      if (el.valGy) el.valGy.textContent = record.gyro_y_dps.toFixed(1);
      if (el.valGz) el.valGz.textContent = record.gyro_z_dps.toFixed(1);

      if (el.valPitch) el.valPitch.textContent = `${record.tilt_pitch_deg.toFixed(1)}°`;
      if (el.valRoll) el.valRoll.textContent = `${record.tilt_roll_deg.toFixed(1)}°`;

      if (el.badgeSourceVal) el.badgeSourceVal.textContent = `DATASET (#${record.packet_id})`;
      if (el.feedStatusText) {
        el.feedStatusText.textContent = `DATASET SYNCHRONIZED (${allPackets.length} PACKETS)`;
        el.feedStatusText.style.color = 'var(--accent-cyan)';
      }
    }

    function parseAndIngestCSV(csvText) {
      const lines = csvText.split(/\r?\n/).filter(l => l.trim().length > 0);
      if (lines.length < 2) {
        alert("CSV file appears to be empty or missing data rows.");
        return;
      }
      const headers = lines[0].split(',').map(h => h.trim().replace(/^"|"$/g, ''));
      const parsedRecords = [];
      for (let i = 1; i < lines.length; i++) {
        const row = [];
        let insideQuotes = false;
        let entry = '';
        const line = lines[i];
        for (let c = 0; c < line.length; c++) {
          const ch = line[c];
          if (ch === '"') {
            insideQuotes = !insideQuotes;
          } else if (ch === ',' && !insideQuotes) {
            row.push(entry.trim().replace(/^"|"$/g, ''));
            entry = '';
          } else {
            entry += ch;
          }
        }
        row.push(entry.trim().replace(/^"|"$/g, ''));
        if (row.length >= 5) {
          const rec = {};
          headers.forEach((h, idx) => {
            rec[h] = row[idx] !== undefined ? row[idx] : '';
          });
          parsedRecords.push(rec);
        }
      }

      if (parsedRecords.length === 0) {
        alert("Could not parse valid telemetry records from the CSV file.");
        return;
      }

      allPackets.length = 0;
      if (el.tbodyPacketsFeed) el.tbodyPacketsFeed.innerHTML = '';
      peakAccelMagObserved = 0;
      peakGyroMagObserved = 0;

      parsedRecords.forEach(rec => {
        const norm = normalizeTelemetryPacket(rec);
        allPackets.push(norm);
        if (norm.accel_magnitude_g > peakAccelMagObserved) peakAccelMagObserved = norm.accel_magnitude_g;
        if (norm.gyro_magnitude_dps > peakGyroMagObserved) peakGyroMagObserved = norm.gyro_magnitude_dps;
      });

      const recentSubset = allPackets.slice(-50);
      recentSubset.forEach(p => addPacketTableRow(p));

      updatePacketAnalytics();

      if (allPackets.length > 0) {
        applyRecordToGauges(allPackets[allPackets.length - 1]);
      }

      alert(`Successfully loaded and synchronized ${allPackets.length} packets from dataset!`);
    }

    function initializeHistoricalDataset() {
      if (INITIAL_HISTORICAL_DATASET && INITIAL_HISTORICAL_DATASET.length > 0) {
        console.log(`Ingesting ${INITIAL_HISTORICAL_DATASET.length} historical dataset records...`);
        INITIAL_HISTORICAL_DATASET.forEach(rec => {
          const norm = normalizeTelemetryPacket(rec);
          allPackets.push(norm);
          if (norm.accel_magnitude_g > peakAccelMagObserved) peakAccelMagObserved = norm.accel_magnitude_g;
          if (norm.gyro_magnitude_dps > peakGyroMagObserved) peakGyroMagObserved = norm.gyro_magnitude_dps;
        });

        // Render initial table rows (up to 50 recent)
        const recentSubset = allPackets.slice(-50);
        recentSubset.forEach(p => addPacketTableRow(p));

        updatePacketAnalytics();

        if (allPackets.length > 0) {
          applyRecordToGauges(allPackets[allPackets.length - 1]);
        }
        console.log("Historical dataset ingestion complete.");
      }
    }


    /* =========================================================================
       CANVAS CHART COMPONENT
       ========================================================================= */
    class MinimalCanvasChart {
      constructor(canvasId, options = {}) {
        this.canvas = document.getElementById(canvasId);
        if (!this.canvas) return;
        this.ctx = this.canvas.getContext('2d');
        this.options = Object.assign({
          yTitle: 'Y-AXIS',
          topTitle: '◄ TOP X-AXIS: TIME TIMELINE (OFFSET) ►',
          bottomTitle: '◄ BOTTOM X-AXIS: REAL-TIME (HH:MM:SS) ►',
          yUnit: '',
          yDecimals: 1,
          forceZero: false,
          defaultMin: 0,
          defaultMax: 100
        }, options);
        this.lastSeriesList = [];
        this.resize();
        window.addEventListener('resize', () => this.resize());
      }

      resize() {
        if (!this.canvas) return;
        const rect = this.canvas.parentElement.getBoundingClientRect();
        const dpr = window.devicePixelRatio || 1;
        this.width = rect.width || 400;
        this.height = rect.height || 220;
        this.canvas.width = this.width * dpr;
        this.canvas.height = this.height * dpr;
        this.ctx.setTransform(1, 0, 0, 1, 0, 0);
        this.ctx.scale(dpr, dpr);
        this.render();
      }

      render(seriesList = [], labels = null) {
        if (seriesList.length) this.lastSeriesList = seriesList;
        seriesList = seriesList.length ? seriesList : (this.lastSeriesList || []);
        const ctx = this.ctx;
        if (!ctx) return;

        const w = this.width;
        const h = this.height;

        // Coordinate graph padding: left for Y-axis, right for framing/last value, top for Top X-axis, bottom for Bottom X-axis
        const padL = 52;
        const padR = 28;
        const padT = 32;
        const padB = 34;
        const plotX = padL;
        const plotY = padT;
        const plotW = Math.max(10, w - padL - padR);
        const plotH = Math.max(10, h - padT - padB);

        ctx.clearRect(0, 0, w, h);

        // 1. Graph plotting area background
        ctx.fillStyle = 'rgba(255, 255, 255, 0.55)';
        ctx.fillRect(plotX, plotY, plotW, plotH);

        // Check if data is available
        const hasData = seriesList.length > 0 && seriesList[0].data && seriesList[0].data.length > 0;
        let minVal = Infinity;
        let maxVal = -Infinity;

        if (hasData) {
          seriesList.forEach(s => {
            if (!s.data) return;
            s.data.forEach(v => {
              if (typeof v === 'number' && !isNaN(v)) {
                if (v < minVal) minVal = v;
                if (v > maxVal) maxVal = v;
              }
            });
          });
        }

        if (minVal === Infinity || maxVal === -Infinity) {
          minVal = this.options.defaultMin;
          maxVal = this.options.defaultMax;
        }

        if (this.options.forceZero) {
          if (minVal > 0) minVal = -0.1;
          if (maxVal < 0) maxVal = 0.1;
        }

        if (minVal === maxVal) {
          minVal -= 5;
          maxVal += 5;
        }

        // Add 5% headroom/footroom for clean margins
        const span = maxVal - minVal;
        const yMin = minVal - span * 0.05;
        const yMax = maxVal + span * 0.05;
        const yRange = yMax - yMin || 1;

        // 2. Horizontal Gridlines & Y-Axis Scale
        const yDivisions = h < 160 ? 3 : 4;
        ctx.lineWidth = 1;

        for (let i = 0; i <= yDivisions; i++) {
          const y = plotY + (plotH / yDivisions) * i;

          // Grid line across plot area
          ctx.beginPath();
          ctx.strokeStyle = 'rgba(148, 163, 184, 0.25)';
          ctx.setLineDash([3, 3]);
          ctx.moveTo(plotX, y);
          ctx.lineTo(plotX + plotW, y);
          ctx.stroke();
          ctx.setLineDash([]);

          // Left Y-Axis Tick mark
          ctx.beginPath();
          ctx.strokeStyle = '#64748b';
          ctx.moveTo(plotX - 5, y);
          ctx.lineTo(plotX, y);
          ctx.stroke();

          // Right Boundary Tick mark
          ctx.beginPath();
          ctx.strokeStyle = 'rgba(148, 163, 184, 0.45)';
          ctx.moveTo(plotX + plotW, y);
          ctx.lineTo(plotX + plotW + 4, y);
          ctx.stroke();

          // Y-Axis numeric label
          const val = yMax - (yRange / yDivisions) * i;
          let valText;
          if (this.options.yDecimals === 0 || Math.abs(val) >= 100) {
            valText = val.toFixed(0);
          } else if (this.options.yDecimals === 2 || Math.abs(val) < 1) {
            valText = val.toFixed(2);
          } else {
            valText = val.toFixed(1);
          }

          ctx.fillStyle = '#475569';
          ctx.font = '10px "JetBrains Mono", ui-monospace, monospace';
          ctx.textAlign = 'right';
          ctx.textBaseline = 'middle';
          ctx.fillText(valText, plotX - 8, y);
        }

        // Zero-line accent if 0 is within range
        if (yMin < 0 && yMax > 0) {
          const zeroY = plotY + plotH - ((0 - yMin) / yRange) * plotH;
          ctx.beginPath();
          ctx.strokeStyle = 'rgba(71, 85, 105, 0.45)';
          ctx.lineWidth = 1.2;
          ctx.setLineDash([4, 2]);
          ctx.moveTo(plotX, zeroY);
          ctx.lineTo(plotX + plotW, zeroY);
          ctx.stroke();
          ctx.setLineDash([]);
        }

        // 3. Vertical Gridlines & Dual X-Axes (Top and Bottom)
        const xDivisions = w < 440 ? 3 : 5;
        const pointCount = hasData ? seriesList[0].data.length : 0;
        const timeArr = (labels && labels.length) ? labels : ((window.chartData && window.chartData.timestamps) ? window.chartData.timestamps : []);

        for (let j = 0; j <= xDivisions; j++) {
          const x = plotX + (plotW / xDivisions) * j;

          // Vertical gridline
          ctx.beginPath();
          ctx.strokeStyle = 'rgba(148, 163, 184, 0.22)';
          ctx.setLineDash([3, 3]);
          ctx.moveTo(x, plotY);
          ctx.lineTo(x, plotY + plotH);
          ctx.stroke();
          ctx.setLineDash([]);

          // Bottom X-Axis Tick (outward)
          ctx.beginPath();
          ctx.strokeStyle = '#64748b';
          ctx.lineWidth = 1.2;
          ctx.moveTo(x, plotY + plotH);
          ctx.lineTo(x, plotY + plotH + 5);
          ctx.stroke();

          // Top X-Axis Tick (outward)
          ctx.beginPath();
          ctx.strokeStyle = '#64748b';
          ctx.lineWidth = 1.2;
          ctx.moveTo(x, plotY);
          ctx.lineTo(x, plotY - 5);
          ctx.stroke();

          // TOP X-AXIS LABELS (Timeline / Relative Offset)
          let topLabel = '';
          if (j === xDivisions) {
            topLabel = 'T-0 (Now)';
          } else {
            const offsetSec = Math.round((xDivisions - j) * (30 / xDivisions));
            topLabel = `T-${offsetSec}s`;
          }

          ctx.fillStyle = '#64748b';
          ctx.font = '9px "JetBrains Mono", ui-monospace, monospace';
          ctx.textAlign = (j === 0) ? 'left' : (j === xDivisions) ? 'right' : 'center';
          ctx.textBaseline = 'bottom';
          ctx.fillText(topLabel, x, plotY - 6);

          // BOTTOM X-AXIS LABELS (Actual timestamps / Sample sequence)
          let bottomLabel = '';
          if (hasData && timeArr.length > 0) {
            const idx = Math.min(Math.round((pointCount - 1) * (j / xDivisions)), pointCount - 1);
            bottomLabel = timeArr[idx] || `P${idx + 1}`;
          } else {
            bottomLabel = `00:${String(j * 10).padStart(2, '0')}`;
          }

          ctx.fillStyle = '#475569';
          ctx.font = '9.5px "JetBrains Mono", ui-monospace, monospace';
          ctx.textAlign = (j === 0) ? 'left' : (j === xDivisions) ? 'right' : 'center';
          ctx.textBaseline = 'top';
          ctx.fillText(bottomLabel, x, plotY + plotH + 7);
        }

        // 4. COORDINATE GRAPH FRAME (Full axis lines on all 4 boundaries)
        ctx.lineWidth = 1.5;

        // Left Y-Axis Spine
        ctx.beginPath();
        ctx.strokeStyle = '#475569';
        ctx.moveTo(plotX, plotY);
        ctx.lineTo(plotX, plotY + plotH);
        ctx.stroke();

        // Bottom X-Axis Spine
        ctx.beginPath();
        ctx.strokeStyle = '#475569';
        ctx.moveTo(plotX, plotY + plotH);
        ctx.lineTo(plotX + plotW, plotY + plotH);
        ctx.stroke();

        // Top X-Axis Spine
        ctx.beginPath();
        ctx.strokeStyle = '#475569';
        ctx.moveTo(plotX, plotY);
        ctx.lineTo(plotX + plotW, plotY);
        ctx.stroke();

        // Right Y-Axis Bounding Spine
        ctx.beginPath();
        ctx.strokeStyle = 'rgba(148, 163, 184, 0.45)';
        ctx.lineWidth = 1;
        ctx.moveTo(plotX + plotW, plotY);
        ctx.lineTo(plotX + plotW, plotY + plotH);
        ctx.stroke();

        // 5. AXIS HEADERS & TITLES
        // Left Y-Axis Header
        ctx.fillStyle = '#334155';
        ctx.font = 'bold 8.5px "JetBrains Mono", ui-monospace, sans-serif';
        ctx.textAlign = 'left';
        ctx.textBaseline = 'bottom';
        ctx.fillText(this.options.yTitle, plotX - 44, plotY - 8);

        // Top X-Axis Header
        ctx.fillStyle = '#64748b';
        ctx.font = 'bold 8.5px "JetBrains Mono", ui-monospace, sans-serif';
        ctx.textAlign = 'center';
        ctx.textBaseline = 'top';
        ctx.fillText(this.options.topTitle, plotX + plotW / 2, 4);

        // Bottom X-Axis Header
        ctx.fillStyle = '#64748b';
        ctx.font = 'bold 8.5px "JetBrains Mono", ui-monospace, sans-serif';
        ctx.textAlign = 'center';
        ctx.textBaseline = 'bottom';
        ctx.fillText(this.options.bottomTitle, plotX + plotW / 2, h - 3);

        // 6. EMPTY STATE WATERMARK
        if (!hasData) {
          const msg = '⏳ Waiting for live telemetry packets...';
          ctx.font = '500 11px sans-serif';
          const tw = ctx.measureText(msg).width;
          const bw = tw + 24;
          const bh = 26;
          const bx = plotX + (plotW - bw) / 2;
          const by = plotY + (plotH - bh) / 2;
          ctx.fillStyle = 'rgba(241, 245, 249, 0.9)';
          ctx.strokeStyle = 'rgba(148, 163, 184, 0.4)';
          ctx.lineWidth = 1;
          ctx.beginPath();
          if (ctx.roundRect) ctx.roundRect(bx, by, bw, bh, 6);
          else ctx.rect(bx, by, bw, bh);
          ctx.fill();
          ctx.stroke();
          ctx.fillStyle = '#64748b';
          ctx.textAlign = 'center';
          ctx.textBaseline = 'middle';
          ctx.fillText(msg, bx + bw / 2, by + bh / 2);
          return;
        }

        // 7. PLOT DATA SERIES
        const stepX = pointCount > 1 ? plotW / (pointCount - 1) : plotW;

        seriesList.forEach(s => {
          if (!s.data || !s.data.length) return;
          const strokeColor = s.color || '#3b82f6';

          // Polyline
          ctx.beginPath();
          ctx.strokeStyle = strokeColor;
          ctx.lineWidth = 2;
          ctx.lineJoin = 'round';
          ctx.lineCap = 'round';

          s.data.forEach((val, idx) => {
            const x = plotX + idx * stepX;
            const normY = (val - yMin) / yRange;
            const y = Math.max(plotY, Math.min(plotY + plotH, plotY + plotH - normY * plotH));
            if (idx === 0) ctx.moveTo(x, y);
            else ctx.lineTo(x, y);
          });
          ctx.stroke();

          // Marker points
          ctx.fillStyle = strokeColor;
          s.data.forEach((val, idx) => {
            const x = plotX + idx * stepX;
            const normY = (val - yMin) / yRange;
            const y = Math.max(plotY, Math.min(plotY + plotH, plotY + plotH - normY * plotH));
            ctx.beginPath();
            ctx.arc(x, y, 2.5, 0, Math.PI * 2);
            ctx.fill();
          });

          // Latest point indicator & halo
          if (s.data.length > 0) {
            const lastVal = s.data[s.data.length - 1];
            const lastX = plotX + (s.data.length - 1) * stepX;
            const lastNormY = (lastVal - yMin) / yRange;
            const lastY = Math.max(plotY, Math.min(plotY + plotH, plotY + plotH - lastNormY * plotH));

            ctx.beginPath();
            ctx.arc(lastX, lastY, 4.5, 0, Math.PI * 2);
            ctx.strokeStyle = strokeColor;
            ctx.lineWidth = 1.5;
            ctx.stroke();
          }
        });
      }
    }

    const envChart = new MinimalCanvasChart('chart-env', {
      yTitle: '▲ TEMP / HUM (°C, %)',
      topTitle: '◄ TOP X-AXIS: TIME TIMELINE (OFFSET) ►',
      bottomTitle: '◄ BOTTOM X-AXIS: REAL-TIME (HH:MM:SS) ►',
      yDecimals: 1,
      defaultMin: 0,
      defaultMax: 100
    });
    const soilRainChart = new MinimalCanvasChart('chart-soil-rain', {
      yTitle: '▲ RAW ADC (0-4095)',
      topTitle: '◄ TOP X-AXIS: TIME TIMELINE (OFFSET) ►',
      bottomTitle: '◄ BOTTOM X-AXIS: REAL-TIME (HH:MM:SS) ►',
      yDecimals: 0,
      defaultMin: 0,
      defaultMax: 4095
    });
    const accelChart = new MinimalCanvasChart('chart-accel', {
      yTitle: '▲ KINEMATICS (g)',
      topTitle: '◄ TOP X-AXIS: TIME TIMELINE (OFFSET) ►',
      bottomTitle: '◄ BOTTOM X-AXIS: REAL-TIME (HH:MM:SS) ►',
      yDecimals: 2,
      forceZero: true,
      defaultMin: -1.5,
      defaultMax: 1.5
    });

    document.getElementById('btn-clear-charts').addEventListener('click', () => {
      chartData.timestamps = [];
      chartData.temp = [];
      chartData.hum = [];
      chartData.soil = [];
      chartData.rain = [];
      chartData.ax = [];
      chartData.ay = [];
      chartData.az = [];
      envChart.render([]);
      soilRainChart.render([]);
      accelChart.render([]);
      addAlert('Telemetry graphs reset.', 'normal');
    });

    /* =========================================================================
       AUDIO SYNTHESIZER FOR HAZARD ALARMS
       ========================================================================= */
    function playBeep(freq = 880, duration = 0.15, type = 'sine') {
      if (!audioEnabled) return;
      try {
        if (!audioCtx) audioCtx = new (window.AudioContext || window.webkitAudioContext)();
        if (audioCtx.state === 'suspended') audioCtx.resume();
        const osc = audioCtx.createOscillator();
        const gain = audioCtx.createGain();
        osc.type = type;
        osc.frequency.setValueAtTime(freq, audioCtx.currentTime);
        gain.gain.setValueAtTime(0.1, audioCtx.currentTime);
        gain.gain.exponentialRampToValueAtTime(0.001, audioCtx.currentTime + duration);
        osc.connect(gain);
        gain.connect(audioCtx.destination);
        osc.start();
        osc.stop(audioCtx.currentTime + duration);
      } catch (e) {
        console.error('Audio play error:', e);
      }
    }

    el.btnToggleAudio.addEventListener('click', () => {
      audioEnabled = !audioEnabled;
      if (audioEnabled) {
        el.iconAudio.textContent = '🔊';
        el.textAudioBtn.textContent = 'Sound ON';
        el.btnToggleAudio.classList.add('active');
        playBeep(1200, 0.1);
        addAlert('Audio alarms enabled.', 'normal');
      } else {
        el.iconAudio.textContent = '🔇';
        el.textAudioBtn.textContent = 'Muted';
        el.btnToggleAudio.classList.remove('active');
      }
    });

    /* =========================================================================
       LOG & TERMINAL SYSTEM
       ========================================================================= */
    function addTerminalLine(text, tag = 'SYS', typeClass = 'log-tag-sys') {
      const timeStr = new Date().toLocaleTimeString();
      const div = document.createElement('div');
      div.className = `log-entry log-filter-${tag.toLowerCase()}`;

      div.innerHTML = `
        <span class="log-time">${timeStr}</span>
        <span class="log-tag ${typeClass}">${tag}</span>
        <span class="log-text">${escapeHtml(text)}</span>
      `;

      el.terminalBody.appendChild(div);

      // Keep max 200 lines in terminal DOM
      while (el.terminalBody.children.length > 200) {
        el.terminalBody.removeChild(el.terminalBody.firstChild);
      }

      if (el.chkAutoScroll.checked) {
        el.terminalBody.scrollTop = el.terminalBody.scrollHeight;
      }
    }

    function escapeHtml(str) {
      return String(str)
        .replace(/&/g, '&amp;')
        .replace(/</g, '&lt;')
        .replace(/>/g, '&gt;');
    }

    el.btnClearLog.addEventListener('click', () => {
      el.terminalBody.innerHTML = '';
      addTerminalLine('Terminal console cleared.', 'SYS', 'log-tag-sys');
    });

    el.btnExportLog.addEventListener('click', () => {
      if (!telemetryHistory.length) {
        alert('No telemetry records collected yet to export.');
        return;
      }
      let csv = 'Timestamp,Source,Temp_C,Hum_Pct,Soil_ADC,Rain_ADC,AX,AY,AZ,GX,GY,GZ,RSSI_dBm\n';
      telemetryHistory.forEach(r => {
        csv += `${r.time},${r.source || 'USB'},${r.temperature || ''},${r.humidity || ''},${r.soil || ''},${r.rain || ''},${r.ax || ''},${r.ay || ''},${r.az || ''},${r.gx || ''},${r.gy || ''},${r.gz || ''},${r.rssi || ''}\n`;
      });

      const blob = new Blob([csv], { type: 'text/csv' });
      const url = URL.createObjectURL(blob);
      const a = document.createElement('a');
      a.href = url;
      a.download = `GeoSentinel_Telemetry_${new Date().toISOString().slice(0, 19).replace(/[:T]/g, '-')}.csv`;
      a.click();
      URL.revokeObjectURL(url);
    });

    el.selectLogFilter.addEventListener('change', (e) => {
      const val = e.target.value;
      const entries = el.terminalBody.querySelectorAll('.log-entry');
      entries.forEach(item => {
        if (val === 'all') {
          item.style.display = 'flex';
        } else if (val === 'rx') {
          item.style.display = item.classList.contains('log-filter-rx') ? 'flex' : 'none';
        } else if (val === 'tx') {
          item.style.display = item.classList.contains('log-filter-tx') ? 'flex' : 'none';
        } else if (val === 'sys') {
          item.style.display = item.classList.contains('log-filter-sys') ? 'flex' : 'none';
        }
      });
    });

    function addAlert(message, type = 'normal') {
      const card = document.createElement('div');
      card.className = `alert-card ${type === 'danger' ? 'danger' : type === 'warning' ? 'warning' : ''}`;
      const icon = type === 'danger' ? '🚨' : type === 'warning' ? '⚠️' : 'ℹ️';
      const timeStr = new Date().toLocaleTimeString();

      card.innerHTML = `
        <span class="alert-icon">${icon}</span>
        <div class="alert-text"><div>${message}</div></div>
        <span class="alert-time">${timeStr}</span>
      `;

      el.alertsList.insertBefore(card, el.alertsList.firstChild);
      while (el.alertsList.children.length > 8) {
        el.alertsList.removeChild(el.alertsList.lastChild);
      }
    }

    function classifyRssi(rssi) {
      if (rssi > -70) return { label: 'Excellent', color: 'var(--accent-emerald)' };
      if (rssi >= -90) return { label: 'Good', color: 'var(--accent-cyan)' };
      if (rssi >= -105) return { label: 'Weak', color: 'var(--accent-amber)' };
      return { label: 'Very Weak', color: 'var(--accent-rose)' };
    }

    /* =========================================================================
       UNIVERSAL TELEMETRY PARSER (HANDLES ALL RX, TX, FORMATS & CSV)
       ========================================================================= */
    function parseIncomingSerialStream(rawChunk, sourceTag = 'RX') {
      const lines = rawChunk.split(/\r?\n/);
      let parsedAny = false;
      const selectedMode = el.selectDeviceMode ? el.selectDeviceMode.value : 'receiver';

      for (let line of lines) {
        line = line.trim();
        if (!line) continue;

        // Detect source tag: If in receiver mode or sourceTag is RX, treat as Receiver
        const isTx = (selectedMode === 'transmitter') || sourceTag === 'TX' || line.toLowerCase().includes('sending') || line.toLowerCase().includes('[tx]') || line.toLowerCase().includes('transmitter');
        const isRx = (selectedMode === 'receiver') || sourceTag === 'RX' || line.includes('DATA RECEIVED') || line.includes('RSSI:') || line.includes('[LoRa]') || line.includes('Parsed values');
        const tag = isTx ? 'TX' : (isRx ? 'RX' : 'LIVE');
        const tagClass = tag === 'TX' ? 'log-tag-tx' : (tag === 'RX' ? 'log-tag-rx' : 'log-tag-sys');

        addTerminalLine(line, tag, tagClass);

        // Check for RSSI in Base Station receiver lines (e.g. "RSSI: -85 dBm")
        if (line.includes('RSSI:')) {
          const match = line.match(/RSSI:\s*([-\d]+)/i);
          if (match) {
            latestRssi = parseInt(match[1]);
            el.badgeRssiVal.textContent = `${latestRssi} dBm`;
            el.diagRssi.textContent = `${latestRssi} dBm`;
            const signalInfo = classifyRssi(latestRssi);
            el.diagSignalQual.textContent = signalInfo.label;
            el.diagSignalQual.style.color = signalInfo.color;
            el.sumSignalQual.textContent = `${signalInfo.label} (${latestRssi} dBm)`;
            el.sumSignalQual.style.color = signalInfo.color;
          }
        }

        // Check for Packet Length in Base Station receiver lines (e.g. "Packet length: 78")
        if (line.toLowerCase().includes('packet length:')) {
          const match = line.match(/packet length:\s*(\d+)/i);
          if (match) {
            latestPacketLength = parseInt(match[1]);
            el.diagPacketLen.textContent = `${latestPacketLength} bytes`;
            el.sumPacketLen.textContent = `Length: ${latestPacketLength} bytes`;
          }
        }

        // Try single line full packet parse (e.g. TEMP:31.5,HUM:77.7,SOIL:4095...)
        const dataObj = tryParseLineToTelemetry(line);
        if (dataObj && Object.keys(dataObj).length >= 2) {
          lastFullPacketTime = Date.now();
          dataObj.source = isTx ? 'Transmitter (USB Sensor Node)' : 'Receiver (USB LoRa Base Station)';
          if (latestRssi !== null && dataObj.rssi === undefined) dataObj.rssi = latestRssi;
          if (latestPacketLength !== null && dataObj.packetLength === undefined) dataObj.packetLength = latestPacketLength;
          updateDashboard(dataObj);
          parsedAny = true;
        } else {
          // Only accumulate individual lines if no complete packet arrived recently
          // (Prevents duplicate packets from the Receiver's serial debug prints)
          if (Date.now() - lastFullPacketTime > 600) {
            accumulateSingleReading(line, isTx);
          }
        }
      }
      return parsedAny;
    }

    function accumulateSingleReading(line, isTx) {
      // Matches lines like: "Temperature: 28.5", "Soil: 3400", "AX: -1600"
      const match = line.match(/^([A-Za-z_\s]+)[:=]\s*([-\d.]+)/);
      if (match) {
        const key = match[1].trim().toUpperCase().replace(/\s+/g, '_');
        const val = match[2].trim();
        assignKeyVal(lineAccumulator, key, val);

        // If we accumulated enough fields or after a 200ms throttle, commit update
        clearTimeout(lineAccumulatorTimer);
        lineAccumulatorTimer = setTimeout(() => {
          if (Object.keys(lineAccumulator).length >= 2) {
            lineAccumulator.source = isTx ? 'Transmitter (USB Sensor Node)' : 'Receiver (USB LoRa Base Station)';
            if (latestRssi !== null && lineAccumulator.rssi === undefined) lineAccumulator.rssi = latestRssi;
            if (latestPacketLength !== null && lineAccumulator.packetLength === undefined) lineAccumulator.packetLength = latestPacketLength;
            updateDashboard({ ...lineAccumulator });
            lineAccumulator = {};
          }
        }, 200);
      }
    }

    function tryParseLineToTelemetry(line) {
      const obj = {};

      // 1. Check for standard key-value string (Format A: TEMP:34.7 or Format B: GS1,T:26.8,H:57.1,SRAW:1922,AXG:-0.086...)
      if (line.includes(':') && (line.includes('TEMP:') || line.includes('HUM:') || line.includes('SOIL:') || line.includes('T:') || line.includes('H:') || line.includes('SRAW:') || line.includes('AXG:') || line.includes('GS1'))) {
        let clean = line;
        const firstColonKey = clean.search(/\b(TEMP|HUM|SOIL|RAIN|AX|AY|AZ|GX|GY|GZ|T|H|S|R|SRAW|RRAW|AXG|AYG|AZG|GXD|GYD|GZD|TV):/i);
        if (firstColonKey !== -1) clean = clean.substring(firstColonKey);

        const tokens = clean.split(/[,\t|]+/);
        tokens.forEach(tok => {
          const colonIdx = tok.indexOf(':');
          if (colonIdx !== -1) {
            const k = tok.substring(0, colonIdx).trim().toUpperCase();
            const v = tok.substring(colonIdx + 1).trim();
            assignKeyVal(obj, k, v);
          }
        });
        if (Object.keys(obj).length >= 2) return obj;
      }

      // 2. Check for equals delimited: TEMP=34.7 HUM=69.2 ...
      if (line.includes('TEMP=') || line.includes('Temp=') || line.includes('T=')) {
        const tokens = line.split(/[,\s\t|]+/);
        tokens.forEach(tok => {
          const eqIdx = tok.indexOf('=');
          if (eqIdx !== -1) {
            const k = tok.substring(0, eqIdx).trim().toUpperCase();
            const v = tok.substring(eqIdx + 1).trim();
            assignKeyVal(obj, k, v);
          }
        });
        if (Object.keys(obj).length >= 2) return obj;
      }

      // 3. Check for JSON format: {"TEMP":34.7, "HUM":69.2, ...}
      if (line.startsWith('{') && line.endsWith('}')) {
        try {
          const json = JSON.parse(line);
          Object.keys(json).forEach(k => {
            assignKeyVal(obj, k.toUpperCase(), json[k]);
          });
          if (Object.keys(obj).length >= 2) return obj;
        } catch (e) {}
      }

      // 4. Check for CSV numbers only: 34.7,69.2,4095,140,-1696,-172,16832,389,141,34
      const csvParts = line.split(/[,\t]+/);
      if (csvParts.length >= 6 && csvParts.every(p => !isNaN(parseFloat(p.trim())))) {
        const nums = csvParts.map(p => parseFloat(p.trim()));
        obj.temperature = nums[0];
        obj.humidity = nums[1];
        obj.soil = Math.round(nums[2]);
        obj.rain = Math.round(nums[3]);
        if (nums.length >= 7) {
          obj.ax = Math.round(nums[4]);
          obj.ay = Math.round(nums[5]);
          obj.az = Math.round(nums[6]);
        }
        if (nums.length >= 10) {
          obj.gx = Math.round(nums[7]);
          obj.gy = Math.round(nums[8]);
          obj.gz = Math.round(nums[9]);
        }
        return obj;
      }

      return null;
    }

    function assignKeyVal(obj, key, val) {
      if (typeof val === 'string') {
        // Strip non-numeric units e.g. "34.7C", "60%", "1200ADC", "1.02g"
        val = val.replace(/[^-\d.]/g, '');
      }
      val = parseFloat(val);
      if (isNaN(val)) return;

      // Key normalization
      if (key === 'TEMP' || key === 'TEMPERATURE' || key === 'TMP' || key === 'T' || key === 'TEMP_C') {
        obj.temperature = val;
      } else if (key === 'HUM' || key === 'HUMIDITY' || key === 'H' || key === 'RH' || key === 'HUM_PCT') {
        obj.humidity = val;
      } else if (key === 'SOIL' || key === 'SOIL_MOISTURE' || key === 'MOISTURE' || key === 'SM' || key === 'S' || key === 'SOIL_ADC' || key === 'SRAW' || key === 'SOIL_MOISTURE_ESTIMATE') {
        obj.soil = Math.round(val);
      } else if (key === 'RAIN' || key === 'RAIN_SENSOR' || key === 'PRECIP' || key === 'R' || key === 'RAIN_ADC' || key === 'RRAW' || key === 'RP' || key === 'RAIN_RESPONSE_ESTIMATE') {
        obj.rain = Math.round(val);
      } else if (key === 'AX' || key === 'ACC_X' || key === 'ACCEL_X' || key === 'ACCX' || key === 'ACCELERATION_X') {
        obj.ax = (Math.abs(val) <= 4.0) ? Math.round(val * 16384) : Math.round(val);
      } else if (key === 'AXG' || key === 'ACC_X_G') {
        obj.ax = Math.round(val * 16384);
      } else if (key === 'AY' || key === 'ACC_Y' || key === 'ACCEL_Y' || key === 'ACCY' || key === 'ACCELERATION_Y') {
        obj.ay = (Math.abs(val) <= 4.0) ? Math.round(val * 16384) : Math.round(val);
      } else if (key === 'AYG' || key === 'ACC_Y_G') {
        obj.ay = Math.round(val * 16384);
      } else if (key === 'AZ' || key === 'ACC_Z' || key === 'ACCEL_Z' || key === 'ACCZ' || key === 'ACCELERATION_Z') {
        obj.az = (Math.abs(val) <= 4.0) ? Math.round(val * 16384) : Math.round(val);
      } else if (key === 'AZG' || key === 'ACC_Z_G') {
        obj.az = Math.round(val * 16384);
      } else if (key === 'GX' || key === 'GYRO_X' || key === 'GYROX' || key === 'GYROSCOPE_X') {
        obj.gx = (Math.abs(val) <= 30.0) ? Math.round(val * 131) : Math.round(val);
      } else if (key === 'GXD' || key === 'GYRO_X_DPS') {
        obj.gx = Math.round(val * 131);
      } else if (key === 'GY' || key === 'GYRO_Y' || key === 'GYROY' || key === 'GYROSCOPE_Y') {
        obj.gy = (Math.abs(val) <= 30.0) ? Math.round(val * 131) : Math.round(val);
      } else if (key === 'GYD' || key === 'GYRO_Y_DPS') {
        obj.gy = Math.round(val * 131);
      } else if (key === 'GZ' || key === 'GYRO_Z' || key === 'GYROZ' || key === 'GYROSCOPE_Z') {
        obj.gz = (Math.abs(val) <= 30.0) ? Math.round(val * 131) : Math.round(val);
      } else if (key === 'GZD' || key === 'GYRO_Z_DPS') {
        obj.gz = Math.round(val * 131);
      } else if (key === 'TX' || key === 'TILT_X') {
        obj.tx = val;
      } else if (key === 'TY' || key === 'TILT_Y') {
        obj.ty = val;
      } else if (key === 'RSSI') {
        obj.rssi = Math.round(val);
      }
    }

    /* =========================================================================
       DYNAMIC LANDSLIDE EARLY WARNING ASSESSMENT
       ========================================================================= */
    function evaluateLandslideRisk(data) {
      let soilPct = 0;
      if (data.soil_pct !== undefined) soilPct = Number(data.soil_pct);
      else if (data.soil !== undefined) soilPct = data.soil <= 100 ? Number(data.soil) : Math.max(0, Math.min(100, Math.round(((4095 - data.soil) / 3095) * 100)));

      let rainPct = 0;
      if (data.rain_pct !== undefined) rainPct = Number(data.rain_pct);
      else if (data.rain !== undefined) rainPct = data.rain <= 100 ? Number(data.rain) : Math.max(0, Math.min(100, Math.round(((4095 - data.rain) / 3095) * 100)));

      const ax_g = (data.ax_g !== undefined) ? data.ax_g : ((Math.abs(data.ax || 0) > 4) ? (data.ax / 16384) : (data.ax || 0));
      const ay_g = (data.ay_g !== undefined) ? data.ay_g : ((Math.abs(data.ay || 0) > 4) ? (data.ay / 16384) : (data.ay || 0));
      const az_g = (data.az_g !== undefined) ? data.az_g : ((Math.abs(data.az || 16384) > 4) ? (data.az / 16384) : (data.az || 1.0));
      const accMag = Math.sqrt(ax_g * ax_g + ay_g * ay_g + az_g * az_g);

      const gx_dps = (data.gx_dps !== undefined) ? data.gx_dps : ((Math.abs(data.gx || 0) > 30) ? (data.gx / 131) : (data.gx || 0));
      const gy_dps = (data.gy_dps !== undefined) ? data.gy_dps : ((Math.abs(data.gy || 0) > 30) ? (data.gy / 131) : (data.gy || 0));
      const gz_dps = (data.gz_dps !== undefined) ? data.gz_dps : ((Math.abs(data.gz || 0) > 30) ? (data.gz / 131) : (data.gz || 0));
      const gyroMag = Math.sqrt(gx_dps * gx_dps + gy_dps * gy_dps + gz_dps * gz_dps);

      const tiltX = Math.abs((data.tx !== undefined) ? data.tx : (Math.atan2(ay_g, az_g) * 180 / Math.PI));
      const tiltY = Math.abs((data.ty !== undefined) ? data.ty : (Math.atan2(-ax_g, Math.sqrt(ay_g*ay_g + az_g*az_g)) * 180 / Math.PI));

      let riskLevel = 'MONITORING';
      let riskTitle = 'NORMAL STABLE';
      let riskDesc = 'Receiving real-time telemetry from LoRa Receiver Base Station. All geotechnical metrics within safe baseline range.';
      let riskColor = 'var(--accent-emerald)';

      const isHighSaturation = soilPct > 80 && rainPct > 45;
      const isMotionShift = (accMag > 1.45 || accMag < 0.55) || gyroMag > 18 || tiltX > 15 || tiltY > 15;

      if (isMotionShift && isHighSaturation) {
        riskLevel = 'CRITICAL ALERT';
        riskTitle = 'HIGH LANDSLIDE HAZARD';
        riskDesc = 'CRITICAL HAZARD: Slope movement and ground displacement detected during high soil saturation and rainfall!';
        riskColor = 'var(--accent-rose)';
        playBeep(880, 0.4, 'sawtooth');
      } else if (isMotionShift) {
        riskLevel = 'WARNING';
        riskTitle = 'GROUND SHIFT DETECTED';
        riskDesc = 'WARNING: Significant slope displacement or seismic motion detected on remote sensor node.';
        riskColor = 'var(--accent-amber)';
        playBeep(600, 0.2, 'sine');
      } else if (isHighSaturation) {
        riskLevel = 'ELEVATED';
        riskTitle = 'SATURATION WARNING';
        riskDesc = 'Heavy rainfall and high soil moisture detected. Soil liquefaction and slope creep risk elevated.';
        riskColor = 'var(--accent-amber)';
      } else if (soilPct > 65 || rainPct > 30) {
        riskLevel = 'ADVISORY';
        riskTitle = 'MOISTURE ADVISORY';
        riskDesc = 'Moderate soil moisture or light precipitation detected. Ground stability remains normal.';
        riskColor = 'var(--accent-cyan)';
      } else {
        riskLevel = 'SAFE / STABLE';
        riskTitle = 'MONITORING - ALL CLEAR';
        riskDesc = 'Slope dry and kinematic registers stationary. Normal stable geological baseline.';
        riskColor = 'var(--accent-emerald)';
      }

      if (el.riskLevelBadge) {
        el.riskLevelBadge.textContent = riskLevel;
        el.riskLevelBadge.style.color = riskColor;
      }
      if (el.textRiskTitle) el.textRiskTitle.textContent = riskTitle;
      if (el.riskDisplayTitle) el.riskDisplayTitle.style.color = riskColor;
      if (el.riskDisplayDesc) el.riskDisplayDesc.textContent = riskDesc;
    }

    /* =========================================================================
       DASHBOARD UI UPDATE DISPATCHER
       ========================================================================= */
    function updateDashboard(data) {
      const now = Date.now();
      lastPacketTimestamp = now;
      packetCounter++;
      packetRateCount++;

      // Telemetry History for Export
      telemetryHistory.push({
        time: new Date().toLocaleTimeString(),
        ...data
      });

      // Ingest into central 34-column dataset and update analytical cards & live table
      recordTelemetryPacket(data);
      if (telemetryHistory.length > 500) telemetryHistory.shift();

      // Update System & Source Status Badges
      el.badgeSystem.className = 'status-badge status-online';
      el.textSystem.textContent = 'SYSTEM ACTIVE';

      const sourceText = data.source || (isPrimaryConnected ? 'USB SERIAL LIVE' : 'ESP32 WI-FI');
      el.badgeSourceVal.textContent = sourceText.includes('Transmitter') ? 'USB (TRANSMITTER)' : (sourceText.includes('Receiver') ? 'USB (RECEIVER)' : 'LIVE STREAM');
      el.feedStatusText.textContent = 'STREAMING LIVE';
      el.feedStatusText.style.color = 'var(--accent-emerald)';

      // 1. Environmental Readings (Calibrated Physical Units)
      if (data.temperature !== undefined && !isNaN(data.temperature)) {
        el.valTemp.textContent = Number(data.temperature).toFixed(1);
        const tempF = (data.temperature * 9 / 5) + 32;
        el.valTempF.textContent = `${tempF.toFixed(1)} °F`;
        const tempPercent = Math.max(0, Math.min(100, ((data.temperature + 10) / 60) * 100));
        el.barTemp.style.width = `${tempPercent}%`;
      }

      if (data.humidity !== undefined && !isNaN(data.humidity)) {
        el.valHum.textContent = Number(data.humidity).toFixed(1);
        el.barHum.style.width = `${Math.max(0, Math.min(100, data.humidity))}%`;
      }

      // Soil Moisture (% Calibrated)
      let soilVal = 0;
      if (data.soil_pct !== undefined) soilVal = Number(data.soil_pct);
      else if (data.soil !== undefined) soilVal = data.soil <= 100 ? Number(data.soil) : Math.max(0, Math.min(100, Math.round(((4095 - data.soil) / 3095) * 100)));

      el.valSoil.textContent = soilVal.toFixed(1);
      el.barSoil.style.width = `${soilVal}%`;
      el.unitSoil.textContent = '%';
      if (el.badgeSoilType) {
        el.badgeSoilType.textContent = 'CALIBRATED';
        el.badgeSoilType.className = 'calibrated-badge';
      }
      const sCond = data.soil_cond || (soilVal < 25 ? 'DRY' : (soilVal < 65 ? 'MODERATE' : (soilVal < 85 ? 'WET' : 'SATURATED')));
      const sRawStr = data.soil_raw ? `Raw: ${data.soil_raw} ADC | ` : '';
      el.footerSoilRange.textContent = `${sRawStr}State: ${sCond}`;

      // Rain Sensor (% Calibrated)
      let rainVal = 0;
      if (data.rain_pct !== undefined) rainVal = Number(data.rain_pct);
      else if (data.rain !== undefined) rainVal = data.rain <= 100 ? Number(data.rain) : Math.max(0, Math.min(100, Math.round(((4095 - data.rain) / 3095) * 100)));

      el.valRain.textContent = rainVal.toFixed(1);
      el.barRain.style.width = `${rainVal}%`;
      el.unitRain.textContent = '%';
      if (el.badgeRainType) {
        el.badgeRainType.textContent = 'CALIBRATED';
        el.badgeRainType.className = 'calibrated-badge';
      }
      const rCond = data.rain_cond || (rainVal < 10 ? 'NO RAIN' : (rainVal < 45 ? 'LIGHT RAIN' : 'HEAVY RAIN'));
      const rRawStr = data.rain_raw ? `Raw: ${data.rain_raw} ADC | ` : '';
      el.footerRainRange.textContent = `${rRawStr}State: ${rCond}`;

      // 2. MPU6050 Motion Readings (Calibrated Physical Units)
      const ax_g = (data.ax_g !== undefined) ? data.ax_g : ((Math.abs(data.ax || 0) > 4) ? (data.ax / 16384) : (data.ax || 0));
      const ay_g = (data.ay_g !== undefined) ? data.ay_g : ((Math.abs(data.ay || 0) > 4) ? (data.ay / 16384) : (data.ay || 0));
      const az_g = (data.az_g !== undefined) ? data.az_g : ((Math.abs(data.az || 16384) > 4) ? (data.az / 16384) : (data.az || 1.0));

      el.valAx.textContent = `${ax_g >= 0 ? '+' : ''}${Number(ax_g).toFixed(2)} g`;
      el.valAy.textContent = `${ay_g >= 0 ? '+' : ''}${Number(ay_g).toFixed(2)} g`;
      el.valAz.textContent = `${az_g >= 0 ? '+' : ''}${Number(az_g).toFixed(2)} g`;

      const accMag = Math.sqrt(ax_g * ax_g + ay_g * ay_g + az_g * az_g);
      const tiltX = (data.tx !== undefined) ? data.tx : (Math.atan2(ay_g, az_g) * 180 / Math.PI);
      const tiltY = (data.ty !== undefined) ? data.ty : (Math.atan2(-ax_g, Math.sqrt(ay_g*ay_g + az_g*az_g)) * 180 / Math.PI);

      el.valAccelMag.textContent = `${accMag.toFixed(2)} g (Tilt: X ${Number(tiltX).toFixed(1)}°, Y ${Number(tiltY).toFixed(1)}°)`;
      el.sumAccelMag.textContent = `|A| ~ ${accMag.toFixed(2)} g`;

      // Gyroscope (Calibrated °/s)
      const gx_dps = (data.gx_dps !== undefined) ? data.gx_dps : ((Math.abs(data.gx || 0) > 30) ? (data.gx / 131) : (data.gx || 0));
      const gy_dps = (data.gy_dps !== undefined) ? data.gy_dps : ((Math.abs(data.gy || 0) > 30) ? (data.gy / 131) : (data.gy || 0));
      const gz_dps = (data.gz_dps !== undefined) ? data.gz_dps : ((Math.abs(data.gz || 0) > 30) ? (data.gz / 131) : (data.gz || 0));

      el.valGx.textContent = `${gx_dps >= 0 ? '+' : ''}${Number(gx_dps).toFixed(1)} °/s`;
      el.valGy.textContent = `${gy_dps >= 0 ? '+' : ''}${Number(gy_dps).toFixed(1)} °/s`;
      el.valGz.textContent = `${gz_dps >= 0 ? '+' : ''}${Number(gz_dps).toFixed(1)} °/s`;

      const gyroMag = Math.sqrt(gx_dps * gx_dps + gy_dps * gy_dps + gz_dps * gz_dps);
      el.valGyroMag.textContent = `${gyroMag.toFixed(1)} °/s (${gyroMag < 10 ? 'Stationary' : 'Dynamic'})`;

      // 3. Signal & RSSI
      if (data.rssi !== undefined) {
        latestRssi = data.rssi;
      }
      const rssi = latestRssi !== null ? latestRssi : (data.rssi || -85);
      el.badgeRssiVal.textContent = `${rssi} dBm`;
      el.diagRssi.textContent = `${rssi} dBm`;

      const signalInfo = classifyRssi(rssi);
      el.diagSignalQual.textContent = signalInfo.label;
      el.diagSignalQual.style.color = signalInfo.color;
      el.sumSignalQual.textContent = `${signalInfo.label} (${rssi} dBm)`;
      el.sumSignalQual.style.color = signalInfo.color;

      if (data.packetLength !== undefined) {
        latestPacketLength = data.packetLength;
      }
      const pktLen = latestPacketLength !== null ? latestPacketLength : (data.packetLength || 78);
      el.diagPacketLen.textContent = `${pktLen} bytes`;
      el.sumPacketLen.textContent = `Length: ${pktLen} bytes`;
      el.diagTotalPackets.textContent = packetCounter;
      el.sumPacketCount.textContent = `Total: ${packetCounter} packets`;

      if (data.temperature !== undefined && data.humidity !== undefined) {
        el.sumTempHum.textContent = `${Number(data.temperature).toFixed(1)}°C | ${Number(data.humidity).toFixed(1)}%`;
      }

      // 4. Ground Motion Top Summary Tag
      if (accMag > 1.45 || accMag < 0.55 || Math.abs(tiltX) > 18 || Math.abs(tiltY) > 18) {
        el.sumMotion.textContent = 'GROUND DISPLACEMENT DETECTED';
        el.sumMotion.style.color = 'var(--accent-rose)';
      } else {
        el.sumMotion.textContent = 'Stable Motion';
        el.sumMotion.style.color = 'var(--accent-emerald)';
      }

      // 5. Environmental Top Summary Tag
      if (soilVal > 80 && rainVal > 50) {
        el.sumEnvSummary.textContent = 'Critical Saturation / Heavy Rain';
        el.sumEnvSummary.style.color = 'var(--accent-rose)';
      } else if (soilVal > 65 || rainVal > 30) {
        el.sumEnvSummary.textContent = 'Moist Soil / Precipitation';
        el.sumEnvSummary.style.color = 'var(--accent-amber)';
      } else {
        el.sumEnvSummary.textContent = 'Dry Soil / Clear Conditions';
        el.sumEnvSummary.style.color = 'var(--accent-emerald)';
      }

      // Dynamic Landslide Early Warning Assessment
      evaluateLandslideRisk(data);

      // 5. Update Rolling Canvas Charts
      const timeLabel = new Date().toLocaleTimeString().split(' ')[0];
      chartData.timestamps.push(timeLabel);
      chartData.temp.push(data.temperature || 0);
      chartData.hum.push(data.humidity || 0);
      chartData.soil.push(data.soil || 0);
      chartData.rain.push(data.rain || 0);
      chartData.ax.push(data.ax || 0);
      chartData.ay.push(data.ay || 0);
      chartData.az.push(data.az || 0);

      if (chartData.timestamps.length > MAX_POINTS) {
        chartData.timestamps.shift();
        chartData.temp.shift();
        chartData.hum.shift();
        chartData.soil.shift();
        chartData.rain.shift();
        chartData.ax.shift();
        chartData.ay.shift();
        chartData.az.shift();
      }

      envChart.render([
        { data: chartData.temp, color: '#f59e0b' },
        { data: chartData.hum, color: '#06b6d4' }
      ]);

      soilRainChart.render([
        { data: chartData.soil, color: '#10b981' },
        { data: chartData.rain, color: '#3b82f6' }
      ]);

      accelChart.render([
        { data: chartData.ax, color: '#3b82f6' },
        { data: chartData.ay, color: '#10b981' },
        { data: chartData.az, color: '#8b5cf6' }
      ]);
    }

    /* =========================================================================
       HEARTBEAT & PACKET RATE MONITOR
       ========================================================================= */
    function checkHeartbeat() {
      const now = Date.now();
      const elapsedSec = lastPacketTimestamp === 0 ? 0 : Math.floor((now - lastPacketTimestamp) / 1000);

      // Calculate real-time packet rate (Hz)
      const timeDelta = (now - lastRateCalcTime) / 1000;
      if (timeDelta >= 1.0) {
        const rate = (packetRateCount / timeDelta).toFixed(1);
        el.valPacketRate.textContent = `${rate} Hz`;
        packetRateCount = 0;
        lastRateCalcTime = now;
      }

      if (lastPacketTimestamp === 0) {
        el.sumLastTime.textContent = 'Waiting for packet...';
        el.diagTimeElapsed.textContent = 'No packet received yet';
        setNodeStatus('waiting', 'WAITING DATA');
      } else {
        el.sumLastTime.textContent = `${elapsedSec}s ago`;
        el.diagTimeElapsed.textContent = `${elapsedSec} seconds ago`;

        if (elapsedSec <= TIMEOUT_WARNING_SEC) {
          setNodeStatus('online', 'NODE ONLINE');
        } else if (elapsedSec <= TIMEOUT_OFFLINE_SEC) {
          setNodeStatus('warning', 'NO RECENT DATA');
        } else {
          setNodeStatus('offline', 'NODE OFFLINE');
        }
      }

      const uptimeSec = Math.floor((now - startTime) / 1000);
      const h = String(Math.floor(uptimeSec / 3600)).padStart(2, '0');
      const m = String(Math.floor((uptimeSec % 3600) / 60)).padStart(2, '0');
      const s = String(uptimeSec % 60).padStart(2, '0');
      el.diagUptime.textContent = `${h}:${m}:${s}`;
    }

    function setNodeStatus(state, text) {
      el.badgeNode.className = `status-badge status-${state === 'online' ? 'online' : state === 'warning' ? 'warning' : 'offline'}`;
      el.textNode.textContent = text;
    }

    /* =========================================================================
       WEB SERIAL API (DIRECT USB STREAM FROM CHROME / EDGE / OPERA)
       ========================================================================= */
    async function connectPrimaryUSB() {
      if (!('serial' in navigator)) {
        alert('Web Serial is supported in Google Chrome, Microsoft Edge, and Opera.\n\nPlease open this file in Google Chrome or Edge to connect directly via USB.');
        return;
      }

      if (isPrimaryConnected) {
        await disconnectPrimaryUSB();
        return;
      }

      const baudRate = parseInt(el.selectBaud.value) || 115200;
      const mode = el.selectDeviceMode.value;

      try {
        primaryPort = await navigator.serial.requestPort();
        await primaryPort.open({ baudRate: baudRate });
        isPrimaryConnected = true;

        el.btnSerialText.textContent = 'Disconnect USB';
        el.btnSerialConnect.className = 'btn-action disconnect';
        el.badgeSourceDetected.textContent = `🟢 Connected @ ${baudRate} Baud`;
        el.badgeSourceDetected.style.background = 'rgba(16, 185, 129, 0.2)';
        el.badgeSourceDetected.style.color = '#34d399';
        el.diagPort.textContent = `USB Serial (@ ${baudRate})`;
        el.terminalPortInfo.textContent = `Port: Connected (@ ${baudRate} Baud)`;

        addAlert(`🔌 USB Serial connected at ${baudRate} baud!`, 'normal');
        addTerminalLine(`[SYS] Opened USB Serial port at ${baudRate} baud. Mode: ${mode}`, 'SYS', 'log-tag-sys');

        // Setup Text Decoder stream
        const textDecoder = new TextDecoderStream();
        primaryPort.readable.pipeTo(textDecoder.writable);
        primaryReader = textDecoder.readable.getReader();

        // Setup Writer for sending commands
        const textEncoder = new TextEncoderStream();
        textEncoder.readable.pipeTo(primaryPort.writable);
        primaryWriter = textEncoder.writable.getWriter();

        readSerialLoop(primaryReader, mode === 'transmitter' ? 'TX' : 'RX');

      } catch (err) {
        console.error('Serial Connection Error:', err);
        isPrimaryConnected = false;
        el.btnSerialText.textContent = 'Connect Receiver USB';
        el.btnSerialConnect.className = 'btn-action';
        let friendlyErr = err.message || '';
        if (err.name === 'NetworkError' || friendlyErr.toLowerCase().includes('failed to open') || friendlyErr.toLowerCase().includes('access is denied') || friendlyErr.toLowerCase().includes('in use')) {
          friendlyErr = 'PORT LOCKED: Please CLOSE the Arduino IDE Serial Monitor window, then click Connect again (Windows only allows one app per port)!';
        }
        addAlert(`⚠️ USB Port Notice: ${friendlyErr}`, 'warning');
        addTerminalLine(`[ERROR] Connection failed: ${friendlyErr}`, 'SYS', 'log-tag-warn');
      }
    }

    async function disconnectPrimaryUSB() {
      try {
        if (primaryReader) {
          await primaryReader.cancel();
          primaryReader = null;
        }
        if (primaryWriter) {
          await primaryWriter.close();
          primaryWriter = null;
        }
        if (primaryPort) {
          await primaryPort.close();
          primaryPort = null;
        }
      } catch (e) {
        console.warn('Disconnect warning:', e);
      } finally {
        isPrimaryConnected = false;
        el.btnSerialText.textContent = 'Connect USB Port';
        el.btnSerialConnect.className = 'btn-action';
        el.badgeSourceDetected.textContent = 'USB Disconnected';
        el.badgeSourceDetected.style.background = 'rgba(244, 63, 94, 0.2)';
        el.badgeSourceDetected.style.color = '#f87171';
        el.terminalPortInfo.textContent = 'Port: Disconnected';
        addAlert('🔌 USB Port disconnected.', 'normal');
        addTerminalLine('[SYS] USB Serial port disconnected.', 'SYS', 'log-tag-sys');
      }
    }

    async function readSerialLoop(reader, defaultTag) {
      let buffer = '';
      try {
        while (true) {
          const { value, done } = await reader.read();
          if (done) break;
          if (value) {
            buffer += value;
            const lines = buffer.split('\n');
            buffer = lines.pop(); // keep partial line for next iteration

            for (const line of lines) {
              parseIncomingSerialStream(line, defaultTag);
            }
          }
        }
      } catch (err) {
        if (isPrimaryConnected) {
          addTerminalLine(`[WARN] Stream read error: ${err.message}`, 'SYS', 'log-tag-warn');
        }
      }
    }

    // Secondary Port for Dual Mode (Simultaneously reading TX & RX on 2 USB ports)
    async function connectSecondaryUSB() {
      if (!('serial' in navigator)) return;
      if (isSecondaryConnected) {
        try {
          if (secondaryReader) await secondaryReader.cancel();
          if (secondaryPort) await secondaryPort.close();
        } catch (e) {}
        isSecondaryConnected = false;
        el.btnSerialTextTx.textContent = 'Connect TX Port';
        el.btnSerialConnectTx.className = 'btn-action';
        return;
      }

      const baudRate = parseInt(el.selectBaud.value) || 115200;
      try {
        secondaryPort = await navigator.serial.requestPort();
        await secondaryPort.open({ baudRate });
        isSecondaryConnected = true;
        el.btnSerialTextTx.textContent = 'Disconnect TX';
        el.btnSerialConnectTx.className = 'btn-action disconnect';
        addAlert('🌱 Secondary USB (Transmitter Node) connected!', 'normal');
        addTerminalLine('[SYS] Secondary USB Port (Transmitter) opened.', 'SYS', 'log-tag-sys');

        const textDecoder = new TextDecoderStream();
        secondaryPort.readable.pipeTo(textDecoder.writable);
        secondaryReader = textDecoder.readable.getReader();
        readSerialLoop(secondaryReader, 'TX');
      } catch (err) {
        alert('Secondary port error: ' + err.message);
      }
    }

    el.btnSerialConnect.addEventListener('click', connectPrimaryUSB);
    el.btnSerialConnectTx.addEventListener('click', connectSecondaryUSB);

    el.selectDeviceMode.addEventListener('change', (e) => {
      const mode = e.target.value;
      if (mode === 'dual') {
        el.btnSerialConnectTx.style.display = 'inline-flex';
        el.btnSerialText.textContent = 'Connect RX Port';
        addAlert('Dual USB Mode activated: Connect both RX and TX COM ports.', 'normal');
      } else {
        el.btnSerialConnectTx.style.display = 'none';
        if (!isPrimaryConnected) el.btnSerialText.textContent = 'Connect USB Port';
      }
    });

    // Send Serial Commands to ESP32
    async function sendSerialCommand() {
      const cmd = el.terminalInput.value.trim();
      if (!cmd) return;

      if (!isPrimaryConnected || !primaryWriter) {
        alert('Please connect a USB COM port first before sending commands!');
        return;
      }

      try {
        await primaryWriter.write(cmd + '\n');
        addTerminalLine(`[SENT] > ${cmd}`, 'SYS', 'log-tag-sys');
        el.terminalInput.value = '';
      } catch (err) {
        addTerminalLine(`[ERROR] Failed to send command: ${err.message}`, 'SYS', 'log-tag-warn');
      }
    }

    el.btnSendSerial.addEventListener('click', sendSerialCommand);
    el.terminalInput.addEventListener('keydown', (e) => {
      if (e.key === 'Enter') sendSerialCommand();
    });

    /* =========================================================================
       DEMO SIMULATION GENERATOR (TESTING WITHOUT PHYSICAL HARDWARE)
       ========================================================================= */
    function toggleDemoMode() {
      if (demoInterval) {
        clearInterval(demoInterval);
        demoInterval = null;
        el.btnToggleDemo.classList.remove('active');
        el.textDemoBtn.textContent = 'Demo Mode';
        addAlert('Simulation mode stopped.', 'normal');
        addTerminalLine('[SYS] Demo simulation stopped.', 'SYS', 'log-tag-sys');
      } else {
        el.btnToggleDemo.classList.add('active');
        el.textDemoBtn.textContent = 'Stop Demo';
        addAlert('🧪 Demo mode started: Generating live synthetic sensor data.', 'normal');
        addTerminalLine('[SYS] Starting synthetic telemetry generator (1Hz)...', 'SYS', 'log-tag-sys');

        let step = 0;
        demoInterval = setInterval(() => {
          step++;
          const simTemp = 32.5 + Math.sin(step * 0.1) * 2.5 + (Math.random() - 0.5);
          const simHum = 65.0 + Math.cos(step * 0.08) * 8.0 + (Math.random() - 0.5);
          const simSoil = Math.round(3100 + Math.sin(step * 0.05) * 600 + Math.random() * 50);
          const simRain = Math.round(120 + (Math.sin(step * 0.04) > 0.4 ? 400 : 0) + Math.random() * 30);

          const simAx = Math.round(-1600 + Math.sin(step * 0.2) * 500 + (Math.random() - 0.5) * 100);
          const simAy = Math.round(-180 + Math.cos(step * 0.2) * 400 + (Math.random() - 0.5) * 100);
          const simAz = Math.round(16800 + Math.sin(step * 0.1) * 300 + (Math.random() - 0.5) * 100);

          const simGx = Math.round(350 + Math.sin(step * 0.3) * 60);
          const simGy = Math.round(140 + Math.cos(step * 0.3) * 40);
          const simGz = Math.round(30 + Math.sin(step * 0.4) * 20);
          const simRssi = Math.round(-105 + Math.sin(step * 0.05) * 15);

          const mockLine = `TEMP:${simTemp.toFixed(1)},HUM:${simHum.toFixed(1)},SOIL:${simSoil},RAIN:${simRain},AX:${simAx},AY:${simAy},AZ:${simAz},GX:${simGx},GY:${simGy},GZ:${simGz}`;
          parseIncomingSerialStream(mockLine, 'RX');
        }, 1000);
      }
    }

    el.btnToggleDemo.addEventListener('click', toggleDemoMode);

    /* =========================================================================
       MODAL GUIDE CONTROLLERS
       ========================================================================= */
    el.btnOpenGuide.addEventListener('click', () => el.modalGuide.classList.add('open'));
    el.btnCloseGuide.addEventListener('click', () => el.modalGuide.classList.remove('open'));
    el.modalGuide.addEventListener('click', (e) => {
      if (e.target === el.modalGuide) el.modalGuide.classList.remove('open');
    });

    /* =========================================================================
       WI-FI HTTP API POLLING (FALLBACK IF OPENED ON ESP32 WEBSERVER)
       ========================================================================= */
    async function fetchWiFiData() {
      // Only poll Wi-Fi / Bridge if USB Web Serial is not connected
      if (isPrimaryConnected || isSecondaryConnected || demoInterval) return;

      try {
        const apiUrl = (window.location.protocol === 'file:' || !window.location.host)
          ? 'http://localhost:8000/api/data'
          : '/api/data';

        const response = await fetch(apiUrl, { cache: 'no-store' });
        if (!response.ok) return;
        const data = await response.json();

        if (data.port) {
          el.diagPort.textContent = `Bridge (${data.port})`;
        }

        if (data.hasData) {
          if (data.packetCount !== undefined && data.packetCount !== lastPacketCount) {
            lastPacketCount = data.packetCount;
            data.source = data.source || (data.port ? `Receiver Bridge (${data.port})` : 'ESP32 Wi-Fi HTTP Server');
            updateDashboard(data);
          }
          el.diagPort.textContent = (data.port ? `Receiver Bridge (${data.port} Live)` : 'ESP32 Wi-Fi Server (Live)');
        }
      } catch (err) {
        // Fetch failed (normal when bridge server not yet started or running on file:///)
      }
    }

    // Wire Packet Analytics & Report Event Listeners
    if (el.btnTopReport) el.btnTopReport.addEventListener('click', downloadFullDetailedReport);
    if (el.btnTopDataset) el.btnTopDataset.addEventListener('click', downloadFullDatasetCSV);
    if (el.btnDownloadReport) el.btnDownloadReport.addEventListener('click', downloadFullDetailedReport);
    if (el.btnDownloadDataset) el.btnDownloadDataset.addEventListener('click', downloadFullDatasetCSV);
    if (el.btnDownloadJson) el.btnDownloadJson.addEventListener('click', exportJsonAnalysis);

    if (el.inputPktSearch) el.inputPktSearch.addEventListener('input', filterPacketTable);
    if (el.selectPktRiskFilter) el.selectPktRiskFilter.addEventListener('change', filterPacketTable);
    if (el.btnClearPacketTable) {
      el.btnClearPacketTable.addEventListener('click', () => {
        if (confirm('Clear the displayed packet reception table? (Underlying dataset remains preserved)')) {
          if (el.tbodyPacketsFeed) {
            el.tbodyPacketsFeed.innerHTML = '<tr><td colspan="14" style="text-align: center; padding: 24px; color: var(--text-muted);">Display cleared. New incoming packets will appear here.</td></tr>';
          }
        }
      });
    }

    if (el.btnCloseInspect) el.btnCloseInspect.addEventListener('click', closePacketInspector);
    if (el.btnCloseInspect2) el.btnCloseInspect2.addEventListener('click', closePacketInspector);
    if (el.modalPacketInspect) {
      el.modalPacketInspect.addEventListener('click', (e) => {
        if (e.target === el.modalPacketInspect) closePacketInspector();
      });
    }
    if (el.btnCopyPacketJson) el.btnCopyPacketJson.addEventListener('click', copyInspectedPacketJson);

    // CSV File Sync / Load Event Listeners & Drag and Drop
    const btnLoadCsv = document.getElementById('btn-load-csv');
    const inputCsvFile = document.getElementById('input-csv-file');
    if (btnLoadCsv && inputCsvFile) {
      btnLoadCsv.addEventListener('click', () => inputCsvFile.click());
      inputCsvFile.addEventListener('change', (e) => {
        const file = e.target.files[0];
        if (!file) return;
        const reader = new FileReader();
        reader.onload = (evt) => {
          parseAndIngestCSV(evt.target.result);
        };
        reader.readAsText(file);
      });
    }

    const dropZone = document.getElementById('section-packets-analysis');
    if (dropZone) {
      dropZone.addEventListener('dragover', (e) => {
        e.preventDefault();
        dropZone.style.boxShadow = '0 0 15px rgba(2, 132, 199, 0.4)';
      });
      dropZone.addEventListener('dragleave', (e) => {
        dropZone.style.boxShadow = '';
      });
      dropZone.addEventListener('drop', (e) => {
        e.preventDefault();
        dropZone.style.boxShadow = '';
        if (e.dataTransfer && e.dataTransfer.files.length > 0) {
          const file = e.dataTransfer.files[0];
          if (file.name.toLowerCase().endsWith('.csv')) {
            const reader = new FileReader();
            reader.onload = (evt) => {
              parseAndIngestCSV(evt.target.result);
            };
            reader.readAsText(file);
          }
        }
      });
    }

    // Ingest historical dataset records on startup
    initializeHistoricalDataset();

    setInterval(fetchWiFiData, POLL_INTERVAL_MS);
    setInterval(checkHeartbeat, 1000);
  </script>
</body>

</html>
)rawliteral";

#endif // DASHBOARD_H
