# Compilation Guide (Windows)

This guide covers building the firmware image from [`hampton-bay-fans.yaml`](../hampton-bay-fans.yaml). It doesn't put anything on the device yet — that's [flashing.md](flashing.md).

Make sure you've completed [dependencies.md](dependencies.md) first (`esphome version` should print a version number).

## Step 1: Open the project

In PowerShell, `cd` into the `esphome` folder inside this repo, e.g.:
```
cd C:\dev\hampton-bay-fan-mqtt\esphome
```

> **Windows path length note:** keep this checkout somewhere shallow, like `C:\dev\...`. Deeply nested paths (e.g. inside `Documents\GitHub\...\some-onedrive-synced-folder\...`) can exceed Windows' 260-character path limit while ESPHome downloads its build dependencies, causing compilation to fail partway through with a cryptic `[WinError 3] The system cannot find the path specified`. If you hit that, move the project to a shorter path (e.g. `C:\esp\hampton-bay-fan-mqtt`) and try again.

## Step 2: Fill in your configuration

Open [`hampton-bay-fans.yaml`](../hampton-bay-fans.yaml) in a text editor and replace the placeholder values:

| Key | What to put there |
|---|---|
| `wifi: ssid` / `wifi: password` | Your WiFi network's credentials |
| `wifi: ap: password` | A password for the fallback hotspot the device creates if it can't join your WiFi |
| `ota: password` | A password of your choosing — protects future over-the-air updates |
| `api: encryption: key` | Already a valid, randomly generated key — safe to leave as-is |

Then edit the `fan:` and `light:` lists at the bottom of the file to match the fans actually wired to this board. Each entry's `fan_id` is the 4-bit DIP switch value (0-15) of that fan/remote pairing — add or remove entries as needed, and give each a `name:` that makes sense in Home Assistant.

## Step 3: Compile

From the `esphome/` folder, run:
```
esphome compile hampton-bay-fans.yaml
```

The **first** run downloads the ESP8266 build toolchain and the CC1101/RF libraries (a few hundred MB total) — this needs an internet connection and can take several minutes. Every run after that is much faster since everything is cached locally.

A successful build ends with something like:
```
RAM:   [====      ]  40.9% (used 33492 bytes from 81920 bytes)
Flash: [=====     ]  46.1% (used 481193 bytes from 1044464 bytes)
...
INFO Successfully compiled program.
```

## Troubleshooting

- **`'esphome' is not recognized`** — the `pip` Scripts folder isn't on `PATH`. Either add it, or run `python -m esphome compile hampton-bay-fans.yaml` instead.
- **`[WinError 3] The system cannot find the path specified`** mid-download — path length issue, see the note in Step 1.
- **`fatal error: SPI.h: No such file or directory`** — you're likely on an outdated copy of `components/cc1101_fan`; pull the latest version of this repo.
- **A Python traceback mentioning `cc1101_fan` before compilation even starts** — the custom component didn't load. Make sure you're running `esphome` from inside the `esphome/` directory (so the `external_components:` path in the YAML resolves correctly).

**Next:** [flashing.md](flashing.md)
