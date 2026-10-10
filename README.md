# 💡 Kortlinje

An open-source firmware port for the IKEA Kortlinje LED display alarm clock.

This project is built upon the architecture of the [Frekvens](https://github.com/VIPnytt/Frekvens) framework. With the hardware interface and communication protocols fully solved, core subsystems, graphics pipelines, and connectivity modules are being incrementally ported over to bring up the custom firmware.

> [!WARNING]
> **Developer Early Access:** Hardware reverse engineering is complete, and the project is now actively in the firmware bring-up phase. It is intended primarily for firmware developers and contributors. A consumer-ready modding guide will follow once the firmware stack and local APIs stabilize.

## Subsystem Status

| Subsystem | Status | Details |
| :--- | :--- | :--- |
| **Display** | Operational | Fully functional driver for the triple VK1640 controllers. |
| **Buttons** | Operational | Resistor ladder decoded. Two buttons cycle demo modes; remaining buttons emit Serial debug events pending feature mapping. |
| **Power** | Operational | Continuous 5V USB-C supply required. |
| **Audio** | Partial | Buzzer driver verified with test beeps on alarm buttons; dedicated sound and alarm manager pending. |
| **Sensors** | Protocol Decoded | Temperature and humidity coprocessor protocol is decoded; firmware driver and MCU integration pending. Ambient light sensor (CdS) is operational and emits Serial debug events. |
| **Networking** | Work in Progress | Framework initialized; local HTTP, REST, and MQTT control APIs are not yet exposed. |

## Active Display Modes

The current firmware includes three standalone render modes switchable via on-device buttons:

- **Text Ticker:** Horizontal scrolling marquee with two embedded fonts (currently hardcoded pending API configuration).
- **Snake:** Autonomous, self-playing demo animation.
- **Equalizer:** Autonomous simulated spectrum visualizer.

## Hardware & Developer Notes

- **Power Requirements:** Continuous 5V via USB-C is required to run the MCU and display drivers.
- **Firmware Build:** Built with PlatformIO targeting RP2350 Arduino framework environments.
- **Hardware Modifications:** Requires opening the enclosure, isolating stock MCU lines, and soldering connections to the VK1640 clock/data buses, button ladder, buzzer driver, and sensor lines.
