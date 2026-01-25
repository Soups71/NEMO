/** \file Sensor.cpp
 *  \brief Implementation of NMEA2000 sensor simulation for maritime research
 *
 *  This file implements the Sensor class which simulates various NMEA2000
 *  marine sensors using analog potentiometer input. The class supports
 *  multiple sensor types including engine monitoring, navigation, environmental,
 *  and vessel systems.
 *
 *  Each sensor instance operates as an independent NMEA2000 device with its
 *  own source address, allowing multiple simulated sensors to coexist on
 *  the same CAN bus network.
 */

#include <Sensor.h>
#include <PGN_Helpers.h>

//*****************************************************************************
/**
 * \brief Read and smooth analog input from potentiometer
 *
 * Performs multiple ADC readings and averages them to reduce noise and
 * provide stable sensor values. The potentiometer reading is inverted
 * (1023 - value) to match expected rotation direction.
 *
 * \return int Smoothed analog value (0-1023), averaged from 5 readings
 */
int Sensor::readAnalog() {
    // Simple averaging for stability - take 5 samples to reduce noise
    int sum = 0;
    for (int i = 0; i < 5; i++) {
      // Invert reading (1023 - value) so clockwise rotation increases value
      sum += (1023-analogRead(pin));
      delay(1);  // Brief delay for ADC stabilization between readings
    }
    return sum / 5;  // Return arithmetic mean of samples
}

//*****************************************************************************
/**
 * \brief Construct a new Sensor object
 *
 * Initializes a sensor instance with the specified analog input pin,
 * message type, CAN interface, and device index. The sensor starts
 * in an inactive state and must be explicitly activated.
 *
 * \param analogPin GPIO pin number for analog potentiometer input (0-1023 range)
 * \param type Initial NMEA2000 message type (PGN) to transmit
 * \param CAN_Interface Pointer to the NMEA2000 Teensy CAN bus interface
 * \param devIndex Device index for multi-device mode (0, 1, 2, etc.)
 */
Sensor::Sensor(uint8_t analogPin, MessageType type, tNMEA2000_Teensyx* CAN_Interface, int devIndex){
    pin = analogPin;
    messageType = type;
    lastUpdate = 0;
    rawValue = 0;
    active = false;  // Default to inactive - must be explicitly enabled
    NMEA2000 = CAN_Interface;
    deviceIndex = devIndex;
    manufacturerCode = 2046;  // Default manufacturer code (reserved range)
    savedAddress = 22 + devIndex;  // Default starting address offset from base
    pinMode(pin, INPUT);

    // Set default custom name based on device index (e.g., "Sensor 1", "Sensor 2")
    snprintf(customName, sizeof(customName), "Sensor %d", devIndex + 1);
}

//*****************************************************************************
/**
 * \brief Set the message type (PGN) this sensor will transmit
 *
 * Changes the NMEA2000 PGN that this sensor transmits. The value mapping
 * range is automatically adjusted based on the message type.
 *
 * \param type New message type from MessageType enumeration
 */
void Sensor::setMessageType(MessageType type) {
    messageType = type;
}

//*****************************************************************************
/**
 * \brief Get the current message type being transmitted
 *
 * \return MessageType Current PGN message type
 */
MessageType Sensor::getMessageType() {
    return messageType;
}

//*****************************************************************************
/**
 * \brief Update sensor reading from analog input
 *
 * Reads the current potentiometer value and stores it for subsequent
 * message transmission. Should be called periodically from the main loop
 * before sendMessage().
 */
void Sensor::update() {
    rawValue = readAnalog();
}

//*****************************************************************************
/**
 * \brief Get normalized sensor value in 0.0-1.0 range
 *
 * Converts the raw ADC reading to a normalized floating-point value
 * suitable for scaling to specific measurement ranges.
 *
 * \return float Normalized value between 0.0 and 1.0
 */
float Sensor::getNormalizedValue() {
    return rawValue / 1023.0;
}

//*****************************************************************************
/**
 * \brief Get raw analog reading from last update
 *
 * Returns the most recent smoothed ADC value without normalization.
 *
 * \return int Raw ADC value (0-1023)
 */
