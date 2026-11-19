<div align="center">

# STM32 Smart Greenhouse

**Sensing, automatic actuation, a live TFTLCD dashboard, and EEPROM history on STM32.**

A team-built embedded controller using STM32F103ZET6, C, STM32 HAL, and CubeMX, with temperature, humidity, soil-moisture, and light monitoring plus pump, fan, and grow-light control.

[![C](https://img.shields.io/badge/C-STM32F103ZET6-00599C?logo=c&logoColor=white)](https://www.st.com/en/microcontrollers-microprocessors/stm32f103ze.html)
[![STM32 HAL](https://img.shields.io/badge/STM32_HAL-CubeMX-0072B2)](https://www.st.com/en/ecosystems/stm32cube.html)
[![Display](https://img.shields.io/badge/TFTLCD-FSMC-6E56CF)](#firmware-features)
[![Hardware Demo](https://img.shields.io/badge/Hardware_Demo-3%3A06-2AA887)](#hardware-evidence)
[![Host Checks](https://img.shields.io/badge/Control_%26_EEPROM-Verified-2AA887)](#verification)

[Overview](#overview) · [Hardware](#hardware) · [Hardware Evidence](#hardware-evidence) · [Firmware](#firmware-features) · [Build](#build-environment) · [Verification](#verification)

**[Team contributions](AUTHORS.md) · [Verification record](docs/verification.md) · [Third-party notices](THIRD_PARTY_NOTICES.md)**

</div>

---

<img src="docs/dashboard.jpg" alt="Smart greenhouse TFTLCD dashboard showing live sensor and actuator state" width="100%">

<a id="overview"></a>
## 1. Overview

This is a **five-person embedded systems coursework project**. The controller reads four environmental signals, applies configurable thresholds, drives three actuators, displays current state, streams UART CSV data, and keeps a circular EEPROM history.

Cao Junjia's primary contributions were overall system design, hardware selection, pin assignment, BOM and purchasing guidance, and CubeMX peripheral configuration. The complete team breakdown is preserved in [`AUTHORS.md`](AUTHORS.md).

The supplied `SmartGreenhouse.ioc` is the authoritative hardware configuration. IDE-specific files containing another developer's absolute paths are intentionally excluded.

<a id="hardware"></a>
## 2. Hardware

| Module | Connection or component |
| --- | --- |
| MCU | STM32F103ZET6, 72 MHz, LQFP144 |
| Air temperature and humidity | DHT22 on `PG11` |
| Soil moisture | ADC channel 1 on `PA1` |
| Light intensity | ADC channel 2 on `PA2` |
| Pump / fan / grow light | `PC0` / `PC1` / `PC2` |
| Display | TFTLCD over 16-bit FSMC |
| History | AT24C02 EEPROM over I²C |
| Communication | USART1 at 115200 baud |
| Alerts | Buzzer `PB8`, status LEDs, and TFTLCD alarm banner |

<a id="hardware-evidence"></a>
## 3. Hardware Evidence

The original team defense materials contain real hardware photographs and a recorded demonstration of the dashboard and actuator control.

- [Full hardware setup](docs/hardware.jpg)
- [Controller and relay connections](docs/controller.png)
- **[Recorded hardware demonstration — 3:06, H.264/AAC](docs/demo.mp4)**

The video is a compressed copy of the original team recording, shared with team permission. It documents the original implementation; the later EEPROM robustness fix is checked separately by the host-side regression tests.

<a id="firmware-features"></a>
## 4. Firmware Features

- Samples all four sensor values every **200 ms** in the current source.
- Supports automatic, manual, settings, and log-view modes.
- Displays sensor values, actuator state, thresholds, mode, alarm state, and a temperature curve.
- Streams temperature, humidity, soil, and light values as UART CSV.
- Accepts `AUTO`, `MANUAL`, and `DUMP` UART commands.
- Stores six-byte records in a circular EEPROM log holding 40 records.
- Includes a `Hardware_SelfTest` routine, which is not invoked automatically by `main()`.

### Default Control Rules

| Condition | Actuator |
| --- | --- |
| Temperature above `25.0 °C` | Fan on |
| Humidity below `30.0 %RH` | Pump on |
| Soil ADC above `2800` | Pump on |
| Light ADC above `3000` | Grow light on |
| Temperature above `30.0 °C` | Critical alarm |
| Humidity below `15.0 %RH` | Critical alarm |

The current code assumes that a **higher** soil ADC value means drier soil and a **higher** light ADC value means less light. Confirm this polarity against the actual sensor modules before hardware testing. The `soil_dry` and `light_low` names should be interpreted together with those comparisons.

<a id="build-environment"></a>
## 5. Build Environment

The original generated makefile references **GNU Tools for STM32 14.3.rel1**. A firmware build requires:

- STM32CubeIDE or an equivalent `arm-none-eabi-gcc` toolchain
- The supplied `.ioc`, application sources, HAL/CMSIS drivers, and linker script

IDE-generated `.cproject`, `.project`, `.settings`, `Debug`, and `.launch` files are not included. Create a project from `SmartGreenhouse.ioc` in STM32CubeIDE and add the supplied source directories; do not reuse the original machine-specific build files.

The current macOS environment does not contain the ARM cross-compiler or target hardware, so firmware linking has not been rerun for this cleaned repository.

<a id="verification"></a>
## 6. Verification

| Check | Result |
| --- | --- |
| Fan and pump thresholds | Passed with host-side actuator stubs |
| Grow-light and alarm thresholds | Passed with host-side actuator stubs |
| EEPROM write wraparound | 41 writes completed without touching reserved addresses |
| EEPROM dump traversal | Exactly 40 records / 240 byte reads; no reserved-address reads |
| Compiler diagnostics | Host checks pass with `-Wall -Wextra -Werror` |
| Hardware evidence | Real team photographs and a 3:06 recorded demonstration included |
| Fresh ARM build and on-device rerun | **Not performed** in the current environment |

Two related EEPROM address-width issues were corrected in `App/Src/bsp_eeprom.c`. The video records the original implementation and does not independently verify that later fix. Full details are in [`docs/verification.md`](docs/verification.md).

## 7. Repository Structure

```text
STM32-Smart-Greenhouse/
├── App/
│   ├── Inc/                  # Application and BSP interfaces
│   └── Src/                  # Control, UI, sensors, EEPROM, UART, and drivers
├── Core/
│   ├── Inc/                  # MCU and peripheral declarations
│   ├── Src/                  # Main loop and peripheral initialization
│   └── Startup/              # MCU startup code
├── Drivers/                  # STM32 HAL and CMSIS with original licences
├── docs/
│   ├── hardware.jpg          # Complete hardware setup
│   ├── dashboard.jpg         # TFTLCD dashboard
│   ├── controller.png        # Controller and relay connections
│   ├── demo.mp4              # Compressed hardware demonstration
│   └── verification.md
├── SmartGreenhouse.ioc       # CubeMX configuration
├── STM32F103ZETX_FLASH.ld    # Linker script
├── AUTHORS.md
├── THIRD_PARTY_NOTICES.md
└── LICENSE
```

## 8. Authors and Licence

This is a **team coursework project**, and the team has confirmed public release under the [MIT License](LICENSE). The licence applies to original team code, documentation, and media.

STMicroelectronics HAL and ARM/CMSIS components retain their original licences; see [`THIRD_PARTY_NOTICES.md`](THIRD_PARTY_NOTICES.md).

---

<div align="center">

**STM32 Smart Greenhouse — Read the environment. Drive the hardware. Keep the evidence.**

STM32F103ZET6 + HAL + TFTLCD + EEPROM + UART

</div>
