# Getting Started

Let's get your NEMO up and running. This guide covers setting up your dev environment and flashing the firmware.

## What You Need

- Computer with USB (Windows, macOS, or Linux)
- NEMO (see [Hardware Guide](PCB_Design.md))
- Micro USB cable for programming

## Step 1: Install PlatformIO

PlatformIO is used for building and uploading. Pick your flavor:

### VS Code Extension (Recommended)

1. Get [Visual Studio Code](https://code.visualstudio.com/)
2. Open VS Code, go to Extensions (Ctrl+Shift+X / Cmd+Shift+X)
3. Search "PlatformIO IDE"
4. Install it
5. Wait for it to finish downloading (few minutes)
6. Restart VS Code

## Step 2: Install Teensy Tools

The Teensy needs its own uploader tool:

1. Grab Teensyduino from [PJRC's site](https://www.pjrc.com/teensy/teensyduino.html)
2. Run the installer
3. When it asks about Arduino, you can skip that.

The Teensy Loader app is what actually pushes firmware to the board.

## Step 3: Get the Code

Clone it:

```bash
git clone https://github.com/Soups71/NEMO.git
cd NEMO
```

## Step 4: Open the Project

### VS Code + PlatformIO

1. Open VS Code
2. File → Open Folder
3. Navigate to `NEMO/SRC` (the firmware folder) and open it
4. Wait for PlatformIO to initialize (first time is slow)

You should see the PlatformIO toolbar at the bottom.

**Important**: Open `SRC` specifically, not the parent `NEMO` folder. PlatformIO needs `platformio.ini` in the root.

### Command Line

```bash
cd /path/to/NEMO/SRC
```

## Step 5: Build


Click the checkmark in the PlatformIO toolbar, or:
- Windows/Linux: Ctrl+Alt+B
- macOS: Cmd+Alt+B

Or use the PlatformIO sidebar (alien head icon) → Build.

First build takes a while - it's downloading libraries and toolchains. After that it's fast.

## Step 6: Upload

### VS Code

Connect the Teensy to your computer.

Click the arrow in the PlatformIO toolbar, or:
- Windows/Linux: Ctrl+Alt+U
- macOS: Cmd+Alt+U

### What Happens

1. PlatformIO builds if needed
2. Teensy Loader receives the hex file
3. If the Teensy is running old firmware, you might need to press its physical button to enter bootloader
4. Loader programs the device and reboots it

## Step 7: Check It Works

After upload:

1. OLED should light up with the main menu
2. Buttons should navigate
3. See [Operations Guide](Operations_Guide.md) for how to use everything

If display stays dark:
- Check I2C wiring
- Verify display power
- Try the reset button on Teensy

## Serial Monitor

For debugging, watch the serial output:

### VS Code

Click the plug icon in PlatformIO toolbar.



## Troubleshooting

### "No Teensy detected"

- USB cable might be charge-only (need data)
- Try different USB port
- Linux: might need udev rules for Teensy
- Press the button on the Teensy to force bootloader

### Build Fails

- Nuke the `.pio` folder and rebuild
- Update PlatformIO
- Check library downloads completed

### Upload Times Out

- Press the Teensy button during upload
- Make sure Teensy Loader is running
- Close anything else using the serial port

### Display Shows Garbage

- Wrong controller? Need SH1106, not SSD1306
- Some displays use 0x3D instead of 0x3C address
- Check for loose wires



## What's Next?

- [Operations Guide](Operations_Guide.md) - Step-by-step instructions for sensors, monitoring, and attacks
- [Attacks](Attacks.md) - See what NEMO can demonstrate
- Hook up two devices for the full attack demo experience
