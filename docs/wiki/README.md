# 💡 IKEA Kortlinje

Article number: `906.204.68`

## 📌 Diagrams

### LED panel diagram

```text
 DA1 ──────────┐
CLK1 ─────────┐│
      ┌───────┼┼────────────────────────┐
      │┌┴┴┴┴┴┴┴┴┴┴┴┴┴┴┐ U1       R31 ┌┴┐│
      │└┬┬┬┬┬┬┬┬┬┬┬┬┬┬┘              └┬┘│
      │                               └─┼─ Buzzer
      │  U2                             │
      │ ┌─┐                          B1 ┼ B1
      │ ┤ ├                          B2 ┼ B2
      │ ┤ ├                          B3 ┼ B3
      │ ┤ ├                          B4 ┼ B4
      │ ┤ ├                          B5 ┼ B5
      │ ┤ ├                             │
      │ ┤ ├    U3                     1 ┤
 DA2 ─┼─┤ ├   ┌─┐                     2 ┤
CLK2 ─┼─┤ ├   ┤ ├                     3 ┤
      │ ┤ ├   ┤ ├    U6               4 ┼─ BAT+
      │ ┤ ├   ┤ ├   ┌─┐                 │
      │ ┤ ├   ┤ ├   ┤ ├             VDD ┼─ +5.0 V DC
      │ ┤ ├   ┤ ├   ┤ ├   Q2  D43   GND ┼─ 0 V DC
      │ ┤ ├   ┤ ├   ┤ ├  ┌─┴─┐┌┴┐    CC ┼─ USB CC
      │ ┤ ├   ┤ ├   ┤ ├  └┬─┬┘└┬┘       │
      │ └─┘   ┤ ├   ┤ ├     └──┴────────┼─ +4.7 V DC
      │       ┤ ├   ┤ ├                 │
      ├ K1    ┤ ├   ┤ ├─────────────────┼─ Data
      ├ K2    ┤ ├   ┤ ├─────────────────┼─ Enable
      ├ K3    ┤ ├   └─┘ R32 ┌─┐         │
      │       └─┘          ─┤ ├┬────────┼─ Power
 DA3 ─┼────────┐            └─┘│        │
CLK3 ─┼───────┐│        R33 ┌─┐│ Q3 ┌───┼─ Light
      │┌┴┴┴┴┴┴┴┴┴┴┴┴┴┴┐    ─┤ ├┘  ┌─┴─┐ │
      │└┬┬┬┬┬┬┬┬┬┬┬┬┬┬┘U4   └─┘   └┬─┬┘ │
      └─────────────────────────────────┘
```

### Buttons top diagram

```text
┌───┐
│ 1 ┼─ 0 V DC    [white]
│ 2 ┼─ Buttons   [red]
│ 3 ┼─ +3.3 V DC [black]
└───┘
```

### Buttons rear diagram

```text
┌───┐
│ 1 ┼─ +3.3 V DC [blue]
│ 2 ┼─ Buttons   [black]
│ 3 ┼─ 0 V DC    [red]
│ 4 ┼─ BAT+      [white]
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

### RP2350 diagram

```text
┌──────┐
│ VBUS ├─ +5.0 V DC
│ VSYS ├─ +4.7 V DC
│  3V3 ├─ +3.3 V DC
│  GND ├─ 0 V DC
│      │
│  ADC ├─ Buttons top
│  ADC ├─ Buttons rear
│      │
│   GP ├─ SCLK1
│   GP ├─ SCLK2
│   GP ├─ SCLK3
│   GP ├─ SDA1
│   GP ├─ SDA2
│   GP ├─ SDA3
│   GP ├─ Buzzer
│   GP ├─ Light
│   GP ├─ Enable
│   GP ├─ Data
│   GP ├─ Power
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
- Skip the *Power* sensing line to `R32`/`R33`

### Wiring

Wire up everything in accordance with diagrams. It’s advised to defer the USB cable, buttons and environmental sensors to the end.

## 🔧 Configuration

