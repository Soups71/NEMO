/**
 * @file Menu_Attacks.cpp
 * @brief Attack display functions for the Menu_Controller class.
 *
 * This file implements all display functions related to the attack features
 * of the NEMO device, including DOS/spam attack configuration and status,
 * impersonation attack device/PGN/field selection, and the attack status
 * overview screen.
 */

#include "Menu_Controller.h"

/**
 * @brief Checks if any attack is currently active.
 *
 * Delegates to the Attack_Controller to determine if either
 * the DOS/spam attack or impersonation attack is running.
 *
 * @return true if any attack is active, false otherwise
 */
bool Menu_Controller::isAttackActive() {
    return attackController->isAttackActive();
}

/**
 * @brief Displays the DOS/spam attack configuration screen.
 *
 * Shows a simple screen prompting the user to start the DOS attack.
 * The attack will claim all known device addresses when started.
 *
 * Display format:
 * - Row 0: Title "DOS Attack"
 * - Row 5: "SELECT to start"
 * - Row 7: "< BACK"
 */
void Menu_Controller::displaySpamConfig() {
    prepScreen();

    screen->drawString(0, 0, "DOS Attack");
    screen->drawString(0, 5, "SELECT to start");
    screen->drawString(0, 7, "< BACK");
}

/**
 * @brief Displays the active DOS/spam attack status screen.
 *
 * Shows real-time status of the running DOS attack including
 * the message count. Initializes the displayedLines cache for
 * efficient partial updates.
 *
 * Display format:
 * - Row 0: Title "DOS ATTACK"
 * - Row 3: Message count "Msgs: [count]"
 * - Row 7: "< BACK"
 *
 * Sets spamActiveInitialized flag after first complete draw.
 */
void Menu_Controller::displaySpamActive() {
    prepScreen();

    // Clear displayedLines cache since we're doing a full redraw
    for (int i = 0; i < 8; i++) {
        displayedLines[i] = "";
    }

    screen->drawString(0, 0, "DOS ATTACK");
    displayedLines[0] = "DOS ATTACK";

    displayedLines[2] = "";

    // Message count (dynamic)
    char line2[17];
    snprintf(line2, sizeof(line2), "Msgs: %-9lu", attackController->getSpamMessageCount());
    screen->drawString(0, 3, line2);
    displayedLines[3] = line2;

    screen->drawString(0, 7, "< BACK");
    displayedLines[7] = "< BACK ";

    spamActiveInitialized = true;
}

/**
 * @brief Updates only the message count on the spam attack screen.
 *
 * Performs an efficient partial update of the spam attack status,
 * only redrawing the message count line. Uses drawLine() for
 * change detection to avoid unnecessary screen writes.
 */
void Menu_Controller::updateSpamActiveValue() {
    // Only update the message count (row 3)
    char line2[17];
    snprintf(line2, sizeof(line2), "Msgs: %-9lu", attackController->getSpamMessageCount());
    drawLine(3, line2);
}

/**
 * @brief Displays the device selection screen for impersonation attack.
 *
 * Shows a filtered list of devices that have PGNs which can be
 * impersonated. Own sensors (Sensor 1, 2, 3) are marked with "[OWN]"
 * suffix, while external devices show their impersonatable PGN count.
 *
 * Display format:
 * - Row 0: Title "SELECT DEVICE"
 * - Rows 1-6: Device entries with ">" indicator for selection
 *   - Own sensors: "[sensor name][OWN]"
 *   - External: "[device name]    [N]" where N is PGN count
 * - Row 7: "< BACK"
 *
 * Shows "No devices with supported PGNs found" if no valid targets.
 */
