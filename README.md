# 💡 Kortlinje

**Kortlinje** is an open-source RP2350 firmware mod for the _IKEA Kortlinje_ LED display alarm clock. It turns the stock clock into a Wi-Fi-connected smart display with a local web interface, smart-home integrations, and over two dozen custom display modes—while keeping its core bedside functionality intact.

## 📋 Table of Contents

- [Features](#-features)
- [Hardware & Modding](#️-hardware--modding)
- [Getting Started & Configuration](#️-getting-started--configuration)
- [Connectivity & API](#-connectivity--api)
- [Contributing & Support](#-contributing--support)

## ✨ Features

- **29 display modes**: Time and date, live weather, animations, autonomous games, scrolling text, and live matrix streaming.
- **Full onboard hardware support**:
  - Alarm clock with repeating timers and snooze
  - Live temperature and humidity readings
  - Ambient light sensor
  - 8-step display brightness control
  - Dual-rail power management (USB-C primary with AAA backup)
- **Web UI**: Responsive browser-based interface with a live display preview, interactive controls, and device settings.
- **Local API**: Full two-way state control and telemetry via HTTP REST, MQTT, and WebSocket.

## ⚙️ Hardware & Modding

Kortlinje runs on the **RP2350** architecture with Wi-Fi support (such as the _Raspberry Pi Pico 2 W_).

Modding the clock requires disconnecting the stock MCU and soldering jumper wires to the internal display drivers, button resistor ladders, buzzer, and onboard sensors. See the [Wiki](https://github.com/VIPnytt/Kortlinje/wiki) for step-by-step instructions, pinouts, and wiring diagrams.

## 🏗️ Getting Started & Configuration

### Prerequisites

This project uses [PlatformIO](https://platformio.org/) for building and uploading. It supports most major IDEs and editors via [integrations](https://platformio.org/install/integration), and is also available as a standalone CLI.

### 1. Configuration

Edit [secrets.toml](https://github.com/VIPnytt/Kortlinje/blob/main/secrets.template.toml) to add your Wi-Fi credentials and local preferences.

### 2. Build and Flash

Connect your RP2350 board, then run the **Upload** task in your editor (or `pio run -t upload` from the command line), to compile and flash the firmware.

## 🤖 Connectivity & API

Once flashed, Kortlinje integrates directly into your local network and smart-home ecosystem:

- **Home Assistant**: Native MQTT discovery for automatic setup of device controls and sensor telemetry.
- **Alexa**: App and voice control for display power and brightness.
- **Unified API**: Bidirectional control and telemetry over HTTP REST, MQTT, and WebSocket for custom scripts and home automation hubs.
- **Wi-Fi Fallback**: Hold the rear cogwheel button while powering on to start a hotspot and configure Wi-Fi credentials via the Web UI.

## 🤝 Contributing & Support

Kortlinje is built by makers, for makers. If you've built a unit or are looking to tinker:

- **Bugs & Issues**: Encountered unexpected behavior or a hardware edge case? Please open an [issue](https://github.com/VIPnytt/Kortlinje/issues).
- **Ideas & Discussions**: Want to share wiring tips, suggest features, or show off your build? Join the [discussions](https://github.com/VIPnytt/Kortlinje/discussions).
