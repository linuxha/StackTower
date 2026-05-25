# StackTower Todo List

## Phase 1: GPIO & LED Driver Foundation

### Project Setup
- [ ] Create ESP-IDF project structure (CMakeLists.txt, main/)
- [ ] Configure sdkconfig.defaults (GPIO pins, FreeRTOS, UART logging)
- [ ] Set up build pipeline (test target configuration)
- [ ] Create project README with build/flash instructions

### GPIO & LED Driver Implementation
- [ ] Create led_driver.c/h module
- [ ] Implement GPIO initialization for all 5 LEDs
- [ ] Implement LED on/off control
- [ ] Implement fast flash mode (100ms on/off)
- [ ] Implement slow flash mode (500ms on/off)
- [ ] Implement PWM fade mode with timing specs
- [ ] Add mode state management (track current mode per LED)
- [ ] Add LED color enum and command parsing

### LED Driver Testing
- [ ] Create test_led_driver.c unit test framework
- [ ] Test TC-LED-001: GPIO initialization
- [ ] Test TC-LED-002: On mode
- [ ] Test TC-LED-003: Off mode (default)
- [ ] Test TC-LED-004: Fast flash timing accuracy (±5%)
- [ ] Test TC-LED-005: Slow flash timing accuracy (±5%)
- [ ] Test TC-LED-006: Fade timing (PWM ramp)
- [ ] Test TC-LED-007: Mode transitions
- [ ] Test TC-LED-008: State persistence across mode changes
- [ ] Validate all unit tests pass

### Phase 1 Exit Criteria
- [ ] All GPIO pins configured and responding
- [ ] All 5 LED modes working and timing within spec
- [ ] Unit tests passing (TC-LED-001 through TC-LED-010)
- [ ] Build succeeds with no warnings
- [ ] Commit Phase 1 complete

---

## Phase 2: MQTT Integration

### MQTT Client Setup
- [ ] Create mqtt_handler.c/h module
- [ ] Integrate PubSubClient or esp_mqtt library
- [ ] Implement WiFi connection (assume credentials pre-configured)
- [ ] Implement MQTT broker connection with reconnect logic
- [ ] Configure MQTT topics (device/cmd, device/state)
- [ ] Add connection status tracking

### Command Processing
- [ ] Create command_parser.c/h module
- [ ] Parse device/cmd messages ("color command" format)
- [ ] Validate color values (red/yellow/green/blue/debug/all)
- [ ] Validate command values (on/off/fflash/sflash/fade)
- [ ] Handle "all on" and "all off" special commands
- [ ] Route parsed commands to LED driver

### State Publishing
- [ ] Implement state JSON schema (per FSD Appendix)
- [ ] Track LED state changes
- [ ] Publish state asynchronously to device/state topic
- [ ] Implement state update rate limiting (if needed)
- [ ] Test state publishing format

### MQTT Integration Testing
- [ ] Test TC-MQTT-001: Broker connection
- [ ] Test TC-MQTT-002: Topic subscription (device/cmd)
- [ ] Test TC-MQTT-003: Command parsing accuracy
- [ ] Test TC-MQTT-004: Invalid command handling
- [ ] Test TC-MQTT-005: State publishing
- [ ] Test TC-MQTT-006: Async command execution
- [ ] Test TC-MQTT-007: Reconnection after broker disconnect
- [ ] Test TC-MQTT-008: QoS message delivery
- [ ] Validate all integration tests pass

### Main Application Loop
- [ ] Create main.c with FreeRTOS task setup
- [ ] Create MQTT task (priority level TBD)
- [ ] Create LED control task (priority level TBD)
- [ ] Implement task coordination and synchronization
- [ ] Add system logging/debug output

### Phase 2 Exit Criteria
- [ ] MQTT connection stable and persistent
- [ ] Commands parsed and executed correctly
- [ ] State publishing working asynchronously
- [ ] Integration tests passing (TC-MQTT-001 through TC-MQTT-008)
- [ ] No memory leaks or stack overflows
- [ ] Commit Phase 2 complete

---

## Phase 3: Complete Feature Set & Optimization