void Menu_Controller::displayImpDeviceSelect() {
    prepScreen();

    screen->drawString(0, 0, "SELECT DEVICE");

    std::vector<uint8_t>& fullDeviceList = monitor->getDeviceList();
    std::map<uint8_t, DeviceInfo>& devices = monitor->getDevices();

    // Build filtered list of devices with impersonatable PGNs
    // Now includes own sensors (Sensor 1, 2, 3) - they will be marked with [OWN]
    impDeviceList.clear();
    for (uint8_t addr : fullDeviceList) {
        if (attackController->getImpersonatablePGNCount(addr) > 0) {
            impDeviceList.push_back(addr);
        }
    }

    if (impDeviceList.empty()) {
        screen->drawString(0, 2, "No devices");
        screen->drawString(0, 3, "with supported");
        screen->drawString(0, 4, "PGNs found");
        screen->drawString(0, 7, "< BACK");
        return;
    }

    // Clamp scroll index to valid range
    if (impDeviceScrollIndex >= (int)impDeviceList.size()) {
        impDeviceScrollIndex = impDeviceList.size() - 1;
    }

    // Display up to 6 devices (rows 1-6), row 7 is for back instruction
    int startIdx = 0;
    if (impDeviceScrollIndex > 5) {
        startIdx = impDeviceScrollIndex - 5;
    }

    int row = 1;
    for (int i = startIdx; i < (int)impDeviceList.size() && row < 7; i++) {
        uint8_t addr = impDeviceList[i];
        DeviceInfo& dev = devices[addr];

        char line[17];
        // Show selection indicator and device info
        char indicator = (i == impDeviceScrollIndex) ? '>' : ' ';

        // Check if this is one of our own sensors
        String devName = dev.name;
        bool isOwnSensor = (devName == "Sensor 1" || devName == "Sensor 2" || devName == "Sensor 3");

        if (isOwnSensor) {
            // Show with [OWN] marker - truncate name more to fit
            if (devName.length() > 7) {
                devName = devName.substring(0, 7);
            }
            snprintf(line, sizeof(line), "%c%s[OWN]", indicator, devName.c_str());
        } else {
            // Normal device - truncate to fit
            if (devName.length() > 10) {
                devName = devName.substring(0, 10);
            }
            snprintf(line, sizeof(line), "%c%s", indicator, devName.c_str());

            // Show impersonatable PGN count on the right for non-own devices
            char pgnCount[5];
            int impCount = attackController->getImpersonatablePGNCount(addr);
            snprintf(pgnCount, sizeof(pgnCount), "[%d]", impCount);
            screen->drawString(12, row, pgnCount);
        }
        screen->drawString(0, row, line);

        row++;
    }

    screen->drawString(0, 7, "< BACK");
}

/**
 * @brief Displays the PGN selection screen for impersonation attack.
 *
 * Shows the list of impersonatable PGNs for the selected target device.
 * The PGN list is built by the Attack_Controller when a device is selected.
 *
 * Display format:
 * - Row 0: Title "PGNs Dev:[address]"
 * - Rows 1-6: PGN name entries with ">" indicator for selection
 * - Row 7: "< BACK"
 *
 * Shows "No supported PGNs found" if device has no impersonatable PGNs.
 */
void Menu_Controller::displayImpPGNSelect() {
    prepScreen();

    // Use filtered impDeviceList to get target address
    uint8_t impTargetAddress = impDeviceList[impDeviceScrollIndex];

    char title[17];
    snprintf(title, sizeof(title), "PGNs Dev:%d", impTargetAddress);
    screen->drawString(0, 0, title);

    // Get PGN list from attack controller (built when device was selected)
    std::vector<uint32_t>& impPGNList = attackController->getImpPGNList();

    if (impPGNList.empty()) {
        screen->drawString(0, 2, "No supported");
        screen->drawString(0, 3, "PGNs found");
        screen->drawString(0, 7, "< BACK");
        return;
    }

    // Display up to 6 PGNs
    int startIdx = 0;
    if (impPGNScrollIndex > 5) {
        startIdx = impPGNScrollIndex - 5;
    }

    int row = 1;
    for (int i = startIdx; i < (int)impPGNList.size() && row < 7; i++) {
        uint32_t pgn = impPGNList[i];
        char line[17];
        char indicator = (i == impPGNScrollIndex) ? '>' : ' ';

        // Get PGN name (truncated)
        String pgnName = monitor->getPGNName(pgn);
        if (pgnName.length() > 14) {
            pgnName = pgnName.substring(0, 14);
        }
        snprintf(line, sizeof(line), "%c%s", indicator, pgnName.c_str());
        screen->drawString(0, row, line);

        row++;
    }

    screen->drawString(0, 7, "< BACK");
}

