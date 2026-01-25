# Security Research and Attack Demos

This doc covers the attacks NEMO can demonstrate, how they work under the hood, and what they would mean on a real boat.



## The Two Attacks

NEMO implements two attack categories:

1. **DOS Attack** - Kicks all devices off the network
2. **Impersonation Attack** - Spoofs valid sensors on the network by modifying fields certain fields and leaving others alone

Both exploit fundamental weaknesses in how NMEA2000 trusts devices on the bus - inherited straight from CAN and ISO 11783.



## DOS Attack

### The Gist

Every NMEA2000 device has a 64-bit NAME that determines its priority in address fights. When two devices claim the same address, the one with the numerically lower NAME wins. Loser has to find a new address or go dark.

NEMO's attack exploits this by:

1. **Building a high-priority NAME** with artificially low values
2. **Claiming all possible addresses** with the superior NAME. This forces all devices to find a new address.
3. **Continuosly override address claims** of the other devices as they try to find a new valid address.

The attack NAME uses:
- Unique Number: 0 (lowest possible)
- Manufacturer Code: 0 (lowest possible)
- Priority: 0 (Highest priority)
- Everything else minimized

This creates a NAME that beats basically any real device.

### What Happens

When a device loses its address claim, it has to either:
- Find an unclaimed address
- Go offline

On an actual boat this could mean:
- GPS/chartplotter stops updating
- Depth sounder data disappears
- Engine monitoring goes away
- Autopilot loses its inputs



## Impersonation Attack (Data Spoofing)

### The Gist

NMEA2000 has zero authentication. Any device can send any PGN from any source address. The impersonation attack:

1. **Monitors traffic** to see legitimate sensor data
2. **Picks a target** device and the PGNs it transmits
3. **Replays modified messages** with our own values
4. **Uses the potentiometer** to control spoofed values in real time

NEMO copies the message structure but swaps in whatever values the user wants.

### Supported PGNs

NEMO can spoof these:

**Navigation:**
- 127250: Vessel Heading
- 129025: Position (Lat/Long)
- 129026: COG/SOG
- 128267: Water Depth

**Engine & Systems:**
- 127488: Engine RPM
- 127489: Engine Parameters (oil pressure, temp, etc.)
- 127505: Fluid Level (fuel tanks)
- 127508: Battery Status

**Environmental:**
- 130306: Wind Data
- 130310/130311: Environmental Parameters
- 130312: Temperature
- 130314: Pressure

### What Could Go Wrong

**Fake Position Data**
Spoofing PGN 129025 (Position) or 129026 (COG/SOG):
- Chartplotter shows wrong location
- AIS broadcasts lies to other vessels
- Autopilot navigates to the wrong place
- ECDIS route monitoring fails

**Manipulated Depth**
Spoofing PGN 128267 (Water Depth):
- Show deeper water than reality → grounding risk
- Show shallower water → unnecessary course changes
- Disable shallow water alarms

**False Engine Data**
Spoofing engine PGNs:
- Hide overheating or low oil pressure
- Show fake RPM
- Mask critical failures until it's too late

**Wrong Wind Data**
Spoofing PGN 130306 (Wind Data):
- Mess up sail trim systems
- Make autopilot wind-steering go haywire
- Mislead crew about conditions

### Why Not Just Spoof Another Sensor?

Devices such as displays and autopilots often require the configuration of devices to ensure that the information displayed is from the correct device.

This means that if an attacker were to just send information on the bus the chance that it would actually be displayed by smart displays is actually pretty low.

By spoofing a device that is alreay on the network, it increases the chances that our modified values will actually appear on displays and be processed by autopilots.

-- 

## References

These attacks are not novel and have been well documented in other fields. Here are some articles that aided in the design of the attacks.

https://www.can-cia.org/fileadmin/cia/documents/publications/cnlm/june_2023/cnlm_23-2_p28_j1939-based_networks_vulnerability_to_address_claim_hunter_cyberattack_chris_quigley_war.pdf