/**
 * @file PGN_Helpers.h
 * @brief Centralized PGN, manufacturer, and sensor definitions for NMEA2000 network simulation.
 *
 * This header provides a single source of truth for all NMEA2000 Parameter Group Number (PGN)
 * definitions, manufacturer codes, and sensor type definitions used throughout the NEMO project.
 * It is designed to make adding or modifying NMEA2000 data types straightforward and centralized.
 */

#ifndef PGN_HELPERS_H
#define PGN_HELPERS_H

#include <Arduino.h>
#include <vector>


/**
 * @brief Maximum number of editable fields per PGN definition.
 *
 * This constant limits the size of the fields array in PGNDef structures.
 * Increase this value if you need to support PGNs with more than 8 fields.
 */
#define MAX_PGN_FIELDS 8

/**
 * @struct PGNFieldDef
 * @brief Definition of a single editable field within a PGN message.
 *
 * Each PGN message can contain multiple data fields. This structure describes
 * one such field, including its valid value range and unit of measurement.

 */
struct PGNFieldDef {
    const char* name;      ///< Field name displayed in UI (e.g., "Heading", "Deviation")
    float minValue;        ///< Minimum valid value for this field
    float maxValue;        ///< Maximum valid value for this field
    const char* unit;      ///< Unit of measurement (e.g., "deg", "m", "kPa", "%")
};

/**
 * @struct PGNDef
 * @brief Complete definition of an NMEA2000 PGN message.
 *
 * This structure serves as the single source of truth for PGN definitions
 * that can be impersonated or simulated. It includes the PGN number, human-readable
 * names, and definitions for all editable fields.
 */
struct PGNDef {
    uint32_t pgn;                       ///< NMEA2000 PGN number (e.g., 127250 for Vessel Heading)
    const char* name;                   ///< Full descriptive name (e.g., "Vessel Heading")
    const char* shortName;              ///< Abbreviated name for display (e.g., "Heading")
    uint8_t fieldCount;                 ///< Number of editable fields (1 to MAX_PGN_FIELDS)
    PGNFieldDef fields[MAX_PGN_FIELDS]; ///< Array of field definitions
};

/**
 * @brief Array of all PGN definitions available for impersonation.
 *
 * This array contains the complete list of NMEA2000 PGNs that the NEMO device
 * can simulate or impersonate. Each entry defines the PGN number, names, and
 * all editable fields with their valid ranges.
 */
