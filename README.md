# USBreeze

This Project aims to provide a USB based PWM chasis fan controller.

🚧🚧 PROJECT UNDER CONSTRUCTION 🚧🚧

## To Compile

### Compoment requirements

Please configure your run-time environment as follows:

```
CMSIS
    - Core              5.5.0
    - RTOS (API)
        = Keil RTX      4.82.0

CMSIS Driver
    - USB Device (API)  2.3.0
        = USB Device    2.2

Device
    - GPIO              1.3
    - Startup           1.0.0
    - StdPeriph Drivers
        = ADC           3.6.0
        = EXTI          3.6.0
        = Framework     3.6.0
        = GPIO          3.6.0
        = RCC           3.6.0
        = TIM           3.6.0

USB                     6.15.0
    - CORE              6.15.0
    - Device    [1]     6.15.0
        = HID   [2]     6.15.0
```

Software Pack Versions
```
ARM::CMSIS              5.8.0
ARM::CMSIS-Driver       2.10.0
Keil::ARM_Compiler      1.6.3
Keil::MDK-Middleware    7.13.0
Keil::STM32F1xx_DFP     2.4.1
```

Please also configure your ARM compiler version to `Version 5` in `Target` tab of target options, and enable `C99 Mode` in `C/C++` tab.

Compilation tested on MDK v5.30.0.0.