int Sensor::getRawValue() {
    return rawValue;
}

//*****************************************************************************
/**
 * \brief Map normalized value to specified output range
 *
 * Linearly interpolates the normalized sensor value to a specified
 * minimum and maximum range for physical units.
 *
 * \param min Minimum output value (corresponds to pot at 0)
 * \param max Maximum output value (corresponds to pot at full)
 * \return float Mapped value: min + (normalized * (max - min))
 */
float Sensor::mapToRange(float min, float max) {
    return min + getNormalizedValue() * (max - min);
}

//*****************************************************************************
/**
 * \brief Transmit NMEA2000 message based on current sensor type
 *
 * Builds and sends the appropriate PGN message using the current
 * raw value and message type setting. Does nothing if sensor is inactive.
 *
 * The message type determines which PGN is transmitted:
 * - MSG_ENGINE_RPM: PGN 127488 (Engine Parameters, Rapid Update)
 * - MSG_WATER_DEPTH: PGN 128267 (Water Depth)
 * - MSG_HEADING: PGN 127250 (Vessel Heading)
 * - MSG_SPEED: PGN 128259 (Speed, Water Referenced)
 * - MSG_RUDDER: PGN 127245 (Rudder)
 * - MSG_WIND_SPEED: PGN 130306 (Wind Data)
 * - MSG_WATER_TEMP: PGN 130311 (Environmental Parameters)
 * - MSG_OUTSIDE_TEMP: PGN 130310 (Environmental Parameters)
 * - MSG_PRESSURE: PGN 130314 (Actual Pressure)
 * - MSG_HUMIDITY: PGN 130313 (Humidity)
 * - MSG_BATTERY_VOLT: PGN 127508 (Battery Status)
 * - MSG_TANK_LEVEL: PGN 127505 (Fluid Level)
 *
 * \sa update(), setMessageType()
 */
void Sensor::sendMessage() {
    // Skip transmission if sensor is deactivated
    if(!active) return;

    tN2kMsg N2kMsg;

    // Dispatch to appropriate PGN handler based on configured message type
    switch (messageType) {
      case MSG_ENGINE_RPM:
        sendEngineRPM(N2kMsg);
        break;
      case MSG_WATER_DEPTH:
        sendWaterDepth(N2kMsg);
        break;
      case MSG_HEADING:
        sendHeading(N2kMsg);
        break;
      case MSG_SPEED:
        sendSpeed(N2kMsg);
        break;
      case MSG_RUDDER:
        sendRudder(N2kMsg);
        break;
      case MSG_WIND_SPEED:
        sendWindSpeed(N2kMsg);
        break;
      case MSG_WIND_ANGLE:
        sendWindAngle(N2kMsg);
        break;
      case MSG_WATER_TEMP:
        sendWaterTemp(N2kMsg);
        break;
      case MSG_OUTSIDE_TEMP:
        sendOutsideTemp(N2kMsg);
        break;
      case MSG_PRESSURE:
        sendPressure(N2kMsg);
        break;
      case MSG_HUMIDITY:
        sendHumidity(N2kMsg);
        break;
      case MSG_BATTERY_VOLT:
        sendBatteryVolt(N2kMsg);
        break;
      case MSG_TANK_LEVEL:
        sendTankLevel(N2kMsg);
        break;
    }
}

//*****************************************************************************
/**
 * \brief Send Engine RPM message (PGN 127488)
 *
 * Transmits Engine Parameters Rapid Update message with simulated
 * engine speed. Maps potentiometer to 0-6000 RPM range.
 *
 * \param N2kMsg Reference to NMEA2000 message object to populate
 */
void Sensor::sendEngineRPM(tN2kMsg &N2kMsg) {
    unsigned char EngineInstance = 0;  // Single engine configuration
    double EngineSpeed = mapToRange(0.0, 6000.0);  // Map pot to 0-6000 RPM
    double EngineBoostPressure = N2kDoubleNA;  // Not applicable
    int8_t EngineTiltTrim = N2kInt8NA;  // Not applicable

    SetN2kEngineParamRapid(N2kMsg, EngineInstance, EngineSpeed,
                           EngineBoostPressure, EngineTiltTrim);
    NMEA2000->SendMsg(N2kMsg, deviceIndex);
}

