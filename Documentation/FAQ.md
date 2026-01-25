# Frequently Asked Questions

Got questions? We've got answers. And probably some sarcastic remarks.



## General Stuff

### What does NEMO stand for?

NMEA2000 Education and Maritime Operations platform.

### Do I need two devices?

No!

This device is designed to be self contained meaning that you can simulate and entire's boat network with just one device.

You can simulate sensors while simultaneously running attacks against them.


### What's the difference between CAN1 and CAN2?

- **CAN1** is the trasnmitter, sends fake sensor data, and launches attacks
- **CAN2** is the listener, just watching traffic and updating device log

## Hardware Stuff

### Can I use a different microcontroller?

Short answer: not easily. The firmware is written for the Teensy 4.0. Teensy 4.1 would work with minor tweaks. Anything else? You're basically rewriting good portions of the code, but it is possible. Have fun with that.

### Why two CAN transceivers?

Each CAN interface needs its own transceiver to convert between Teensy logic levels and the differential signaling CAN uses. One per bus, no sharing.

### My display is blank / dead / showing nothing

Debug checklist:
1. Is it powered?
2. I2C wired right?
3. Right display? Firmware expects **SH1106**, not SSD1306. They look identical but aren't.
4. Still nothing? Run an I2C scanner sketch to see if anything responds at all.

### The buttons don't do anything

- Buttons should connect GPIO pin to ground when pressed
- Firmware uses internal pullups, so pressing = pin goes low
- Got a multimeter? Check if the button actually makes contact
- Check your solder joints. Cold joints are sneaky.

### Can I use different buttons or a display?

Code assumes:
- Active-low buttons (pressed = ground)
- SH1106 128x64 I2C OLED at address 0x3C

If you make changes to these then your milage may vary and you may have to update the code to reflect your changes.

## Protocol Stuff

### What NMEA2000 devices can NEMO see?

Anything transmitting on the bus. NEMO parses common PGNs and shows the data nicely. Unknown PGNs get dumped as hex—you can still see them, just gotta decode yourself.

The information received is also dumped out via serial so you can use other tools to process the data too!

### Why can't I impersonate some devices?

Impersonation only works for PGNs where the message structure has been defined in the code. Some proprietary or obscure PGNs aren't in there. PRs welcome if you reverse engineer more.

### Does NEMO support NMEA 0183?

Nope. Totally different protocol (serial ASCII sentences). NEMO only speaks NMEA2000/CAN. If you want 0183, you need different hardware.

### What baud rate?

250 kbps. That's the NMEA2000 standard. If your gear uses something else, it's not really NMEA2000.



## Attack Stuff

### How do I know if impersonation is working?

- The victim's display—values should change when you turn the pot
- Check serial output for debugging
- If nothing changes, make sure you're actually connected to the right network and the target device is receiving

### Can these attacks be detected?

Theoretically, yes. You could build an IDS that watches for:
- Spam-level address claim rates
- Multiple devices fighting over addresses
- Data that makes no physical sense (100mph wind speed, 10000ft depth)
- Timing patterns that don't match legit devices

In practice? Most marine gear has zero monitoring. It just trusts everything. That's... why we're here.

### Do these attacks work on all NMEA2000 networks?

The vulnerabilities are baked into the protocol spec, so yes, any compliant network is vulnerable. But real-world results vary:
- Some devices handle address conflicts weirdly
- Network topology matters—gotta be on the right segment
- Proprietary extensions might do unexpected things

Test it yourself. That's the fun part.



## Development Stuff

It's a bit scattered, but that's the pattern.

### Where do I find PGN documentation?

The official NMEA2000 appendix costs money. But:
- The NMEA2000 Arduino library has tons of definitions
- CANboat project has community-documented PGNs


### Can I contribute?

Open issues for bugs. PRs for new PGN support, fixes, or docs.


## Troubleshooting

### "Undefined reference" build errors

Library installation probably borked. Nuke the `.pio` folder and rebuild. Usually fixes it.

### Upload fails with "No Teensy detected"

- Is Teensy Loader running?
- Press the physical button on the Teensy to force bootloader mode
- Your USB cable might be charge-only (no data lines). Try a different one.
- Linux users: check your udev rules for Teensy

### Devices appear and disappear from the list

- Normal if devices transmit infrequently—some only send every few seconds
- "Stale Cleanup" in Device Config removes inactive devices after a minute
- Check for loose connections causing intermittent comms

### Sensor values don't change when I turn the pot

- Pot wired right? Middle pin goes to analog input
- Is the sensor set to Active?
- Is an attack running? Attacks use the pot differently

### My attack isn't doing anything

- Is CAN1 actually connected to the target network?
- Got termination on both ends?
- Can you see the target in Sensor Readings?
- Some devices take a while to notice they lost their address



## Safety & Legal

### Could this damage my equipment?

NEMO sends valid CAN messages. It can't fry your hardware through the protocol. However:
- Bad wiring can definitely damage things
- Attacks can make marine gear behave unexpectedly (that's the point)
- **Never** test on systems where malfunction could cause actual harm

### What if something goes wrong during an attack?

1. Stop the attack
2. Wait for devices to reclaim addresses
3. Power cycle anything acting weird
4. Disconnect NEMO, if needed

### Can I brick a device with these attacks?

Properly implemented NMEA2000 devices should recover just fine. But I can't guarantee every manufacturer got it right. Cheap/sketchy gear might behave unpredictably. Another reason why I take no responsibility for how you use this device.

### Is this legal?

Yes, testing in an approved setting, against approved hardware this device is legal (in America at least). If you start using this device on hardware you don't own or on boats you don't have permission to test on, the most likely answer is no. 

I take no responsibility for how you use the device. 
