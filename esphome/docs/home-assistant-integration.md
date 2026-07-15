# Home Assistant Integration

Once a board is flashed and joined to your WiFi (see [flashing.md](flashing.md)), Home Assistant can pick it up automatically through ESPHome's native API — there's no MQTT setup involved.

## Adding the device

1. In Home Assistant, go to **Settings > Devices & Services**.
2. If discovery finds it, you'll see a "Discovered — ESPHome" notification/card for your device (named after the `name:` / `friendly_name:` substitution in `hampton-bay-fans.yaml`, e.g. "Hampton Bay Fans"). Click **Configure**.
   - If it isn't auto-discovered (e.g. Home Assistant and the device are on different subnets, or discovery is disabled), add it manually: **Settings > Devices & Services > Add Integration > ESPHome**, then enter the device's IP address or `<name>.local` hostname (e.g. `hampton-bay-fans.local`) and port `6053`.
3. Home Assistant will ask for the device's **encryption key** — this is the value under `api: encryption: key:` in your `hampton-bay-fans.yaml`. Copy it in exactly.
4. Confirm. Home Assistant creates one device with a `fan` entity and a `light` entity for each `fan_id` you configured (e.g. "Living Room Fan", "Living Room Fan Light").

## Verifying it works

- Toggle a fan or light entity from the Home Assistant UI and confirm the physical fan responds.
- Press a button on the **physical remote** for that fan and confirm the Home Assistant entity's state updates within a second or two. This passive listening is what keeps Home Assistant in sync no matter which control path was used.

## Organizing entities

From the device's page (**Settings > Devices & Services > [device]**), you can rename entities, assign an **Area** (e.g. "Living Room"), and set icons — same as any other Home Assistant integration. Add the fan/light entities to dashboards and automations as you would any other entity.

## Retiring the old MQTT-based firmware

If you're migrating a fan that was previously controlled by the original `homefans.ino` MQTT bridge:

1. Flash this ESPHome firmware (to the same board or a replacement) and confirm the new entities work as described above.
2. In Home Assistant, remove whatever manually-configured MQTT `fan`/`light` entities pointed at the old `home/fans/<id>/...` topics (wherever they're defined — YAML config, MQTT discovery config, or a `configuration.yaml` include) so you don't end up with duplicate entities for the same physical fan.
3. Decommission the old device (reflash or relabel it) so it stops publishing to those MQTT topics.
4. Update any automations or dashboards that referenced the old MQTT entity IDs to point at the new ESPHome entities instead.

## Optional: manage this device from inside Home Assistant

If your Home Assistant runs as Home Assistant OS or Supervised, you can install the **ESPHome Device Builder** add-on (**Settings > Add-ons > Add-on Store**, search "ESPHome"). It hosts the same ESPHome Dashboard used in [flashing.md](flashing.md), but inside Home Assistant's UI.

To manage this project from it, copy `hampton-bay-fans.yaml` and the `components/` folder into the add-on's config directory (typically `/config/esphome/` — reachable via the Samba/File Editor add-ons, or SSH) so the dashboard can see them, then use its built-in compile/install buttons instead of the local CLI from [compilation.md](compilation.md)/[flashing.md](flashing.md).
