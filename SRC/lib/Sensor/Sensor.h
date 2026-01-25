/** \file Sensor.h
 *  \brief NMEA2000 sensor simulation for maritime research
 *
 *  This file contains the Sensor class which simulates various NMEA2000
 *  marine sensors using analog potentiometer input. Each sensor instance
 *  appears as a separate device on the NMEA2000 network.
 *
 *  Supported sensor types include engine RPM, water depth, heading, speed,
 *  rudder angle, wind data, temperature, pressure, humidity, battery voltage,
 *  and tank level.
 */

#ifndef SENSOR_H
#define SENSOR_H

#include <Arduino.h>
#include <NMEA2000.h>
#include <N2kMessages.h>
#include <NMEA2000_Teensyx.h>

/** \enum MessageType
 *  \brief Enumeration of supported NMEA2000 message types
 *
 *  Defines the types of NMEA2000 PGN messages that sensors can transmit.
 *  Each type maps to a specific PGN and has associated value ranges.
 *
 *  \note Values must match Menu_Controller.h for consistent UI display
 */
enum MessageType {
    MSG_ENGINE_RPM = 0,      ///< Engine RPM (PGN 127488), range: 0-6000 RPM
    MSG_WATER_DEPTH = 1,     ///< Water depth (PGN 128267), range: 0-100 m
    MSG_HEADING = 2,         ///< Vessel heading (PGN 127250), range: 0-360 degrees
    MSG_SPEED = 3,           ///< Speed through water (PGN 128259), range: 0-20 knots
    MSG_RUDDER = 4,          ///< Rudder angle (PGN 127245), range: -45 to +45 degrees
    MSG_WIND_SPEED = 5,      ///< Wind speed (PGN 130306), range: 0-50 m/s
    MSG_WIND_ANGLE = 6,      ///< Wind angle (PGN 130306), range: 0-360 degrees
    MSG_WATER_TEMP = 7,      ///< Water temperature (PGN 130311), range: -5 to +40°C
    MSG_OUTSIDE_TEMP = 8,    ///< Outside air temperature (PGN 130310), range: -20 to +50°C
    MSG_PRESSURE = 9,        ///< Atmospheric pressure (PGN 130314), range: 80000-110000 Pa
    MSG_HUMIDITY = 10,       ///< Relative humidity (PGN 130313), range: 0-100%
    MSG_BATTERY_VOLT = 11,   ///< Battery voltage (PGN 127508), range: 0-30 V
    MSG_TANK_LEVEL = 12      ///< Tank fluid level (PGN 127505), range: 0-100%
};

/** \class Sensor
 *  \brief Simulates an NMEA2000 sensor using analog input
 *
 *  The Sensor class reads an analog potentiometer value and converts it to
 *  NMEA2000 messages for transmission on a CAN bus. Each sensor operates as
 *  a separate NMEA2000 device with its own source address.
 *
 *  Features:
 *  - Configurable message type (PGN)
 *  - Configurable manufacturer code for device spoofing
 *  - Active/inactive state control
 *  - Automatic value smoothing via analog averaging
 *
 *  \sa MessageType, tNMEA2000_Teensyx
 */
class Sensor {
private:
    uint8_t pin;                    ///< Analog input pin number
    MessageType messageType;        ///< Current message type being transmitted
    unsigned long lastUpdate;       ///< Timestamp of last sensor update
    int rawValue;                   ///< Raw analog reading (0-1023)
    bool active;                    ///< True if sensor is actively transmitting
    tNMEA2000_Teensyx* NMEA2000;   ///< Pointer to NMEA2000 CAN interface
    int deviceIndex;                ///< Device index for multi-device mode
    uint16_t manufacturerCode;      ///< NMEA2000 manufacturer code for NAME
    char customName[33];            ///< Custom device name (max 32 chars + null)
    uint8_t savedAddress;           ///< Saved source address when going inactive

    /** \brief Read and smooth analog input
     *  \return Smoothed analog value (average of 5 readings)
     */
    int readAnalog();

    /** \brief Get NMEA2000 device function code for message type
     *  \param type The message type
     *  \return Device function code per NMEA2000 specification
     */
    unsigned char getDeviceFunctionForType(MessageType type);

    /** \brief Get NMEA2000 device class for message type
     *  \param type The message type
     *  \return Device class code per NMEA2000 specification
     */
    unsigned char getDeviceClassForType(MessageType type);

    /** \brief Get product name string for message type
     *  \param type The message type
     *  \return Pointer to product name string
     */
    const char* getProductNameForType(MessageType type);

    /** \brief Send Engine RPM message (PGN 127488)
     *  \param N2kMsg Reference to message object to populate
     */
    void sendEngineRPM(tN2kMsg &N2kMsg);

    /** \brief Send Water Depth message (PGN 128267)
     *  \param N2kMsg Reference to message object to populate
     */
    void sendWaterDepth(tN2kMsg &N2kMsg);

