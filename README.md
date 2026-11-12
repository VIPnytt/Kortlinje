# 💡 Kortlinje

**Kortlinje** is an open-source RP2350 firmware mod for the *IKEA Kortlinje* LED display alarm clock, adding Wi-Fi connectivity, a web interface, smart-home integrations, and custom display modes.

Built upon the modular architecture of the [Frekvens](https://github.com/VIPnytt/Frekvens) framework, this project replaces the stock controller to unlock full control over the display, onboard sensors, and alarm subsystems.

> [!NOTE]
> **Ready for Modders:** The [hardware modding guide](https://github.com/VIPnytt/Kortlinje/wiki) and core firmware are ready for makers to flash, use, and tinker with. Because the project is under active pre-release development, some areas are still being refined and breaking changes may occur. Testing, feedback, and contributions are very welcome!

## 📋 Table of Contents

- [Features](#-features)
- [Display modes](#️-display-modes)
- [Hardware](#️-hardware)
- [Getting started](#️-getting-started)
- [Upload to the device](#️-upload-to-the-device)
- [Connectivity & API](#-connectivity--api)
- [Contributing](#-contributing)

## ✨ Features

Kortlinje retains the clock's original bedside functionality while expanding it into a connected, customizable smart display.

### Key Highlights

- **12 display modes**:
  - Time & Date
  - Environmental telemetry
  - Autonomous games & animations
  - Scrolling text
  - Live weather
- **Full onboard hardware support**:
  - Alarm clock with repeating timers and snooze
  - Live temperature and humidity readings
  - Ambient light sensing
  - 8-step display brightness control
  - Dual-rail power management (USB-C + AAA standby)
- **Web UI** with live display preview
- **API interfaces** (HTTP/REST and MQTT)
- Step-by-step [modding documentation](https://github.com/VIPnytt/Kortlinje/wiki)

## 🖥️ Display modes

The firmware includes 12 standalone render modes switchable directly via the on-device buttons or the web interface:

- **Breakout:** Autonomous, self-playing demo animation.
- **Clock:** Displays the current time in hours and minutes.
- **Date:** Displays the current month and day of the month.
- **Equalizer:** Autonomous simulated spectrum visualizer.
- **Humidity:** Displays live ambient humidity telemetry from the onboard sensor.
- **Ping-pong:** Autonomous, self-playing demo animation.
- **Scan:** Autonomous, self-playing demo animation.
- **Snake:** Autonomous, self-playing demo animation.
- **Temperature:** Displays live ambient temperature telemetry from the onboard sensor.
- **Text Ticker:** Horizontal scrolling marquee displaying custom text.
- **Weather:** Displays live weather telemetry from a configured provider.
- **Weekday:** Displays the current day of the week.

## ⚙️ Hardware

### Microcontroller

Kortlinje targets the **RP2350** architecture with Wi-Fi support (such as the *Raspberry Pi Pico 2 W*).

### Built-in Peripherals

Unlike pure LED matrix panels, the IKEA Kortlinje includes several integrated peripherals that the firmware natively drives:

- **Display:** 3× VK1640 LED controller ICs
- **Climate sensor:** Dedicated onboard temperature and humidity module
- **Photocell:** Ambient light sensor for display adaptation
- **Buzzer:** Internal piezo transducer for alarms and timer alerts
- **Buttons:** 4 top and 4 rear control buttons (resistor ladders)
- **Battery backup:** 2× AAA compartment for low-power standby

### Wiring & Modding

Modding the device requires opening the enclosure, isolating the stock MCU lines, and soldering connections to the VK1640 clock/data buses, button ladders, buzzer driver, and sensor lines.

- See the **[Kortlinje Wiki](https://github.com/VIPnytt/Kortlinje/wiki)** for complete, step-by-step disassembly and wiring diagrams.

## 🏗️ Getting started

[PlatformIO IDE](https://platformio.org/platformio-ide) are required to build the project. PlatformIO provides [integrations](https://platformio.org/install/integration) for a wide range of code editors.

### Configuration

Wi-Fi credentials and pins are set in [secrets.h](https://github.com/VIPnytt/Kortlinje/blob/main/firmware/include/config/secrets.h), while weather coordinates and device settings are set in the [.env](https://github.com/VIPnytt/Kortlinje/blob/main/.env) config file.

## ⬆️ Upload to the device

Using PlatformIO, run **Upload** to compile and flash the firmware to the RP2350.

## 🤖 Connectivity & API

Provides local network control and telemetry through a built-in Web UI and a unified, bidirectional API:

- **Web UI:** Browser-based control panel with a live display preview, interactive controls, and multi-network Wi-Fi management.
- **Hotspot Mode:** Hold the rear cogwheel button if no known networks are available to connect directly to the device and configure Wi-Fi credentials.
- **Home Assistant:** Native integration via MQTT device discovery for automatic setup of controls and sensor telemetry.
- **Unified API:** Full two-way state reporting and control across **HTTP REST**, **MQTT**, and **WebSocket**.

## 🤝 Contributing

Kortlinje is open to makers, testers, and contributors. If you have modded a unit:

- **Issues & Bugs:** Found a bug or unexpected behavior? Please [open an issue](https://github.com/VIPnytt/Kortlinje/issues).
- **Discussions & Ideas:** Have questions about wiring or want to coordinate on new features? Jump into [GitHub Discussions](https://github.com/VIPnytt/Kortlinje/discussions).
