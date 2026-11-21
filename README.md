# 💡 Kortlinje

**Kortlinje** is an open-source RP2350 firmware mod for the _IKEA Kortlinje_ LED display alarm clock, adding Wi-Fi connectivity, a web interface, smart-home integrations, and custom display modes.

It transforms the stock clock into a connected smart display featuring a local web interface, smart-home integrations, and over two dozen custom display modes—all while retaining its core bedside functionality.

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
  - Ambient light sensor and 8-step brightness control
  - 8-step display brightness control
  - Dual-rail power management (USB-C primary with AAA backup)
- **Web UI**: Responsive browser-based app featuring a live real-time display preview and device controls and settings.
- **Local API**: Full two-way state control and telemetry via HTTP REST, WebSockets, and MQTT.

## ⚙️ Hardware & Modding

Kortlinje runs on the **RP2350** architecture with Wi-Fi support (such as the _Raspberry Pi Pico 2 W_).

Modding the clock requires swapping the stock MCU and soldering jumper wires to the internal display drivers, button resistor ladders, buzzer, and onboard sensors. Check out the [Wiki](https://github.com/VIPnytt/Kortlinje/wiki) for step-by-step instructions and pin/wiring diagrams.

## 🏗️ Getting Started & Configuration

### Prerequisites

The project uses [PlatformIO IDE](https://platformio.org/platformio-ide) for dependency management and building. PlatformIO integrates directly major editors via both official and 3rd party [integrations](https://platformio.org/install/integration).

The build scripts require PlatformIO to run on Python 3.13 or newer. If PlatformIO's built-in Python is older, install a recent Python, disable the `platformio-ide.useBuiltinPython` setting in VS Code, delete `~/.platformio/penv` and restart VS Code.

### 1. Configure Secrets

Copy or rename your configuration settings in [secrets.toml](https://github.com/VIPnytt/Kortlinje/blob/main/secrets.toml) to include your Wi-Fi credentials and local preferences.

### 2. Build and Flash

Connect your RP2350 board, open the project workspace in PlatformIO, and trigger the Upload task to compile and flash the firmware.

## 🤖 Connectivity & API

Once flashed, Kortlinje integrates directly into your local network and smart home ecosystem:

- **Wi-Fi Provisioning**: If no configured Wi-Fi network is found on boot, hold the rear cogwheel button to spin up an AP hotspot and configure credentials directly from your phone or PC.
- **Home Assistant**: Native discovery support via MQTT for automatic entity creation, switches, and sensor telemetry.
- **Voice Control**: Alexa bridge emulation for app and voice control of display power and brightness.
- **Developer API**: REST endpoints and real-time WebSocket streams for custom scripts and home automation hubs.

## 🤝 Contributing & Support

Kortlinje is built by makers, for makers. If you've built a unit or are looking to tinker:

- **Bugs & Issues**: Encountered unexpected behavior or a hardware edge case? Please open an [issue](https://github.com/VIPnytt/Kortlinje/issues).
- **Ideas & Discussions**: Want to share wiring tips, suggest features, or show off your build? Join the [discussions](https://github.com/VIPnytt/Kortlinje/discussions).