inline const PGNDef IMPERSONATABLE_PGN_DEFS[] = {
    /* --- Steering and Rudder --- */

    /** @brief PGN 127245 - Rudder angle measurement */
    {127245, "Rudder", "Rudder", 1, {
        {"Rudder", -45, 45, "deg"},
    }},

    /* --- Navigation and Attitude --- */

    /** @brief PGN 127250 - Vessel heading with magnetic deviation and variation */
    {127250, "Vessel Heading", "Heading", 3, {
        {"Heading", 0, 360, "deg"},
        {"Deviation", -30, 30, "deg"},
        {"Variation", -30, 30, "deg"},
    }},

    /** @brief PGN 127251 - Rate of turn (angular velocity) */
    {127251, "Rate of Turn", "Rate of Turn", 1, {
        {"Rate", -180, 180, "deg/min"},
    }},

    /** @brief PGN 127252 - Heave (vertical motion) measurement */
    {127252, "Heave", "Heave", 2, {
        {"Heave", -10, 10, "m"},
        {"Delay", 0, 10, "s"},
    }},

    /** @brief PGN 127257 - Vessel attitude (yaw, pitch, roll) */
    {127257, "Attitude", "Attitude", 3, {
        {"Yaw", -180, 180, "deg"},
        {"Pitch", -90, 90, "deg"},
        {"Roll", -180, 180, "deg"},
    }},

    /** @brief PGN 127258 - Magnetic variation at current location */
    {127258, "Magnetic Variation", "Mag Variation", 1, {
        {"Variation", -30, 30, "deg"},
    }},

    /* --- Engine and Propulsion --- */

    /** @brief PGN 127488 - Engine parameters, rapid update (RPM and boost) */
    {127488, "Engine Parameters Rapid", "Engine Rapid", 2, {
        {"RPM", 0, 8000, "rpm"},
        {"Boost", 0, 500, "kPa"},
    }},

    /** @brief PGN 127489 - Engine parameters, dynamic (detailed engine data) */
    {127489, "Engine Parameters Dynamic", "Engine Dynamic", 7, {
        {"Oil Press", 0, 1000, "kPa"},
        {"Oil Temp", 233, 400, "K"},
        {"Coolant", 233, 400, "K"},
        {"Alt Volt", 0, 32, "V"},
        {"Fuel Rate", 0, 200, "L/h"},
        {"Hours", 0, 100000, "h"},
        {"Load", 0, 100, "%"},
    }},

    /** @brief PGN 127493 - Transmission parameters (gear, oil pressure/temp) */
    {127493, "Transmission Parameters", "Transmission", 3, {
        {"Gear", 0, 3, ""},
        {"Oil Press", 0, 1000, "kPa"},
        {"Oil Temp", 233, 400, "K"},
    }},

    /** @brief PGN 127497 - Trip fuel consumption parameters */
    {127497, "Trip Fuel Parameters", "Trip Fuel", 2, {
        {"Trip Fuel", 0, 10000, "L"},
        {"Avg Rate", 0, 200, "L/h"},
    }},

    /* --- Tanks and Fluid Levels --- */

    /** @brief PGN 127505 - Fluid level (fuel, water, waste, etc.) */
    {127505, "Fluid Level", "Fluid Level", 1, {
        {"Level", 0, 100, "%"},
    }},

    /* --- Electrical Systems --- */

    /** @brief PGN 127506 - DC battery detailed status (SOC, health, capacity) */
    {127506, "DC Detailed Status", "DC Status", 3, {
        {"SOC", 0, 100, "%"},
        {"Health", 0, 100, "%"},
        {"Capacity", 0, 1000, "Ah"},
    }},

    /** @brief PGN 127507 - Battery charger status */
    {127507, "Charger Status", "Charger", 2, {
        {"State", 0, 7, ""},
        {"Enabled", 0, 1, ""},
    }},

    /** @brief PGN 127508 - Battery voltage and current status */
    {127508, "Battery Status", "Battery", 2, {
        {"Voltage", 0, 32, "V"},
        {"Current", -500, 500, "A"},
    }},

    /* --- Speed and Distance --- */

    /** @brief PGN 128000 - Leeway angle (side slip) */
    {128000, "Leeway", "Leeway", 1, {
        {"Leeway", -30, 30, "deg"},
    }},

    /** @brief PGN 128259 - Speed referenced to water and ground */
    {128259, "Speed Water Referenced", "Speed Water", 2, {
        {"Water Spd", 0, 20, "m/s"},
        {"Ground Spd", 0, 20, "m/s"},
    }},

    /** @brief PGN 128267 - Water depth below transducer */
    {128267, "Water Depth", "Water Depth", 2, {
        {"Depth", 0, 200, "m"},
        {"Offset", -10, 10, "m"},
    }},

    /* --- Position and Course --- */

    /** @brief PGN 129025 - GPS position, rapid update (lat/lon) */
    {129025, "Position Rapid Update", "Position", 2, {
        {"Latitude", -90, 90, "deg"},
        {"Longitude", -180, 180, "deg"},
    }},

    /** @brief PGN 129026 - Course over ground and speed over ground */
    {129026, "COG & SOG Rapid Update", "COG & SOG", 2, {
        {"COG", 0, 360, "deg"},
        {"SOG", 0, 20, "m/s"},
    }},

    /* --- Wind Data --- */

    /** @brief PGN 130306 - Wind speed and angle */
    {130306, "Wind Data", "Wind Data", 2, {
        {"Wind Spd", 0, 50, "m/s"},
        {"Wind Ang", 0, 360, "deg"},
    }},

    /* --- Environmental Parameters --- */

    /** @brief PGN 130310 - Environmental parameters (outside: water temp, air temp, pressure) */
    {130310, "Environmental Parameters Outside", "Env Outside", 3, {
        {"Water Temp", 233, 333, "K"},
        {"Air Temp", 233, 333, "K"},
        {"Pressure", 80000, 110000, "Pa"},
    }},

    /** @brief PGN 130311 - Environmental parameters (general: temp, humidity, pressure) */
    {130311, "Environmental Parameters", "Env Params", 3, {
        {"Temp", 233, 333, "K"},
        {"Humidity", 0, 100, "%"},
        {"Pressure", 80000, 110000, "Pa"},
    }},

    /** @brief PGN 130312 - Temperature (actual and set point) */
    {130312, "Temperature", "Temperature", 2, {
        {"Actual", 233, 400, "K"},
        {"Set", 233, 400, "K"},
    }},

    /** @brief PGN 130313 - Humidity (actual and set point) */
    {130313, "Humidity", "Humidity", 2, {
        {"Actual", 0, 100, "%"},
        {"Set", 0, 100, "%"},
    }},

    /** @brief PGN 130314 - Atmospheric or hydraulic pressure */
    {130314, "Pressure", "Pressure", 1, {
        {"Pressure", 80000, 110000, "Pa"},
    }},

    /** @brief PGN 130316 - Temperature extended range (actual and set point) */
    {130316, "Temperature Extended Range", "Temp Extended", 2, {
        {"Actual", 233, 400, "K"},
        {"Set", 233, 400, "K"},
    }},

    /* --- Trim and Control Surfaces --- */

    /** @brief PGN 130576 - Trim tab position (port and starboard) */
    {130576, "Trim Tab Status", "Trim Tab", 2, {
        {"Port", -100, 100, "%"},
        {"Starboard", -100, 100, "%"},
    }},

    /** @brief PGN 130577 - Direction data (comprehensive navigation data) */
    {130577, "Direction Data", "Direction", 5, {
        {"COG", 0, 360, "deg"},
        {"SOG", 0, 20, "m/s"},
        {"Heading", 0, 360, "deg"},
        {"Set", 0, 360, "deg"},
        {"Drift", 0, 10, "m/s"},
    }},
};

