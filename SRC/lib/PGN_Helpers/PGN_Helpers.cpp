/**
 * @file PGN_Helpers.cpp
 * @brief Implementation of lookup functions for NMEA2000 PGN, manufacturer, and sensor definitions.
 *
 * This file provides the implementation for all lookup functions declared in PGN_Helpers.h.
 * These functions enable efficient access to PGN definitions, manufacturer information,
 * and sensor type definitions used throughout the NEMO project.
 */

#include "PGN_Helpers.h"

/**
 * @brief Find a PGN definition by its PGN number.
 *
 * Performs a linear search through the IMPERSONATABLE_PGN_DEFS array to find
 * a matching PGN number. Returns a pointer to the definition structure if found.
 *
 * @param pgn The NMEA2000 PGN number to search for (e.g., 127250 for Vessel Heading)
 * @return Pointer to the PGNDef structure if found, nullptr otherwise
 */
const PGNDef* getPGNDef(uint32_t pgn) {
    for (int i = 0; i < IMPERSONATABLE_PGN_COUNT; i++) {
        if (IMPERSONATABLE_PGN_DEFS[i].pgn == pgn) {
            return &IMPERSONATABLE_PGN_DEFS[i];
        }
    }
    return nullptr;
}

/**
 * @brief Get the full descriptive name for a PGN.
 *
 * Retrieves the human-readable full name of a PGN, suitable for detailed
 * displays or documentation purposes.
 *
 * @param pgn The NMEA2000 PGN number to look up
 * @return Full name string (e.g., "Vessel Heading"), or nullptr if PGN not found
 */
const char* getPGNName(uint32_t pgn) {
    const PGNDef* def = getPGNDef(pgn);
    return def ? def->name : nullptr;
}

/**
 * @brief Get the abbreviated display name for a PGN.
 *
 * Retrieves a shortened version of the PGN name, suitable for display in
 * space-constrained UI elements such as menu items or list views.
 *
 * @param pgn The NMEA2000 PGN number to look up
 * @return Short name string (e.g., "Heading"), or nullptr if PGN not found
 */
const char* getPGNShortName(uint32_t pgn) {
    const PGNDef* def = getPGNDef(pgn);
    return def ? def->shortName : nullptr;
}

/**
 * @brief Get the number of editable fields for a PGN.
 *
 * Returns the count of data fields that can be edited or simulated for
 * the specified PGN message type.
 *
 * @param pgn The NMEA2000 PGN number to query
 * @return Number of fields (1 to MAX_PGN_FIELDS), or 0 if PGN not found
 */
int getPGNFieldCount(uint32_t pgn) {
    const PGNDef* def = getPGNDef(pgn);
    return def ? def->fieldCount : 0;
}

/**
 * @brief Get a specific field definition from a PGN.
 *
 * Retrieves the complete field definition including name, value range,
 * and unit of measurement for a specific field within a PGN message.
 *
 * @param pgn The NMEA2000 PGN number containing the field
 * @param fieldIndex Zero-based index of the field (0 to fieldCount-1)
 * @return Pointer to PGNFieldDef structure, or nullptr if PGN not found
 *         or fieldIndex is out of bounds
 */
const PGNFieldDef* getPGNField(uint32_t pgn, int fieldIndex) {
    const PGNDef* def = getPGNDef(pgn);
    if (def && fieldIndex >= 0 && fieldIndex < def->fieldCount) {
        return &def->fields[fieldIndex];
    }
    return nullptr;
}

/**
 * @brief Get the valid value range for a PGN field.
 *
 * Retrieves the minimum and maximum valid values for a specific field
 * within a PGN message. This is useful for value validation and UI slider
 * configuration.
 *
 * If the specified PGN or field index is not found, default values of
 * 0.0 and 100.0 are returned to provide safe fallback behavior.
 *
 * @param pgn The NMEA2000 PGN number containing the field
 * @param fieldIndex Zero-based index of the field
 * @param[out] minOut Reference to store the minimum valid value
 * @param[out] maxOut Reference to store the maximum valid value
 */
void getPGNFieldRange(uint32_t pgn, int fieldIndex, float& minOut, float& maxOut) {
    const PGNFieldDef* field = getPGNField(pgn, fieldIndex);
    if (field) {
        minOut = field->minValue;
        maxOut = field->maxValue;
    } else {
        // Default range if PGN or field not found
        minOut = 0;
        maxOut = 100;
    }
}

/**
 * @brief Get a list of all field names for a PGN.
 *
 * Constructs and returns a vector containing the names of all editable
 * fields for the specified PGN, in order of their field index. This is
 * useful for populating UI selection lists or generating field documentation.
 *
 * @param pgn The NMEA2000 PGN number to query
 * @return Vector of field name strings in field index order;
 *         empty vector if PGN not found
 */
std::vector<String> getPGNFieldNames(uint32_t pgn) {
    std::vector<String> names;
    const PGNDef* def = getPGNDef(pgn);
    if (def) {
        for (int i = 0; i < def->fieldCount; i++) {
            names.push_back(def->fields[i].name);
        }
    }
    return names;
}

/**
 * @brief Check if a PGN is in the impersonatable list.
 *
 * Determines whether the specified PGN can be simulated or impersonated
 * by the NEMO device. This is useful for validating user input or
 * filtering available options in the UI.
 *
 * @param pgn The NMEA2000 PGN number to check
 * @return true if the PGN is defined in IMPERSONATABLE_PGN_DEFS, false otherwise
 */
bool isImpersonatablePGN(uint32_t pgn) {
    return getPGNDef(pgn) != nullptr;
}