//*****************************************************************************
/**
 * \brief Send Water Depth message (PGN 128267)
 *
 * Transmits Water Depth message with simulated depth below transducer.
 * Maps potentiometer to 0-100 meter range.
 *
 * \param N2kMsg Reference to NMEA2000 message object to populate
 */
void Sensor::sendWaterDepth(tN2kMsg &N2kMsg) {
    unsigned char SID = 1;  // Sequence ID for data correlation
    double DepthBelowTransducer = mapToRange(0.0, 100.0);  // Map pot to 0-100 meters
    double Offset = 0.5;  // Transducer offset from waterline (meters)
    double Range = 100.0;  // Maximum measurable depth (meters)

    SetN2kWaterDepth(N2kMsg, SID, DepthBelowTransducer, Offset, Range);
    NMEA2000->SendMsg(N2kMsg, deviceIndex);
}

//*****************************************************************************
/**
 * \brief Send Vessel Heading message (PGN 127250)
 *
 * Transmits Magnetic Heading message with simulated compass heading.
 * Maps potentiometer to 0-360 degree range.
 *
 * \param N2kMsg Reference to NMEA2000 message object to populate
 */
void Sensor::sendHeading(tN2kMsg &N2kMsg) {
    unsigned char SID = 1;  // Sequence ID for data correlation
    // Convert degrees to radians as required by NMEA2000 specification
    double Heading = DegToRad(mapToRange(0.0, 360.0));  // Map pot to 0-360 degrees
    double Deviation = DegToRad(0.0);  // Compass deviation (calibration error)
    double Variation = DegToRad(-5.0);  // Magnetic variation (5 degrees West)

    SetN2kMagneticHeading(N2kMsg, SID, Heading, Deviation, Variation);
    NMEA2000->SendMsg(N2kMsg, deviceIndex);
}

//*****************************************************************************
/**
 * \brief Send Speed message (PGN 128259)
 *
 * Transmits Speed Water Referenced message with simulated boat speed.
 * Maps potentiometer to 0-20 knots (0-10.29 m/s) range.
 *
 * \param N2kMsg Reference to NMEA2000 message object to populate
 */
void Sensor::sendSpeed(tN2kMsg &N2kMsg) {
    unsigned char SID = 1;  // Sequence ID for data correlation
    // 20 knots = 10.29 m/s (NMEA2000 uses m/s)
    double WaterReferenced = mapToRange(0.0, 10.29);  // Map pot to 0-20 knots
    double GroundReferenced = N2kDoubleNA;  // GPS speed not available
    tN2kSpeedWaterReferenceType SWRT = N2kSWRT_Paddle_wheel;  // Sensor type

    SetN2kBoatSpeed(N2kMsg, SID, WaterReferenced, GroundReferenced, SWRT);
    NMEA2000->SendMsg(N2kMsg, deviceIndex);
}

//*****************************************************************************
/**
 * \brief Send Rudder Angle message (PGN 127245)
 *
 * Transmits Rudder message with simulated rudder position.
 * Maps potentiometer to -45 to +45 degree range (port to starboard).
 *
 * \param N2kMsg Reference to NMEA2000 message object to populate
 */
void Sensor::sendRudder(tN2kMsg &N2kMsg) {
    // Convert degrees to radians; negative = port, positive = starboard
    double RudderPosition = DegToRad(mapToRange(-45.0, 45.0));  // Map pot to +/-45 degrees
    unsigned char Instance = 0;  // Rudder instance (single rudder)
    tN2kRudderDirectionOrder RudderDirectionOrder = N2kRDO_NoDirectionOrder;
    double AngleOrder = N2kDoubleNA;  // Commanded angle not available

    SetN2kRudder(N2kMsg, RudderPosition, Instance, RudderDirectionOrder, AngleOrder);
    NMEA2000->SendMsg(N2kMsg, deviceIndex);
}

