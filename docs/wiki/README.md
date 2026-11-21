# 💡 IKEA Kortlinje

Article number: `906.204.68`

## 📌 Diagrams

### LED panel diagram

```text
 display_sda1 ──────────┐
display_sclk1 ─────────┐│
               ┌───────┼┼────────────────────────┐
               │┌┴┴┴┴┴┴┴┴┴┴┴┴┴┴┐ U1       R31 ┌┴┐│
               │└┬┬┬┬┬┬┬┬┬┬┬┬┬┬┘              └┬┘│
               │                               └─┼─ audio_buzzer
               │  U2                             │
               │ ┌─┐                          B1 ┼ B1
               │ ┤ ├                          B2 ┼ B2
               │ ┤ ├                          B3 ┼ B3
               │ ┤ ├                          B4 ┼ B4
               │ ┤ ├                          B5 ┼ B5
               │ ┤ ├                             │
               │ ┤ ├    U3                     1 ┤
 display_sda2 ─┼─┤ ├   ┌─┐                     2 ┤
display_sclk2 ─┼─┤ ├   ┤ ├                     3 ┤
               │ ┤ ├   ┤ ├    U6               4 ┼─ BAT+
               │ ┤ ├   ┤ ├   ┌─┐                 │
               │ ┤ ├   ┤ ├   ┤ ├             VDD ┼─ +5.0 V DC
               │ ┤ ├   ┤ ├   ┤ ├   Q2  D43   GND ┼─ 0 V DC
               │ ┤ ├   ┤ ├   ┤ ├  ┌─┴─┐┌┴┐    CC ┼─ USB CC
               │ ┤ ├   ┤ ├   ┤ ├  └┬─┬┘└┬┘       │
               │ └─┘   ┤ ├   ┤ ├     └──┴────────┼─ +4.7 V DC
               │       ┤ ├   ┤ ├                 │
               ├ K1    ┤ ├   ┤ ├─────────────────┼─ climate_pdm
               ├ K2    ┤ ├   ┤ ├─────────────────┼─ climate_en
               ├ K3    ┤ ├   └─┘ R32 ┌─┐         │
               │       └─┘          ─┤ ├┬────────┼─ power_sense
 display_sda3 ─┼────────┐            └─┘│        │
display_sclk3 ─┼───────┐│        R33 ┌─┐│ Q3 ┌───┼─ light_sense
               │┌┴┴┴┴┴┴┴┴┴┴┴┴┴┴┐    ─┤ ├┘  ┌─┴─┐ │
               │└┬┬┬┬┬┬┬┬┬┬┬┬┬┬┘U4   └─┘   └┬─┬┘ │
               └─────────────────────────────────┘
```

### Buttons top diagram

```text
┌───┐
│ 1 ┼─ 0 V DC     [white]
│ 2 ┼─ button_top [red]
│ 3 ┼─ +3.3 V DC  [black]
└───┘
```

### Buttons rear diagram

```text
┌───┐
│ 1 ┼─ +3.3 V DC   [blue]
│ 2 ┼─ button_rear [black]
│ 3 ┼─ 0 V DC      [red]
│ 4 ┼─ BAT+        [white]
└───┘
```

### Leg diagram

```text
┌────┐
│ B1 ┼─ B1 [white]
│ B2 ┼─ B2 [red]
│ B3 ┼─ B3 [black]
│ B4 ┼─ B4 [blue]
│ B5 ┼─ B5 [yellow]
└────┘
```

### USB cable diagram

```text
──────┐
White ┼─ +5.0 V DC
Black ┼─ 0 V DC
Green ┼─ USB CC
──────┘
```

### CO2 sensor diagram (optional)

```text
┌─────┐
│ VDD ┼─ +3.3 V DC
│ GND ┼─ 0 V DC
│ SCL ┼─ co2_scl
│ SDA ┼─ co2_sda
└─────┘
```

### RP2350 diagram

```text
┌──────┐
│ VBUS ├─ +5.0 V DC
│ VSYS ├─ +4.7 V DC
│  3V3 ├─ +3.3 V DC
│  GND ├─ 0 V DC
│      │
│  ADC ├─ button_top
│  ADC ├─ button_rear
│      │
│   GP ├─ audio_buzzer
│   GP ├─ display_sclk1
│   GP ├─ display_sclk2
│   GP ├─ display_sclk3
│   GP ├─ display_sda1
│   GP ├─ display_sda2
│   GP ├─ display_sda3
│   GP ├─ climate_en
│   GP ├─ climate_pdm
│   GP ├─ co2_scl
│   GP ├─ co2_sda
│   GP ├─ light_sense
│   GP ├─ power_sense
└──────┘              
```

## 🚀 Getting started

### Opening up

The back cover is held in place by 5 screws under the battery cover and two clips on the right side, which can be pried open with a flathead screwdriver or similar tool. Once the clips are released, the back cover can be removed to access the internal components.

It is recommended to desolder the various wiring harnesses from the main PCB to get better access, as there’s a lot of fine pitch soldering to be done.

### Removing the `U3` chip

The `U3` chip handles display outputs, buzzer output, button inputs, and light input. It is necessary to remove this chip as the RP2350 will be taking over these functions. The `U3` chip can be removed by heating the pins with a soldering iron and gently prying it off the PCB.

### Battery backup

For RP2350 boards with both `VBUS` and `VSYS` pins, offline battery backup is possible. The AAA batteries keep internal time through power cuts and reboots without needing Wi-Fi. Simpler wiring is also possible, but bypasses the battery circuit and the device will need to resync time on every boot.

