# ESP32 S2

## Features

- based ESP32-S2FN4R2 WIFI IC
- Type-C USB
- 4MB Flash
- 2MB PSRAM
- 27x IO
- ADC, DAC, I2C, SPI, UART, USB OTG
- Compatible with LOLIN D1 mini shields
- Compatible with MicroPython, Arduino, CircuitPython and ESP-IDF


## Technical specs

| Operating Voltage | 3.3V        |
| Digital I/O Pins  | 27          |
| Clock Speed       | 240MHz      |
| Flash             | 4M Bytes    |
| PSRAM             | 2M Bytes    |
| Size              | 34.3*25.4mm |
| Weight            | 2.4g        |

## Overview of ESP32-S2 Mini Pinout

The ESP32-S2 Mini is a compact development board featuring a total of 29 GPIOs. These pins are versatile and can be utilized for various functions, including digital I/O, analog-to-digital conversion (ADC), and touch sensing. Below is a detailed breakdown of the pinout and their functionalities.

| GPIO     | Pin                          | Functions        |
| GPIO Pin | Functionality                | Special Features |
| GPIO0    | Digital I/O                  | Boot, Strapping  |
| GPIO1    | Digital I/O, ADC1_0          | Touch Sensor     |
| GPIO2    | Digital I/O, ADC1_1          | Touch Sensor     |
| GPIO3    | Digital I/O, ADC1_2          | Touch Sensor     |
| GPIO4    | Digital I/O, ADC1_3          | Touch Sensor     |
| GPIO5    | Digital I/O, ADC1_4          | Touch Sensor     |
| GPIO6    | Digital I/O, ADC1_5          | Touch Sensor     |
| GPIO7    | Digital I/O, ADC1_6          | Touch Sensor     |
| GPIO8    | Digital I/O, I2C SDA, ADC1_7 | Touch Sensor     |
| GPIO9    | Digital I/O, I2C SCL, ADC1_8 | Touch Sensor     |
| GPIO10   | Digital I/O, ADC2_0          | Touch Sensor     |
| GPIO11   | Digital I/O, ADC2_1          | Touch Sensor     |
| GPIO12   | Digital I/O, ADC2_2          | Touch Sensor     |
| GPIO13   | Digital I/O, ADC2_3          | Touch Sensor     |
| GPIO14   | Digital I/O, ADC2_4          | Touch Sensor     |
| GPIO15   | Digital I/O, ADC2_5          | Touch Sensor     |
| GPIO16   | Digital I/O                  |                  |
| GPIO17   | Digital I/O                  |                  |
| GPIO18   | Digital I/O                  |                  |
| GPIO19   | Digital I/O                  |                  |
| GPIO20   | Digital I/O                  |                  |
| GPIO21   | Digital I/O                  |                  |
| GPIO22   | Digital I/O                  |                  |
| GPIO23   | Digital I/O                  |                  |
| GPIO24   | Digital I/O                  |                  |
| GPIO25   | Digital I/O                  |                  |
| GPIO26   | Digital I/O                  |                  |
| GPIO27   | Digital I/O                  |                  |
| GPIO28   | Digital I/O                  |                  |
| GPIO29   | Digital I/O                  |                  |

## Communication Protocols

The ESP32-S2 Mini supports several communication protocols, enhancing its versatility for various applications:

I2C: Used for connecting multiple devices with two wires (SDA and SCL).
SPI: A high-speed interface for connecting peripherals.
UART: Serial communication for debugging and data transfer.
This pinout configuration makes the ESP32-S2 Mini an excellent choice for IoT projects, allowing for extensive connectivity and functionality.



## Sources

- https://www.wemos.cc/en/latest/s2/s2_mini.html
- https://docs.zephyrproject.org/latest/boards/wemos/esp32s2_lolin_mini/doc/index.html
