# USBreeze

A STM32F103-based USB PWM fan controller for PCs with RGB/ARGB lighting support (Windows 11 Dynamic Lighting).  

> [!WARNING]
> 🚧🚧 PROJECT UNDER CONSTRUCTION 🚧🚧 – use at your own risk.

## Features

- 8× PWM fan channels with RPM monitoring
- Offline fan control using the STM32 on-chip temperature sensor and customizable fan curves (works without host software)
- HID LampArray–implemented ARGB control; compatible with Windows 11 Dynamic Lighting
- 3 configurable ARGB channels, supporting up to 256 LEDs in total

## Supported Hardware

- The **USBreeze board**
- Any **STM32F103 (C6T6 or above)** dev board

~~Please refer [Hardware Readme](Hardware/README.md) for pinouts and more info.~~ TODO

## Firmware
- **Toolchain/IDE**: Keil MDK-ARM v5.30.0.0
- **How to build**: See [Firmware Readme](Firmware/README.md)

## Using USBreeze

### Lighting Control

#### Basic Controls

Control lighting from **Settings -> Personalization -> Dynamic lighting** (effects, brightness, color).

> [!Note]
> Only Windows 11 Dynamic Lighting tested so far. Effects on other platforms are not guaranteed.

#### Channel Configurations

- TODO

### Fan Control

#### 

- TODO

#### Offline Fan Curve Configurations

- TODO

## License

See [LICENSE](LICENSE)

This repository contains hardware design files and firmware source code released
under different licenses. Unless a file states otherwise via an
SPDX-License-Identifier, the following defaults apply:

1) Hardware/  (schematics, PCB layout, BOM, fabrication/mechanical sources)
   - License: [CERN Open Hardware Licence v2 – Weakly Reciprocal](LICENSES/CERN-OHL-W-2.0.txt)
   - SPDX: CERN-OHL-W-2.0
   - Notes: Keep copyright and license notices. When you distribute products or
          outputs based on modified hardware, provide the "source" as defined
          by the licence and include the Source Location.

2) Firmware/  (software/firmware source code)
   - License: [MIT License](LICENSES/MIT.txt)
   - SPDX: MIT

Copyright © 2025 mr258876
