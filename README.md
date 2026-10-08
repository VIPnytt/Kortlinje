# 💡 Kortlinje

An open-source firmware port for the IKEA Kortlinje LED display alarm clock.

This project is built upon the architecture of the [Frekvens](https://github.com/VIPnytt/Frekvens) framework. Core subsystems, graphics pipelines, and connectivity modules are being incrementally ported over as the Kortlinje hardware interface is decoded and stabilized.

> [!WARNING]
> **Developer Early Access:** This repository is currently in an active hardware bring-up phase. It is intended solely for firmware developers and hardware hackers. A consumer modding guide will be provided once hardware interfaces and local APIs are stabilized.

## Subsystem Status

| Subsystem | Status | Details |
| :--- | :--- | :--- |
| **Display** | Operational | Fully functional driver for the triple VK1640 controllers. |
| **Buttons** | Partial | Resistor ladder decoded. Two buttons cycle demo modes; remaining buttons emit raw Serial debug events. |
| **Power** | Operational | Continuous 5V USB-C supply is required. Battery contacts serve strictly as a passive ADC voltage monitor. |
| **Audio** | Partial | Basic test beep implemented on alarm buttons; dedicated sound and alarm manager pending. |
| **Sensors** | Reverse Engineering | Temperature and humidity daughterboard lines require further mapping before MCU integration. The CdS is partially up and running, emitting raw Serial debug events. |
| **Networking** | Work in Progress | Framework initialized; local HTTP, REST, and MQTT control APIs are not yet exposed. |

## Active Display Modes

The current firmware includes three standalone render modes switchable via on-device buttons:

* **Text Ticker:** Horizontal scrolling marquee with two embedded fonts (currently hardcoded pending API configuration).
* **Snake:** Autonomous, self-playing demo animation.
* **Equalizer:** Autonomous simulated spectrum visualizer.

## Hardware & Developer Notes

* **Power Requirements:** The display drivers and MCU require 5V via USB-C. The AAA battery compartment cannot sustain display operation and is used solely for voltage telemetry.
* **Firmware Build:** Built with PlatformIO targeting RP2350 Arduino framework environments.
* **Hardware Modifications:** Requires opening the enclosure, isolating the stock MCU lines, and soldering jumper connections to the VK1640 clock/data bus and button signals.

## Community & Feedback

Feedback, testing, and contributions are welcome. Insights or assistance on the hardware side—especially from anyone with experience analyzing or interfacing with the rear environmental sensor daughterboard—are greatly appreciated.