//*****************************************************************************
/**
 * \brief Send Wind Speed message (PGN 130306)
 *
 * Transmits Wind Data message with variable wind speed and fixed angle.
 * Maps potentiometer to 0-50 m/s wind speed range.
 *
 * \param N2kMsg Reference to NMEA2000 message object to populate
 */
void Sensor::sendWindSpeed(tN2kMsg &N2kMsg) {
    double windSpeed = mapToRange(0.0, 50.0);  // Map pot to 0-50 m/s
    double windAngle = DegToRad(45.0);  // Fixed angle at 45 degrees
    tN2kWindReference windRef = N2kWind_Apparent;  // Apparent wind (relative to boat)

    SetN2kWindSpeed(N2kMsg, 1, windSpeed, windAngle, windRef);
    NMEA2000->SendMsg(N2kMsg, deviceIndex);
}

//*****************************************************************************
/**
 * \brief Send Wind Angle message (PGN 130306)
 *
 * Transmits Wind Data message with variable wind angle and fixed speed.
 * Maps potentiometer to 0-360 degree wind angle range.
 *
 * \param N2kMsg Reference to NMEA2000 message object to populate
 */
void Sensor::sendWindAngle(tN2kMsg &N2kMsg) {
    double windSpeed = 10.0;  // Fixed speed at 10 m/s
    double windAngle = DegToRad(mapToRange(0.0, 360.0));  // Map pot to 0-360 degrees
    tN2kWindReference windRef = N2kWind_Apparent;  // Apparent wind (relative to boat)

    SetN2kWindSpeed(N2kMsg, 1, windSpeed, windAngle, windRef);
    NMEA2000->SendMsg(N2kMsg, deviceIndex);
}

//*****************************************************************************
/**
 * \brief Send Water Temperature message (PGN 130311)
 *
 * Transmits Environmental Parameters message with simulated sea temperature.
 * Maps potentiometer to -5 to +40 degrees Celsius range.
 *
 * \param N2kMsg Reference to NMEA2000 message object to populate
 */
void Sensor::sendWaterTemp(tN2kMsg &N2kMsg) {
    unsigned char SID = 1;  // Sequence ID for data correlation
    tN2kTempSource TempSource = N2kts_SeaTemperature;  // Sea water temperature
    // Convert Celsius to Kelvin as required by NMEA2000
    double Temperature = CToKelvin(mapToRange(-5.0, 40.0));  // Map pot to -5 to +40 C
    tN2kHumiditySource HumiditySource = N2khs_Undef;  // Not used
    double Humidity = N2kDoubleNA;
    double AtmosphericPressure = N2kDoubleNA;

    SetN2kEnvironmentalParameters(N2kMsg, SID, TempSource, Temperature,
                                  HumiditySource, Humidity, AtmosphericPressure);
    NMEA2000->SendMsg(N2kMsg, deviceIndex);
}

//*****************************************************************************
/**
 * \brief Send Outside Temperature message (PGN 130310)
 *
 * Transmits Environmental Parameters message with simulated air temperature.
 * Maps potentiometer to -20 to +50 degrees Celsius range.
 *
 * \param N2kMsg Reference to NMEA2000 message object to populate
 *
 */
void Sensor::sendOutsideTemp(tN2kMsg &N2kMsg) {
    unsigned char SID = 1;  // Sequence ID for data correlation
    tN2kTempSource TempSource = N2kts_OutsideTemperature;  // Outside air temperature
    // Convert Celsius to Kelvin as required by NMEA2000
    double Temperature = CToKelvin(mapToRange(-20.0, 50.0));  // Map pot to -20 to +50 C
    tN2kHumiditySource HumiditySource = N2khs_Undef;  // Not used
    double Humidity = N2kDoubleNA;
    double AtmosphericPressure = N2kDoubleNA;

    SetN2kEnvironmentalParameters(N2kMsg, SID, TempSource, Temperature,
                                  HumiditySource, Humidity, AtmosphericPressure);
    NMEA2000->SendMsg(N2kMsg, deviceIndex);
}

//*****************************************************************************
/**
 * \brief Send Atmospheric Pressure message (PGN 130314)
 *
 * Transmits Actual Pressure message with simulated barometric pressure.
 * Maps potentiometer to 80000-110000 Pascal range (typical atmospheric range).
 *
 * \param N2kMsg Reference to NMEA2000 message object to populate
 *


 */