### Error Handling & Recovery
- [ ] Implement watchdog timer (WDT) integration
- [ ] Add factory reset procedure
- [ ] Add re-provisioning logic
- [ ] Implement safe-mode (all LEDs off, LED driver disabled)
- [ ] Add error logging and diagnostics

### Performance Optimization
- [ ] Profile memory usage
- [ ] Profile CPU utilization
- [ ] Optimize MQTT message rates
- [ ] Fine-tune task priorities and stack sizes
- [ ] Validate power consumption targets (if applicable)

### Production Testing
- [ ] Test TC-ACC-001: End-to-end LED control via MQTT
- [ ] Test TC-ACC-002: 100+ rapid command cycles
- [ ] Test TC-ACC-003: Broker disconnect/reconnect stability (1 hour+)
- [ ] Test TC-ACC-004: LED visual verification (all modes)
- [ ] Test TC-ACC-005: State consistency under load
- [ ] Stress test with high message rate (100 msg/sec)

### Documentation & Deployment
- [ ] Write deployment guide (flashing, provisioning)
- [ ] Document configuration options (SSID, broker address, timeout values)
- [ ] Create troubleshooting guide (updated from FSD)
- [ ] Document MQTT command examples
- [ ] Create hardware assembly guide (if needed)
- [ ] Update README with all features

### Future Enhancements
- [ ] Piezo buzzer control and patterns
- [ ] OTA firmware update support
- [ ] NVS configuration storage
- [ ] Multiple device coordination
- [ ] Home Assistant integration

### Phase 3 Exit Criteria
- [ ] All acceptance tests passing
- [ ] No critical bugs or memory issues
- [ ] Documentation complete
- [ ] Ready for production deployment
- [ ] Commit Phase 3 complete

---

## Ongoing

### Documentation & Knowledge Transfer
- [ ] Update docs/chat-history.md after each session
- [ ] Document any blockers or design decisions in ADR format (if needed)
- [ ] Keep dev-context.md synchronized with progress

### Code Quality
- [ ] Ensure consistent naming conventions (snake_case for functions/vars)
- [ ] Add meaningful comments (critical logic only)
- [ ] Follow ESP-IDF coding style guidelines
- [ ] Run static analysis / linting (if available)

### Version Control
- [ ] Push commits frequently (after each logical unit of work)
- [ ] Use descriptive commit messages
- [ ] Review git log for clarity and traceability

---

## Summary by Requirement Category

### Functional Requirements Coverage
- **FR-1.x (GPIO)**: Phase 1, Tasks: GPIO initialization, LED control
- **FR-2.x (LED Modes)**: Phase 1, Tasks: Flash, fade implementations
- **FR-3.x (MQTT)**: Phase 2, Tasks: Broker connection, command parsing, state publishing
- **FR-4.x (State)**: Phase 2, Tasks: State tracking, JSON publishing

### Non-Functional Requirements Coverage
- **NFR-1.x (Performance)**: Phase 3, Tasks: Profiling, optimization
- **NFR-2.x (Reliability)**: Phase 2 & 3, Tasks: Reconnect logic, watchdog, error handling
- **NFR-3.x (Power)**: Phase 3, Task: Power measurement and optimization
- **NFR-4.x (Security)**: Phase 2, Task: MQTT authentication (pre-configured)

### Test Coverage
- **Unit Tests**: Phase 1 (LED driver), ~10 tests
- **Integration Tests**: Phase 2 (MQTT), ~8 tests
- **Acceptance Tests**: Phase 3, ~5 tests
- **Total**: ~23 test cases mapped to FSD traceability matrix

---

## Blocked / Waiting On

- [ ] WiFi SSID/password provisioning method (deferred to Phase 2 decision)
- [ ] MQTT broker address & credentials (deferred to Phase 2 configuration)
- [ ] Piezo buzzer implementation approach (deferred to Phase 3+)
- [ ] Debug GPIO validation (verify GPIO 01 on hardware)

---

## Completed ✅

- [x] GitHub repository created (public)
- [x] GitHub authentication configured
- [x] Canonical FSD written (fsd-writer skill)
- [x] Development context documented
- [x] Todo list created
