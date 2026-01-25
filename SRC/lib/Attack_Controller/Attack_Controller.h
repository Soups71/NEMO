/**
 * \file Attack_Controller.h
 * \brief Attack simulation controller for NMEA2000 maritime security research.
 *
 * This module provides attack simulation capabilities for demonstrating
 * vulnerabilities in NMEA2000 networks. It supports multiple attack types
 * including denial-of-service (spam) attacks and device impersonation attacks.
 */

#ifndef ATTACK_CONTROLLER_H
#define ATTACK_CONTROLLER_H

#include <Arduino.h>
#include <N2kMessages.h>
#include <NMEA2000.h>
#include <NMEA2000_Teensyx.h>
#include <N2K_Monitor.h>
#include "constants.h"

/**
 * \enum AttackType
 * \brief Enumeration of supported attack simulation types.
 *
 * Defines the different attack modes that can be simulated on the
 * NMEA2000 network for security research and demonstration purposes.
 */
enum AttackType {
    ATTACK_NONE = 0,      ///< No attack active - normal operation mode
    ATTACK_SPAM,          ///< Denial-of-service spam attack via address claiming
    ATTACK_IMPERSONATE    ///< Device impersonation attack with spoofed messages
};

// Forward declaration
class Sensor;

/**
 * \class Attack_Controller
 * \brief Controller for simulating NMEA2000 network attacks.
 *
 * The Attack_Controller class manages attack simulations on NMEA2000 networks
 * for maritime cybersecurity research. It provides two main attack modes:
 *
 * 1. **Spam Attack (DOS)**: Floods the network with ISO Address Claims to
 *    demonstrate denial-of-service vulnerabilities in the address claiming
 *    protocol.
 *
 * 2. **Impersonate Attack**: Spoofs messages from target devices with
 *    manipulated field values to demonstrate message authenticity vulnerabilities.
 */
class Attack_Controller {
private:
    tNMEA2000_Teensyx* nmea2000_can1;  // CAN interface for sending attack messages
    N2K_Monitor* monitor;              // Reference to network monitor for device/PGN data
    Sensor* valueSensor;               //Reference to sensor for potentiometer value reading

    // Spam Attack State
    bool spamAttackActive;             // Flag indicating if spam attack is currently running
    unsigned long spamMessageCount;    // Counter for total spam messages sent
    unsigned long lastSpamTime;        // Timestamp of last spam message transmission
    uint8_t currentSpamAddress;        // Current address being claimed (0-254)

    // ISO Address Takeover tracking
    std::vector<uint8_t> claimedAddresses;  // List of addresses successfully claimed
    uint64_t attackerName;                   // High-priority NAME used for address claims



    /**
     * \brief Constructs a high-priority NAME for address claiming.
     * \return 64-bit NAME value with maximum priority settings
     *
     * Builds an NMEA2000 NAME with the lowest possible values in priority
     * fields to win address claim arbitration against legitimate devices.
     */
    uint64_t buildHighPriorityName();

    /**
     * \brief Checks if a source address belongs to this device.
     * \param source The source address to check
     * \return true if the source is owned by this device, false otherwise
     */
    bool isOwnSource(uint8_t source);

    // Impersonate Attack State
    bool impersonateActive;            // Flag indicating if impersonate attack is running
    uint8_t impTargetAddress;          // Target device's source address being impersonated
    uint32_t impTargetPGN;             // Target PGN number to spoof
    int impSelectedFieldIndex;         // Currently selected field index for manipulation
    float impFieldValue;               // Current field value (from sensor or locked)
    unsigned long lastImpTime;         // Timestamp of last impersonate transmission
    float impFieldMin;                 // Minimum value for selected field
    float impFieldMax;                 // Maximum value for selected field
    std::vector<uint32_t> impPGNList;  // List of PGNs available for selected device

    // Per-field locking for impersonate attack
    bool impFieldLocked[MAX_IMP_FIELDS];        // Lock state per field (MAX_IMP_FIELDS from constants.h)
    float impFieldLockedValues[MAX_IMP_FIELDS]; // Locked value storage per field

    // Own-sensor impersonation tracking
    bool impersonatingOwnSensor;       // True if impersonating device's own sensor (Sensor 1/2/3)
    uint8_t impOwnSensorIndex;         // Index of own sensor being impersonated (0, 1, or 2)