void Sensor::sendPressure(tN2kMsg &N2kMsg) {
    unsigned char SID = 1;  // Sequence ID for data correlation
    unsigned char PressureInstance = 0;  // Sensor instance
    tN2kPressureSource PressureSource = N2kps_Atmospheric;  // Barometric pressure
    // Map to typical atmospheric pressure range (800-1100 hPa in Pascals)
    double Pressure = mapToRange(80000.0, 110000.0);  // Map pot to 80000-110000 Pa

    SetN2kPressure(N2kMsg, SID, PressureInstance, PressureSource, Pressure);
    NMEA2000->SendMsg(N2kMsg, deviceIndex);
}

//*****************************************************************************
/**
 * \brief Send Humidity message (PGN 130313)
 *
 * Transmits Humidity message with simulated relative humidity.
 * Maps potentiometer to 0-100 percent range.
 *
 * \param N2kMsg Reference to NMEA2000 message object to populate
 *
 */
void Sensor::sendHumidity(tN2kMsg &N2kMsg) {
    unsigned char SID = 1;  // Sequence ID for data correlation
    unsigned char HumidityInstance = 0;  // Sensor instance
    tN2kHumiditySource HumiditySource = N2khs_InsideHumidity;  // Inside cabin humidity
    double ActualHumidity = mapToRange(0.0, 100.0);  // Map pot to 0-100%
    double SetHumidity = N2kDoubleNA;  // Target humidity not applicable

    SetN2kHumidity(N2kMsg, SID, HumidityInstance, HumiditySource,
                   ActualHumidity, SetHumidity);
    NMEA2000->SendMsg(N2kMsg, deviceIndex);
}

//*****************************************************************************
/**
 * \brief Send Battery Voltage message (PGN 127508)
 *
 * Transmits DC Battery Status message with simulated battery voltage.
 * Maps potentiometer to 0-30 Volt range.
 *
 * \param N2kMsg Reference to NMEA2000 message object to populate
 *
 */
void Sensor::sendBatteryVolt(tN2kMsg &N2kMsg) {
    unsigned char BatteryInstance = 0;  // Battery bank instance
    double BatteryVoltage = mapToRange(0.0, 30.0);  // Map pot to 0-30 V
    double BatteryCurrent = N2kDoubleNA;  // Current not measured
    double BatteryTemperature = N2kDoubleNA;  // Temperature not measured
    unsigned char SID = 1;  // Sequence ID for data correlation

    SetN2kDCBatStatus(N2kMsg, BatteryInstance, BatteryVoltage, BatteryCurrent,
                      BatteryTemperature, SID);
    NMEA2000->SendMsg(N2kMsg, deviceIndex);
}

//*****************************************************************************
/**
 * \brief Send Tank Level message (PGN 127505)
 *
 * Transmits Fluid Level message with simulated fuel tank level.
 * Maps potentiometer to 0-100 percent range.
 *
 * \param N2kMsg Reference to NMEA2000 message object to populate
 *
 */
void Sensor::sendTankLevel(tN2kMsg &N2kMsg) {
    unsigned char Instance = 0;  // Tank instance
    tN2kFluidType FluidType = N2kft_Fuel;  // Fuel tank
    double Level = mapToRange(0.0, 100.0);  // Map pot to 0-100%
    double Capacity = 200.0;  // Tank capacity in liters

    SetN2kFluidLevel(N2kMsg, Instance, FluidType, Level, Capacity);
    NMEA2000->SendMsg(N2kMsg, deviceIndex);
}

//*****************************************************************************
/**
 * \brief Set sensor active/inactive state
 *
 * Enables or disables the sensor on the NMEA2000 network. When activated,
 * the sensor claims a network address and begins transmitting. When
 * deactivated, the sensor releases its address and stops all transmissions.
 *
 * Activation sequence:
 * 1. Restore saved network source address
 * 2. Enable 60-second heartbeat interval
 * 3. Update device information (NAME, product info)
 * 4. Send ISO Address Claim to announce presence
 * 5. Broadcast product information to network
 *
 * Deactivation sequence:
 * 1. Save current source address for later restoration
 * 2. Disable heartbeat transmission
 * 3. Set source address to null (254) to release bus presence
 *
 * \param status True to activate sensor, false to deactivate
 */