/**
 * @brief Displays the field selection and value control screen for impersonation.
 *
 * Shows the currently selected field for the active impersonation attack,
 * including the current value, valid range, and lock status. Users can
 * navigate between fields and toggle value locking.
 *
 * Display format:
 * - Row 0: Header "[addr] [pgn]"
 * - Row 2: Field name with ">" indicator
 * - Row 3: Current value "Val: [value]"
 * - Row 4: Valid range "[min-max]"
 * - Row 5: Lock status "LOCKED" (inverse) or "SEL=Lock"
 * - Row 6: Field navigation "Field N/M"
 * - Row 7: "< BACK"
 *
 * Sets impFieldSelectInitialized flag after first complete draw.
 */
void Menu_Controller::displayImpFieldSelect() {
    prepScreen();

    // Clear displayedLines cache since we're doing a full redraw
    for (int i = 0; i < 8; i++) {
        displayedLines[i] = "";
    }

    uint8_t impTargetAddress = attackController->getImpTargetAddress();
    uint32_t impTargetPGN = attackController->getImpTargetPGN();
    int impSelectedFieldIndex = attackController->getImpSelectedFieldIndex();

    // Header with target info
    char header[17];
    snprintf(header, sizeof(header), "D:%d P:%lu", impTargetAddress, (unsigned long)impTargetPGN);
    screen->drawString(0, 0, header);

    // Get editable field names for this PGN (only fields that can be spoofed)
    std::vector<String> editableFields = attackController->getEditableFieldNames(impTargetPGN);
    int numFields = editableFields.size();

    if (numFields == 0) {
        screen->drawString(0, 2, "No fields");
        screen->drawString(0, 7, "< BACK");
        return;
    }

    // Display selected field name (row 2)
    if (impSelectedFieldIndex < numFields) {
        String fieldName = editableFields[impSelectedFieldIndex];
        if (fieldName.length() > 15) {
            fieldName = fieldName.substring(0, 15);
        }

        char fieldLine[17];
        snprintf(fieldLine, sizeof(fieldLine), ">%s", fieldName.c_str());
        screen->drawString(0, 2, fieldLine);
    }

    // Display current value (row 3) - track in cache for efficient updates
    char valueLine[17];
    snprintf(valueLine, sizeof(valueLine), "Val: %-9.1f", attackController->getImpFieldValue());
    screen->drawString(0, 3, valueLine);
    displayedLines[3] = valueLine;

    // Display range (row 4)
    char rangeLine[17];
    snprintf(rangeLine, sizeof(rangeLine), "[%.0f-%.0f]", attackController->getImpFieldMin(), attackController->getImpFieldMax());
    screen->drawString(0, 4, rangeLine);

    // Lock status (row 5) - track in cache for efficient updates
    // Check if current field is locked
    bool currentFieldLocked = attackController->isFieldLocked(impSelectedFieldIndex);
    if (currentFieldLocked) {
        screen->setInverseFont(1);
        screen->drawString(0, 5, " LOCKED         ");
        screen->setInverseFont(0);
        displayedLines[5] = " LOCKED ";
    } else {
        screen->drawString(0, 5, "SEL=Lock        ");
        displayedLines[5] = "SEL=Lock        ";
    }

    // Field navigation hint (row 6)
    char navHint[17];
    snprintf(navHint, sizeof(navHint), "Field %d/%d", impSelectedFieldIndex + 1, numFields);
    screen->drawString(0, 6, navHint);

    screen->drawString(0, 7, "< BACK");
}

/**
 * @brief Updates only the dynamic values on the impersonate field screen.
 *
 * Performs an efficient partial update of the field selection screen,
 * only redrawing the value and lock status lines. Uses drawLine() for
 * change detection to avoid unnecessary screen writes.
 */
void Menu_Controller::updateImpFieldSelectValue() {
    // Only update the value line (row 3) and lock status (row 5)
    // Uses drawLine to avoid redrawing if unchanged

    int impSelectedFieldIndex = attackController->getImpSelectedFieldIndex();

    // Update value (row 3)
    char valueLine[17];
    snprintf(valueLine, sizeof(valueLine), "Val: %-9.1f", attackController->getImpFieldValue());
    drawLine(3, valueLine);

    // Update lock status (row 5) - check per-field lock state
    bool currentFieldLocked = attackController->isFieldLocked(impSelectedFieldIndex);
    if (currentFieldLocked) {
        // Draw locked status with inverse
        if (displayedLines[5] != " LOCKED ") {
            displayedLines[5] = " LOCKED ";
            screen->setInverseFont(1);
            screen->drawString(0, 5, " LOCKED         ");
            screen->setInverseFont(0);
        }
    } else {
        drawLine(5, "SEL=Lock");
    }
}