/**
 * @brief Total number of PGNs defined in IMPERSONATABLE_PGN_DEFS.
 *
 * Use this constant when iterating over all available PGN definitions.
 */
inline constexpr const int IMPERSONATABLE_PGN_COUNT = sizeof(IMPERSONATABLE_PGN_DEFS) / sizeof(IMPERSONATABLE_PGN_DEFS[0]);


/**
 * @struct ManufacturerDef
 * @brief Definition of an NMEA2000 manufacturer.
 *
 * Contains the manufacturer's name and their assigned NMEA2000 manufacturer code.
 * These codes are standardized by the NMEA organization.
 */
struct ManufacturerDef {
    const char* name;  ///< Manufacturer name (e.g., "Garmin", "Raymarine")
    uint16_t code;     ///< NMEA2000 assigned manufacturer code
};

/**
 * @brief Array of known NMEA2000 manufacturers and their codes.
 *
 * This array contains major marine electronics manufacturers with their
 * NMEA-assigned manufacturer codes. The NEMO device can impersonate
 * devices from any of these manufacturers.
 */
inline const ManufacturerDef MANUFACTURERS[] = {
    {"Garmin",      229},   ///< Garmin International
    {"Raymarine",   1851},  ///< Raymarine (FLIR Systems)
    {"Simrad",      1857},  ///< Simrad (Navico brand)
    {"Navico",      275},   ///< Navico (parent company)
    {"Lowrance",    140},   ///< Lowrance Electronics
    {"Furuno",      1855},  ///< Furuno Electric
    {"B&G",         381},   ///< B&G (Navico brand)
    {"Mercury",     144},   ///< Mercury Marine
    {"Yamaha",      1862},  ///< Yamaha Motor
    {"Volvo Penta", 174},   ///< Volvo Penta
    {"Maretron",    137},   ///< Maretron
};

/**
 * @brief Total number of manufacturers defined in MANUFACTURERS array.
 *
 * Use this constant when iterating over all available manufacturers.
 */
