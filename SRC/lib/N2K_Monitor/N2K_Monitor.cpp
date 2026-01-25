/**
 * \file N2K_Monitor.cpp
 * \brief Core implementation of NMEA2000 Network Monitoring functionality
 *
 * This file contains the core implementation of the N2K_Monitor class, which
 * provides comprehensive monitoring capabilities for NMEA2000 networks. The
 * implementation handles device discovery, message processing, PGN data storage,
 * and automatic cleanup of stale network entries.
 *
 * The module is split into multiple files for maintainability:
 *   - N2K_Monitor.cpp (this file) - Constructor and core functions
 *   - N2K_PGNNames.cpp - PGN name lookup tables and functions
 *   - N2K_PGNParser.cpp - Comprehensive PGN parsing implementations
 */

#include "N2K_Monitor.h"

/**
 * \brief Construct a new N2K_Monitor instance
 *
 * Initializes the monitor with default settings:
 * - Stale entry cleanup is disabled by default
 * - Last cleanup check timestamp is set to 0
 *
 * The devices map and device list are automatically initialized
 * as empty by their default constructors.
 *
 * \note Call setStaleCleanupEnabled(true) after construction if you
 *       want automatic cleanup of devices that leave the network.
 */
N2K_Monitor::N2K_Monitor() {
    staleCleanupEnabled = false;
    lastCleanupCheck = 0;
}

/**
 * \brief Perform periodic maintenance tasks
 *
 * This method should be called regularly from the main loop to perform
 * periodic maintenance. Currently implements:
 * - Stale entry cleanup (every 5 seconds when enabled)
 *
 * The 5-second interval prevents excessive CPU usage while still
 * maintaining reasonable responsiveness for device removal.
 */
void N2K_Monitor::update() {
    // Periodically check for stale entries (every 5 seconds)
    unsigned long currentTime = millis();
    if(currentTime - lastCleanupCheck > 5000) {
        lastCleanupCheck = currentTime;
        cleanupStaleEntries();
    }
}

/**
 * \brief Retrieve a device by its source address
 *
 * Looks up a device in the internal map by its NMEA2000 source address.
 * This provides direct access to the device's information including
 * name, timing data, and all received PGNs.
 *
 * \param address The NMEA2000 source address (0-252) to look up
 *
 * \return Pointer to the DeviceInfo structure if the device exists,
 *         nullptr if no device with that address has been seen
 */
DeviceInfo* N2K_Monitor::getDevice(uint8_t address) {
    // Check if the device exists in the map
    if(devices.find(address) != devices.end()) {
        return &devices[address];
    }
    return nullptr;
}

/**
 * \brief Retrieve PGN data for a specific device and PGN number
 *
 * Looks up stored PGN data for a given device and PGN combination.
 * This allows access to parsed field data without iterating through
 * the device's entire PGN map.
 *
 * \param deviceAddress The NMEA2000 source address of the device
 * \param pgn The PGN number to retrieve data for
 *
 * \return Pointer to the PGNData structure if found, nullptr if either
 *         the device doesn't exist or hasn't sent that PGN
 */
PGNData* N2K_Monitor::getPGNData(uint8_t deviceAddress, uint32_t pgn) {
    // First check if the device exists
    if(devices.find(deviceAddress) != devices.end()) {
        // Then check if the PGN exists for this device
        if(devices[deviceAddress].pgns.find(pgn) != devices[deviceAddress].pgns.end()) {
            return &devices[deviceAddress].pgns[pgn];
        }
    }
    return nullptr;
}

/**
 * \brief Process an incoming NMEA2000 message
 *
 * This is the primary entry point for message processing. It should be
 * called from your NMEA2000 library message handler callback for each
 * received message.
 *
 * \param N2kMsg Reference to the received NMEA2000 message from the library
 */