void Sensor::setActive(bool status){
    active = status;

    // Enable or disable this device on the NMEA2000 bus
    if(active) {
        // Restore the saved address for this device
        NMEA2000->SetN2kSource(savedAddress, deviceIndex);

        // Restore default 60-second heartbeat interval (NMEA2000 requirement)
        NMEA2000->SetHeartbeatIntervalAndOffset(60000, 0, deviceIndex);

        // Update device info with current settings (name, manufacturer, etc.)
        updateDeviceInfo();

        // Reinitialize the device on the network by sending ISO Address Claim
        // Broadcast address (0xff) announces to all devices
        NMEA2000->SendIsoAddressClaim(0xff, deviceIndex, 0);

        // Broadcast product information so other devices know our capabilities
        NMEA2000->SendProductInformation(deviceIndex);
    } else {
        // Save the current address before going offline for later restoration
        savedAddress = NMEA2000->GetN2kSource(deviceIndex);

        // Disable heartbeat by setting interval to 0
        NMEA2000->SetHeartbeatIntervalAndOffset(0, 0, deviceIndex);

        // Set device to null address (254) - this removes it from the bus
        // The library will no longer respond to protocol requests for this device
        // N2kNullCanBusAddress = 254 per NMEA2000 specification
        NMEA2000->SetN2kSource(254, deviceIndex);
    }
}

//*****************************************************************************
/**
 * \brief Check if sensor is actively transmitting
 *
 * \return bool True if sensor is active and transmitting messages
 */
bool Sensor::isActive(){
    return active;
}

//*****************************************************************************
/**
 * \brief Get device index for this sensor
 *
 * Returns the device index assigned during construction, used for
 * multi-device CAN bus addressing.
 *
 * \return int Device index (0, 1, 2, etc.)
 */
int Sensor::getDeviceIndex(){
    return deviceIndex;
}

//*****************************************************************************
/**
 * \brief Get NMEA2000 device function code for message type
 *
 * Returns the appropriate NMEA2000 device function code based on
 * the sensor type. Uses centralized definitions from constants.h.
 *
 * \param type The message type to look up
 * \return unsigned char Device function code per NMEA2000 specification
 */
unsigned char Sensor::getDeviceFunctionForType(MessageType type) {
    return getSensorDeviceFunction(static_cast<int>(type));
}

//*****************************************************************************
/**
 * \brief Get NMEA2000 device class for message type
 *
 * Returns the appropriate NMEA2000 device class code based on
 * the sensor type. Uses centralized definitions from constants.h.
 *
 * Device classes include:
 * - 50: Propulsion (engines)
 * - 60: Navigation (heading, speed, depth)
 * - 75: Sensor Communication Interface (environmental)
 * - 35: Electrical Generation/Distribution (battery)
 *
 * \param type The message type to look up
 * \return unsigned char Device class code per NMEA2000 specification
 */
unsigned char Sensor::getDeviceClassForType(MessageType type) {
    return getSensorDeviceClass(static_cast<int>(type));
}

//*****************************************************************************
/**
 * \brief Get product name string for message type
 *
 * Returns a human-readable product name based on the sensor type.
 * Used for NMEA2000 product information broadcasts.
 *
 * \param type The message type to look up
 * \return const char* Pointer to product name string (e.g., "Engine Monitor")
 */
const char* Sensor::getProductNameForType(MessageType type) {
    return getSensorProductName(static_cast<int>(type));
}

//*****************************************************************************
/**
 * \brief Update NMEA2000 device information
 *
 * Updates both product information and device information for this sensor
 * based on current settings. This includes the device NAME (64-bit unique
 * identifier), product name, serial code, and device class/function.
 *
 * Product information includes:
 * - Serial code (unique per device, e.g., "SEN00001")
 * - Product code (100 + device index)
 * - Model ID (custom name or default for type)
 * - Software and model versions
 *
 * Device information includes:
 * - Unique number (device index + 1)
 * - Device function (sensor-type specific)
 * - Device class (sensor-type specific)
 * - Manufacturer code (configurable for spoofing)
 * - Industry group (4 = Marine)
 */