inline constexpr const int MANUFACTURER_COUNT = sizeof(MANUFACTURERS) / sizeof(MANUFACTURERS[0]);

/**
 * @struct SensorDef
 * @brief Definition of a sensor type for network simulation.
 *
 * Each sensor definition includes display information, the associated PGN,
 * and the NMEA2000 device class/function codes that identify the device type
 * on the network.
 
 */
struct SensorDef {
    const char* displayName;    ///< Display name for menus (e.g., "Engine RPM")
    const char* productName;    ///< NMEA2000 product name string (e.g., "Engine Sensor")
    uint32_t pgn;               ///< Associated PGN number for this sensor type
    uint8_t deviceFunction;     ///< NMEA2000 device function code
    uint8_t deviceClass;        ///< NMEA2000 device class code
};

/**
 * @brief Array of all sensor types available for simulation.
 *
 * This array defines the sensor types that the NEMO device can impersonate
 * on an NMEA2000 network. Each entry specifies how the sensor appears to
 * other devices on the network.
 
 */
inline const SensorDef SENSOR_DEFS[] = {
    /* --- Propulsion Sensors (Device Class 50) --- */
    {"Engine RPM",    "Engine Sensor",    127488, 140, 50},   ///< Engine RPM and parameters

    /* --- Navigation Sensors (Device Class 60) --- */
    {"Water Depth",   "Depth Sensor",     128267, 130, 60},   ///< Depth sounder
    {"Heading",       "Compass Sensor",   127250, 140, 60},   ///< Magnetic compass
    {"Speed",         "Speed Sensor",     128259, 155, 60},   ///< Speed through water
    {"Rudder",        "Rudder Sensor",    127245, 150, 60},   ///< Rudder angle sensor

    /* --- Environmental Sensors (Device Class 85) --- */
    {"Wind Speed",    "Wind Spd Sensor",  130306, 130, 85},   ///< Anemometer (speed)
    {"Wind Angle",    "Wind Ang Sensor",  130306, 130, 85},   ///< Anemometer (direction)
    {"Water Temp",    "Water Temp Sens",  130311, 130, 85},   ///< Water temperature
    {"Outside Temp",  "Air Temp Sensor",  130310, 130, 85},   ///< Air temperature
    {"Pressure",      "Pressure Sensor",  130314, 130, 85},   ///< Barometric pressure
    {"Humidity",      "Humidity Sensor",  130313, 130, 85},   ///< Humidity sensor

    /* --- Electrical Sensors (Device Class 35) --- */
    {"Battery Volt",  "Battery Monitor",  127508, 170, 35},   ///< Battery monitor

    /* --- Tank Sensors (Device Class 75) --- */
    {"Tank Level",    "Tank Sensor",      127505, 190, 75},   ///< Fluid level sensor
};

/**
 * @brief Total number of sensor types defined in SENSOR_DEFS array.
 *
 * Use this constant when iterating over all available sensor types.
 */
inline constexpr const int SENSOR_COUNT = sizeof(SENSOR_DEFS) / sizeof(SENSOR_DEFS[0]);


/**
 * @brief Find a PGN definition by its PGN number.
 *
 * Searches the IMPERSONATABLE_PGN_DEFS array for a matching PGN number
 * and returns a pointer to its definition.
 *
 * @param pgn The PGN number to search for (e.g., 127250)
 * @return Pointer to the PGNDef structure, or nullptr if not found
 */
const PGNDef* getPGNDef(uint32_t pgn);

/**
 * @brief Get the full descriptive name for a PGN.
 *
 * @param pgn The PGN number to look up
 * @return Full name string (e.g., "Vessel Heading"), or nullptr if not found
 */
const char* getPGNName(uint32_t pgn);

/**
 * @brief Get the abbreviated display name for a PGN.
 *
 * @param pgn The PGN number to look up
 * @return Short name string (e.g., "Heading"), or nullptr if not found
 */
const char* getPGNShortName(uint32_t pgn);

/**
 * @brief Get the number of editable fields for a PGN.
 *
 * @param pgn The PGN number to look up
 * @return Number of fields (1 to MAX_PGN_FIELDS), or 0 if PGN not found
 */