void N2K_Monitor::handleN2kMessage(const tN2kMsg &N2kMsg) {
    uint8_t source = N2kMsg.Source;

    
    // Device Discovery and Creation
    
    // Check if this is a new device (source address not in our map)
    if(devices.find(source) == devices.end()) {
        // Create a new device entry with default values
        DeviceInfo newDevice;
        newDevice.sourceAddress = source;
        newDevice.name = "Device " + String(source);  // Default name until we get more info
        newDevice.lastSeen = millis();
        newDevice.lastHeartbeat = 0;  // No heartbeat received yet
        devices[source] = newDevice;

        // Rebuild the ordered device list for consistent UI display
        deviceList.clear();
        for(auto& pair : devices) {
            deviceList.push_back(pair.first);
        }

    }

    
    // Timing Updates
    
    // Always update the lastSeen timestamp for this device
    devices[source].lastSeen = millis();

    // Track heartbeat messages separately for stale detection
    // PGN 126993 is the NMEA2000 Heartbeat message
    if(N2kMsg.PGN == 126993) {
        devices[source].lastHeartbeat = millis();
    }

    
    // Device Name Resolution
    
    // ISO Address Claim (PGN 60928) provides manufacturer code and device function
    // This is used as a fallback if Product Information is not available
    if(N2kMsg.PGN == 60928 && N2kMsg.DataLen >= 8) {
        // Only use Address Claim info if we don't have a Model ID yet
        // (Product Information provides better names when available)
        if(devices[source].name.startsWith("Device ")) {
            // Parse the 8-byte NAME field from ISO Address Claim
            // The NAME is transmitted in little-endian byte order
            uint64_t name = 0;
            for(int i = 0; i < 8; i++) {
                name |= ((uint64_t)N2kMsg.Data[i]) << (i * 8);
            }

            // Extract manufacturer code (bits 21-31, 11 bits)
            uint16_t mfrCode = (name >> 21) & 0x7FF;

            // Extract device function (bits 40-47, 8 bits)
            uint8_t devFunction = (name >> 40) & 0xFF;

            // Build a descriptive name using the manufacturer code
            String devName = "Mfr" + String(mfrCode);

            // Add a function hint based on the device function code ranges
            // These ranges are approximate groupings of related functions
            if(devFunction >= 130 && devFunction <= 140) devName += " Nav";       // Navigation devices
            else if(devFunction >= 140 && devFunction <= 160) devName += " Eng";  // Engine/propulsion
            else if(devFunction >= 170 && devFunction <= 180) devName += " Pwr";  // Power management

            devices[source].name = devName;
        }
    }

    // Product Information (PGN 126996) provides the actual Model ID string
    // This is the preferred source for device names when available
    if(N2kMsg.PGN == 126996) {
        // Buffers for parsing the Product Information fields
        unsigned short N2kVersion, ProductCode;
        unsigned char CertificationLevel, LoadEquivalency;
        char ModelID[33], SwCode[41], ModelVersion[25], ModelSerialCode[33];

        // Use the NMEA2000 library's parser for this complex multi-field PGN
        if(ParseN2kPGN126996(N2kMsg, N2kVersion, ProductCode,
                             sizeof(ModelID), ModelID,
                             sizeof(SwCode), SwCode,
                             sizeof(ModelVersion), ModelVersion,
                             sizeof(ModelSerialCode), ModelSerialCode,
                             CertificationLevel, LoadEquivalency)) {
            // Only update if we got a non-empty Model ID
            if(strlen(ModelID) > 0) {
                devices[source].name = String(ModelID);
                devices[source].name.trim();  // Remove any padding whitespace
            }
        }
    }

    
    // PGN Data Storage and Parsing
    
    // Parse the message into human-readable fields and store
    PGNData pgnData;
    parsePGNData(N2kMsg, pgnData);  // Implemented in N2K_PGNParser.cpp
    devices[source].pgns[N2kMsg.PGN] = pgnData;

    
    // Legacy API Support
    
    // Also call legacy handler for backward compatibility with older code
    // that uses the simple PGN tracking system
    if(pgnData.fields.size() > 0) {
        double value = 0;
        // Extract the primary value (first field) for legacy tracking
        if(pgnData.fields.size() > 0) {
            value = pgnData.fields[0].value.toFloat();
        }
        registerPGN(N2kMsg.PGN, pgnData.name, value);
    }
}

