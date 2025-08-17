# USBreeze

A STM32-based USB PWM fan controller for PCs with RGB/ARGB lighting support (Windows 11 Dynamic Lighting).  

> [!WARNING]
> 🚧🚧 PROJECT UNDER CONSTRUCTION 🚧🚧 – use at your own risk.

## Features

- ~~8× PWM fan channels with RPM monitoring~~ TODO
- ~~Offline fan control using the STM32 on-chip temperature sensor and customizable fan curves (works without host software)~~ TODO
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
