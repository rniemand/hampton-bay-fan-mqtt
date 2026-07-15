# Required Dependencies Setup

Before you can compile or flash the ESPHome firmware in this folder, you need:

1. Python 3.9 or newer
2. The `esphome` command-line tool (installed via `pip`)
3. A USB-to-UART driver, so your computer can talk to the D1 Mini board over USB for the initial flash

Pick your OS below. Once `esphome version` prints a version number, you're ready for [compilation.md](compilation.md).

## Windows

1. **Install Python.** Download the installer from [python.org/downloads](https://www.python.org/downloads/) rather than the Microsoft Store version — the Store's Python has caused path/permission issues with `pip`-installed tools for some users. During install, check **"Add python.exe to PATH"**.

   Verify in PowerShell:
   ```
   python --version
   ```

2. **Install ESPHome:**
   ```
   python -m pip install esphome
   ```
   `pip` installs the `esphome` command into a `Scripts` folder (e.g. `C:\Users\<you>\AppData\Roaming\Python\Python3xx\Scripts`) that isn't always on `PATH`. If typing `esphome version` gives `'esphome' is not recognized`, either:
   - add that `Scripts` folder to your `PATH` environment variable, or
   - just run `python -m esphome version` instead (works the same, no PATH changes needed).

3. **Install the USB-to-UART driver.** The D1 Mini's onboard USB chip is a Silicon Labs CP210x. This repo already ships the driver at [`_drivers/CP210x_Universal_Windows_Driver.zip`](../../_drivers/CP210x_Universal_Windows_Driver.zip) — unzip it and run the installer for your Windows version (see [`_drivers/notes.txt`](../../_drivers/notes.txt) for the original download link if you'd rather grab a fresh copy).

   Plug in the board, then check **Device Manager > Ports (COM & LPT)** for a "Silicon Labs CP210x USB to UART Bridge (COMx)" entry. Note the COM number — you'll need it when flashing.

## macOS

1. **Install Python 3** via Homebrew (macOS's bundled Python is outdated/restricted for this purpose):
   ```
   brew install python3
   ```

2. **Install ESPHome:**
   ```
   python3 -m pip install esphome
   ```
   Verify:
   ```
   esphome version
   ```

3. **USB driver:** recent macOS versions include CP210x support out of the box. If the board doesn't show up after plugging in, install the Silicon Labs VCP driver (link in [`_drivers/notes.txt`](../../_drivers/notes.txt)) and reboot.

   Verify by plugging in the board and running:
   ```
   ls /dev/cu.*
   ```
   You should see something like `/dev/cu.SLAB_USBtoUART` or `/dev/cu.usbserial-XXXX`.

## Ubuntu / Debian-based Linux

1. **Install Python and pip:**
   ```
   sudo apt update && sudo apt install python3-pip python3-venv
   ```

2. **Install ESPHome** — newer Ubuntu/Debian releases block system-wide `pip install` ("externally managed environment"), so use a virtual environment:
   ```
   python3 -m venv ~/esphome-venv
   source ~/esphome-venv/bin/activate
   pip install esphome
   ```
   You'll need to re-run the `source ~/esphome-venv/bin/activate` line in any new terminal before using `esphome`.

   Verify:
   ```
   esphome version
   ```

3. **USB driver:** the `cp210x` kernel module ships with Ubuntu already — no separate driver install needed.

4. **Serial port permissions:** add your user to the `dialout` group so you can access the port without `sudo`:
   ```
   sudo usermod -aG dialout $USER
   ```
   Log out and back in (or reboot) for this to take effect.

   Verify by plugging in the board and running:
   ```
   ls /dev/ttyUSB*
   ```

## Alternative: Home Assistant's ESPHome Dashboard

If your Home Assistant instance runs as Home Assistant OS or Supervised, you can skip local installation entirely and use the **ESPHome Device Builder** add-on instead — it runs the same ESPHome tooling inside a web UI hosted by Home Assistant. See [home-assistant-integration.md](home-assistant-integration.md) for details. The rest of the guides in this folder assume the local CLI setup described above.

**Next:** [compilation.md](compilation.md)