/**
 * \brief Register a PGN in the legacy tracking system
 *
 * Adds or updates a PGN in the legacy detectedPGNs vector. This method
 * is maintained for backward compatibility with code that doesn't use
 * device-based PGN tracking.
 *
 * If the PGN already exists in the vector, its value and timestamp are
 * updated. Otherwise, a new PGNInfo entry is created and added.
 *
 * \param pgn The PGN number to register
 * \param name Human-readable name for the PGN
 * \param value The primary numeric value from the PGN message
 */
void N2K_Monitor::registerPGN(uint32_t pgn, String name, double value) {
    // Search for existing PGN entry
    for(auto& pgnInfo : detectedPGNs) {
        if(pgnInfo.pgn == pgn) {
            // Found - update existing entry
            pgnInfo.value = value;
            pgnInfo.lastUpdate = millis();
            pgnInfo.received = true;
            return;
        }
    }

    // Not found - create and add new entry
    PGNInfo info;
    info.pgn = pgn;
    info.name = name;
    info.value = value;
    info.received = true;
    info.lastUpdate = millis();
    detectedPGNs.push_back(info);
}

/**
 * \brief Remove stale devices and PGN entries from the monitor
 *
 * Iterates through all tracked devices and removes those that have not
 * been active within the STALE_TIMEOUT_MS period (defined in constants.h).
 * Also removes individual PGN entries from active devices if those specific
 * PGNs have not been received within the timeout period.
 *
 * Activity determination:
 * - For devices that send heartbeats (PGN 126993), lastHeartbeat is used
 * - For devices without heartbeats, lastSeen is used as fallback
 *
 * Protected PGNs:
 * - ISO Address Claim (PGN 60928) is never removed independently; it is
 *   only removed when the entire device is removed
 *
 * This method performs a two-pass removal:
 * 1. First pass: Identify stale entries (can't modify while iterating)
 * 2. Second pass: Remove the identified entries
 */
void N2K_Monitor::cleanupStaleEntries() {
    // Early exit if cleanup is disabled
    if(!staleCleanupEnabled) return;

    unsigned long currentTime = millis();

    
    // Pass 1: Identify stale devices
    
    std::vector<uint8_t> devicesToRemove;

    for(auto& devicePair : devices) {
        uint8_t addr = devicePair.first;
        DeviceInfo& device = devicePair.second;

        // Determine the most recent activity timestamp
        // Prefer heartbeat if available (more reliable for active devices)
        // Fall back to lastSeen for devices that don't send heartbeats
        unsigned long lastActivity = (device.lastHeartbeat > 0) ? device.lastHeartbeat : device.lastSeen;

        // Check if device has exceeded the stale timeout
        if(currentTime - lastActivity > STALE_TIMEOUT_MS) {
            devicesToRemove.push_back(addr);
            continue;  // Skip PGN cleanup for devices being removed entirely
        }

        // =====================================================================
        // Check for stale PGNs within this active device
        // =====================================================================
        std::vector<uint32_t> pgnsToRemove;

        for(auto& pgnPair : device.pgns) {
            uint32_t pgn = pgnPair.first;
            PGNData& pgnData = pgnPair.second;

            // ISO Address Claim is special - only remove with the device
            // This ensures we maintain device identity information
            if(pgn == 60928) continue;

            // Check if this PGN has exceeded the stale timeout
            if(currentTime - pgnData.lastUpdate > STALE_TIMEOUT_MS) {
                pgnsToRemove.push_back(pgn);
            }
        }

        // Remove stale PGNs from this device
        for(uint32_t pgn : pgnsToRemove) {
            device.pgns.erase(pgn);
        }
    }

    
    // Pass 2: Remove stale devices
    
    for(uint8_t addr : devicesToRemove) {
        // Remove from the main devices map
        devices.erase(addr);

        // Also remove from the ordered deviceList for consistent UI
        for(auto it = deviceList.begin(); it != deviceList.end(); ++it) {
            if(*it == addr) {
                deviceList.erase(it);
                break;  // Each address appears only once
            }
        }
    }
}
