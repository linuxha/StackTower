# StackTower Development Context

**Date:** 2026-05-25  
**Status:** FSD Complete, Phase 1 Planning

## Project Summary

StackTower is an ESP32-S2 Mini-based MQTT-controlled LED stack tower controller with a piezo buzzer. The system provides granular LED control via MQTT commands with multiple operational modes (on/off, fast flash, slow flash, fade).

## Current Status

### Completed
- ✅ GitHub Repository Setup (public, authenticated)
- ✅ Canonical FSD Generated (using fsd-writer skill)
- ✅ Documentation Structure Created
- ✅ Initial Repository Commits

### In Progress
- None

### Pending
- [ ] Phase 1: GPIO & LED Driver Foundation
- [ ] Phase 2: MQTT Integration
- [ ] Phase 3: Complete Feature Set

## Hardware Specification

**Device:** ESP32-S2 Mini  
**Framework:** ESP-IDF

### GPIO Mapping
| Component | GPIO | Type |
|-----------|------|------|
| Red LED | 15 | Digital Output |
| Yellow LED | 14 | Digital Output |
| Green LED | 13 | Digital Output |
| Blue LED | 12 | Digital Output |
| Debug LED | 01 | Digital Output |
| Piezo Buzzer | 04 | Digital Output (reserved) |

## LED Control Modes

1. **off** - LED off (default state)
2. **on** - LED on continuously
3. **fflash** - Fast flash (100ms on / 100ms off)
4. **sflash** - Slow flash (500ms on / 500ms off)
5. **fade** - PWM fade in/out

## MQTT Interface

### Topics
- **Command:** `device/cmd` (subscribe)
- **State:** `device/state` (publish)

### Command Format
```
<color> <command>
```

**Colors:** `red`, `yellow`, `green`, `blue`, `debug`, `all`  
**Commands:** `on`, `off`, `fflash`, `sflash`, `fade`

**Examples:**
```
red on
all off
yellow fflash
green fade
```

### State Publishing
Current state of all LEDs published asynchronously (JSON format defined in FSD).

## Architecture Overview

### Logical Architecture
1. **MQTT Client Layer** - Handles broker connection, pub/sub
2. **Command Parser** - Parses device/cmd messages
3. **LED Driver** - Controls individual LED states and modes
4. **GPIO Interface** - Direct hardware control

### Software Components (Planned)
- `led_driver.c/h` - LED control module
- `mqtt_handler.c/h` - MQTT communication
- `main.c` - Application entry, FreeRTOS task setup
- `config.h` - GPIO pins, timing constants
- `test_led_driver.c` - Unit tests

### FreeRTOS Tasks (Planned)
- MQTT task - Handle messages
- LED control task - Update LED states based on mode
- System monitor task (optional)

## Repository Structure

```
StackTower/
├── Documents/
│   ├── stacktower-fsd.md      # Canonical FSD (11 sections, 30+ tests)
│   ├── dev-context.md          # This file
│   ├── chat-history.md         # Session notes
│   └── ESP32-S2-mini.md        # Hardware reference
├── src/                        # Source code (not yet created)
├── test/                       # Test suite (not yet created)
├── CMakeLists.txt             # Build configuration (not yet created)
├── sdkconfig.defaults         # ESP-IDF config (not yet created)
├── .github/                   # GitHub Actions (template ready)
└── README.md                  # Project overview
```

## Key Requirements Summary

### Functional Requirements (23 total)
- FR-1.x: GPIO initialization and control (5 requirements)
- FR-2.x: LED mode management (5 requirements)
- FR-3.x: MQTT communication (7 requirements)
- FR-4.x: State management and reporting (6 requirements)

### Non-Functional Requirements (14 total)
- NFR-1.x: Performance & latency
- NFR-2.x: Reliability & uptime
- NFR-3.x: Power consumption
- NFR-4.x: Security (MQTT authentication)

### Testing Strategy
- **Unit Tests:** LED driver modes, state transitions
- **Integration Tests:** MQTT command parsing, state publishing
- **Hardware Tests:** GPIO timing, LED visual verification
- **Traceability:** All Must/Should requirements mapped to test cases

## Implementation Phases

### Phase 1: GPIO & LED Driver Foundation
**Scope:**
- ESP-IDF project scaffolding
- GPIO configuration for 5 LEDs
- Basic LED control (on/off)
- Fast/slow flash timing implementation
- PWM fade implementation

**Exit Criteria:**
- All GPIO pins working
- All 5 LED modes functioning
- Unit tests passing (TC-LED-001 through TC-LED-010)

### Phase 2: MQTT Integration
**Scope:**
- MQTT broker connection
- Command subscription (device/cmd)
- State publishing (device/state)
- Command parser implementation

**Exit Criteria:**
- MQTT connection stable
- Commands parsed correctly
- State updates published asynchronously

### Phase 3: Complete Feature Set
**Scope:**
- System optimization
- Error recovery procedures
- Watchdog integration
- Production testing

**Exit Criteria:**
- All acceptance tests passing
- Deployment procedures documented

## Assumptions

1. MQTT broker available at runtime (assumed external service)
2. WiFi credentials pre-configured or provided via provisioning
3. ESP-IDF environment available (v5.0+)
4. LEDs are driven directly from GPIO pins (no external drivers)
5. Piezo buzzer support deferred to Phase 3+

## Dependencies

- ESP-IDF (v5.0 or later)
- PubSubClient or esp_mqtt library
- FreeRTOS (bundled with ESP-IDF)
- CMake build system

## Next Steps

1. **Create ESP-IDF project structure**
   - Initialize CMakeLists.txt
   - Configure sdkconfig.defaults
   - Set up build environment

2. **Implement LED driver module**
   - GPIO initialization
   - Mode management (on/off/flash/fade)
   - Timing validation against FSD specs

3. **Add MQTT client scaffolding**
   - Broker connection logic
   - Topic subscription
   - Message callback handlers

4. **Create unit test framework**
   - Test harness for LED modes
   - Mock MQTT interface

## Key Files to Review

- `Documents/stacktower-fsd.md` - Full specification (read Section 6 for interface specs, Section 8 for tests)
- `docs/ESP32-S2-mini.md` - Hardware reference guide
- `docs/esp32-s2_datasheet_en.pdf` - Datasheet
- `docs/esp32-s2_technical_reference_manual_en.pdf` - Technical reference

## Development Mode: Caveman
- Succinct, no unnecessary chatter
- Push changes frequently with clear commit messages
- Document learnings in chat-history.md
- Ask clarifying questions only when essential
