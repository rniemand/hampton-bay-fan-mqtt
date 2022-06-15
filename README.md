# Hampton Bay Fan MQTT
> **Note**: This project is a copy of [Ben Owen](https://github.com/owenb321) original [hampton-bay-fan-mqtt](https://github.com/rniemand/hampton-bay-fan-mqtt) repo modified to work for my fans - most of the credit should go to him!

## Overview
ESP8266 project enabling MQTT control for a Hampton Bay fan with a wireless receiver. Wireless communication is performed with a **CC1101** wireless transceiver operating at `~303 MHz`.

This will also monitor for Hampton Bay RF signals so the state will stay in sync even if the original remote is used to control the fan.

## Dependencies
This project uses the following libraries that are available through the `Arduino IDE` and have been added locally just in case:
* [SmartRC-CC1101-Driver-Lib](https://github.com/LSatan/SmartRC-CC1101-Driver-Lib) by LSatan
* [rc-switch](https://github.com/sui77/rc-switch) by sui77
* [PubSubClient](https://pubsubclient.knolleary.net/) by Nick O'Leary

## Hardware
* ESP8266 development board (Tested with `D1 Mini`)
* CC1101 wireless transceiver
  * Wiring info can be found in the [SmartRC-CC1101-Driver-Lib readme](https://github.com/LSatan/SmartRC-CC1101-Driver-Lib#wiring)

## Setup
### Configuration
Modify the appropriate configuration in `./homefans/config.h` to work with your WiFi \ MQTT settings.

Change the `WIFI_*` and `MQTT_*` definitions in the sketch to match your network settings before uploading to the ESP.
### MQTT
By default, the state/command topics will be
* Fan on/off (payload `on` or `off`)
  * `home/fans/<fan_id>/on/state`
  * `home/fans/<fan_id>/on/set`
* Fan speed (payload `low`, `medium`, `high`, or `off`)
  * `home/fans/<fan_id>/speed/state`
  * `home/fans/<fan_id>/speed/set`
* Light on/off (payload `ON` or `OFF`)
  * `home/fans/<fan_id>/light/state`
  * `home/fans/<fan_id>/light/set`

`fan_id` is a 4-digit binary number determined by the dip switch settings on the transmitter/receiver where up = 1 and down = 0. For example, the dip setting:

|1|2|3|4|
|-|-|-|-|
|↓|↓|↓|↑|

...corresponds to a fan ID of `0001`

### Home Assistant
To use this in Home Assistant as an MQTT Fan and MQTT Light, I'm using this config
```yaml
mqtt:
  fan:
    - name: "Master Bedroom Fan"
      state_topic: "home/fans/0001/light/state"
      command_topic: "home/fans/0001/light/set"
      preset_mode_state_topic: "home/fans/0001/speed/state"
      preset_mode_command_topic: "home/fans/0001/speed/set"
      preset_modes:
        - "off"
        - "low"
        - "medium"
        - "high"
      qos: 0
      payload_on: "on"
      payload_off: "off"

  light:
    - name: "Master Bedroom Fan Light"
      state_topic: "home/fans/0001/light/state"
      command_topic: "home/fans/0001/light/set"
      qos: 0
      payload_on: "on"
      payload_off: "off"
      optimistic: false
```

![](/images/01.png)

![](/images/02.png)

![](/images/03.png)
