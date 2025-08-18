# USBreeze Firmware

Firmware of USBreeze PWM Chasis Fan & RGB Controller.

## Compiling Firmware

> [!Note]
> Compilation tested on MDK v5.30.0.0.

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
        = HID   [4]     6.15.0
```

Software Pack Versions
```
ARM::CMSIS              5.8.0
ARM::CMSIS-Driver       2.10.0
Keil::ARM_Compiler      1.6.3
Keil::MDK-Middleware    7.13.0
Keil::STM32F1xx_DFP     2.4.1
```

Memory Regions
```
Segments    Start       Size
ROM         0x8000000   0x7800
RAM         0x20000000  0x2800
```
- The last 2K bytes is reserved for config storage.

Please also configure your Xtal freq to `8.0` Mhz, ARM compiler version to `Version 5` in `Target` tab of target options,  and enable `C99 Mode` in `C/C++` tab.

### Configureations

Please configure these values in following files:

- CMSMS -> RTX_Conf_CM.c
```c
// Number of concurrent running user threads
#define OS_TASKCNT     7
// Default Thread stack size
#define OS_STKSIZE     64   // 256 bytes
// Number of threads with user-provided stack size
#define OS_PRIVCNT     5
// Total stack size for threads with user-provided stack size
#define OS_PRIVSTKSIZE 640  // 2560 bytes

// RTOS Kernel Timer input clock frequency [Hz]
#define OS_CLOCK       72000000
```

- Device -> RTE_Device.h
```c
// High-speed External Clock
#define RTE_HSE                         8000000

// USB Device Full-speed
#define RTE_USB_DEVICE                  1
// CON On/Off Pin
#define RTE_USB_DEVICE_CON_PIN          0
```

- USB -> USBD_Config_0.c
```c
// Vendor ID assigned by USB-IF (idVendor)
#define USBD0_DEV_DESC_IDVENDOR         0x16C0  // <- Temporary use
// Product ID assigned by manufacturer (idProduct)
#define USBD0_DEV_DESC_IDPRODUCT        0x05DF  // <- Temporary use
// evice Release Number in binary-coded decimal (bcdDevice)
#define USBD0_DEV_DESC_BCDDEVICE        0x0100

// Configuration Settings
#define USBD0_CFG_DESC_BMATTRIBUTES     0xC0    // Self-powered, No remote-wakeup

// Manufacturer String
#define USBD0_STR_DESC_MAN              L"Mr258876's"
// Product String
#define USBD0_STR_DESC_PROD             L"USBreeze"

// Enable Serial Number String
#define USBD0_STR_DESC_SER_EN           1
// Serial Number String Default value
#define USBD0_STR_DESC_SER              L"0001A0000000"
// Serial Number Maximum Length
#define USBD0_STR_DESC_SER_MAX_LEN      16
```

- USB -> USBD_Config_HID_0.h
```c
// Assign Device Class to USB Device #
#define USBD_HID0_DEV                             0

// Interrupt IN Endpoint #
#define USBD_HID0_EP_INT_IN                       1
// Interrupt OUT Endpoint #
#define USBD_HID0_EP_INT_OUT                      1

// HID Interface String
#define USBD_HID0_STR_DESC                        L"USBreeze_HID_Fan"
// Number of Input Reports
#define USBD_HID0_IN_REPORT_NUM                   2
// Number of Output Reports
#define USBD_HID0_OUT_REPORT_NUM                  2
// Maximum Input Report Size (in bytes)
#define USBD_HID0_IN_REPORT_MAX_SZ                33
// Maximum Output Report Size (in bytes)
#define USBD_HID0_OUT_REPORT_MAX_SZ               33
// Maximum Feature Report Size (in bytes)
#define USBD_HID0_FEAT_REPORT_MAX_SZ              9
// Use User Provided HID Report Descriptor
#define USBD_HID0_USER_REPORT_DESCRIPTOR          1
// User Provided HID Report Descriptor Size (in bytes)
#define USBD_HID0_USER_REPORT_DESCRIPTOR_SIZE     54
```

- USB -> USBD_Config_HID_1.h
```c
// Assign Device Class to USB Device #
#define USBD_HID0_DEV                             0

