# StackTower - Functional Specification Document (FSD)

**Version:** 1.0  
**Date:** 2024  
**Status:** Release Candidate  
**Prepared by:** Engineering Team  
**Document Classification:** Technical Specification

---

## Table of Contents

1. [System Overview](#system-overview)
2. [System Architecture](#system-architecture)
3. [Implementation Phases](#implementation-phases)
4. [Functional & Non-Functional Requirements](#functional--non-functional-requirements)
5. [Risks, Assumptions & Dependencies](#risks-assumptions--dependencies)
6. [Interface Specifications](#interface-specifications)
7. [Operational Procedures](#operational-procedures)
8. [Verification & Validation](#verification--validation)
9. [Troubleshooting Guide](#troubleshooting-guide)
10. [Appendix](#appendix)

---

## 1. System Overview

### 1.1 Purpose

The StackTower system is an IoT-enabled LED stack tower controller that provides remote control of a 5-LED stack tower and piezo buzzer via MQTT. The system shall enable real-time monitoring and control of indicator lights through network connectivity, making it suitable for production line status indication, alarm systems, and visual feedback applications.

### 1.2 Scope

This FSD defines the complete functional and non-functional requirements for the StackTower system based on the ESP32-S2 Mini microcontroller platform with ESP-IDF firmware development framework.

**In Scope:**
- MQTT-based command reception and state publication
- LED control with multiple operational modes
- Piezo buzzer interface (reserved for future use)
- Real-time state tracking and reporting
- GPIO pin management and PWM control
- Network connectivity via Wi-Fi

**Out of Scope:**
- Actual buzzer sound generation (Reserved)
- Legacy parallel port interfaces
- Wireless protocols other than Wi-Fi
- Encryption/TLS for MQTT connections (assumed handled at platform level)
- Web-based UI or mobile applications

### 1.3 Key Features

- **Multi-Mode LED Control:** Off, On, Fast Flash (100ms), Slow Flash (500ms), PWM Fade
- **MQTT Interface:** Standard pub/sub messaging for remote control
- **5-Color Stack Tower:** Red, Yellow, Green, Blue + Debug LED
- **Real-Time State Publishing:** Automatic state synchronization after commands
- **Modular Architecture:** Independent LED control routines

---

## 2. System Architecture

### 2.1 Logical Architecture

```
┌─────────────────────────────────────────────────────┐
│                   IoT Network / MQTT Broker         │
└────────────────────────┬────────────────────────────┘
                         │
                    Subscribe/Publish
                    device/cmd, device/state
                         │
        ┌────────────────┴────────────────┐
        │                                 │
   ┌────▼─────────┐            ┌──────────▼──────┐
   │ MQTT Handler │            │ Message Parser  │
   │ Task         │            │ & Dispatcher    │
   └────┬─────────┘            └──────────┬──────┘
        │                                 │
        ├──────────────┬──────────────────┤
        │              │                  │
   ┌────▼──────┐  ┌────▼──────┐  ┌───────▼──────┐
   │LED Control│  │ GPIO      │  │Buzzer        │
   │ Manager   │  │ Manager   │  │Control       │
   │ (PWM/Bits)│  │           │  │              │
   └─────┬─────┘  └─────┬─────┘  └────────┬─────┘
         │              │                 │
         └──────┬───────┴────────┬────────┘
                │                │
          ┌─────▼────────────────▼──────┐
          │   GPIO & Peripheral Layer   │
          │   (ESP-IDF Drivers)         │
          └─────────────┬───────────────┘
                        │
          ┌─────────────▼──────────────────────┐
          │   Hardware Layer                   │
          │   GPIO 15, 14, 13, 12, 04, 01      │
          │   PWM, Timer Peripherals           │
          └────────────────────────────────────┘
```

### 2.2 Hardware Architecture

#### 2.2.1 Processor
- **Microcontroller:** ESP32-S2 Mini
- **CPU:** Xtensa 32-bit single-core (240 MHz)
- **RAM:** 320 KB
- **Flash:** 4 MB
- **GPIO Pins:** 23 total (21 usable)
- **PWM Channels:** 8 LED PWM
- **Timers:** 4x 32-bit timers

#### 2.2.2 Peripheral Connections

| Function | GPIO | Direction | Type | Mode |
|----------|------|-----------|------|------|
| Red LED | 15 | Output | Digital/PWM | Active High |
| Yellow LED | 14 | Output | Digital/PWM | Active High |
| Green LED | 13 | Output | Digital/PWM | Active High |
| Blue LED | 12 | Output | Digital/PWM | Active High |
| Piezo Buzzer | 04 | Output | PWM | Active High (Reserved) |
| Debug LED | 01 | Output | Digital/PWM | Active High |

#### 2.2.3 Electrical Specifications

- **LED Operating Voltage:** 3.3V (GPIO direct drive for low-power indicators)
- **LED Current per Pin:** ≤40 mA (typical operation)
- **PWM Frequency:** 1000 Hz (assumed for LED control)
- **Buzzer Interface:** GPIO 04 (reserved, not actively used)

### 2.3 Software Architecture

#### 2.3.1 Firmware Stack

```
┌──────────────────────────────────────────┐
│  Application Layer                       │
│  ├─ MQTT Task & Event Handlers           │
│  ├─ LED Command Processor                │
│  └─ State Management                     │
├──────────────────────────────────────────┤
│  Middleware Layer                        │
│  ├─ MQTT Client (esp_mqtt)               │
│  ├─ PWM Control Abstraction              │
│  └─ Timer Management                     │
├──────────────────────────────────────────┤
│  ESP-IDF Core Drivers                    │
│  ├─ GPIO Driver                          │
│  ├─ Timer Driver (LED_PWM)               │
│  ├─ WiFi Driver                          │
│  └─ TCP/IP Stack                         │
├──────────────────────────────────────────┤
│  FreeRTOS Real-Time Kernel               │
│  ├─ Task Scheduler                       │
│  ├─ Queue Management                     │
│  └─ Synchronization Primitives           │
└──────────────────────────────────────────┘
```

#### 2.3.2 Task Architecture

**Main Tasks:**
1. **WiFi/Network Task** - Manages network connectivity (esp_netif, WiFi provisioning)
2. **MQTT Task** - Handles broker connection and message routing
3. **LED Control Task** - Processes LED commands and maintains state
4. **Timer/PWM Task** - Manages fade and flash timing

---

## 3. Implementation Phases

### Phase 1: Hardware Initialization & GPIO Setup (Weeks 1-2)

**Deliverables:**
- GPIO pin configuration for 6 outputs (R, Y, G, B, Debug, Buzzer)
- Basic digital LED control (on/off only)
- Verification of hardware connections
- GPIO interrupt and state verification

**Acceptance Criteria:**
- All GPIO pins toggle correctly via CLI commands
- No pin conflicts or initialization errors
- State persistence across resets

### Phase 2: LED Control Modes & PWM Implementation (Weeks 3-4)

**Deliverables:**
- Fast flash mode (100ms on/off)
- Slow flash mode (500ms on/off)
- PWM fade implementation (0-100% brightness)
- Timer-based state management
- LED mode transition logic

**Acceptance Criteria:**
- All 5 flash modes function correctly
- PWM fade smooth and linear
- No mode conflicts or state corruption
- Timer accuracy ±10%

### Phase 3: MQTT Integration & System Testing (Weeks 5-7)

**Deliverables:**
- MQTT client connection and reconnection logic
- Command parsing and validation
- State publishing on device/state topic
- Integration testing with Mosquitto broker
- System stress testing

**Acceptance Criteria:**
- MQTT commands processed correctly
- State synchronized across multiple clients
- Broker reconnection within 5 seconds
- System stability for 24+ hours continuous operation

---

## 4. Functional & Non-Functional Requirements

### 4.1 Functional Requirements

#### 4.1.1 LED Control

**REQ-F1.1:** The system shall support **off** mode where all GPIO outputs are pulled low (LED disabled).

**REQ-F1.2:** The system shall support **on** mode where GPIO output is held high at 3.3V (LED continuously lit).

**REQ-F1.3:** The system shall support **fflash** (fast flash) mode with 100±5ms on-time and 100±5ms off-time per cycle.

**REQ-F1.4:** The system shall support **sflash** (slow flash) mode with 500±10ms on-time and 500±10ms off-time per cycle.

**REQ-F1.5:** The system shall support **fade** mode where LED brightness transitions smoothly from 0% to 100% and back using PWM, with duty cycle resolution of 1% steps.

**REQ-F1.6:** LED mode changes shall be applied within 50ms of command receipt.

#### 4.1.2 MQTT Interface

**REQ-F2.1:** The system shall subscribe to the MQTT topic `device/cmd` for incoming control commands.

**REQ-F2.2:** The system shall publish state updates to MQTT topic `device/state` after every command execution.

**REQ-F2.3:** Command format shall be "color command" (e.g., "red on", "yellow fflash") or special command "all command" (e.g., "all off", "all on").

**REQ-F2.4:** Valid color identifiers shall be: red, yellow, green, blue, debug.

**REQ-F2.5:** Valid commands shall be: on, off, fflash, sflash, fade.

**REQ-F2.6:** Invalid commands shall be rejected with error message published to `device/state` topic.

**REQ-F2.7:** The system shall publish current state of all LEDs when connecting to MQTT broker (as availability heartbeat).

#### 4.1.3 State Management

**REQ-F3.1:** The system shall maintain current state for each LED (color, mode, brightness percentage for fade mode).

**REQ-F3.2:** State shall persist across MQTT disconnections and restore upon reconnection.

**REQ-F3.3:** The system shall publish complete state JSON to `device/state` containing all LED states after each command.

**REQ-F3.4:** State publication shall include timestamp of state change (Unix epoch).

#### 4.1.4 Special Commands

**REQ-F4.1:** Command "all on" shall set all 5 LEDs (red, yellow, green, blue, debug) to **on** mode simultaneously.

**REQ-F4.2:** Command "all off" shall set all 5 LEDs to **off** mode simultaneously.

### 4.2 Non-Functional Requirements

#### 4.2.1 Performance

**REQ-N1.1:** Command processing latency shall be ≤50ms from MQTT message receipt to GPIO state change.

**REQ-N1.2:** MQTT publish latency (state updates) shall be ≤100ms after command execution.

**REQ-N1.3:** System shall support command reception rate of at least 100 commands per second without message loss.

**REQ-N1.4:** Flash timing accuracy shall be ±10% of specified intervals (100ms for fflash, 500ms for sflash).

#### 4.2.2 Reliability

**REQ-N2.1:** System shall recover from MQTT broker disconnection and automatically reconnect within 5 seconds.

**REQ-N2.2:** System shall maintain LED state during MQTT disconnection (fail-safe: LEDs remain in last commanded state).

**REQ-N2.3:** System shall detect and recover from GPIO driver errors without system reset required.

**REQ-N2.4:** Mean time between failures (MTBF) shall be ≥720 hours (30 days continuous operation).

#### 4.2.3 Availability

**REQ-N3.1:** System shall be operational 24/7 under normal network conditions.

**REQ-N3.2:** System shall handle network outages gracefully (no crashes, state preservation).

**REQ-N3.3:** System shall reconnect to MQTT broker within 5 seconds of broker availability.

#### 4.2.4 Security & Safety

**REQ-N4.1:** GPIO outputs shall default to **off** state on system startup (all LEDs disabled until commanded).

**REQ-N4.2:** System shall validate command format before processing (reject malformed messages).

**REQ-N4.3:** System shall limit PWM frequency to range 500-2000 Hz to prevent damage to indicator LEDs. (assumed)

**REQ-N4.4:** System shall enforce GPIO pin isolation (no pin conflicts or shared I/O).

#### 4.2.5 Maintainability

**REQ-N5.1:** Firmware shall include debug output on UART (assumed baud rate 115200) for troubleshooting.

**REQ-N5.2:** Configuration parameters (WiFi SSID, MQTT broker IP, GPIO pins) shall be externally configurable. (assumed)

**REQ-N5.3:** Code shall include inline documentation for LED mode algorithms.

#### 4.2.6 Scalability

**REQ-N6.1:** Architecture shall support addition of up to 8 total GPIO-controlled outputs (assumed).

**REQ-N6.2:** MQTT topic structure shall support multiple device instances via device ID parameterization. (assumed)

---

## 5. Risks, Assumptions & Dependencies

### 5.1 Risks

| Risk ID | Description | Likelihood | Impact | Mitigation |
|---------|-------------|------------|--------|-----------|
| R1 | MQTT broker unreachable | Medium | High | Implement automatic reconnection with exponential backoff; offline command queuing |
| R2 | GPIO driver conflicts | Low | Critical | Dedicated GPIO initialization during boot; mutex protection on GPIO access |
| R3 | Timer interrupt overrun during heavy flash mode | Low | Medium | Ensure timer ISR completes <10ms; use hardware timers (not software) |
| R4 | WiFi connectivity instability | Medium | Medium | Implement WiFi monitoring task; auto-reconnect with scan |
| R5 | Command message parsing errors | Low | Low | Validate all inputs; reject malformed commands; publish error state |
| R6 | PWM frequency out of tolerance | Low | Low | Calibrate timer dividers; periodic frequency verification |
| R7 | Memory exhaustion (FreeRTOS heap) | Low | Critical | Monitor heap usage; implement static buffers for MQTT payloads |
| R8 | Electromagnetic interference affecting GPIO | Medium | Medium | Use shielded cables for LED outputs; add RC filters if needed |

### 5.2 Assumptions

| Assumption ID | Description | Rationale |
|---------------|-------------|-----------|
| A1 | ESP32-S2 Mini is pre-flashed with ESP-IDF bootloader | Standard platform baseline |
| A2 | MQTT broker runs on accessible network (no TLS required) | Assumed for development; production requires security review |
| A3 | WiFi credentials provided via configuration file or provisioning | Standard IoT deployment pattern |
| A4 | LED indicators are current-limiting type (no external resistors required) | GPIO output directly drives indicator LEDs at 3.3V |
| A5 | Piezo buzzer control via GPIO 04 reserved for future use | Currently not implemented per specification |
| A6 | PWM frequency of 1000 Hz is optimal for LED visual control | Standard for indicator LEDs; imperceptible flicker |
| A7 | System operates in temperature range 0°C to 40°C | Typical industrial environment |
| A8 | MQTT message format is plain text JSON | No binary protocols required |
| A9 | Single MQTT broker instance (no clustering) | Simplified deployment model |
| A10 | Debug LED always controlled independently (not affected by "all" commands) | Separate indicator for firmware status |

### 5.3 Dependencies

| Dependency ID | Component | Version | Type |
|---------------|-----------|---------|------|
| D1 | ESP-IDF Framework | 5.0+ | Build-time |
| D2 | FreeRTOS | Integrated in ESP-IDF 5.0+ | Runtime |
| D3 | esp_mqtt client library | Integrated in ESP-IDF 5.0+ | Runtime |
| D4 | MQTT Broker | Mosquitto 1.6+ (or compatible) | Runtime |
| D5 | WiFi Network | IEEE 802.11 b/g/n, 2.4 GHz | Runtime |
| D6 | CMake | 3.5+ | Build-time |
| D7 | Python | 3.6+ (for ESP-IDF tools) | Build-time |

---

## 6. Interface Specifications

### 6.1 MQTT Interface

#### 6.1.1 Broker Configuration

- **Broker Address:** Configurable via application config (assumed `mqtt://192.168.1.100:1883`)
- **Port:** 1883 (standard MQTT)
- **Protocol:** MQTT 3.1.1
- **Keep-Alive:** 60 seconds (assumed)
- **Quality of Service (QoS):** Level 1 (acknowledged delivery)

#### 6.1.2 Command Topic: `device/cmd`

**Subscription:** System subscribes to this topic for control commands

**Message Format:** Plain ASCII text

**Valid Commands:**

```
red on              → Set Red LED continuous on
red off             → Set Red LED off
red fflash          → Set Red LED to fast flash (100ms on/off)
red sflash          → Set Red LED to slow flash (500ms on/off)
red fade            → Set Red LED to PWM fade
yellow on|off|fflash|sflash|fade  → Yellow LED control
green on|off|fflash|sflash|fade   → Green LED control
blue on|off|fflash|sflash|fade    → Blue LED control
debug on|off|fflash|sflash|fade   → Debug LED control
all on              → All LEDs to on (including debug)
all off             → All LEDs to off (including debug)
all fflash|sflash|fade → All LEDs to specified mode
```

**Example Command Payloads:**
```
red on
yellow fflash
green fade
all off
```

#### 6.1.3 State Topic: `device/state`

**Publication:** System publishes state updates to this topic after command execution

**Message Format:** JSON-formatted ASCII text

**State JSON Schema:**
```json
{
  "timestamp": 1704067200,
  "red": {
    "mode": "on|off|fflash|sflash|fade",
    "brightness": 0-100,
    "cycle_count": 12345
  },
  "yellow": {
    "mode": "on|off|fflash|sflash|fade",
    "brightness": 0-100,
    "cycle_count": 12345
  },
  "green": {
    "mode": "on|off|fflash|sflash|fade",
    "brightness": 0-100,
    "cycle_count": 12345
  },
  "blue": {
    "mode": "on|off|fflash|sflash|fade",
    "brightness": 0-100,
    "cycle_count": 12345
  },
  "debug": {
    "mode": "on|off|fflash|sflash|fade",
    "brightness": 0-100,
    "cycle_count": 12345
  },
  "status": "ok|error",
  "error_message": "null or error description"
}
```

**Example State Messages:**
```json
{
  "timestamp": 1704067200,
  "red": {"mode": "on", "brightness": 100, "cycle_count": 0},
  "yellow": {"mode": "off", "brightness": 0, "cycle_count": 0},
  "green": {"mode": "fflash", "brightness": 100, "cycle_count": 5432},
  "blue": {"mode": "fade", "brightness": 50, "cycle_count": 1200},
  "debug": {"mode": "on", "brightness": 100, "cycle_count": 0},
  "status": "ok",
  "error_message": null
}
```

#### 6.1.4 Error Handling

**Invalid Command Response:**
```json
{
  "timestamp": 1704067201,
  "status": "error",
  "error_message": "Invalid command: 'red invalid_mode'. Valid modes: on, off, fflash, sflash, fade",
  "last_valid_command": "red on"
}
```

#### 6.1.5 Connection/Disconnection Messages

**On MQTT Connection (LWT - Last Will & Testament):**
- System publishes full state (see 6.1.3) indicating successful connection
- LWT message (if broker is lost): `{"status": "offline"}` 

### 6.2 GPIO Interface

#### 6.2.1 GPIO Pin Definitions

```c
#define GPIO_RED       15   // Active High
#define GPIO_YELLOW    14   // Active High
#define GPIO_GREEN     13   // Active High
#define GPIO_BLUE      12   // Active High
#define GPIO_BUZZER    04   // Active High (Reserved)
#define GPIO_DEBUG     01   // Active High
```

#### 6.2.2 GPIO Configuration

All GPIO pins shall be configured as:
- **Mode:** Output (push-pull)
- **Initial State:** Low (LED off)
- **Drive Strength:** Strong (20mA capable)
- **Slew Rate:** Default (no edge slew limiting)

#### 6.2.3 PWM Configuration for Fade Mode

- **Frequency:** 1000 Hz
- **Resolution:** 8-bit (256 levels, 0-100% duty cycle mapping)
- **Channels:** 1 per LED (6 channels used)
- **Timer Group:** Timer Group 0 (assumed)

**Duty Cycle Mapping:**
```
Brightness %  → Duty Cycle
0%           → 0/256 (0)
50%          → 128/256 (50%)
100%         → 256/256 (100%)
```

#### 6.2.4 GPIO Startup Behavior

- On system boot, all GPIO outputs default to **Low** (LEDs off)
- No LED shall illuminate until explicit MQTT command received
- GPIO initialization completes before MQTT task startup

### 6.3 Hardware Timing Specifications

#### 6.3.1 Flash Mode Timing

**Fast Flash (fflash):**
- On-time: 100 ± 5 ms
- Off-time: 100 ± 5 ms
- Period: 200 ms
- Frequency: 5 Hz

**Slow Flash (sflash):**
- On-time: 500 ± 10 ms
- Off-time: 500 ± 10 ms
- Period: 1000 ms
- Frequency: 1 Hz

#### 6.3.2 Fade Mode Timing

**Fade Cycle:**
- Rising ramp: 0% → 100% in 2 seconds (assumed)
- Falling ramp: 100% → 0% in 2 seconds (assumed)
- Total cycle: 4 seconds
- Step interval: 20 ms (resolution ~1% per step)

---

## 7. Operational Procedures

### 7.1 Build & Deployment

#### 7.1.1 Prerequisites
- ESP-IDF v5.0+ installed and configured
- ESP32-S2 Mini board connected via USB
- Python 3.6+ environment active

#### 7.1.2 Build Process
```bash
# Set IDF target
idf.py set-target esp32s2

# Configure project
idf.py menuconfig
# Set WiFi SSID, Password, MQTT Broker IP

# Build firmware
idf.py build

# Flash to board
idf.py flash monitor

# View logs on UART
# (Monitor window shows real-time debug output)
```

#### 7.1.3 Configuration Parameters (menuconfig)
```
WiFi Configuration:
  - SSID
  - Password
  - Channel (auto-detect assumed)

MQTT Configuration:
  - Broker IP address
  - Broker Port (default 1883)
  - Device ID (for topic parameterization)
  - Username (optional)
  - Password (optional)

LED Configuration:
  - GPIO pin assignments (can be overridden)
  - PWM frequency (default 1000 Hz)
  - Fade ramp duration (default 2s)
```

### 7.2 Runtime Operation

#### 7.2.1 System Startup Sequence
1. ESP32-S2 Mini boots from flash
2. GPIO pins initialized to low (all LEDs off)
3. FreeRTOS kernel starts
4. WiFi task initializes; connects to configured SSID
5. MQTT task initializes; connects to broker
6. LED control task ready for commands
7. Debug output on UART confirms system ready

#### 7.2.2 Normal Operation
- System waits for MQTT commands on `device/cmd` topic
- Upon command receipt, LED state updated immediately
- State published to `device/state` topic
- System loop continues monitoring for new commands

#### 7.2.3 LED Command Flow
```
MQTT Message Received
        ↓
Parse Command String
        ↓
Validate Format & Arguments
        ↓
Lookup LED Identifier
        ↓
Update LED Control State
        ↓
Apply to GPIO/PWM
        ↓
Update State Structure
        ↓
Publish State JSON to device/state
```

### 7.3 Maintenance & Troubleshooting

#### 7.3.1 Health Monitoring
- **Heartbeat:** Publish state every 60 seconds (assumed) or on command
- **Log Level:** INFO for normal operation, DEBUG for troubleshooting
- **Error Logging:** All command failures logged with timestamp and reason

#### 7.3.2 Recovery Procedures

**WiFi Connection Loss:**
- Automatic reconnection initiated within 5 seconds
- Debug LED may indicate connection status (assumed)
- LED states preserved during disconnection

**MQTT Broker Unavailable:**
- Automatic reconnection with exponential backoff
- Commands received during outage queued (if buffers available)
- System remains responsive to local diagnostics

**GPIO Malfunction:**
- System detects GPIO error via driver
- Error published to `device/state`
- Affected LED state shows error flag
- No system reset required (graceful degradation)

---

## 8. Verification & Validation

### 8.1 Test Strategy

**V&V Phases:**
1. Unit Testing (GPIO, PWM, MQTT parsing)
2. Integration Testing (GPIO + MQTT coordination)
3. System Testing (full command-state cycle)
4. Stress Testing (high command rate, long duration)
5. Acceptance Testing (client verification)

### 8.2 Test Cases & Traceability

#### 8.2.1 GPIO & LED Control Tests

| Test ID | Requirement | Description | Pass Criteria | Traceability |
|---------|-------------|-------------|---------------|--------------|
| T-GPIO-001 | REQ-F1.1 | Red LED off mode | GPIO 15 reads low; LED extinguished | REQ-F1.1 |
| T-GPIO-002 | REQ-F1.1 | Yellow LED off mode | GPIO 14 reads low; LED extinguished | REQ-F1.1 |
| T-GPIO-003 | REQ-F1.1 | Green LED off mode | GPIO 13 reads low; LED extinguished | REQ-F1.1 |
| T-GPIO-004 | REQ-F1.1 | Blue LED off mode | GPIO 12 reads low; LED extinguished | REQ-F1.1 |
| T-GPIO-005 | REQ-F1.1 | Debug LED off mode | GPIO 01 reads low; LED extinguished | REQ-F1.1 |
| T-GPIO-010 | REQ-F1.2 | Red LED on mode | GPIO 15 reads high; LED illuminated continuously | REQ-F1.2 |
| T-GPIO-011 | REQ-F1.2 | All LEDs on mode | All GPIO pins high; all LEDs illuminated | REQ-F1.2 |
| T-GPIO-020 | REQ-F1.3 | Red LED fast flash | GPIO 15 toggles at ~5 Hz; 100ms on/off ±5% | REQ-F1.3, REQ-N1.4 |
| T-GPIO-021 | REQ-F1.3 | Yellow LED fast flash | GPIO 14 toggles at ~5 Hz; 100ms on/off ±5% | REQ-F1.3, REQ-N1.4 |
| T-GPIO-030 | REQ-F1.4 | Red LED slow flash | GPIO 15 toggles at 1 Hz; 500ms on/off ±5% | REQ-F1.4, REQ-N1.4 |
| T-GPIO-031 | REQ-F1.4 | Green LED slow flash | GPIO 13 toggles at 1 Hz; 500ms on/off ±5% | REQ-F1.4, REQ-N1.4 |
| T-GPIO-040 | REQ-F1.5 | Red LED fade mode | GPIO 15 PWM cycles 0-100% in 4s total; smooth brightness | REQ-F1.5 |
| T-GPIO-041 | REQ-F1.5 | LED fade brightness levels | PWM duty cycle spans 0, 25, 50, 75, 100%; linear steps | REQ-F1.5 |
| T-GPIO-050 | REQ-F1.6 | Mode change latency | GPIO state changes <50ms after command | REQ-F1.6, REQ-N1.1 |

#### 8.2.2 MQTT Interface Tests

| Test ID | Requirement | Description | Pass Criteria | Traceability |
|---------|-------------|-------------|---------------|--------------|
| T-MQTT-001 | REQ-F2.1 | Subscription to device/cmd | System connects and subscribes without errors | REQ-F2.1 |
| T-MQTT-002 | REQ-F2.2 | Publish to device/state | State JSON published after each command | REQ-F2.2 |
| T-MQTT-003 | REQ-F2.3 | Valid command format | Message "red on" parsed and processed correctly | REQ-F2.3 |
| T-MQTT-004 | REQ-F2.3 | "all" command format | Message "all off" sets all LEDs to off | REQ-F2.3, REQ-F4.2 |
| T-MQTT-005 | REQ-F2.4 | Color validation | Command rejected if color not in [red, yellow, green, blue, debug] | REQ-F2.4, REQ-F2.6 |
| T-MQTT-006 | REQ-F2.5 | Command validation | Command rejected if mode not in [on, off, fflash, sflash, fade] | REQ-F2.5, REQ-F2.6 |
| T-MQTT-007 | REQ-F2.6 | Invalid command handling | Error message published to device/state | REQ-F2.6 |
| T-MQTT-008 | REQ-F2.7 | Connection heartbeat | State published on MQTT broker connect | REQ-F2.7 |
| T-MQTT-009 | REQ-F3.2 | State persistence | LED state restored after MQTT reconnection | REQ-F3.2 |
| T-MQTT-010 | REQ-F3.3 | State JSON completeness | JSON contains all 5 LED states and status | REQ-F3.3 |
| T-MQTT-011 | REQ-F3.4 | State timestamp | Published JSON includes Unix timestamp | REQ-F3.4 |
| T-MQTT-012 | REQ-F4.1 | "all on" command | Command "all on" sets red, yellow, green, blue, debug to on | REQ-F4.1 |
| T-MQTT-013 | REQ-F4.2 | "all off" command | Command "all off" sets all LEDs to off | REQ-F4.2 |

#### 8.2.3 Performance & Reliability Tests

| Test ID | Requirement | Description | Pass Criteria | Traceability |
|---------|-------------|-------------|---------------|--------------|
| T-PERF-001 | REQ-N1.1 | Command latency | Measure GPIO transition time after MQTT rx; <50ms | REQ-N1.1 |
| T-PERF-002 | REQ-N1.2 | Publish latency | Measure time from command to device/state publish; <100ms | REQ-N1.2 |
| T-PERF-003 | REQ-N1.3 | High-rate commands | Send 100 commands/sec for 10 seconds; zero message loss | REQ-N1.3 |
| T-PERF-004 | REQ-N1.4 | Flash timing accuracy | Measure actual on/off times; within ±10% of spec | REQ-N1.4 |
| T-RELIAB-001 | REQ-N2.1 | MQTT reconnection | Disconnect broker; system reconnects within 5 seconds | REQ-N2.1 |
| T-RELIAB-002 | REQ-N2.2 | Fail-safe state preservation | LEDs maintain state during MQTT disconnect | REQ-N2.2 |
| T-RELIAB-003 | REQ-N2.3 | GPIO error recovery | Inject GPIO error; system recovers without reset | REQ-N2.3 |
| T-RELIAB-004 | REQ-N2.4 | Extended operation | Run system for 72 hours; monitor for crashes | REQ-N2.4 |
| T-RELIAB-005 | REQ-N4.1 | Safe startup | System boots; all LEDs off (GPIO low) | REQ-N4.1 |
| T-RELIAB-006 | REQ-N4.2 | Command validation | Malformed JSON rejected; no system fault | REQ-N4.2 |

#### 8.2.4 Integration Tests

| Test ID | Requirement | Description | Pass Criteria | Traceability |
|---------|-------------|-------------|---------------|--------------|
| T-INT-001 | REQ-F1.1, REQ-F2.3 | MQTT→GPIO off command | Send "red off" via MQTT; GPIO 15 goes low | REQ-F1.1, REQ-F2.3 |
| T-INT-002 | REQ-F1.2, REQ-F2.3 | MQTT→GPIO on command | Send "yellow on" via MQTT; GPIO 14 goes high | REQ-F1.2, REQ-F2.3 |
| T-INT-003 | REQ-F1.3, REQ-F2.3 | MQTT→GPIO flash command | Send "green fflash" via MQTT; GPIO 13 toggles | REQ-F1.3, REQ-F2.3 |
| T-INT-004 | REQ-F3.1, REQ-F3.3 | State tracking | Change LED modes; state JSON reflects all changes | REQ-F3.1, REQ-F3.3 |
| T-INT-005 | REQ-F4.1, REQ-F2.3 | All-on macro | Send "all on" via MQTT; all GPIO pins go high | REQ-F4.1, REQ-F2.3 |
| T-INT-006 | REQ-F4.2, REQ-F2.3 | All-off macro | Send "all off" via MQTT; all GPIO pins go low | REQ-F4.2, REQ-F2.3 |

### 8.3 Acceptance Criteria

**System shall pass all tests in sections 8.2.1-8.2.4 with 100% success rate.**

**Additional acceptance:**
- No compiler warnings or errors
- Code coverage >80% for critical paths
- Documentation complete and reviewed
- No known critical bugs in defect tracking

---

## 9. Troubleshooting Guide

### 9.1 Common Issues & Solutions

#### Issue 1: LEDs Not Responding to Commands

**Symptoms:** MQTT commands received but GPIO state doesn't change; LEDs remain off

**Diagnosis Steps:**
1. Check UART debug output for command receipt messages
2. Verify GPIO pin definitions in configuration
3. Check for "GPIO driver error" in state JSON
4. Measure GPIO voltage with multimeter (should be 0V or 3.3V)
5. Inspect hardware connections for loose wires

**Solutions:**
- Verify GPIO pin numbers match hardware pinout (Section 10.2)
- Check GPIO configuration in menuconfig
- Recompile and reflash firmware
- Test with logic analyzer to capture GPIO transitions
- Replace board if hardware is faulty

#### Issue 2: MQTT Connection Fails

**Symptoms:** System boots but doesn't connect to MQTT broker; "MQTT_CONNECT_FAILED" in logs

**Diagnosis Steps:**
1. Verify WiFi connection successful (check UART logs)
2. Ping MQTT broker from development machine
3. Verify broker IP address in configuration
4. Check MQTT broker is running and listening on port 1883
5. Verify firewall allows port 1883 traffic

**Solutions:**
- Confirm MQTT broker address in menuconfig
- Start MQTT broker: `mosquitto -v`
- Check network connectivity: `idf.py monitor` (watch for WiFi connect)
- Add verbose logging in MQTT task to debug connection sequence

#### Issue 3: Flash Timing Inaccurate

**Symptoms:** Flash modes appear to run at wrong frequency; timing measurements off by >10%

**Diagnosis Steps:**
1. Capture GPIO transitions with logic analyzer
2. Measure on-time and off-time durations
3. Check ESP32-S2 timer frequency calibration
4. Verify FreeRTOS tick rate (assumed 1000 Hz)
5. Monitor CPU load during flash operation

**Solutions:**
- Recalibrate timer dividers in LED control task
- Check for task preemption during timing-critical sections
- Use hardware timers instead of software delays
- Verify FreeRTOS tick configuration (CONFIG_FREERTOS_HZ)

#### Issue 4: State JSON Malformed or Incomplete

**Symptoms:** Received state messages have syntax errors or missing LED entries

**Diagnosis Steps:**
1. Capture raw MQTT payload to file
2. Validate JSON syntax with online validator
3. Check LED state structure initialization
4. Monitor heap usage for buffer overflow
5. Verify JSON formatting code

**Solutions:**
- Add JSON validation before publication
- Ensure all 5 LED entries populated before publish
- Increase MQTT buffer size if truncation suspected
- Add unit tests for JSON generation function

#### Issue 5: System Crashes or Unexpected Resets

**Symptoms:** UART output stops abruptly; watchdog resets; logs show stack overflow or panic

**Diagnosis Steps:**
1. Capture reset reason from boot logs (ESP32 provides reset reason code)
2. Check stack usage for each FreeRTOS task
3. Monitor heap fragmentation
4. Look for infinite loops or blocked mutex waits
5. Check for NULL pointer dereferences in logs

**Solutions:**
- Increase stack size for affected task (menuconfig)
- Enable core dump feature for post-mortem analysis
- Add watchdog timer with sufficient timeout (60 seconds assumed)
- Review task priorities to prevent priority inversion
- Profile memory usage with esp_heap_trace

### 9.2 Debug Features

#### 9.2.1 UART Debug Output

**Enable Debug Logging:**
```bash
idf.py menuconfig
# Set Component config → Log output → Default log verbosity → Debug
idf.py build flash monitor
```

**Expected Debug Output:**
```
I (1000) main: GPIO initialization complete
I (1050) main: WiFi connecting to SSID "MyWiFi"
I (2000) main: WiFi connected, IP: 192.168.1.100
I (2100) main: MQTT connecting to broker 192.168.1.50:1883
I (2500) main: MQTT connected
I (2600) main: System ready - waiting for commands
D (3000) mqtt: RX: "red on"
D (3010) led_control: Setting red LED to ON mode
I (3020) state: Publish state JSON (status=ok)
```

#### 9.2.2 Debug LED Indicator

**Purpose:** Visual indication of system state

**Expected Behavior (assumed):**
- Steady on: System operational, MQTT connected
- Slow blink: WiFi connecting
- Fast blink: MQTT disconnected
- Off: System offline or error state

#### 9.2.3 GPIO State Monitor

**CLI Command (assumed):**
```bash
# SSH into board (if enabled) or use:
idf.py monitor

# View GPIO state and LED mode in real-time
# Search for "GPIO STATE" in debug output
```

---

## 10. Appendix

### 10.1 Pin Assignment Reference

#### 10.1.1 ESP32-S2 Mini GPIO Pinout

```
ESP32-S2 Mini Module
┌─────────────────────────────────────┐
│ ╭─────────────────────────────────╮ │
│ │   USB         GND               │ │
│ ├─────────────────────────────────┤ │
│ │ GND      GP43  GP44  GP01(DBG)  │ │
│ │ 5V       GP45  GP12(BLE)        │ │
│ │ 3V3      GP46  GP13(GRN)        │ │
│ │ EN       GP47  GP14(YEL)        │ │
│ │ GP48     GP21  GP15(RED)        │ │
│ │ GP47     GP20  GND              │ │
│ │ GP46     GP19  GND              │ │
│ │ GP45     GP18  5V               │ │
│ ├─────────────────────────────────┤ │
│ │ SDA/GP08  SCL/GP09  GP04(BUZZ)  │ │
│ │ GP03      GP02      GND         │ │
│ ╰─────────────────────────────────╯ │
└─────────────────────────────────────┘
```

#### 10.1.2 Signal Assignments

| Signal | GPIO | Function | Direction | Notes |
|--------|------|----------|-----------|-------|
| RED_LED | 15 | Red indicator | Output | Active High, 3.3V |
| YELLOW_LED | 14 | Yellow indicator | Output | Active High, 3.3V |
| GREEN_LED | 13 | Green indicator | Output | Active High, 3.3V |
| BLUE_LED | 12 | Blue indicator | Output | Active High, 3.3V |
| BUZZER | 04 | Piezo buzzer | Output | PWM capable (Reserved) |
| DEBUG_LED | 01 | Debug status | Output | Active High, 3.3V |
| SDA | 08 | I2C data (future use) | I/O | Optional, not used |
| SCL | 09 | I2C clock (future use) | I/O | Optional, not used |

#### 10.1.3 Hardware Connections

**LED Connection Diagram:**
```
GPIO 15 (Red)    ──┬──→ Red LED (current-limiting assumed built-in)
                    └──→ GND

GPIO 14 (Yellow) ──┬──→ Yellow LED
                    └──→ GND

GPIO 13 (Green)  ──┬──→ Green LED
                    └──→ GND

GPIO 12 (Blue)   ──┬──→ Blue LED
                    └──→ GND

GPIO 01 (Debug)  ──┬──→ Debug LED
                    └──→ GND

GPIO 04 (Buzzer) ──┬──→ Piezo Buzzer (Reserved)
                    └──→ GND
```

### 10.2 Default Configuration

#### 10.2.1 GPIO Defaults

```c
// LED GPIO Pin Definitions
#define GPIO_LED_RED        15
#define GPIO_LED_YELLOW     14
#define GPIO_LED_GREEN      13
#define GPIO_LED_BLUE       12
#define GPIO_LED_DEBUG      1
#define GPIO_BUZZER         4

// GPIO Configuration
#define GPIO_OUTPUT_PIN_SEL ((1ULL << GPIO_LED_RED) | \
                            (1ULL << GPIO_LED_YELLOW) | \
                            (1ULL << GPIO_LED_GREEN) | \
                            (1ULL << GPIO_LED_BLUE) | \
                            (1ULL << GPIO_LED_DEBUG) | \
                            (1ULL << GPIO_BUZZER))

// Initial State: All LEDs OFF
#define GPIO_INITIAL_STATE  0
```

#### 10.2.2 MQTT Defaults

```
Broker Address:     localhost (127.0.0.1)
Port:              1883
Topic Subscribe:   device/cmd
Topic Publish:     device/state
Device ID:         "stacktower-01" (assumed)
QoS:               1
Keep-Alive:        60 seconds
```

#### 10.2.3 LED Timing Defaults

```
Fast Flash (fflash):
  - On-time:   100 ms
  - Off-time:  100 ms
  - Period:    200 ms
  - Freq:      5 Hz

Slow Flash (sflash):
  - On-time:   500 ms
  - Off-time:  500 ms
  - Period:    1000 ms
  - Freq:      1 Hz

Fade Mode:
  - Ramp up:   2 seconds (0% → 100%)
  - Ramp down: 2 seconds (100% → 0%)
  - Cycle:     4 seconds
  - Step size: ~1% per 20 ms
```

#### 10.2.4 FreeRTOS Defaults

```
Tick Rate (CONFIG_FREERTOS_HZ):  1000 Hz (1 ms tick)
Idle Task Stack Size:             2048 bytes
Task Watchdog Timeout:            5 seconds
Heap Size:                        ~320 KB (depends on build)
```

### 10.3 Command Quick Reference

#### 10.3.1 LED Command Examples

```bash
# Single LED commands
mosquitto_pub -t device/cmd -m "red on"
mosquitto_pub -t device/cmd -m "red off"
mosquitto_pub -t device/cmd -m "yellow fflash"
mosquitto_pub -t device/cmd -m "green sflash"
mosquitto_pub -t device/cmd -m "blue fade"
mosquitto_pub -t device/cmd -m "debug on"

# Macro commands
mosquitto_pub -t device/cmd -m "all on"
mosquitto_pub -t device/cmd -m "all off"
mosquitto_pub -t device/cmd -m "all fflash"
```

#### 10.3.2 State Query Example

```bash
# Subscribe to state topic (in separate terminal)
mosquitto_sub -t device/state -v

# In another terminal, send command
mosquitto_pub -t device/cmd -m "red on"

# Expected output in subscriber:
# device/state {"timestamp":1704067200,"red":{"mode":"on",...},"status":"ok",...}
```

### 10.4 Bill of Materials (BoM)

| Component | Part Number | Qty | Purpose |
|-----------|-------------|-----|---------|
| ESP32-S2 Mini | — | 1 | Microcontroller |
| Red LED | 3mm, 20mA, 2V | 1 | Red indicator |
| Yellow LED | 3mm, 20mA, 2V | 1 | Yellow indicator |
| Green LED | 3mm, 20mA, 2V | 1 | Green indicator |
| Blue LED | 3mm, 20mA, 2V | 1 | Blue indicator |
| Indicator LED | 3mm, 20mA, 2V | 1 | Debug indicator |
| Piezo Buzzer | 5V passive | 1 | Audio output (reserved) |
| Current-limiting Resistor | 150Ω, 1/4W | 6 | LED protection |
| USB Cable | Type-C | 1 | Power & programming |

### 10.5 Glossary

| Term | Definition |
|------|-----------|
| **ESP32-S2 Mini** | Espressif Systems 32-bit microcontroller with WiFi, optimized for IoT |
| **ESP-IDF** | Espressif IoT Development Framework; official SDK for ESP32 devices |
| **GPIO** | General-Purpose Input/Output; digital pins for controlling external hardware |
| **PWM** | Pulse Width Modulation; technique for controlling LED brightness via duty cycle |
| **MQTT** | Message Queuing Telemetry Transport; lightweight pub/sub messaging protocol |
| **Broker** | MQTT server that routes messages between publishers and subscribers |
| **QoS** | Quality of Service; MQTT delivery guarantee level (0, 1, or 2) |
| **FreeRTOS** | Real-time operating system kernel used by ESP-IDF |
| **LWT** | Last Will & Testament; MQTT feature for offline status messages |
| **Fade** | Smooth brightness transition using PWM duty cycle modulation |
| **Flash** | Repetitive on/off cycling of LED (fast or slow) |

---

## Related Documentation

- [StackTower Hardware Design](StackTower-hardware.md) (TBD)
- [StackTower API Reference](StackTower-api.md) (TBD)
- [StackTower Installation Guide](StackTower-installation.md) (TBD)
- [StackTower Test Report](StackTower-test-report.md) (TBD)
- [ESP-IDF Official Documentation](https://docs.espressif.com/projects/esp-idf/)
- [MQTT 3.1.1 Specification](https://docs.oasis-open.org/mqtt/mqtt/v3.1.1/mqtt-v3.1.1.html)

---

**Document Control**

| Version | Date | Author | Change Summary |
|---------|------|--------|-----------------|
| 0.1 | 2024-01-15 | Engineering | Initial draft |
| 1.0 | 2024-01-20 | Engineering | Release candidate; all sections reviewed |

---

*End of Document*
