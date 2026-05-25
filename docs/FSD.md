# StackTower Function Specifications Document (FSD)

## Overview
Control a 5-LED stack tower and piezo buzzer via MQTT using ESP32-S2 Mini board with IDF.

## Hardware
- ESP32-S2 Mini
- 5-LED Stack Tower (Red, Yellow, Green, Blue, Debug)
- Piezo Buzzer (GPIO 04 - not currently used)

## Pin Mapping
| Component | GPIO |
|-----------|------|
| Red LED   | 15   |
| Yellow LED | 14  |
| Green LED | 13   |
| Blue LED  | 12   |
| Debug LED | 01   |
| Piezo     | 04   |

## LED Control Modes
- **off**: LED off (default)
- **on**: LED on continuously
- **fflash**: Fast flash (100ms on/100ms off)
- **sflash**: Slow flash (500ms on/500ms off)
- **fade**: Fade on and fade off (PWM)

## MQTT Interface

### Command Topic: `device/cmd`
Format: `color command` or `all command`

**Colors**: red, yellow, green, blue, debug (or "all")
**Commands**: on, off, fflash, sflash, fade

Examples:
```
red on
all off
yellow fflash
green fade
```

### State Topic: `device/state`
Current state of all LEDs published asynchronously (format TBD)

## Architecture
1. **LED Driver**: Individual LED control with mode management
2. **MQTT Client**: Subscribe to cmd topic, publish state
3. **Main Loop**: Process MQTT messages, update LED states

## Testing Strategy
- Unit tests for LED control modes
- Integration tests for MQTT command parsing
- Hardware validation on ESP32-S2 Mini

## Deliverables
1. FSD (this document)
2. LED driver module
3. MQTT integration
4. Test suite
5. Build configuration (ESP-IDF)