| Label        | Type           | Constant     |
| ------------ | -------------- | ------------ |
| `SCLK1`      | Digital output | `PIN_SCLK1`  |
| `SCLK2`      | Digital output | `PIN_SCLK2`  |
| `SCLK3`      | Digital output | `PIN_SCLK3`  |
| `SDA1`       | Digital output | `PIN_SDA1`   |
| `SDA2`       | Digital output | `PIN_SDA2`   |
| `SDA3`       | Digital output | `PIN_SDA3`   |
| Buzzer       | Digital output | `PIN_BUZZER` |
| Enable       | Digital output | `PIN_EN`     |
| Data         | Digital input  | `PIN_PDM`    |
| Light        | Digital input  | `PIN_LIGHT`  |
| Power        | Digital input  | `PIN_POWER`  |
| Buttons top  | Analog input   | `PIN_TOP`    |
| Buttons rear | Analog input   | `PIN_REAR`   |

### Power and ground

- Connect 5.0 V `VBUS` to `VDD`
- Connect `VSYS` (after Scchottky diode) to `Q2` (source) and/or `D43` (left side).

> [!CAUTION]
> To prevent backfeeding, ensure only one power source is connected at a time.

### LED driver clock

Any 5 V tolerant GPIO pin can be used for the `VK1640` `SCLK` clock lines.

Configure in [secrets.h](https://github.com/VIPnytt/Kortlinje/blob/main/firmware/include/config/secrets.h):

```h
#define PIN_SCLK1 1
#define PIN_SCLK2 2
#define PIN_SCLK3 3
```

### LED driver data

Any 5 V tolerant GPIO pin can be used for the `VK1640` `DIN` data lines.

Configure in [secrets.h](https://github.com/VIPnytt/Kortlinje/blob/main/firmware/include/config/secrets.h):

```h
#define PIN_SDA1 4
#define PIN_SDA2 5
#define PIN_SDA3 6
```

### Buzzer

Any GPIO pin can be used for control signals to the buzzer.

Configure in [secrets.h](https://github.com/VIPnytt/Kortlinje/blob/main/firmware/include/config/secrets.h):

```h
#define PIN_BUZZER 7
```

### Environment sensor enable

Any GPIO pin can be used to activate the `U6` chip.

Configure in [secrets.h](https://github.com/VIPnytt/Kortlinje/blob/main/firmware/include/config/secrets.h):

```h
#define PIN_EN 8
```

### Environment sensor data

Any 5 V tolerant GPIO pin can be used to read temperature and humidity data from the `U6` chip.

Configure in [secrets.h](https://github.com/VIPnytt/Kortlinje/blob/main/firmware/include/config/secrets.h):

```h
#define PIN_PDM 9
```

### Ambient light sensor

Any 5 V tolerant GPIO pin can be used to detect ambient light presence.

Configure in [secrets.h](https://github.com/VIPnytt/Kortlinje/blob/main/firmware/include/config/secrets.h):

```h
#define PIN_LIGHT 10
```

### Power source detection

- `R32` and `R33` are interconnected on the right side, attach a wire to either of them.

Used to wake up the RP2350 from sleep when USB power is applied, and to put it back to sleep when switching to battery backup power.

Any 5 V tolerant GPIO pin can be used to detect the presence of USB power.

Configure in [secrets.h](https://github.com/VIPnytt/Kortlinje/blob/main/firmware/include/config/secrets.h):

```h
#define PIN_POWER 11
```

### Snooze, brightness, cycle and alarm buttons

Any analog input pin can be used to detect button presses.

Configure in [secrets.h](https://github.com/VIPnytt/Kortlinje/blob/main/firmware/include/config/secrets.h):

```h
#define PIN_TOP 12
```

### Config, alarm, plus and minus buttons

Any analog input pin can be used to detect button presses.

Configure in [secrets.h](https://github.com/VIPnytt/Kortlinje/blob/main/firmware/include/config/secrets.h):

```h
#define PIN_REAR 13
```

## 🔗 Resources

A collection of external links for deeper exploration — provided for reference only and with no formal connection to this project.

- [Hackaday: Another IKEA Product Yields An LED Matrix](https://hackaday.com/2026/10/10/another-ikea-product-yields-an-led-matrix/)
- [IKEA: KORTLINJE](https://www.ikea.com/se/en/p/kortlinje-alarm-clock-with-hygro-thermometer-red-90620468/)
- [RevSpace forum: ikea kortlinje klok hacken](https://forum.revspace.nl/t/ikea-kortlinje-klok-hacken/1129)
