# Flashing Guide

There are two phases: a one-time flash over **USB**, then all later updates go over **WiFi (OTA)** — no more cables after the first flash.

Make sure you've completed [compilation.md](compilation.md) first (or just let the steps below compile for you — `esphome run` compiles automatically if needed).

## Tools

- The `esphome` CLI (same one used to compile) — it drives both USB and OTA flashing
- The USB-to-UART driver from [dependencies.md](dependencies.md)
- A USB data cable connected from your PC to the D1 Mini board

## Phase 1: initial USB flash

1. Plug the D1 Mini into your PC via USB.
2. Confirm your PC can see it:
   - **Windows:** Device Manager > Ports (COM & LPT) > "Silicon Labs CP210x USB to UART Bridge (COMx)" — note the COM number.
   - **macOS/Linux:** `ls /dev/cu.*` or `ls /dev/ttyUSB*` (see [dependencies.md](dependencies.md)).
3. From the `esphome/` folder, run:
   ```
   esphome run hampton-bay-fans.yaml
   ```
4. ESPHome compiles (if it hasn't already) and then asks which upload method to use — choose the listed USB/COM port.
5. Watch the upload progress bar. When it finishes, ESPHome switches to streaming the device's serial log so you can confirm it boots and connects to WiFi. Press `Ctrl+C` to stop watching logs — this doesn't affect the device.
6. Unplug the USB cable. You shouldn't need it again unless something needs fixing that OTA can't reach (e.g. wrong WiFi credentials).

### If the upload fails or hangs

- Make sure nothing else has the port open (Arduino IDE's Serial Monitor, PuTTY, etc.) — only one program can use a serial port at a time.
- Some D1 Mini clones need the BOOT/FLASH button held during the "Connecting..." phase — check your specific board's documentation if uploads consistently time out.
- Try a different USB cable or port — some cables are power-only and carry no data.

## Phase 2: over-the-air (OTA) updates

Once the device has flashed once and joined your WiFi, every later change (adding a fan, tweaking settings, etc.) goes over the network — run the exact same command:
```
esphome run hampton-bay-fans.yaml
```
This time ESPHome finds the device on your network via mDNS and uploads over WiFi directly — no cable, no picking a port. You'll be prompted for the OTA password from the YAML if it isn't already cached.

If OTA can't find the device:
- Confirm it's powered on and connected to WiFi (check your router's client list).
- Confirm your computer and the device are on the same network/VLAN — mDNS discovery doesn't cross most VLAN or guest-network boundaries.
- As a fallback, plug it back into USB — `esphome run` detects both options and lets you choose.

## Alternative: ESPHome Dashboard

Instead of running `esphome run` from a terminal, you can start a local web UI with:
```
esphome dashboard .
```
(run from the `esphome/` folder) which gives you the same compile/upload/log actions plus a browser-based YAML editor. If you'd rather manage this from inside Home Assistant, see the ESPHome Device Builder add-on section in [home-assistant-integration.md](home-assistant-integration.md).

**Next:** [home-assistant-integration.md](home-assistant-integration.md) to bring the flashed device into Home Assistant.
