# Project Overview

## What's NEMO?

NEMO (NMEA2000 Education and Maritime Operations platform) is a Teensy 4.0-based research platform that lets you simulate a boat network in the palm of your hand. Security researchers, students, and maritime folks can poke around and see how NMEA2000 actually works - and more importantly, how it breaks.

The device can:

- Pretend to be multiple marine sensors on an NMEA2000 network
- Sniff traffic from other devices on the bus
- Impersonate other devices on the CAN network
- Perform DOS attack that prevents all devices from communicating on the BUS.

Simply stated, you can hack a boat without actually having a boat.

## Why Build This?

Modern boats are basically networked computers floating on water. Most information such as Navigation, autopilot messages, depth readings, and more are sent over NMEA2000. This connectivity is great for interoperability but terrible for security.

Here's the problem: NMEA2000 was designed in the 90s and released in 2001 when "network security" meant securing the boat physically. There's no authentication, no encryption, and any device on the bus can say whatever it wants. This was to make it a cheap and versitle platform that enabled communication across manufacture and device.

However, One bad device can spoof sensor readings, steal addresses, or flood the network with garbage.

This project exists to:

1. **Show people** how these attacks actually work
2. **Give researchers** a platform for developing defenses
3. **Raise awareness** that maritime cybersecurity is a real problem

## The Big Picture

Ships are critical infrastructure. Cargo vessels move 90% of global trade. Ferries carry millions of people. Fishing boats feed communities. 

Researchers have shown attacks that could:

- Display fake GPS positions, sending vessels off course
- Manipulate depth readings, potentially causing groundings
- Hide engine problems, masking critical failures
- Confuse autopilots with conflicting commands

By making these vulnerabilities tangible, we're trying to push the industry toward actually fixing this stuff.



## Architecture

NEMO runs on a Teensy 4.0 with two independent CAN interfaces:

- **CAN1**: Handles the transmission of information on to the bus. This includes three fake NMEA2000 sensors, spoofed NMEA messages, and carries out the DOS attacks.
- **CAN2** : Listen-only mode for monitoring the network.

This dual-bus design lets you both participate in AND watch the network at the same time. Hook two NEMO devices together and one can play victim while the other demonstrates attacks.

For the full code breakdown, see [Source Code Architecture](Source_Code_Architecture.md).

### User Interface

The UI runs on a 1.3" SH1106 OLED (128x64 pixels) with four buttons:
- **Up/Down**: Navigate menus, change values
- **Left**: Go back, cancel
- **Right**: Select, confirm

Three 10K pots provide analog input for:
- Controlling fake sensor values
- Adjusting attack parameters on the fly

### What It Can Do

**Sensor Simulation** (13 types):

- Navigation: Heading, Depth, Speed, Position, COG/SOG
- Engine: RPM, Rudder Angle
- Environment: Wind, Temperature, Pressure, Humidity
- Systems: Battery Voltage, Tank Level

**Network Monitoring**:

- Auto-discovery via ISO Address Claim and Product Info
- Real-time PGN parsing and display
- Heartbeat-based stale device detection
- CAN Data Dump through serial interface

**Attack Demonstrations**:

- DOS Attack (Address Claim Takeover)
- Impersonation Attack (Data Spoofing with field locking)

**Data Dump**:

- All data that is received by the device is dumped out over the Serial Console in the standard CANDUMP format.
- This can then be processed by `candump2analyzer` and `analyzer` procided by the CANBOAT organization. [CANBOAT](https://github.com/canboat/canboat)



## Hardware Design

The custom PCB puts everything together. Check [PCB Design Reference](PCB_Design.md) for:

- Complete schematics
- Part specs and numbers
- Layout considerations
- Gerbers for manufacturing



## Credit

This project would not be possible without the amazing work put together by [Timo Lappalainen](https://github.com/ttlappalainen) and the people over at the [CANBOAT Project](https://github.com/canboat). They both have put in a ton of effort that has enabled this project. Please go check out their work if you're interested in Boat Hacking and NMEA2000.




## Where To Go From Here

- Learn about [CAN Bus](CAN_Bus.md) and [NMEA2000](NMEA2000.md) protocols
- Check the [Hardware Guide](PCB_Guide.md) for build instructions
- Follow [Getting Started](Getting_Started.md) to flash the firmware
- Dig into the [Source Code Architecture](Source_Code_Architecture.md) if you want to hack on it