/**
 * @brief Get a manufacturer definition by array index.
 *
 * Retrieves a manufacturer definition by its position in the MANUFACTURERS
 * array. This is useful when iterating through all manufacturers or when
 * the index is known from a UI selection.
 *
 * @param index Zero-based index into the MANUFACTURERS array (0 to MANUFACTURER_COUNT-1)
 * @return Pointer to ManufacturerDef structure, or nullptr if index is out of bounds
 */
const ManufacturerDef* getManufacturer(int index) {
    if (index >= 0 && index < MANUFACTURER_COUNT) {
        return &MANUFACTURERS[index];
    }
    return nullptr;
}

/**
 * @brief Get a manufacturer name by their NMEA2000 code.
 *
 * Searches for a manufacturer by their assigned NMEA2000 manufacturer code
 * and returns their name. Manufacturer codes are standardized by the NMEA
 * organization.
 *
 * @param code The NMEA2000 manufacturer code (e.g., 229 for Garmin, 1851 for Raymarine)
 * @return Manufacturer name string, or nullptr if code not found in the list
 */
const char* getManufacturerName(uint16_t code) {
    for (int i = 0; i < MANUFACTURER_COUNT; i++) {
        if (MANUFACTURERS[i].code == code) {
            return MANUFACTURERS[i].name;
        }
    }
    return nullptr;
}

/**
 * @brief Find the array index of a manufacturer by code.
 *
 * Searches for a manufacturer by their NMEA2000 code and returns their
 * position in the MANUFACTURERS array. This is useful for setting selection
 * indices in UI dropdown menus or list views.
 *
 * @param code The NMEA2000 manufacturer code to search for
 * @return Zero-based index into MANUFACTURERS array, or -1 if code not found
 */
int getManufacturerIndex(uint16_t code) {
    for (int i = 0; i < MANUFACTURER_COUNT; i++) {
        if (MANUFACTURERS[i].code == code) {
            return i;
        }
    }
    return -1;
}

/**
 * @brief Get a sensor definition by type index.
 *
 * Retrieves a sensor definition by its position in the SENSOR_DEFS array.
 * This provides access to all sensor properties including display name,
 * product name, associated PGN, and device class/function codes.
 *
 * @param typeIndex Zero-based index into the SENSOR_DEFS array (0 to SENSOR_COUNT-1)
 * @return Pointer to SensorDef structure, or nullptr if index is out of bounds
 */
const SensorDef* getSensorDef(int typeIndex) {
    if (typeIndex >= 0 && typeIndex < SENSOR_COUNT) {
        return &SENSOR_DEFS[typeIndex];
    }
    return nullptr;
}

/**
 * @brief Get the display name for a sensor type.
 *
 * Retrieves the human-readable display name for a sensor type, suitable
 * for use in menu items, list displays, and user-facing UI elements.
 *
 * @param typeIndex Zero-based index into the SENSOR_DEFS array
 * @return Display name string (e.g., "Engine RPM", "Water Depth"),
 *         or "Unknown" if index is out of bounds
 */
const char* getSensorDisplayName(int typeIndex) {
    const SensorDef* def = getSensorDef(typeIndex);
    return def ? def->displayName : "Unknown";
}

/**
 * @brief Get the NMEA2000 product name for a sensor type.
 *
 * Retrieves the product name that is broadcast on the NMEA2000 network
 * to identify the simulated device. This name appears in network device
 * lists on chartplotters and other NMEA2000 devices.
 *
 * @param typeIndex Zero-based index into the SENSOR_DEFS array
 * @return Product name string (e.g., "Engine Sensor", "Depth Sensor"),
 *         or "Generic Sensor" if index is out of bounds
 *
 * @note Product names are limited to 16 characters per NMEA2000 specification.
 */
const char* getSensorProductName(int typeIndex) {
    const SensorDef* def = getSensorDef(typeIndex);
    return def ? def->productName : "Generic Sensor";
}

/**
 * @brief Get the NMEA2000 device function code for a sensor type.
 *
 * Retrieves the device function code that identifies the specific type
 * of device within its device class. These codes are defined by the
 * NMEA2000 specification and are used for device identification on
 * the network.
 *
 * @param typeIndex Zero-based index into the SENSOR_DEFS array
 * @return Device function code as defined by NMEA2000 specification,
 *         or 130 (generic sensor function) if index is out of bounds
 */
uint8_t getSensorDeviceFunction(int typeIndex) {
    const SensorDef* def = getSensorDef(typeIndex);
    return def ? def->deviceFunction : 130;  // Default: generic sensor
}

/**
 * @brief Get the NMEA2000 device class code for a sensor type.
 *
 * Retrieves the device class code that identifies the general category
 * of the device. Device classes are defined by the NMEA2000 specification
 * and group related device types together.
 *
 * Common device classes include:
 * - 25: Inter/Intranetwork Device
 * - 35: Electrical Generation
 * - 50: Propulsion
 * - 60: Navigation
 * - 75: Sensor Communication Interface
 * - 85: External Environment
 *
 * @param typeIndex Zero-based index into the SENSOR_DEFS array
 * @return Device class code as defined by NMEA2000 specification,
 *         or 25 (Inter/Intranetwork Device) if index is out of bounds
 */
uint8_t getSensorDeviceClass(int typeIndex) {
    const SensorDef* def = getSensorDef(typeIndex);
    return def ? def->deviceClass : 25;  // Default: Inter/Intranetwork Device
}

/** @} */ /* End of Sensor_Lookup_Functions group */
