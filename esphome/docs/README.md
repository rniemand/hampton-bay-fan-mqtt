# Getting Started

This is the entry point for deploying the ESPHome-based firmware (`hampton-bay-fans.yaml` +
the `cc1101_fan` component) to a physical board and into Home Assistant. It ties together the
four stages covered in detail by the other docs in this folder.

## 1. Configure and build the image

Fill in [`../hampton-bay-fans.yaml`](../hampton-bay-fans.yaml): WiFi credentials, an OTA
password, and the `fan:`/`light:` list (one pair per physical fan, keyed by its 4-bit DIP
switch `fan_id`, 0-15). The `cc1101_fan:` block's `frequency:` may need tuning per fan, the
same way `RF_FREQUENCY` did in the original `homefans/config.h`.

Then, from this `esphome/` folder:
```
esphome compile hampton-bay-fans.yaml
```

Full steps and troubleshooting (path-length issues, missing `esphome` on `PATH`, first-run
toolchain download): [dependencies.md](dependencies.md) (do this first if `esphome version`
doesn't work yet) → [compilation.md](compilation.md).

## 2. Flash the device

One-time USB flash, then all later updates go over WiFi:
```
esphome run hampton-bay-fans.yaml
```
Pick the USB/COM port when prompted the first time; ESPHome finds the device over mDNS on
every run after that. Full steps: [flashing.md](flashing.md).

> **Alternative to steps 1-2:** if your Home Assistant runs as OS or Supervised, you can skip
> the local CLI entirely and build/flash from a web UI hosted inside Home Assistant instead —
> see [device-builder-addon.md](device-builder-addon.md).

## 3. Add it to Home Assistant

No MQTT setup required — this firmware uses ESPHome's native API, which Home Assistant
auto-discovers. Go to **Settings → Devices & Services**, confirm/add the discovered
"Hampton Bay Fans" device, and paste in the `api: encryption: key:` value from the YAML when
asked. One `fan` entity and one `light` entity are created per `fan_id` you configured — no
separate entity configuration step. Full steps and verification:
[home-assistant-integration.md](home-assistant-integration.md).

## 4. Updating later

Whenever `hampton-bay-fans.yaml` changes (new fan, new frequency, etc.), re-run the same
command used to flash:
```
esphome run hampton-bay-fans.yaml
```
ESPHome pushes the update over WiFi (OTA) automatically. Fall back to USB only if the device
can't be reached over the network. Details: [flashing.md](flashing.md#phase-2-over-the-air-ota-updates).

## Migrating from the old MQTT firmware

If a fan is currently running `homefans/homefans.ino` and you're switching it to this
firmware, see the "Retiring the old MQTT-based firmware" section of
[home-assistant-integration.md](home-assistant-integration.md#retiring-the-old-mqtt-based-firmware).
