# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

ESP8266 (Arduino) firmware that bridges MQTT and 303 MHz RF to control Hampton Bay ceiling fans. It transmits RF commands via a CC1101 transceiver to mimic the fan's wireless remote, and also passively listens for RF transmissions from the *real* remote so that MQTT-reported state stays in sync regardless of which control path is used.

All firmware code lives in `homefans/`:
- `homefans.ino` — setup/loop, MQTT callback, RF transmit/receive logic, RF command encode/decode
- `homefans.h` — includes, pin definitions, fan state struct/table, global objects (`mySwitch`, `client`, `fans[16]`)
- `config.h` — WiFi/MQTT credentials and CC1101 RF tuning constants (frequency, protocol, repeats, pulse length); **not committed with real secrets** — placeholder values must be filled in locally before flashing

## Build / Flash / Test

This is an Arduino sketch, not a project with a CLI build system — there is no test suite.

- IDE: Arduino IDE with the `esp8266` board package (`http://arduino.esp8266.com/stable/package_esp8266com_index.json`)
- Board: tested against `D1 Mini` (ESP8266)
- Open `homefans/homefans.ino` directly (do not open `homefans.h` or `config.h` as the sketch entry point)
- Required libraries (install via Library Manager, or use the zips under `libs/` if a specific pinned version is needed):
  - `SmartRC-CC1101-Driver-Lib` (LSatan)
  - `rc-switch` (sui77)
  - `PubSubClient` (Nick O'Leary)
- Before compiling/uploading: edit `homefans/config.h` with real `WIFI_SSID`, `WIFI_PASS`, `MQTT_HOST`, and other MQTT settings for the target network
- "Test" here means: Verify/Compile in the Arduino IDE, then flash to hardware and observe behavior over serial (`Serial.begin(9600)`) and/or MQTT — there's no host-side unit testing
- Once the device has done an initial USB flash with `ArduinoOTA` included, subsequent uploads can go over WiFi (OTA, port 8266) instead of USB
- `_drivers/` contains the CP210x USB-UART driver needed to see the board over USB on Windows (see `_drivers/notes.txt`)

## Architecture

### RF protocol encoding

Fan/remote state is packed into a 24-bit RF command using protocol 11 (rc-switch). The bit layout, built in `generateCommand()` (`homefans/homefans.ino:197`):

```
[ constant header/footer bits ] [ 4-bit fan ID (inverted via XOR 0b1111) ] [ constant mode bits ] [ 4-bit command ]
```

- `baseCommand = 0b111111000110000000000000` is the fixed header
- Fan ID comes from the transmitter/receiver DIP switches (4 bits, up=1/down=0) and is bitwise-inverted before being shifted into place
- The trailing 4-bit `command` nibble encodes the actual action (fan speed, light, power) — see the comment block above `generateCommand()` for the known command values

Received RF frames are decoded symmetrically in `loop()` (`homefans/homefans.ino:306`): the header is subtracted out, the ID nibble is extracted and re-inverted, and a `cmdMode`/`command` pair is derived to update in-memory fan state. `cmdMode == 0` (light color/temperature commands) is recognized but not yet handled — see the TODO in that block if extending this.

Because RF is a shared broadcast medium, any of the 16 possible fan IDs sharing this frequency will have their frames observed; only frames with `prot == 11 && bits == 24` and `id < 16` are treated as valid.

### State model and MQTT topics

State per fan is an in-memory `fan` struct (`homefans/homefans.h:38`) — `fanState` (bool), `fanSpeed` (`FAN_LOW`/`FAN_MED`/`FAN_HI`), `lightState` (bool) — held in a fixed `fans[16]` array indexed by the 4-bit fan ID. There is no persistence: state resets to off on reboot until either an MQTT `set` command or an RF frame from the physical remote is observed.

Topic shape is `home/fans/<4-bit binary id>/<attr>/<action>` where `attr` is `on`, `speed`, or `light` and `action` is `set` or `state`. The MQTT `callback()` (`homefans/homefans.ino:64`) parses the fan ID out of the topic string by fixed offset (relies on `BASE_TOPIC` length + a hardcoded `+5`/`+4`), so changing `BASE_TOPIC` or the topic structure requires updating that offset arithmetic, not just `config.h`.

Two things happen for every inbound `set`/`state` MQTT message: local `fans[]` state is updated to match the payload, and if the action was `set`, `transmitState()` is called to actually emit the RF command and then re-publish the resulting state (retained) back over MQTT via `postStateUpdate()`. This keeps Home Assistant (or any MQTT consumer) in sync whether the change originated from MQTT or from the physical RF remote.

### Transmit/receive switching

The CC1101 and `RCSwitch` are half-duplex on a single set of pins: `transmitState()` explicitly switches the CC1101 into TX mode and disables the RF receiver before sending, then switches back to RX mode and re-enables the receiver afterward. Any new code path that transmits must follow the same switch-to-TX / send / switch-back-to-RX pattern to avoid missing incoming remote frames.

### Debug logging

Verbose serial logging of inbound MQTT and outbound RF traffic is gated behind `DEBUG_MODE` in `config.h` (compile-time `#if`, not runtime) — flip it to `true` and reflash to get traffic tracing over serial.
