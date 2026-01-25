# Operations Guide

Step-by-step instructions for performing each NEMO operation from a fresh device boot.

**Prerequisites**: NEMO hardware is assembled and firmware is flashed. See [Getting Started](Getting_Started.md) if you haven't done this yet.



## 1. Simulate a Network Sensor

This configures NEMO to act as a fake NMEA2000 sensor that broadcasts data on the CAN bus.

### Hardware Setup

1. Connect NMEO to the NMEA2000 network
2. Ensure 120 ohm termination resistors are at each end of the CAN bus
3. Power on NEMO via USB 
   
### Steps from Fresh Boot

1. **Power on** - OLED displays the main menu:
   ```
       MAIN MENU

   > Sensor Readings
     Configure
     Attacks
     About
   ```

2. **Navigate to Configure** - Press **Down** once to highlight "Configure", then press **Right** to enter

3. **Select a Sensor** - You'll see:
   ```
      CONFIGURE

   > Sensor 1
     Sensor 2
     Sensor 3
     Device Config
   ```
   Press **Right** to configure Sensor 1 (or navigate to Sensor 2/3)

4. **Set Manufacturer** - The first field is "Manufacturer"
   - Press **Right** to edit
   - Use **Up/Down** to select: Garmin, Raymarine, Simrad, Navico, Lowrance, Furuno, B&G, Mercury, Yamaha, Volvo Penta, or Maretron
   - Press **Right** to confirm

5. **Set Device Type** - You're now on "Device Type"
   - Use **Up/Down** to select the sensor type you want to simulate:
     - Engine RPM (PGN 127488, range 0-6000 RPM)
     - Water Depth (PGN 128267, range 0-100m)
     - Heading (PGN 127250, range 0-360 degrees)
     - Speed (PGN 128259, range 0-20 m/s)
     - Rudder (PGN 127245, range -45 to +45 degrees)
     - Wind Speed (PGN 130306, range 0-50 m/s)
     - Wind Angle (PGN 130306, range 0-360 degrees)
     - Water Temp (PGN 130311, range -5 to +40 C)
     - Outside Temp (PGN 130310, range -20 to +50 C)
     - Pressure (PGN 130314, range 80-110 kPa)
     - Humidity (PGN 130313, range 0-100%)
     - Battery Voltage (PGN 127508, range 0-30V)
     - Tank Level (PGN 127505, range 0-100%)
   - Press **Right** to confirm

6. **Activate the Sensor** - You're now on "Active"
   - Press **Right** to toggel between "Yes" and "No"

7. **Control Sensor Value** - Turn the corresponding potentiometer to change the sensor's output value:
   - **Sensor 1**: Pot R1
   - **Sensor 2**: Pot R2
   - **Sensor 3**: Pot R3

8. **Exit** - Press **Left** repeatedly to return to the main menu

The sensor is now broadcasting NMEA2000 messages on CAN1. You can configure up to 3 independent sensors simultaneously.



## 2. View Live Data on the Network

This monitors NMEA2000 traffic and displays parsed sensor data in real-time.

### Hardware Setup

1. Connect NMEO to the NMEA2000 network
2. Ensure 120 ohm termination resistors are at each end of the CAN bus
3. Power on NEMO via USB

### Steps from Fresh Boot

1. **Power on** - OLED displays the main menu

2. **Enter Live Data** - "Live Data" is already highlighted, press **Right** to enter

3. **View Discovered Devices** - After a moment, you'll see devices on the network:
   ```
   Live Daya

   > Sensor 1 [22]
     GPS Unit [50]
     Depth Sounder [51]
   ```
   - Device name shown with source address in brackets
   - Use **Up/Down** to scroll through devices
   - Devices auto-appear when they send ISO Address Claims

4. **Select a Device** - Press **Right** on a device to see its details:
   ```
   Device: GPS Unit
   Addr: 50

   > 129025 Position
     129026 COG/SOG
     60928  Addr Claim
   ```

5. **View PGN Data** - Press **Right** on a PGN to see parsed fields:
   ```
    129025 Position

    Latitude: 42.3601
    Longitude: -71.0589
   ```
   Values update in real-time as new messages arrive

6. **Navigate Back** - Press **Left** to go back to device list or main menu

### Troubleshooting

- **No devices showing**: Check wiring, termination, and that devices are actually transmitting
- **Stale devices**: Devices disappear after 60 seconds of silence (configurable in Device Config)



## 3. Run an Impersonation Attack

This spoofs a legitimate device's messages by sending modified NMEA2000 data from its source address.

### Hardware Setup

1. Connect NMEO to the NMEA2000 network
2. Ensure 120 ohm termination resistors are at each end of the CAN bus
3. Power on NEMO via USB

### Steps from Fresh Boot

1. **Power on** - OLED displays the main menu

2. **Wait for Device Discovery** - Give NEMO a few seconds to discover devices. You can verify by checking "Live Data" first.

3. **Navigate to Attacks** - Press **Down** twice to highlight "Attacks", then press **Right**

4. **Select Impersonate** - You'll see:
   ```
       ATTACKS

   > DOS Attack
     Impersonate
   ```
   Press **Down** to highlight "Impersonate", then press **Right**

5. **Select Target Device** - A list of devices with impersonatable PGNs appears:
   ```
   SELECT TARGET

   > GPS Unit [50]
     Depth Sounder [51]
     Sensor 1 [22]
   ```
   - Only devices sending supported PGNs are shown
   - Use **Up/Down** to select, press **Right** to confirm

6. **Select Target PGN** - Choose which message type to spoof:
   ```
   SELECT PGN

   > 129025 Position
     129026 COG/SOG
   ```
   Press **Right** to confirm