/**
 * @brief Displays the attack status overview screen.
 *
 * Shows information about the currently active attack, accessible
 * from the Attacks menu when an attack is running. Displays different
 * information based on attack type (DOS vs impersonate).
 *
 * Display format for DOS attack:
 * - Row 0: "ATTACK ACTIVE"
 * - Row 2: "Type: DOS Attack"
 * - Row 3: Message count "Msgs: [count]"
 * - Row 7: "SELECT = STOP" (inverse)
 *
 * Display format for impersonate attack:
 * - Row 0: "ATTACK ACTIVE"
 * - Row 2: "Type: Impersonate"
 * - Row 3: "Target: [device name]"
 * - Row 4: "PGN: [pgn name]"
 * - Row 5: "[OWN SENSOR]" (if impersonating own sensor)
 * - Row 7: "SELECT = STOP" (inverse)
 *
 * Sets attackStatusInitialized flag after first complete draw.
 */
void Menu_Controller::displayAttackStatus() {
    prepScreen();

    // Clear displayedLines cache since we're doing a full redraw
    for (int i = 0; i < 8; i++) {
        displayedLines[i] = "";
    }

    // Reset scroll offset when first displaying
    attackStatusScrollOffset = 0;

    screen->drawString(0, 0, "ATTACK ACTIVE");
    displayedLines[0] = "ATTACK ACTIVE";

    AttackType attackType = attackController->getActiveAttackType();

    if (attackType == ATTACK_SPAM) {
        screen->drawString(0, 2, "Type: DOS Attack");
        displayedLines[2] = "DOS Attack";  // Store type name for consistency

        char line[17];
        snprintf(line, sizeof(line), "Msgs: %lu", attackController->getSpamMessageCount());
        screen->drawString(0, 3, line);
        displayedLines[3] = line;
    } else if (attackType == ATTACK_IMPERSONATE) {
        // "Type: Impersonate" is 17 chars, too long for 16-char display
        // Store full type name for scrolling, display truncated initially
        String typeName = "Impersonate";
        displayedLines[2] = typeName;  // Store for scrolling

        int typeAvailWidth = 16 - 6;  // "Type: " = 6 chars
        String displayType = typeName.substring(0, typeAvailWidth);
        char typeLine[17];
        snprintf(typeLine, sizeof(typeLine), "Type: %s", displayType.c_str());
        screen->drawString(0, 2, typeLine);

        // Show target device (may need scrolling)
        uint8_t targetAddr = attackController->getImpTargetAddress();
        DeviceInfo* device = monitor->getDevice(targetAddr);
        char line[17];
        String targetLabel = "Target: ";
        String devName = device ? device->name : ("Addr " + String(targetAddr));
        int availableWidth = 16 - targetLabel.length();

        // Display initial view (scrolling handled in update)
        String displayName = devName.substring(0, availableWidth);
        snprintf(line, sizeof(line), "%s%s", targetLabel.c_str(), displayName.c_str());
        screen->drawString(0, 3, line);
        displayedLines[3] = devName;  // Store full name for scrolling

        // Show target PGN (may need scrolling)
        uint32_t targetPGN = attackController->getImpTargetPGN();
        String pgnName = monitor->getPGNName(targetPGN);
        String pgnLabel = "PGN: ";
        availableWidth = 16 - pgnLabel.length();

        String displayPGN = pgnName.substring(0, availableWidth);
        snprintf(line, sizeof(line), "%s%s", pgnLabel.c_str(), displayPGN.c_str());
        screen->drawString(0, 4, line);
        displayedLines[4] = pgnName;  // Store full name for scrolling

        // Show if impersonating own sensor
        if (attackController->isImpersonatingOwnSensor()) {
            screen->drawString(0, 5, "[OWN SENSOR]");
            displayedLines[5] = "[OWN SENSOR]";
        }
    }

    // Bottom row - stop instruction
    screen->setInverseFont(1);
    screen->drawString(0, 7, "SELECT = STOP   ");
    screen->setInverseFont(0);
    displayedLines[7] = "SELECT = STOP   ";

    attackStatusInitialized = true;
}