int getPGNFieldCount(uint32_t pgn);

/**
 * @brief Get a specific field definition from a PGN.
 *
 * @param pgn The PGN number containing the field
 * @param fieldIndex Zero-based index of the field (0 to fieldCount-1)
 * @return Pointer to PGNFieldDef, or nullptr if PGN or field not found
 */
const PGNFieldDef* getPGNField(uint32_t pgn, int fieldIndex);

/**
 * @brief Get the valid value range for a PGN field.
 *
 * Retrieves the minimum and maximum values for a specific field.
 * If the PGN or field is not found, sets default range of 0-100.
 *
 * @param pgn The PGN number containing the field
 * @param fieldIndex Zero-based index of the field
 * @param[out] minOut Reference to store minimum value
 * @param[out] maxOut Reference to store maximum value
 *

 */
void getPGNFieldRange(uint32_t pgn, int fieldIndex, float& minOut, float& maxOut);

/**
 * @brief Get a list of all field names for a PGN.
 *
 * Returns a vector containing the names of all editable fields for the
 * specified PGN, in order of their field index.
 *
 * @param pgn The PGN number to query
 * @return Vector of field name strings; empty if PGN not found
 */
std::vector<String> getPGNFieldNames(uint32_t pgn);

/**
 * @brief Check if a PGN is in the impersonatable list.
 *
 * @param pgn The PGN number to check
 * @return true if the PGN can be impersonated, false otherwise
 */
bool isImpersonatablePGN(uint32_t pgn);

/**
 * @brief Get a manufacturer definition by array index.
 *
 * @param index Zero-based index into the MANUFACTURERS array
 * @return Pointer to ManufacturerDef, or nullptr if index out of bounds

 */
const ManufacturerDef* getManufacturer(int index);

/**
 * @brief Get a manufacturer name by their NMEA2000 code.
 *
 * @param code The NMEA2000 manufacturer code (e.g., 229 for Garmin)
 * @return Manufacturer name string, or nullptr if code not found

 */
const char* getManufacturerName(uint16_t code);

/**
 * @brief Find the array index of a manufacturer by code.
 *
 * Useful for setting selection indices in UI elements.
 *
 * @param code The NMEA2000 manufacturer code to search for
 * @return Zero-based index into MANUFACTURERS array, or -1 if not found
 */
int getManufacturerIndex(uint16_t code);

/**
 * @brief Get a sensor definition by type index.
 *
 * @param typeIndex Zero-based index into the SENSOR_DEFS array
 * @return Pointer to SensorDef, or nullptr if index out of bounds
 */
const SensorDef* getSensorDef(int typeIndex);

/**
 * @brief Get the display name for a sensor type.
 *
 * @param typeIndex Zero-based index into the SENSOR_DEFS array
 * @return Display name string (e.g., "Engine RPM"), or "Unknown" if out of bounds
 */
const char* getSensorDisplayName(int typeIndex);

/**
 * @brief Get the NMEA2000 product name for a sensor type.
 *
 * This name is broadcast on the NMEA2000 network to identify the device.
 *
 * @param typeIndex Zero-based index into the SENSOR_DEFS array
 * @return Product name string (e.g., "Engine Sensor"), or "Generic Sensor" if out of bounds
 */
const char* getSensorProductName(int typeIndex);

/**
 * @brief Get the NMEA2000 device function code for a sensor type.
 *
 * Device function codes identify the specific type of device within a device class.
 *
 * @param typeIndex Zero-based index into the SENSOR_DEFS array
 * @return Device function code, or 130 (generic sensor) if out of bounds
 */
uint8_t getSensorDeviceFunction(int typeIndex);

/**
 * @brief Get the NMEA2000 device class code for a sensor type.
 *
 * Device class codes identify the general category of the device
 * (e.g., Navigation, Propulsion, Electrical).
 *
 * @param typeIndex Zero-based index into the SENSOR_DEFS array
 * @return Device class code, or 25 (Inter/Intranetwork Device) if out of bounds
 */
uint8_t getSensorDeviceClass(int typeIndex);

#endif // PGN_HELPERS_H
