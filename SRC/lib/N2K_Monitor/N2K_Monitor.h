/**
 * \file N2K_Monitor.h
 * \brief NMEA2000 Network Monitoring and Device Tracking
 *
 * This module provides comprehensive monitoring capabilities for NMEA2000 networks.
 * It tracks devices on the network, records PGN messages, parses data fields, and
 * maintains timing information for network analysis.
 *
 * The N2K_Monitor class is designed to work with the NMEA2000 library and provides:
 * - Automatic device discovery and tracking by source address
 * - PGN message recording with parsed field data
 * - Stale entry cleanup for devices that leave the network
 * - Legacy compatibility functions for simple PGN tracking
 *
 */

#ifndef N2K_MONITOR_H
#define N2K_MONITOR_H

#include <Arduino.h>
#include <N2kMessages.h>
#include <NMEA2000.h>
#include <vector>
#include <map>
#include "constants.h"

/**
 * \struct PGNField
 * \brief Represents a single parsed field from a PGN message
 *
 * Each PGN message contains multiple data fields. This structure holds
 * the parsed representation of a single field, including its name,
 * human-readable value, and unit of measurement.
 *
 * \note Field parsing is PGN-specific and handled by parsePGNData()
 */
struct PGNField {
    String name;    ///< Field name (e.g., "Speed Over Ground", "Heading")
    String value;   ///< Parsed value as human-readable string
    String unit;    ///< Unit of measurement (e.g., "kn", "deg", "m")
};

/**
 * \struct PGNData
 * \brief Contains all data associated with a specific PGN from a device
 *
 * This structure stores both the raw message data and parsed field information
 * for a PGN received from a specific device. The raw data is retained to allow
 * re-parsing if needed.
 *
 * \note The rawData buffer is sized at 256 bytes which exceeds the maximum
 *       NMEA2000 fast-packet size of 223 bytes.
 */
struct PGNData {
    uint32_t pgn;                   ///< PGN number (Parameter Group Number)
    String name;                    ///< Human-readable name of the PGN
    unsigned long lastUpdate;       ///< Timestamp (millis) of last message received
    std::vector<PGNField> fields;   ///< Parsed fields for display
    uint8_t rawData[256];           ///< Raw message data buffer for re-parsing
    uint8_t dataLen;                ///< Length of valid data in rawData buffer
};

/**
 * \struct DeviceInfo
 * \brief Represents a device discovered on the NMEA2000 network
 *
 * This structure maintains all information known about a device on the network,
 * including its source address, name (if available), timing information, and
 * a map of all PGNs received from this device.
 *
 * Devices are identified by their source address which may change during
 * address claiming. The lastSeen timestamp is used for stale entry cleanup.
 */
struct DeviceInfo {
    uint8_t sourceAddress;              ///< NMEA2000 source address (0-252)
    String name;                        ///< Device name from ISO Address Claim (if available)
    unsigned long lastSeen;             ///< Timestamp (millis) of last message from device
    unsigned long lastHeartbeat;        ///< Timestamp of last heartbeat PGN (0 if never received)
    std::map<uint32_t, PGNData> pgns;   ///< Map of PGN numbers to their data
};

/**
 * \struct PGNInfo
 * \brief Legacy structure for simple PGN tracking
 *
 * This structure provides backward compatibility with older code that used
 * simple PGN tracking without device association. New code should use
 * DeviceInfo and PGNData instead.
 */
struct PGNInfo {
    uint32_t pgn;               ///< PGN number
    String name;                ///< Human-readable PGN name
    double value;               ///< Simple numeric value (first field only)
    bool received;              ///< Flag indicating if PGN has been received
    unsigned long lastUpdate;   ///< Timestamp of last update
};

/**
 * \class N2K_Monitor
 * \brief NMEA2000 Network Monitor for device and message tracking
 *
 * The N2K_Monitor class provides comprehensive monitoring of NMEA2000 networks.
 * It automatically discovers devices, tracks PGN messages, parses data fields,
 * and maintains timing information for network analysis.
 *
 * Key features:
 * - Automatic device discovery by source address
 * - PGN message storage with parsed field data
 * - Configurable stale entry cleanup
 * - Thread-safe device and PGN access
 * - Legacy API for backward compatibility
 */
class N2K_Monitor {
private:
    /**
     * \brief Map of all discovered devices keyed by source address
     *
     * Devices are automatically added when messages are received from
     * new source addresses. Entries may be removed by stale cleanup.
     */
    std::map<uint8_t, DeviceInfo> devices;

    /**
     * \brief Ordered list of device addresses for sequential access
     *
     * Maintains the order in which devices were discovered, useful for
     * consistent display ordering in user interfaces.
     */
    std::vector<uint8_t> deviceList;

    /**
     * \brief Flag to enable/disable automatic stale entry cleanup
     *
     * When enabled, devices that haven't been seen within STALE_TIMEOUT_MS
     * (defined in constants.h) will be automatically removed.
     */
    bool staleCleanupEnabled;