    /**
     * \brief Builds a spoofed NMEA2000 message with manipulated field value.
     * \param N2kMsg Reference to the message object to populate
     * \param pgn The PGN number of the message to spoof
     * \param fieldIndex Index of the field to manipulate
     * \param value The spoofed value to insert into the field
     *
     * Constructs a complete NMEA2000 message with the specified PGN,
     * inserting the manipulated value at the designated field position.
     */
    void buildSpoofedMessage(tN2kMsg &N2kMsg, uint32_t pgn, int fieldIndex, float value);

public:
    /**
     * \brief Constructs an Attack_Controller instance.
     * \param can1 Pointer to the NMEA2000 CAN interface for message transmission
     * \param mon Pointer to the network monitor for device discovery data
     * \param sensor Pointer to the sensor for reading potentiometer values
     *
     * Initializes the attack controller with references to required system
     * components. The CAN interface is used for transmitting attack messages,
     * the monitor provides discovered device information, and the sensor
     * provides real-time value input for impersonation attacks.
     */
    Attack_Controller(tNMEA2000_Teensyx* can1, N2K_Monitor* mon, Sensor* sensor);

    /**
     * \brief Main update loop for attack processing.
     *
     * Should be called from the main loop to process active attacks.
     * Handles timing and state updates for all attack types.
     */
    void update();

    /**
     * \brief Checks if any attack is currently active.
     * \return true if spam or impersonate attack is running, false otherwise
     */
    bool isAttackActive();

    
    // Spam Attack Methods
    

    /**
     * \brief Starts the spam (DOS) attack.
     *
     * Initiates a denial-of-service attack by flooding the network with
     * ISO Address Claims. The attack cycles through addresses 0-254,
     * attempting to claim each one with a high-priority NAME.
     */
    void startSpamAttack();

    /**
     * \brief Stops the spam attack.
     *
     * Halts the ongoing spam attack and resets attack state variables.
     */
    void stopSpamAttack();

    /**
     * \brief Handles incoming ISO Address Claims during spam attack.
     * \param N2kMsg The received NMEA2000 message to process
     *
     * Responds to ISO Address Claims (PGN 60928) from other devices
     * by sending competing claims with higher priority.
     */
    void attackHandler(const tN2kMsg &N2kMsg);

    /**
     * \brief Checks if spam attack is currently active.
     * \return true if spam attack is running, false otherwise
     */
    bool isSpamActive() { return spamAttackActive; }

    /**
     * \brief Gets the total count of spam messages sent.
     * \return Number of spam messages transmitted
     */
    unsigned long getSpamMessageCount() { return spamMessageCount; }

    /**
     * \brief Gets the current address being claimed in spam attack.
     * \return Current target address (0-254)
     */
    uint8_t getCurrentSpamAddress() { return currentSpamAddress; }
    
    /**
     * \brief Sends a high-priority ISO Address Claim for the target address.
     * \param targetAddress The NMEA2000 source address to claim (0-254)
     *
     * Transmits an ISO Address Claim (PGN 60928) with a high-priority NAME
     * to take over the specified address from its current owner.
     */
    void sendHighPriorityAddressClaim(uint8_t targetAddress);

    
    // Impersonate Attack Methods
    

    /**
     * \brief Starts an impersonation attack on a target device.
     * \param targetAddress The source address of the device to impersonate
     * \param targetPGN The PGN to spoof from the target device
     *
     * Initiates an impersonation attack where spoofed messages appear to
     * originate from the target device. Field values can be manipulated
     * using the potentiometer or locked to specific values.
     */
    void startImpersonate(uint8_t targetAddress, uint32_t targetPGN);

    /**
     * \brief Stops the impersonation attack.
     *
     * Halts the ongoing impersonation attack and resets state variables.
     */
    void stopImpersonate();

    /**
     * \brief Updates impersonation attack state and sends spoofed messages.
     *
     * Called internally by update() to handle impersonation timing
     * and message transmission.
     */
    void updateImpersonate();

    /**
     * \brief Checks if impersonation attack is currently active.
     * \return true if impersonation attack is running, false otherwise
     */
    bool isImpersonateActive() { return impersonateActive; }

    /**
     * \brief Gets the target device address being impersonated.
     * \return Target device source address
     */
    uint8_t getImpTargetAddress() { return impTargetAddress; }

    /**
     * \brief Gets the target PGN being spoofed.
     * \return Target PGN number
     */
    uint32_t getImpTargetPGN() { return impTargetPGN; }

    /**
     * \brief Gets the currently selected field index for manipulation.
     * \return Selected field index
     */
    int getImpSelectedFieldIndex() { return impSelectedFieldIndex; }

