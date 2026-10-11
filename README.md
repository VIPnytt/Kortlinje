# 💡 Kortlinje

An open-source firmware port for the IKEA Kortlinje LED display alarm clock.

This project is built upon the architecture of the [Frekvens](https://github.com/VIPnytt/Frekvens) framework. With the hardware interface and communication protocols fully solved, core subsystems, graphics pipelines, and connectivity modules are being incrementally ported over to bring up the custom firmware.

> [!WARNING]
> **Developer Early Access / Experimental State:** All hardware interfaces are now solved and operational, and an initial [hardware modding guide](https://github.com/VIPnytt/Kortlinje/wiki) is available for early contributors and makers. However, the firmware remains in an active experimental stage with core features pending, and it is not yet consumer-ready.

## Subsystem Status

| Subsystem | Status | Details |
| :--- | :--- | :--- |
| **Display** | Operational | Fully functional driver for the triple VK1640 controllers with multi-step brightness control. |
| **Buttons** | Operational | Resistor ladder decoded. Dedicated buttons cycle demo modes and display brightness; remaining buttons emit Serial debug events pending feature mapping. |
| **Sensors** | Operational | Temperature and humidity coprocessor protocol is fully decoded and active; live readings are piped to the display. Ambient light sensor (CdS) is operational. |
| **Audio** | Partial | Buzzer driver verified with test beeps on alarm buttons; dedicated sound and alarm manager pending. |
| **Power** | Partial | Continuous 5V USB-C supply required. Battery sleep and USB wakeup pending. |
| **Networking** | Work in Progress | Framework initialized; local HTTP, REST, and MQTT control APIs are not yet exposed. |

## Active Display Modes

The firmware currently includes three standalone render modes switchable via on-device buttons:

- **Text Ticker:** Horizontal scrolling marquee displaying text along with live ambient temperature and humidity telemetry from the onboard sensors.
- **Snake:** Autonomous, self-playing demo animation.
- **Equalizer:** Autonomous simulated spectrum visualizer.

## Hardware & Modding

- **Modding Documentation:** Step-by-step disassembly and wiring instructions are documented in the [Kortlinje Wiki](https://github.com/VIPnytt/Kortlinje/wiki).
- **Power Requirements:** Continuous 5V via USB-C is required to power the MCU, display drivers, and sensors.
- **Firmware Build:** Built with PlatformIO targeting RP2350 Arduino framework environments.
- **Hardware Modifications:** Requires opening the enclosure, isolating stock MCU lines, and soldering connections to the VK1640 clock/data buses, button ladder, buzzer driver, and sensor lines.
