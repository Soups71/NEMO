/**
 * @file Attack_Controller.cpp
 * @brief Core functionality for the Attack_Controller class
 *
 * This file contains the constructor and core functions for the Attack_Controller,
 * which manages attack state and coordinates between different attack modules.
 */

#include "Attack_Controller.h"
#include <Sensor.h>
#include "constants.h"

/**
 * @brief Constructs an Attack_Controller instance
 *
 * Initializes all attack state variables to their default values, including
 * spam attack state, impersonate attack state, and per-field lock arrays.
 *
 * @param can1 Pointer to the NMEA2000 CAN interface for sending messages
 * @param mon Pointer to the N2K_Monitor for device tracking and PGN data
 * @param sensor Pointer to the Sensor used for reading analog values during impersonation
 */
Attack_Controller::Attack_Controller(tNMEA2000_Teensyx* can1, N2K_Monitor* mon, Sensor* sensor) {
    nmea2000_can1 = can1;
    monitor = mon;
    valueSensor = sensor;

    // Spam attack state
    spamAttackActive = false;
    spamMessageCount = 0;
    lastSpamTime = 0;
    currentSpamAddress = 0;

    // Impersonate attack state
    impersonateActive = false;
    impTargetAddress = 0;
    impTargetPGN = 0;
    impSelectedFieldIndex = 0;
    impFieldValue = 0.0f;
    lastImpTime = 0;
    impFieldMin = 0.0f;
    impFieldMax = 100.0f;

    // Initialize per-field lock arrays
    for (int i = 0; i < MAX_IMP_FIELDS; i++) {
        impFieldLocked[i] = false;
        impFieldLockedValues[i] = 0.0f;
    }

    // Own-sensor impersonation tracking
    impersonatingOwnSensor = false;
    impOwnSensorIndex = 0;
}

/**
 * @brief Main update loop for active attacks
 *
 * Called periodically to update the state of any active attack. Delegates
 * to the appropriate attack update function based on which attack is active.
 */
void Attack_Controller::update() {
    if (impersonateActive) {
        updateImpersonate();
    }
}

/**
 * @brief Checks if any attack is currently active
 *
 * @return true if either spam or impersonate attack is active
 * @return false if no attacks are active
 */
bool Attack_Controller::isAttackActive() {
    return spamAttackActive || impersonateActive;
}

/**
 * @brief Gets the type of currently active attack
 *
 * @return AttackType The type of attack currently running (ATTACK_SPAM,
 *         ATTACK_IMPERSONATE, or ATTACK_NONE)
 */
AttackType Attack_Controller::getActiveAttackType() {
    if (spamAttackActive) return ATTACK_SPAM;
    if (impersonateActive) return ATTACK_IMPERSONATE;
    return ATTACK_NONE;
}

/**
 * @brief Sets whether impersonating own sensor
 *
 * Configures the controller to track when impersonating one of the device's
 * own registered sensors rather than an external device.
 *
 * @param own True if impersonating own sensor, false otherwise
 * @param sensorIndex Index of the own sensor being impersonated (0-2)
 */
void Attack_Controller::setImpersonatingOwnSensor(bool own, uint8_t sensorIndex) {
    impersonatingOwnSensor = own;
    impOwnSensorIndex = sensorIndex;
}

/**
 * @brief Gets a human-readable status string for the current attack
 *
 * Returns a descriptive string indicating the current attack state,
 * including device name information for impersonate attacks.
 *
 * @return String Description of current attack ("DOS Attack", "Imp:<device>", or "None")
 *
 * @note Used for display purposes in the UI to show attack status.
 */
String Attack_Controller::getAttackStatusString() {
    if (spamAttackActive) return "DOS Attack";
    if (impersonateActive) {
        DeviceInfo* device = monitor->getDevice(impTargetAddress);
        if (device) return "Imp:" + device->name.substring(0, 10);
        return "Impersonate";
    }
    return "None";
}
