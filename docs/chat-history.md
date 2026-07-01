# StackTower Development Chat History

## Session 1: FSD Creation (2026-05-25)
- Created Function Specifications Document
- Defined LED modes: off, on, fflash, sflash, fade
- Specified MQTT interface (device/cmd, device/state)
- Outlined project architecture
- Established testing strategy

## Session 2: Firmware Scaffold (2026-05-25)
- Added ESP-IDF project structure for ESP32-S2
- Implemented StackTower LED state engine and MQTT command parsing
- Added GPIO PWM output layer with configurable pins
- Added host-side tests for parser, timing, fade, and JSON state output
