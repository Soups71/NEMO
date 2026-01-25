# Source Code Architecture

Welcome to the guts of NEMO. This doc breaks down how the firmware works, what each module does, and where to find stuff when you want to hack on it yourself.

## Project Structure

We're using PlatformIO because fighting with Arduino IDE dependency hell isn't fun for anyone.


## Build Setup

**File**: `SRC/platformio.ini`

Nothing fancy here:

```ini
[env:teensy40]
platform = teensy
board = teensy40
framework = arduino
```

### Dependencies

| Library | What it does |
|---------|--------------|
| `U8g2` | Talks to the OLED |
| `Adafruit SSD1306` | More display stuff |
| `Adafruit GFX Library` | Drawing primitives |
| `VEGA_SH1106` | Our specific display controller |
| `NMEA2000` | The protocol stack |
| `NMEA2000_Teensyx` | Teensy CAN driver |

The NMEA2000 libs are forked versions for stability. 



## The Main Loop (`main.cpp`)

**File**: `SRC/src/main.cpp`

### Pin Mapping

```cpp
// Pots for controlling values
#define SENSOR_1 16    // A2 - Pot R1
#define SENSOR_2 15    // A1 - Pot R2
#define SENSOR_3 14    // A0 - Pot R3

// D-pad buttons (active-low, pullups enabled)
#define BUTTON_UP 5
#define BUTTON_DOWN 3
#define BUTTON_LEFT 2
#define BUTTON_RIGHT 4
```

### The Dual-Bus Trick

The Teensy 4.0 has two easily accessible CAN controllers:

```cpp
tNMEA2000_Teensyx NMEA2000_CAN1(tNMEA2000_Teensyx::CAN1);  // TX bus
tNMEA2000_Teensyx NMEA2000_CAN2(tNMEA2000_Teensyx::CAN2);  // Sniffer bus
```

**Why two buses?**

- **CAN1**: This is transmits all messages on the bus, including three fake sensors, plus all the attack traffic.
- **CAN2** : Listen-only mode. NEMO sniffs everything without participating in arbitration - nobody knows we're watching.

You can be on the network AND monitor it at the same time. Pretty useful for seeing the effects of your attacks in real-time.

### Multi-Device Set-Up

```cpp
#define DEVICE_MANUFACTURER_CODE 2046  // Fake Manufacture Code by default
#define DEVICE_CLASS 25                 // "Inter/Intranetwork Device"
#define NUM_DEVICES 3                   // Three fake devices
```

Each sensor is a fully independent NMEA2000 device with its own:
- Source address
- 64-bit NAME 
- Product info (model name, serial, version)
- Heartbeat

### Boot Sequence

When `setup()` runs:

1. Serial at 115200 for debug output
2. Configure all the pins
3. Spin up CAN1 with three devices starting at address 22
4. Spin up CAN2 in sniff mode with a fat 2048-frame buffer
5. Init the OLED
6. Create the monitor, attack controller, and menu controller
7. Show the main menu


### Debug Output

Everything received on CAN2 gets dumped in `candump` format:

```
can1  09F80106   [8]  00 00 23 FF FF FF FF FF
can1  09F11001   [8]  01 18 27 4A 00 FF 64 00
```

Hook up a terminal at 115200 baud and you can pipe this to analysis tools. Fast-packet messages get reassembled so you see the original frames.



## Constants (`constants.h`)

**File**: `SRC/include/constants.h`

This is where all the tunables live. Mess with these to change behavior without touching logic.

### Debug Flag

```cpp
#define DEBUG false  // true = verbose, false = candump only
```

### Timing

```cpp
STALE_TIMEOUT_MS = 60000   // Remove devices after 1 min silence
SCROLL_DELAY_MS = 400      // Text scroll speed
MAX_IMP_FIELDS = 16        // Max fields you can lock during impersonate
```


## Sensor Types

13 different sensors you can simulate:

| # | Type | PGN | Range |
|---|------|-----|-------|
| 0 | Engine RPM | 127488 | 0-6000 |
| 1 | Water Depth | 128267 | 0-100m |
| 2 | Heading | 127250 | 0-360° |
| 3 | Speed | 128259 | 0-20 knots |
| 4 | Rudder | 127245 | ±45° |
| 5 | Wind Speed | 130306 | 0-50 m/s |
| 6 | Wind Angle | 130306 | 0-360° |
| 7 | Water Temp | 130311 | -5 to 40°C |
| 8 | Outside Temp | 130310 | -20 to 50°C |
| 9 | Pressure | 130314 | 80-110 kPa |
| 10 | Humidity | 130313 | 0-100% |
| 11 | Battery | 127508 | 0-30V |
| 12 | Tank Level | 127505 | 0-100% |

## Fake Manufacturer IDs

Want to pretend you're Garmin? Raymarine?

| Manufacturer | Code |
|--------------|------|
| Garmin | 229 |
| Raymarine | 1851 |
| Simrad | 1857 |
| Navico | 275 |
| Lowrance | 140 |
| Furuno | 1855 |
| B&G | 381 |
| Mercury | 144 |
| Yamaha | 1862 |
| Volvo Penta | 174 |
| Maretron | 137 |

## Spoofable PGNs

28 message types NEMO can impersonate - basically anything useful for navigation, engine monitoring, or environmental data:

```cpp
127245 (Rudder), 127250 (Heading), 127251 (Rate of Turn),
127252 (Heave), 127257 (Attitude), 127258 (Mag Variation),
127488 (Engine Rapid), 127489 (Engine Dynamic), 127493 (Transmission),
127497 (Trip Fuel), 127505 (Fluid Level), 127506 (DC Status),
127507 (Charger), 127508 (Battery), 128000 (Leeway),
128259 (Speed Water), 128267 (Depth), 129025 (Position),
129026 (COG/SOG), 130306 (Wind), 130310-130316 (Environmental),
130576 (Trim Tab), 130577 (Direction)
```






## Debugging Tips

Serial output at 115200 shows all sniffed traffic in candump format. Pipe it to your favorite analysis tools.

| Problem | Likely Cause | Fix |
|---------|--------------|-----|
| No serial | Wrong baud | Set to 115200 |
| Black screen | I2C wiring | Check SDA(18), SCL(19) |
| No CAN traffic | Missing termination | Add 120 ohm resistors |
| Sensors not broadcasting | Inactive | Enable in Configure menu |



That's the architecture. Go hack some boats (in a lab enviroment, with your own equipment, obviously).
