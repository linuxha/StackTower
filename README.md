# StackTower

ESP-IDF firmware for an ESP32-S2 Mini that drives a 5-light stack tower over MQTT.

## Features

- Controls red, yellow, green, blue, and debug LEDs
- Supports `off`, `on`, `fflash`, `sflash`, and `fade` modes
- Subscribes to `device/cmd`
- Publishes JSON state and errors to `device/state`
- Defaults all outputs to off at boot

## Hardware defaults

| Output | GPIO |
| --- | ---: |
| Red | 15 |
| Yellow | 14 |
| Green | 13 |
| Blue | 12 |
| Debug | 1 |
| Piezo | 4 (reserved, not used) |

GPIO assignments, Wi-Fi credentials, broker URI, and MQTT topics are configurable in `idf.py menuconfig`.

## Commands

Command payloads are plain text:

- `red on`
- `yellow fflash`
- `green sflash`
- `blue fade`
- `debug off`
- `all on`
- `all off`
- `all fade`

## Build

```bash
idf.py set-target esp32s2
idf.py build
```

## Test

Host-side logic tests:

```bash
cmake -S tests -B tests/build
ctest --test-dir tests/build --output-on-failure
```