7. **Select Field to Manipulate** - Choose which data field to control:
   ```
   SELECT FIELD

   > Latitude
     Longitude
   ```
   Press **Right** to confirm

8. **Adjust Value** - The attack is now active:
   ```
      IMPERSONATE

    Device: GPS Unit
    PGN: 129025 Position

    Latitude: 42.3601

    Pot: 42.3601
   ```
   - Turn **Pot R1** (Pin 16) to change the spoofed value
   - The display shows the current value and range

9. **Lock Field Value** (Optional) - Press **Right** to lock the current value:
   - The field now shows `[Locked]` prefix
   - You can select another field to manipulate
   - Locked fields maintain their value while you adjust others
   - You can lock up to 16 fields simultaneously

10. **Stop Attack** - Press **Left** to exit the menu. You will see "Attack Active" on the bottom. To check on the status, enter the attacks menu again. You will have the opprotunity to stop the attack or just view the current status.

### Example: Spoofing Position Data

1. Target: GPS Unit
2. PGN: 129025 (Position)
3. Lock Latitude at desired value (e.g., 42.3601)
4. Select Longitude field
5. Lock Longitude at desired value (e.g., -71.0589)
6. All connected chartplotters now display the fake position



## 4. Run a DOS Attack

This attack kicks devices off the network by winning all address arbitration fights.

### Hardware Setup

1. Connect NMEO to the NMEA2000 network
2. Ensure 120 ohm termination resistors are at each end of the CAN bus
3. Power on NEMO via USB

### Steps from Fresh Boot

1. **Power on** - OLED displays the main menu

2. **Navigate to Attacks** - Press **Down** twice to highlight "Attacks", then press **Right**

3. **Select Address Claim** - "DOS Attack" is already highlighted:
   ```
       ATTACKS

   > DOS Attack
     Impersonate
   ```
   Press **Right** to start

4. **Attack Begins Immediately** - The display shows:
   ```
     DOS Attack

    Status: ACTIVE
    Messages: 0

    Press < to stop
   ```
   - **Messages**: Counter increments as claims are sent
   - **Target**: Current address being claimed
   - Attack cycles through addresses 1-252

5. **Observe Effects** - Devices on the network will:
   - Lose their address claims
   - Attempt to find new addresses
   - Eventually go offline or become unresponsive
   - GPS, depth sounders, engine monitors all stop working

6. **Stop Attack** - Press **Left** to return to main menu:
   - "Attack Active" shows on the main menu
   - Enter the attack menu again to view attack status.
   - Press **Select** to stop the DOS attacl

### How It Works

NEMO sends ISO Address Claims (PGN 60928) with a high-priority NAME:
- Unique Number: 0 (lowest possible)
- Manufacturer Code: 0 (lowest possible)
- All priority fields minimized

This NAME beats any legitimate device in arbitration, forcing them off the bus.



## 5. Run Attack + Sensor Simultaneously

This demonstrates attack effects by running sensors and attacks together so you can see real-time impact.

### Hardware Setup: 

1. Power on NEMO via USB

### Steps from Fresh Boot

#### On NEMO:

1. **Power on**

2. **Configure Sensors** - Go to Configure > Sensor 2:
   - Set Manufacturer: Garmin (or any)
   - Set Device Type: Water Depth
   - Set Active: YES

3. **Configure Second Sensor** - Go to Configure > Sensor 3:
   - Set Manufacturer: Raymarine (or any)
   - Set Device Type: Heading
   - Set Active: YES

4. **Return to Main Menu** - Press **Left** until you're at the main menu

5. **Open Live Data** - Press **Right** on "Live Data" to monitor your own sensors

6. **Return to Main Menu** - Press **Left**

7. **Launch Impersonation Attack**:
   - Go to Attacks > Impersonate
   - Select "Sensor 2" or "Sensor 3"
   - Select PGN 128267 (Water Depth)
   - Select "Depth" field
   - Turn potentiometer on Sensor 1 to set a fake depth value.
   - Lock the value in and return to main menu.

#### Observe the Effect:

- Watch Live Data > Sensor 2 > PGN 128267
- The depth value will flicker between:
  - Real value (Sensor 2)
  - Spoofed value (Sensor 1 value)


### DOS Attack Demo

1. Configure and activate sensors 1, 2, and 3

2. Launch DOS attack (Attacks > DOS Attack)

3. Return to "Live Data" menu:
   - Sensors disappear from Sensor Readings
   - Network becomes unusable
   - Device names may flicker as they fight for addresses

4. **Stop Attack**

5. **Watch Recovery**:
   - Sensors reappear after a few seconds
   - Normal operation resumes



## Quick Reference

| Operation | Menu Path | Key Controls |
|-----------|-----------|--------------|
| Simulate Sensor | Configure > Sensor 1/2/3 | Pot controls value |
| View Live Data | Sensor Readings | Up/Down scroll, Right to drill down |
| Impersonate | Attacks > Impersonate | Pot controls spoofed value, Right locks |
| DOS Attack | Attacks > DOS Attack | Starts immediately|

## Button Reference

| Button | Function |
|--------|----------|
| **Up** | Navigate up / Increase value |
| **Down** | Navigate down / Decrease value |
| **Left** | Go back / Stop attack |
| **Right** | Select / Confirm / Lock field |

## Potentiometer Assignment

| Potentiometer | Pin | Controls |
|---------------|-----|----------|
| Pot R1 | 16 | Sensor 1 value / Attack field value |
| Pot R2 | 15 | Sensor 2 value |
| Pot R3 | 14 | Sensor 3 value |
