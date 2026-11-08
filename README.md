# 💡 Kortlinje

An open-source firmware port for the IKEA Kortlinje LED display alarm clock.

This project is built upon the architecture of the [Frekvens](https://github.com/VIPnytt/Frekvens) framework. With the hardware interface and communication protocols fully solved, core subsystems, graphics pipelines, and connectivity modules are being incrementally ported over to bring up the custom firmware.

> [!NOTE]
> **Early Access:** An initial [hardware modding guide](https://github.com/VIPnytt/Kortlinje/wiki) is available for makers. However, the firmware remains in an active development stage.

## Subsystem Status

| Subsystem | Status | Details |
| :--- | :--- | :--- |
| **Audio** | Operational | Alarm-clock with repeating timers and snooze functionality available. |
| **Buttons** | Operational | All buttons are mapped to available functionality. |
| **Display** | Operational | Fully solved driver with 8-step brightness control. |
| **Networking** | Work in Progress | Framework initialized; local HTTP, REST, and MQTT control APIs are not yet exposed. |
| **Power** | Experimental | Sleep and USB-restore wake logic implemented. Dual-rail battery backup and USB operation supported. |
| **Sensors** | Operational | Temperature and humidity live readings are piped to the display. Ambient light sensor is operational. |
| **Web UI** | Work in Progress | Functional with live display-preview, pending tighter device integration. |
| **Build Environment** | Work in Progress | Python build automation initialized, integration and configuration options pending. |

## Active Display Modes

The firmware currently includes the following standalone render modes switchable via on-device buttons:

- **Clock:** Displays the current time in hours and minutes.
- **Date:** Displays the current month and day of the month.
- **Equalizer:** Autonomous simulated spectrum visualizer.
- **Humidity:** Displays live ambient humidity telemetry from the onboard sensor.
- **Ping-pong:** Autonomous, self-playing demo animation.
- **Snake:** Autonomous, self-playing demo animation.
- **Temperature:** Displays live ambient temperature telemetry from the onboard sensor.
- **Text Ticker:** Horizontal scrolling marquee displaying text.
- **Weather:** Displays live weather telemetry from a configured provider.
- **Weekday:** Displays the current day of the week.

## Hardware & Modding

- **Modding Documentation:** Step-by-step disassembly and wiring instructions are documented in the [Wiki](https://github.com/VIPnytt/Kortlinje/wiki).
- **Power Requirements:** Designed for 5V USB-C operation with support for low-power battery standby (2× AAA).
- **Firmware Build:** Built with PlatformIO targeting RP2350 Arduino framework environments.
- **Hardware Modifications:** Requires opening the enclosure, isolating stock MCU lines, and soldering connections to the VK1640 clock/data buses, button ladder, buzzer driver, and sensor lines.