void Sensor::updateDeviceInfo() {
    // Use custom name if set, otherwise use default name for message type
    const char* deviceName = (customName[0] != '\0') ? customName : getProductNameForType(messageType);

    // Create unique serial code for this device (e.g., "SEN00001", "SEN00002")
    char serialCode[16];
    snprintf(serialCode, sizeof(serialCode), "SEN%05d", deviceIndex + 1);

    // Update product information for this device
    // Parameters: SerialCode, ProductCode, ModelID, SwCode, ModelVersion,
    //             LoadEquivalency, N2kVersion, CertLevel, iDev
    NMEA2000->SetProductInformation(
        serialCode,                              // 1: Unique serial code per device
        100 + deviceIndex,                       // 2: Product code
        deviceName,                              // 3: Model ID (custom or default)
        "1.0.0",                                 // 4: Software version
        "1.0.0",                                 // 5: Model version
        1,                                       // 6: Load equivalency (LEN)
        2101,                                    // 7: N2kVersion (NMEA2000 version 2.101)
        1,                                       // 8: Certification level
        deviceIndex                              // 9: Device index - critical for multi-device
    );

    // Update device information (NAME) for this device
    NMEA2000->SetDeviceInformation(
        deviceIndex + 1,                         // Unique number (1, 2, 3, ...)
        getDeviceFunctionForType(messageType),   // Device function per sensor type
        getDeviceClassForType(messageType),      // Device class per sensor type
        manufacturerCode,                        // Manufacturer code (configurable)
        4,                                       // Industry group: 4 = Marine
        deviceIndex                              // Device index for multi-device mode
    );
}

//*****************************************************************************
/**
 * \brief Set manufacturer code for device NAME
 *
 * Changes the manufacturer code in the NMEA2000 device NAME field.
 * This can be used to impersonate devices from different manufacturers
 * for research and testing purposes.
 *
 * After changing the manufacturer code, device information is updated
 * and a new ISO Address Claim is sent (if sensor is active) to announce
 * the new device identity to the network.
 *
 * \param code NMEA2000 manufacturer code (e.g., 229 = Garmin, 137 = Raymarine)
 */
void Sensor::setManufacturerCode(uint16_t code) {
    manufacturerCode = code;
    updateDeviceInfo();

    // Send ISO Address Claim to announce the new device identity
    if(active) {
        NMEA2000->SendIsoAddressClaim(0xff, deviceIndex, 0);
    }
}

//*****************************************************************************
/**
 * \brief Get current manufacturer code
 *
 * Returns the manufacturer code currently set in the device NAME.
 *
 * \return uint16_t Current manufacturer code
 */
uint16_t Sensor::getManufacturerCode() {
    return manufacturerCode;
}

//*****************************************************************************
/**
 * \brief Set custom display name for sensor
 *
 * Sets a custom name for this sensor that will be used in NMEA2000
 * product information. If an empty string is set, the default name
 * for the current message type will be used instead.
 *
 * After setting the name, device information is updated and a new
 * ISO Address Claim is sent (if sensor is active) to announce the
 * updated identity.
 *
 * \param name Null-terminated name string (max 32 characters)
 */
void Sensor::setCustomName(const char* name) {
    // Copy name to fixed buffer (max 32 chars + null terminator)
    strncpy(customName, name, sizeof(customName) - 1);
    customName[sizeof(customName) - 1] = '\0';  // Ensure null termination

    updateDeviceInfo();

    // Send ISO Address Claim to announce the new device identity
    if(active) {
        NMEA2000->SendIsoAddressClaim(0xff, deviceIndex, 0);
    }
}

//*****************************************************************************
/**
 * \brief Get custom display name
 *
 * Returns the custom name set for this sensor, or the default name
 * assigned during construction.
 *
 * \return const char* Pointer to custom name string
 */
const char* Sensor::getCustomName() {
    return customName;
}
