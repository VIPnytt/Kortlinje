# 💡 Kortlinje

**Kortlinje** is an open-source RP2350 firmware mod for the *IKEA Kortlinje* LED display alarm clock, adding Wi-Fi connectivity, a web interface, smart-home integrations, and custom display modes.

## 📋 Table of Contents

- [Features](#-features)
- [Hardware](#️-hardware)
- [Getting started](#️-getting-started)
- [Upload to the device](#️-upload-to-the-device)
- [Connectivity & API](#-connectivity--api)
- [Contributing](#-contributing)

## ✨ Features

Kortlinje retains the clock's original bedside functionality while expanding it into a connected, customizable smart display.

### Key Highlights

- **29 display modes**:
  - Time & date
  - Live weather
  - Animations & autonomous games
  - Scrolling text
  - Streaming
- **Full onboard hardware support**:
  - Alarm clock with repeating timers and snooze
  - Live temperature and humidity readings
  - Ambient light sensing
  - 8-step display brightness control
  - Dual-rail power management (USB-C + AAA standby)
- **Web UI** with live display preview
- **API interfaces** (HTTP/REST and MQTT)
- Step-by-step [modding documentation](https://github.com/VIPnytt/Kortlinje/wiki)

## ⚙️ Hardware

### Microcontroller

Kortlinje targets the **RP2350** architecture with Wi-Fi support (such as the *Raspberry Pi Pico 2 W*).

### Wiring & Modding

Modding the device requires removing the stock MCU, and soldering connections to the VK1640 clock/data buses, button ladders, buzzer driver, and sensor lines.

- See the **[Kortlinje Wiki](https://github.com/VIPnytt/Kortlinje/wiki)** for complete, step-by-step disassembly and wiring diagrams.

## 🏗️ Getting started

[PlatformIO IDE](https://platformio.org/platformio-ide) are required to build the project. PlatformIO provides [integrations](https://platformio.org/install/integration) for a wide range of code editors.

### Configuration

Pins and configuration options are set in the [secrets.toml](https://github.com/VIPnytt/Kortlinje/blob/main/secrets.toml) config file.

## ⬆️ Upload to the device

Using PlatformIO, run **Upload** to compile and flash the firmware to the RP2350.

## 🤖 Connectivity & API

Provides local network control and telemetry through a built-in Web UI and a unified, bidirectional API:

- **Web UI:** Browser-based control panel with a live display preview, interactive controls, and multi-network Wi-Fi management.
- **Home Assistant:** Native integration via MQTT device discovery for automatic setup of controls and sensor telemetry.
- **Alexa:** App and voice control of display and brightness through Alexa app and smart home devices.
- **Unified API:** Full two-way state reporting and control across **HTTP REST**, **MQTT**, and **WebSocket**.
- **Hotspot Mode:** Hold the rear cogwheel button if no known networks are available to connect directly to the device and configure Wi-Fi credentials.

## 🤝 Contributing

Kortlinje is open to makers, testers, and contributors. If you have modded a unit:

- **Issues & Bugs:** Found a bug or unexpected behavior? Please [open an issue](https://github.com/VIPnytt/Kortlinje/issues).
- **Discussions & Ideas:** Have questions about wiring or want to coordinate on new features? Jump into the [discussions](https://github.com/VIPnytt/Kortlinje/discussions) section.