/**
 * @brief Updates the attack status display with scrolling text and live values.
 *
 * Handles periodic updates for the attack status screen:
 * - For DOS attack: Updates the message count
 * - For impersonate attack: Handles horizontal scrolling for long type name,
 *   device name, and PGN name
 *
 * Scrolling is synchronized across all fields that need it, with wrap-around
 * when the end is reached.
 */
void Menu_Controller::updateAttackStatusDisplay() {
    AttackType attackType = attackController->getActiveAttackType();

    if (attackType == ATTACK_SPAM) {
        // Update message count
        char line[17];
        snprintf(line, sizeof(line), "Msgs: %lu", attackController->getSpamMessageCount());
        drawLine(3, line);
    } else if (attackType == ATTACK_IMPERSONATE) {
        // Handle scrolling for long type name, device name and PGN name
        unsigned long currentTime = millis();

        if (currentTime - lastAttackStatusScrollUpdate > SCROLL_DELAY_MS) {
            lastAttackStatusScrollUpdate = currentTime;

            // Check if any text needs scrolling
            String typeName = displayedLines[2];  // Full type name stored here
            String devName = displayedLines[3];   // Full device name stored here
            String pgnName = displayedLines[4];   // Full PGN name stored here

            int typeAvailWidth = 16 - 6;    // "Type: " = 6 chars
            int targetAvailWidth = 16 - 8;  // "Target: " = 8 chars
            int pgnAvailWidth = 16 - 5;     // "PGN: " = 5 chars

            bool needsScroll = (typeName.length() > (unsigned int)typeAvailWidth) ||
                              (devName.length() > (unsigned int)targetAvailWidth) ||
                              (pgnName.length() > (unsigned int)pgnAvailWidth);

            if (needsScroll) {
                // Calculate max scroll needed
                int maxScroll = 0;
                if (typeName.length() > (unsigned int)typeAvailWidth) {
                    int typeScroll = typeName.length() - typeAvailWidth + 3;  // +3 for wrap padding
                    if (typeScroll > maxScroll) maxScroll = typeScroll;
                }
                if (devName.length() > (unsigned int)targetAvailWidth) {
                    int devScroll = devName.length() - targetAvailWidth + 3;
                    if (devScroll > maxScroll) maxScroll = devScroll;
                }
                if (pgnName.length() > (unsigned int)pgnAvailWidth) {
                    int pgnScroll = pgnName.length() - pgnAvailWidth + 3;
                    if (pgnScroll > maxScroll) maxScroll = pgnScroll;
                }

                // Update scroll offset with wrap-around
                attackStatusScrollOffset++;
                if (attackStatusScrollOffset > maxScroll) {
                    attackStatusScrollOffset = 0;
                }

                // Update type row if it needs scrolling
                if (typeName.length() > (unsigned int)typeAvailWidth) {
                    String scrollText = typeName + "   " + typeName;
                    String visible = scrollText.substring(attackStatusScrollOffset, attackStatusScrollOffset + typeAvailWidth);
                    char line[17];
                    snprintf(line, sizeof(line), "Type: %s", visible.c_str());
                    screen->drawString(0, 2, line);
                }

                // Update device name row if it needs scrolling
                if (devName.length() > (unsigned int)targetAvailWidth) {
                    String scrollText = devName + "   " + devName;
                    String visible = scrollText.substring(attackStatusScrollOffset, attackStatusScrollOffset + targetAvailWidth);
                    char line[17];
                    snprintf(line, sizeof(line), "Target: %s", visible.c_str());
                    screen->drawString(0, 3, line);
                }

                // Update PGN name row if it needs scrolling
                if (pgnName.length() > (unsigned int)pgnAvailWidth) {
                    String scrollText = pgnName + "   " + pgnName;
                    String visible = scrollText.substring(attackStatusScrollOffset, attackStatusScrollOffset + pgnAvailWidth);
                    char line[17];
                    snprintf(line, sizeof(line), "PGN: %s", visible.c_str());
                    screen->drawString(0, 4, line);
                }
            }
        }
    }
}