    /**
     * \brief Sets the field index for value manipulation.
     * \param index The field index to select
     *
     * Selects which field of the spoofed message will have its value
     * controlled by the potentiometer or locked value.
     */
    void setImpSelectedFieldIndex(int index);

    /**
     * \brief Gets the current field value being spoofed.
     * \return Current manipulated field value
     */
    float getImpFieldValue() { return impFieldValue; }

    /**
     * \brief Gets the minimum value for the selected field.
     * \return Minimum allowable field value
     */
    float getImpFieldMin() { return impFieldMin; }

    /**
     * \brief Gets the maximum value for the selected field.
     * \return Maximum allowable field value
     */
    float getImpFieldMax() { return impFieldMax; }

    
    // Field Locking Methods
    

    /**
     * \brief Toggles the lock state for the currently selected field.
     *
     * When locked, the field value is frozen at its current value.
     * When unlocked, the value is controlled by the potentiometer.
     */
    void toggleValueLock();

    /**
     * \brief Checks if a specific field is locked.
     * \param index The field index to check
     * \return true if the field is locked, false otherwise
     */
    bool isFieldLocked(int index) { return index < MAX_IMP_FIELDS && impFieldLocked[index]; }

    /**
     * \brief Gets the locked value for a specific field.
     * \param index The field index to query
     * \return The locked value, or 0.0f if index is out of range
     */
    float getLockedValue(int index) { return index < MAX_IMP_FIELDS ? impFieldLockedValues[index] : 0.0f; }

    
    // Attack Status Methods
    

    /**
     * \brief Gets the currently active attack type.
     * \return AttackType enum value indicating active attack
     */
    AttackType getActiveAttackType();

    /**
     * \brief Checks if impersonating this device's own sensor.
     * \return true if impersonating Sensor 1, 2, or 3
     */
    bool isImpersonatingOwnSensor() { return impersonatingOwnSensor; }

    /**
     * \brief Gets the index of the own sensor being impersonated.
     * \return Sensor index (0, 1, or 2)
     */
    uint8_t getImpOwnSensorIndex() { return impOwnSensorIndex; }

    /**
     * \brief Sets own-sensor impersonation mode.
     * \param own true to enable own-sensor impersonation
     * \param sensorIndex Index of the sensor to impersonate (0, 1, or 2)
     */
    void setImpersonatingOwnSensor(bool own, uint8_t sensorIndex = 0);

    /**
     * \brief Gets a human-readable string describing the current attack.
     * \return Status string (e.g., "DOS Attack" or "Imp: DeviceName")
     */
    String getAttackStatusString();

    
    // Field Range and PGN Utility Methods
    

    /**
     * \brief Gets the valid value range for a field in a PGN.
     * \param pgn The PGN number to query
     * \param fieldIndex The field index within the PGN
     * \param min Reference to store the minimum value
     * \param max Reference to store the maximum value
     *
     * Retrieves the minimum and maximum valid values for the specified
     * field, used to constrain potentiometer input to realistic ranges.
     */
    void getFieldRange(uint32_t pgn, int fieldIndex, float &min, float &max);

    /**
     * \brief Gets the list of editable field names for a PGN.
     * \param pgn The PGN number to query
     * \return Vector of field names that can be manipulated
     *
     * Returns only fields that can be meaningfully spoofed,
     * excluding read-only or system fields.
     */
    std::vector<String> getEditableFieldNames(uint32_t pgn);

    /**
     * \brief Gets the count of editable fields for a PGN.
     * \param pgn The PGN number to query
     * \return Number of fields that can be manipulated
     */
    int getEditableFieldCount(uint32_t pgn);

    /**
     * \brief Builds the list of impersonatable PGNs for a device.
     * \param deviceAddress The source address of the target device
     * \return Reference to the internal PGN list
     *
     * Populates the internal PGN list with PGNs that can be spoofed
     * from the specified device, filtered for impersonation suitability.
     */
    std::vector<uint32_t>& buildImpPGNList(uint8_t deviceAddress);

    /**
     * \brief Gets the current impersonation PGN list.
     * \return Reference to the internal PGN list
     */
    std::vector<uint32_t>& getImpPGNList() { return impPGNList; }

    /**
     * \brief Gets the count of impersonatable PGNs for a device.
     * \param deviceAddress The source address of the target device
     * \return Number of PGNs that can be impersonated
     *
     * Returns the count without modifying the internal PGN list.
     */
    int getImpersonatablePGNCount(uint8_t deviceAddress);
};

#endif // ATTACK_CONTROLLER_H