For simplified wiring without battery backup, these deviations take place:

- Connect 5 V to `VDD`
- Do not connect 4.7 V to `Q2`/`D43`
- Skip the `power_sense` wire to `R32`/`R33`

### Wiring

Wire up everything in accordance with diagrams. It’s advised to defer the USB cable, buttons and environmental sensors to the end. The CO2 sensor is an optional add-on and can be wired up last, or skipped altogether.

## 🔧 Configuration

### Power and ground

- Connect 5.0 V `VBUS` to `VDD`
- Connect `VSYS` (after Scchottky diode) to `Q2` (source) and/or `D43` (left side).

> [!CAUTION]
> To prevent backfeeding, ensure only one power source is connected at a time.

### LED driver clock

Any 5 V tolerant GPIO pin can be used for the `VK1640` `SCLK` clock lines.

Configure in [secrets.toml](https://github.com/VIPnytt/Kortlinje/blob/main/secrets.toml):

```toml
[pin]
display_sclk1 = 1
display_sclk2 = 2
display_sclk3 = 3
```

### LED driver data

Any 5 V tolerant GPIO pin can be used for the `VK1640` `DIN` data lines.

Configure in [secrets.toml](https://github.com/VIPnytt/Kortlinje/blob/main/secrets.toml):

```toml
[pin]
display_sda1 = 4
display_sda2 = 5
display_sda3 = 6
```

### Buzzer

Any GPIO pin can be used for control signals to the buzzer.

Configure in [secrets.toml](https://github.com/VIPnytt/Kortlinje/blob/main/secrets.toml):

```toml
[pin]
audio_buzzer = 7
```

### Environment sensor enable

Any GPIO pin can be used to activate the `U6` chip.

Configure in [secrets.toml](https://github.com/VIPnytt/Kortlinje/blob/main/secrets.toml):

```toml
[pin]
climate_en = 8
```

### Environment sensor data

Any 5 V tolerant GPIO pin can be used to read temperature and humidity data from the `U6` chip.

Configure in [secrets.toml](https://github.com/VIPnytt/Kortlinje/blob/main/secrets.toml):

```toml
[pin]
climate_pdm = 9
```

### Ambient light sensor

Any 5 V tolerant GPIO pin can be used to detect ambient light presence.

Configure in [secrets.toml](https://github.com/VIPnytt/Kortlinje/blob/main/secrets.toml):

```toml
[pin]
light_sense = 10
```

### Power source detection

- `R32` and `R33` are interconnected on the right side, attach a wire to either of them.

Used to wake up the RP2350 from sleep when USB power is applied, and to put it back to sleep when switching to battery backup power.

Any 5 V tolerant GPIO pin can be used to detect the presence of USB power.

Configure in [secrets.toml](https://github.com/VIPnytt/Kortlinje/blob/main/secrets.toml):

```toml
[pin]
power_sense = 11
```

### Snooze, brightness, cycle and alarm buttons

Any analog input pin can be used to detect button presses.

Configure in [secrets.toml](https://github.com/VIPnytt/Kortlinje/blob/main/secrets.toml):

```toml
[pin]
button_top = 26
```

### Config, alarm, plus and minus buttons

Any analog input pin can be used to detect button presses.

Configure in [secrets.toml](https://github.com/VIPnytt/Kortlinje/blob/main/secrets.toml):

```toml
[pin]
button_rear = 27
```

### CO2 sensor (optional)

A Sensirion `SCD40`, `SCD41` or `SCD43` breakout adds a CO2 reading in ppm, shown by the `CO2` display mode and reported over REST, WebSocket and MQTT, including Home Assistant discovery.

- Power the sensor from `3V3`, not `VBUS`, so the I2C lines stay at 3.3 V.
- Most breakout boards include pull-up resistors. If yours doesn't, add 4.7 kΩ pull-ups from `SDA` and `SCL` to `3V3`.

Any free I2C pin pair can be used, where `co2_sda` is an even pin and `co2_scl` is the pin right after it, such as `12`/`13`, `14`/`15`, `16`/`17`, `18`/`19` or `20`/`21`.

Configure in [secrets.toml](https://github.com/VIPnytt/Kortlinje/blob/main/secrets.toml):

```toml
[pin]
co2_scl = 13
co2_sda = 12
```

Remove both lines to disable the sensor. If the sensor doesn’t respond at boot, it is skipped and the clock runs as usual.

> [!NOTE]
> The `SCD40`, `SCD41` and `SCD43` share the same pinout and I2C protocol, so they are interchangeable. The exact variant is detected automatically and reported in the serial log and in the `variant` field of the `SCD4x` extension state.

> [!TIP]
> Automatic self-calibration is enabled by default. It assumes the sensor sees fresh outdoor air (about 400 ppm) at least once a week, so air out the room regularly to keep readings accurate.

## 🔗 Resources

A collection of external links for deeper exploration — provided for reference only and with no formal connection to this project.

- [Hackaday: Another IKEA Product Yields An LED Matrix](https://hackaday.com/2026/10/10/another-ikea-product-yields-an-led-matrix/)
- [IKEA: KORTLINJE](https://www.ikea.com/se/en/p/kortlinje-alarm-clock-with-hygro-thermometer-red-90620468/)
- [RevSpace forum: ikea kortlinje klok hacken](https://forum.revspace.nl/t/ikea-kortlinje-klok-hacken/1129)
- [Sensirion: SCD4x CO2 sensor](https://sensirion.com/products/catalog/SCD41)
