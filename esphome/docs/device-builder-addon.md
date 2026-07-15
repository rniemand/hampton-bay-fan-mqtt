# Provisioning via the ESPHome Device Builder Add-on

This is an alternative to the local CLI workflow in [compilation.md](compilation.md) and
[flashing.md](flashing.md) — instead of running `esphome` from a terminal on your PC, you run
it from a web UI hosted inside Home Assistant itself. Useful if you'd rather not install Python
locally, or want to manage/update the device from the same machine running Home Assistant.

**Requirement:** Home Assistant **OS** or **Supervised** only — add-ons aren't available on
Container or Core installs. If that's your setup, use the local CLI path instead.

## 1. Install the add-on

1. In Home Assistant: **Settings → Add-ons → Add-on Store**.
2. Search for **ESPHome** and install the **ESPHome Device Builder** add-on (the official one,
   published by ESPHome/Nabu Casa).
3. Start it, and optionally enable **Show in sidebar** so it's a click away.
4. Open its **Web UI** — this is the same ESPHome Dashboard used by `esphome dashboard .`
   locally, just running inside Home Assistant.

## 2. Copy this project's files in

The dashboard only sees YAML files (and any `external_components` they reference) that live in
its config directory — normally `/config/esphome/` on the Home Assistant host. Reach that
folder with whichever of these you already have set up: the **Samba share** add-on
(`\\homeassistant\config\esphome\`), the **File editor** add-on, **Studio Code Server**, or SSH.

Copy, preserving the relative layout:
- [`hampton-bay-fans.yaml`](../hampton-bay-fans.yaml) → `/config/esphome/hampton-bay-fans.yaml`
- [`components/cc1101_fan/`](../components/cc1101_fan/) → `/config/esphome/components/cc1101_fan/`

The nesting matters: the YAML's `external_components:` block points at a local `components`
path relative to itself (`type: local, path: components`), so `components/cc1101_fan` has to
land as a sibling of the YAML file, not inside some other folder.

Don't use the dashboard's **+ New Device** wizard for this — it only scaffolds configs for
ESPHome's built-in components and won't know about the custom `cc1101_fan` component. Copying
the already-written YAML in is what makes the dashboard pick it up correctly.

## 3. Fill in the configuration

If you haven't already (per [README.md](README.md#1-configure-and-build-the-image)), open
`hampton-bay-fans.yaml` — either through the copy on your PC before uploading it, or via the
dashboard's built-in file editor (pencil icon on the device tile) — and fill in the WiFi
credentials, OTA password, and the `fan:`/`light:` list.

Optional cleanup: since the add-on's config directory is shared across every device it manages,
you can move repeated secrets (WiFi SSID/password, OTA password) into a single
`/config/esphome/secrets.yaml` and reference them with `!secret wifi_ssid` etc., instead of
hardcoding them per-device. Not required for a single-device setup — inline placeholders work
fine, same convention as `homefans/config.h`.

Refresh the dashboard's device list (or wait a moment) and `hampton-bay-fans` should now appear
as a tile.

## 4. Initial flash (USB, one-time)

Click the tile's **Install** button, then **Plug into this computer**.

This uses your **browser's** WebSerial support (Chrome or Edge; Firefox/Safari don't support
it) to flash over USB — the board needs to be physically plugged into whatever machine your
browser is running on, **not** necessarily the Home Assistant host. If you're viewing the
dashboard from your PC with the board plugged into that same PC, this just works. If the board
is only reachable from a different machine, use **Manual download** instead: it builds the
firmware and gives you a `.bin` to flash with a separate tool (e.g. `esptool`, or
[web.esphome.io](https://web.esphome.io)) from whichever computer the board is actually plugged
into.

Watch the in-browser log for the build and upload to finish, then confirm the device joins
WiFi.

## 5. Later updates (OTA, from the dashboard)

Once the device is on WiFi, every subsequent change just needs: edit the YAML (via the
dashboard's file editor or by re-copying an updated file), then click **Install** on the tile
again and choose **Wirelessly**. The add-on finds the device over the network and pushes the
update — no cable, no browser USB requirement, works the same whether you're on the Home
Assistant host or a phone/tablet with the sidebar open.

## 6. Add it to Home Assistant

No different from the CLI-flashed path — see [home-assistant-integration.md](home-assistant-integration.md).
Since the add-on runs alongside Home Assistant, the device is typically discovered even faster.