    /**
     * \brief Timestamp of last stale cleanup check
     *
     * Used to throttle cleanup operations to avoid performance impact.
     */
    unsigned long lastCleanupCheck;

    /**
     * \brief Legacy PGN tracking vector
     *
     * Maintained for backward compatibility with older code that doesn't
     * use device-based PGN tracking.
     */
    std::vector<PGNInfo> detectedPGNs;

    /**
     * \brief Parse PGN message data into human-readable fields
     *
     * Extracts and parses all fields from a raw NMEA2000 message,
     * populating the PGNData structure with human-readable values.
     *
     * \param N2kMsg Reference to the received NMEA2000 message
     * \param pgnData Reference to PGNData structure to populate
     *
     * \note Parsing is PGN-specific. Unknown PGNs will have raw hex data only.
     */
    void parsePGNData(const tN2kMsg &N2kMsg, PGNData &pgnData);

public:
    /**
     * \brief Construct a new N2K_Monitor object
     *
     * Initializes the monitor with stale cleanup disabled by default.
     * Call setStaleCleanupEnabled(true) and update() periodically to
     * enable automatic cleanup of stale device entries.
     */
    N2K_Monitor();

    /**
     * \brief Process an incoming NMEA2000 message
     *
     * This is the primary entry point for message processing. Call this
     * method from your NMEA2000 message handler callback for each received
     * message.
     *
     * The method will:
     * - Create a new device entry if the source address is new
     * - Update device timing information
     * - Store and parse the PGN data
     *
     * \param N2kMsg Reference to the received NMEA2000 message
     */
    void handleN2kMessage(const tN2kMsg &N2kMsg);

    /**
     * \brief Get reference to the devices map
     *
     * Provides direct access to the internal devices map for iteration
     * and bulk operations.
     *
     * \return Reference to std::map<uint8_t, DeviceInfo> of all devices
     *
     * \warning Modifying the returned map directly may cause inconsistencies.
     *          Use provided methods for device management when possible.
     */
    std::map<uint8_t, DeviceInfo>& getDevices() { return devices; }

    /**
     * \brief Get reference to the ordered device list
     *
     * Returns the list of device addresses in discovery order, useful
     * for consistent UI display ordering.
     *
     * \return Reference to std::vector<uint8_t> of device addresses
     */
    std::vector<uint8_t>& getDeviceList() { return deviceList; }

    /**
     * \brief Get a specific device by source address
     *
     * \param address NMEA2000 source address (0-252)
     * \return Pointer to DeviceInfo if found, nullptr otherwise
     */
    DeviceInfo* getDevice(uint8_t address);

    /**
     * \brief Get PGN data for a specific device and PGN number
     *
     * \param deviceAddress NMEA2000 source address of the device
     * \param pgn PGN number to retrieve
     * \return Pointer to PGNData if found, nullptr otherwise
     */
    PGNData* getPGNData(uint8_t deviceAddress, uint32_t pgn);

    /**
     * \brief Get human-readable name for a PGN number
     *
     * Looks up the standard name for a given PGN number.
     *
     * \param pgn PGN number to look up
     * \return String containing PGN name, or "Unknown PGN" if not found
     */
    String getPGNName(uint32_t pgn);

    /**
     * \brief Enable or disable automatic stale entry cleanup
     *
     * When enabled, the update() method will remove devices that haven't
     * been seen within STALE_TIMEOUT_MS milliseconds.
     *
     * \param enabled true to enable cleanup, false to disable
     */
    void setStaleCleanupEnabled(bool enabled) { staleCleanupEnabled = enabled; }

    /**
     * \brief Check if stale entry cleanup is enabled
     *
     * \return true if stale cleanup is enabled, false otherwise
     */
    bool isStaleCleanupEnabled() { return staleCleanupEnabled; }

    /**
     * \brief Remove stale devices and PGN entries
     *
     * Removes devices that haven't been seen within STALE_TIMEOUT_MS.
     * This method is called automatically by update() if stale cleanup
     * is enabled.
     */
    void cleanupStaleEntries();

    /**
     * \brief Periodic update function
     *
     * Call this method periodically (e.g., in loop()) to perform
     * maintenance tasks such as stale entry cleanup.
     */
    void update();

    /**
     * \brief Register a PGN for legacy tracking
     *
     * Adds a PGN to the legacy tracking system with an initial value.
     *
     * \param pgn PGN number to register
     * \param name Human-readable name for the PGN
     * \param value Initial numeric value
     */
    void registerPGN(uint32_t pgn, String name, double value);


    /**
     * \brief Get reference to legacy detected PGNs vector
     *
     * \return Reference to std::vector<PGNInfo> of registered PGNs
     */
    std::vector<PGNInfo>& getDetectedPGNs() { return detectedPGNs; }
};

#endif // N2K_MONITOR_H