// Interrupt IN Endpoint #
#define USBD_HID0_EP_INT_IN                       2
// Interrupt OUT Endpoint #
#define USBD_HID0_EP_INT_OUT                      2

// HID Interface String
#define USBD_HID0_STR_DESC                        L"USBreeze_HID_RGB_A"
// Number of Input Reports
#define USBD_HID0_IN_REPORT_NUM                   7
// Number of Output Reports
#define USBD_HID0_OUT_REPORT_NUM                  7
// Maximum Input Report Size (in bytes)
#define USBD_HID0_IN_REPORT_MAX_SZ                1
// Maximum Output Report Size (in bytes)
#define USBD_HID0_OUT_REPORT_MAX_SZ               1
// Maximum Feature Report Size (in bytes)
#define USBD_HID0_FEAT_REPORT_MAX_SZ              63
// Use User Provided HID Report Descriptor
#define USBD_HID0_USER_REPORT_DESCRIPTOR          1
// User Provided HID Report Descriptor Size (in bytes)
#define USBD_HID0_USER_REPORT_DESCRIPTOR_SIZE     308
```

- USB -> USBD_Config_HID_2.h
```c
// Assign Device Class to USB Device #
#define USBD_HID0_DEV                             0

// Interrupt IN Endpoint #
#define USBD_HID0_EP_INT_IN                       3
// Interrupt OUT Endpoint #
#define USBD_HID0_EP_INT_OUT                      3

// HID Interface String
#define USBD_HID0_STR_DESC                        L"USBreeze_HID_RGB_B"
// Number of Input Reports
#define USBD_HID0_IN_REPORT_NUM                   7
// Number of Output Reports
#define USBD_HID0_OUT_REPORT_NUM                  7
// Maximum Input Report Size (in bytes)
#define USBD_HID0_IN_REPORT_MAX_SZ                1
// Maximum Output Report Size (in bytes)
#define USBD_HID0_OUT_REPORT_MAX_SZ               1
// Maximum Feature Report Size (in bytes)
#define USBD_HID0_FEAT_REPORT_MAX_SZ              63
// Use User Provided HID Report Descriptor
#define USBD_HID0_USER_REPORT_DESCRIPTOR          1
// User Provided HID Report Descriptor Size (in bytes)
#define USBD_HID0_USER_REPORT_DESCRIPTOR_SIZE     308
```

- USB -> USBD_Config_HID_3.h
```c
// Assign Device Class to USB Device #
#define USBD_HID0_DEV                             0

// Interrupt IN Endpoint #
#define USBD_HID0_EP_INT_IN                       3
// Interrupt OUT Endpoint #
#define USBD_HID0_EP_INT_OUT                      3

// HID Interface String
#define USBD_HID0_STR_DESC                        L"USBreeze_HID_RGB_C"
// Number of Input Reports
#define USBD_HID0_IN_REPORT_NUM                   7
// Number of Output Reports
#define USBD_HID0_OUT_REPORT_NUM                  7
// Maximum Input Report Size (in bytes)
#define USBD_HID0_IN_REPORT_MAX_SZ                1
// Maximum Output Report Size (in bytes)
#define USBD_HID0_OUT_REPORT_MAX_SZ               1
// Maximum Feature Report Size (in bytes)
#define USBD_HID0_FEAT_REPORT_MAX_SZ              63
// Use User Provided HID Report Descriptor
#define USBD_HID0_USER_REPORT_DESCRIPTOR          1
// User Provided HID Report Descriptor Size (in bytes)
#define USBD_HID0_USER_REPORT_DESCRIPTOR_SIZE     308
```
