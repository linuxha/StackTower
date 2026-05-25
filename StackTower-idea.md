# StackTower

As a Senior Software Engineer paln, docuement, code and build a
project that takes an ESP-32 S2 Mini board and one LED Stack Tower
with 5 LEDs (Red, Yellow, Green, Blue, Debug) and a piezo buzzer,
control each with a ESP32 S2 Mini board. Provide support for
individual On, Off, Flash fast (100ms on/100ms off), Flach slow (500ms
on/500ms off), fade on and fade off flash. Default to off.

Load and use se the Espressif IDF.

- ESP32 S2 mini
  - GPIO 15 - Red
  - GPIO 14 - Yellow
  - GPIO 13 - Green
  - GPIO 12 - Blue
  - GPIO 04 - Piezo (currently not used)
  - GPIO 01 - Debug ??? Still need to check this, just needs to be a digital output
- MQTT
  - accept command strings from the device/cmd topic
    - coomands are in the format of "color cmd"
      - where command is:
        - on
        - off
        - fflash (fast flash)
        - sflash (slow flash)
        - fade
      - where color is
        - red
        - green
        - yellow
        - blue
      - addition commands:
        - all off
        - all on
      - commands can be issued asynchronously
  - states can be monitored on device/state topic
- Flash fast
- Flash slow
- Fade on and off

# Create FSD

Create a Function Specifications Document (FSD), code, and test plan.

Use caveman mode, be succinct, no friendly chatter. Push changes to the repos often with descriptions. Keep the running chat in a chat-history.md file in the docs directory. Ask question as needed but don't repeat material already learned.