    /** \brief Send Vessel Heading message (PGN 127250)
     *  \param N2kMsg Reference to message object to populate
     */
    void sendHeading(tN2kMsg &N2kMsg);

    /** \brief Send Speed message (PGN 128259)
     *  \param N2kMsg Reference to message object to populate
     */
    void sendSpeed(tN2kMsg &N2kMsg);

    /** \brief Send Rudder Angle message (PGN 127245)
     *  \param N2kMsg Reference to message object to populate
     */
    void sendRudder(tN2kMsg &N2kMsg);

    /** \brief Send Wind Speed message (PGN 130306)
     *  \param N2kMsg Reference to message object to populate
     */
    void sendWindSpeed(tN2kMsg &N2kMsg);

    /** \brief Send Wind Angle message (PGN 130306)
     *  \param N2kMsg Reference to message object to populate
     */
    void sendWindAngle(tN2kMsg &N2kMsg);

    /** \brief Send Water Temperature message (PGN 130311)
     *  \param N2kMsg Reference to message object to populate
     */
    void sendWaterTemp(tN2kMsg &N2kMsg);

    /** \brief Send Outside Temperature message (PGN 130310)
     *  \param N2kMsg Reference to message object to populate
     */
    void sendOutsideTemp(tN2kMsg &N2kMsg);

    /** \brief Send Atmospheric Pressure message (PGN 130314)
     *  \param N2kMsg Reference to message object to populate
     */
    void sendPressure(tN2kMsg &N2kMsg);

    /** \brief Send Humidity message (PGN 130313)
     *  \param N2kMsg Reference to message object to populate
     */
    void sendHumidity(tN2kMsg &N2kMsg);

    /** \brief Send Battery Voltage message (PGN 127508)
     *  \param N2kMsg Reference to message object to populate
     */
    void sendBatteryVolt(tN2kMsg &N2kMsg);

    /** \brief Send Tank Level message (PGN 127505)
     *  \param N2kMsg Reference to message object to populate
     */
    void sendTankLevel(tN2kMsg &N2kMsg);

public:
    /** \brief Construct a new Sensor object
     *  \param analogPin GPIO pin number for analog input
     *  \param type Initial message type to transmit
     *  \param CAN_Interface Pointer to NMEA2000 CAN interface
     *  \param devIndex Device index for multi-device mode (0, 1, 2, etc.)
     */
    Sensor(uint8_t analogPin, MessageType type, tNMEA2000_Teensyx* CAN_Interface, int devIndex);

    /** \brief Set the message type (PGN) this sensor transmits
     *  \param type New message type
     *  \sa MessageType
     */
    void setMessageType(MessageType type);

    /** \brief Get current message type
     *  \return Current MessageType being transmitted
     */
    MessageType getMessageType();

    /** \brief Update sensor reading from analog input
     *
     *  Reads the analog pin and updates the internal raw value.
     *  Should be called periodically from the main loop.
     */
    void update();

    /** \brief Get normalized sensor value
     *  \return Value normalized to 0.0-1.0 range
     */
    float getNormalizedValue();

    /** \brief Get raw analog reading
     *  \return Raw ADC value (0-1023)
     */
    int getRawValue();

    /** \brief Map normalized value to specified range
     *  \param min Minimum output value
     *  \param max Maximum output value
     *  \return Mapped value between min and max
     */
    float mapToRange(float min, float max);

    /** \brief Transmit NMEA2000 message based on current type
     *
     *  Builds and sends the appropriate PGN message using the
     *  current raw value and message type setting.
     */
    void sendMessage();

    /** \brief Set sensor active/inactive state
     *  \param status True to activate, false to deactivate
     *
     *  When deactivated, the sensor stops transmitting and
     *  releases its network address.
     */
    void setActive(bool status);

    /** \brief Check if sensor is active
     *  \return True if sensor is actively transmitting
     */
    bool isActive();

    /** \brief Get device index for this sensor
     *  \return Device index (0, 1, 2, etc.)
     */
    int getDeviceIndex();

    /** \brief Update NMEA2000 device information
     *
     *  Updates the device NAME, product info, and configuration
     *  based on current message type. Call after changing type.
     */
    void updateDeviceInfo();

    /** \brief Set manufacturer code for device NAME
     *  \param code NMEA2000 manufacturer code (e.g., 229 for Garmin)
     *
     *  Changes the manufacturer code in the device NAME and
     *  triggers a new ISO Address Claim.
     */
    void setManufacturerCode(uint16_t code);

    /** \brief Get current manufacturer code
     *  \return Current manufacturer code
     */
    uint16_t getManufacturerCode();

    /** \brief Set custom display name for sensor
     *  \param name Null-terminated name string (max 32 chars)
     */
    void setCustomName(const char* name);

    /** \brief Get custom display name
     *  \return Pointer to custom name string
     */
    const char* getCustomName();
};

#endif // SENSOR_H
