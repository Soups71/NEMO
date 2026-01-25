/**
 * @file Menu_Callbacks.cpp
 * @brief Static callback functions and sensor configuration for Menu_Controller.
 *
 * This file implements all static callback functions used by the menu system
 * to handle menu option selections. It also contains sensor configuration
 * functions for setting PGN types, manufacturer codes, and active states.
 *
 * Callback functions follow a naming convention of callback_[MenuOption] and
 * are registered in the menu initialization. They use the static instance
 * pointer to access Menu_Controller member functions.
 */

#include "Menu_Controller.h"

// =============================================================================
// STATIC CALLBACK IMPLEMENTATIONS
// =============================================================================

/**
 * @brief Callback for "Live Data" menu option.
 *
 * Navigates to the device list view which shows all detected NMEA2000
 * devices on the network. This replaced the legacy PGN list view with
 * a more intuitive device-centric navigation.
 */
void Menu_Controller::callback_SensorReadings() {
    // Go to device list instead of old PGN list
    if(instance) instance->changeMenu(MENU_DEVICE_LIST);
}

/**
 * @brief Callback for "Configure" menu option.
 *
 * Navigates to the configuration menu which provides access to
 * sensor configuration, device settings, and other options.
 */
void Menu_Controller::callback_Configure() {
    if(instance) instance->changeMenu(MENU_CONFIGURE);
}

/**
 * @brief Callback for "Attacks" menu option.
 *
 * If an attack is currently active, displays the attack status screen
 * showing details about the running attack. Otherwise, navigates to
 * the attacks menu where DOS and Impersonate attacks can be initiated.
 */
void Menu_Controller::callback_Attacks() {
    if(instance) {
        if(instance->attackController->isAttackActive()) {
            // Show attack status screen instead of attacks menu
            instance->currentMenuID = MENU_ATTACK_STATUS;
            instance->attackStatusInitialized = false;
            instance->inSpecialMode = true;
            instance->displayAttackStatus();
        } else {
            instance->changeMenu(MENU_ATTACKS);
        }
    }
}

/**
 * @brief Callback for "About" menu option.
 *
 * Displays the about information screen showing project details,
 * version information, and GitHub repository link.
 */
void Menu_Controller::callback_About() {
    if(instance) {
        instance->currentMenuID = MENU_ABOUT_INFO;
        instance->displayAboutInfo();
    }
}

/**
 * @brief Callback for "Sensor 1" configuration option.
 *
 * Navigates to the Sensor 1 configuration menu where manufacturer,
 * device type (PGN), and active state can be configured.
 */
void Menu_Controller::callback_ConfigSensor1() {
    if(instance) instance->changeMenu(MENU_CONFIGURE_SENSOR1);
}

/**
 * @brief Callback for "Sensor 2" configuration option.
 *
 * Navigates to the Sensor 2 configuration menu where manufacturer,
 * device type (PGN), and active state can be configured.
 */
void Menu_Controller::callback_ConfigSensor2() {
    if(instance) instance->changeMenu(MENU_CONFIGURE_SENSOR2);
}

/**
 * @brief Callback for "Sensor 3" configuration option.
 *
 * Navigates to the Sensor 3 configuration menu where manufacturer,
 * device type (PGN), and active state can be configured.
 */
void Menu_Controller::callback_ConfigSensor3() {
    if(instance) instance->changeMenu(MENU_CONFIGURE_SENSOR3);
}

/**
 * @brief Callback for Sensor 1 "Device Type" option.
 *
 * Navigates to the PGN type selection menu for Sensor 1, allowing
 * the user to choose which type of NMEA2000 message this sensor
 * will transmit (e.g., Engine RPM, Water Depth, Heading, etc.).
 */
void Menu_Controller::callback_Sensor1PGNType() {
    if(instance) instance->changeMenu(MENU_SENSOR1_PGN_TYPE);
}

/**
 * @brief Callback for Sensor 1 "Active" toggle option.
 *
 * Toggles Sensor 1 between active and inactive states. When active,
 * the sensor transmits NMEA2000 messages on the CAN bus. When inactive,
 * the sensor releases its network address and stops transmitting.
 */
void Menu_Controller::callback_Sensor1Active() {
    if(instance) instance->toggleSensorActive(0);
}

/**
 * @brief Callback for Sensor 2 "Device Type" option.
 *
 * Navigates to the PGN type selection menu for Sensor 2, allowing
 * the user to choose which type of NMEA2000 message this sensor
 * will transmit.
 */
void Menu_Controller::callback_Sensor2PGNType() {
    if(instance) instance->changeMenu(MENU_SENSOR2_PGN_TYPE);
}

/**
 * @brief Callback for Sensor 2 "Active" toggle option.
 *
 * Toggles Sensor 2 between active and inactive states.
 */
void Menu_Controller::callback_Sensor2Active() {
    if(instance) instance->toggleSensorActive(1);
}

/**
 * @brief Callback for Sensor 3 "Device Type" option.
 *
 * Navigates to the PGN type selection menu for Sensor 3, allowing
 * the user to choose which type of NMEA2000 message this sensor
 * will transmit.
 */
void Menu_Controller::callback_Sensor3PGNType() {
    if(instance) instance->changeMenu(MENU_SENSOR3_PGN_TYPE);
}

/**
 * @brief Callback for Sensor 3 "Active" toggle option.
 *
 * Toggles Sensor 3 between active and inactive states.
 */
void Menu_Controller::callback_Sensor3Active() {
    if(instance) instance->toggleSensorActive(2);
}

/**
 * @brief Callback for "DOS Attack" menu option.
 *
 * Enters the DOS/spam attack configuration screen. The DOS attack
 * uses ISO Address Claim messages to claim all network addresses,
 * effectively denying service to legitimate devices.
 */
void Menu_Controller::callback_SpamAttack() {
    if(instance) {
        instance->currentMenuID = MENU_SPAM_CONFIG;
        instance->inSpecialMode = true;
        instance->displaySpamConfig();
    }
}

/**
 * @brief Callback for "Impersonate" menu option.
 *
 * Enters the impersonation attack flow, starting with device selection.
 * The user will select a target device, then a PGN to spoof, then
 * control field values using the analog sensor input.
 */
void Menu_Controller::callback_Impersonate() {
    if(instance) {
        instance->impDeviceScrollIndex = 0;
        instance->currentMenuID = MENU_IMP_DEVICE_SELECT;
        instance->inSpecialMode = true;
        instance->displayImpDeviceSelect();
    }
}

/**
 * @brief Callback for "Device Config" menu option.
 *
 * Navigates to the device configuration menu which contains settings
 * that affect device behavior such as stale entry cleanup.
 */
void Menu_Controller::callback_DeviceConfig() {
    if(instance) {
        // Don't call pushMenu - changeMenu already pushes the current menu
        instance->changeMenu(MENU_DEVICE_CONFIG);
    }
}

/**
 * @brief Callback for "Stale Cleanup" toggle option.
 *
 * Navigates to the stale cleanup configuration screen where the user
 * can enable or disable automatic removal of devices and PGNs that
 * haven't been seen recently.
 */
void Menu_Controller::callback_StaleCleanupToggle() {
    if(instance) {
        // Don't push - navigateBack for MENU_STALE_CLEANUP goes directly to MENU_DEVICE_CONFIG
        instance->changeMenu(MENU_STALE_CLEANUP);
    }
}

/**
 * @brief Callback for "Info" option in About menu.
 *
 * Displays the about information screen with project details.
 */
void Menu_Controller::callback_AboutInfo() {
    if(instance) {
        instance->currentMenuID = MENU_ABOUT_INFO;
        instance->displayAboutInfo();
    }
}

/**
 * @brief Callback for "Supported PGNs" option in About menu.
 *
 * Displays a scrollable list of all PGNs that the NEMO device
 * can transmit or impersonate, using IMPERSONATABLE_PGN_DEFS
 * from constants.h.
 */
void Menu_Controller::callback_AboutPGNs() {
    if(instance) {
        instance->aboutPGNScrollIndex = 0;
        instance->currentMenuID = MENU_ABOUT_PGNS;
        instance->displaySupportedPGNs();
    }
}

/**
 * @brief Callback for Sensor 1 "Manufacturer" option.
 *
 * Navigates to the manufacturer selection screen for Sensor 1.
 * The selected manufacturer code will be used in the NMEA2000
 * device NAME field, allowing impersonation of various vendors.
 */
void Menu_Controller::callback_Sensor1Manufacturer() {
    if(instance) {
        instance->currentSensorBeingConfigured = 0;
        instance->changeMenu(MENU_MANUFACTURER_SELECT);
    }
}

/**
 * @brief Callback for Sensor 2 "Manufacturer" option.
 *
 * Navigates to the manufacturer selection screen for Sensor 2.
 */
void Menu_Controller::callback_Sensor2Manufacturer() {
    if(instance) {
        instance->currentSensorBeingConfigured = 1;
        instance->changeMenu(MENU_MANUFACTURER_SELECT);
    }
}

/**
 * @brief Callback for Sensor 3 "Manufacturer" option.
 *
 * Navigates to the manufacturer selection screen for Sensor 3.
 */
void Menu_Controller::callback_Sensor3Manufacturer() {
    if(instance) {
        instance->currentSensorBeingConfigured = 2;
        instance->changeMenu(MENU_MANUFACTURER_SELECT);
    }
}

// =============================================================================
// SENSOR CONFIGURATION FUNCTIONS
// =============================================================================

/**
 * @brief Sets the PGN/message type for a sensor.
 *
 * Changes which type of NMEA2000 message the specified sensor will
 * transmit. After setting the type, updates the device information
 * to reflect the new sensor class and function codes, then navigates
 * back to the sensor configuration menu.
 *
 * @param sensorNum The sensor index (0, 1, or 2)
 * @param pgnType The MessageType enum value for the desired PGN
 */
void Menu_Controller::setSensorPGNType(int sensorNum, int pgnType) {
    Sensor* targetSensor = nullptr;

    switch(sensorNum) {
        case 0: targetSensor = sensor1; break;
        case 1: targetSensor = sensor2; break;
        case 2: targetSensor = sensor3; break;
    }

    if(targetSensor != nullptr) {
        targetSensor->setMessageType((MessageType)pgnType);

        // Update the NMEA2000 device information to match the new sensor type
        // This ensures the device appears correctly on the bus with appropriate
        // device function, class, and product name for the selected message type
        targetSensor->updateDeviceInfo();

        // Go back to sensor config menu
        navigateBack();
    }
}

/**
 * @brief Toggles a sensor between active and inactive states.
 *
 * When toggled to active, the sensor begins transmitting NMEA2000
 * messages on the CAN bus. When toggled to inactive, the sensor
 * stops transmitting and releases its network address.
 *
 * Also updates the menu display text to show the current state
 * ("Active: YES" or "Active: NO").
 *
 * @param sensorNum The sensor index (0, 1, or 2)
 */
void Menu_Controller::toggleSensorActive(int sensorNum) {
    // Toggle the sensor active state
    Sensor* targetSensor = nullptr;
    Menu* sensorMenu = nullptr;

    switch(sensorNum) {
        case 0:
            targetSensor = sensor1;
            sensorMenu = configureSensor1Menu;
            break;
        case 1:
            targetSensor = sensor2;
            sensorMenu = configureSensor2Menu;
            break;
        case 2:
            targetSensor = sensor3;
            sensorMenu = configureSensor3Menu;
            break;
    }

    if(sensorMenu != nullptr && targetSensor != nullptr) {
        // Toggle the text in the menu options and the actual sensor state
        // Active is at index 2 (after Manufacturer and Device Type)
        String currentText = sensorMenu->options[2].name;
        if(currentText.indexOf("YES") != -1) {
            sensorMenu->options[2].name = String("Active: NO");
            targetSensor->setActive(false);
        } else {
            sensorMenu->options[2].name = String("Active: YES");
            targetSensor->setActive(true);
        }

        // Redraw using custom display, not printMenu()
        updateSensorConfigDisplay(sensorNum);
    }
}

/**
 * @brief Sets the manufacturer code for a sensor.
 *
 * Updates the NMEA2000 device NAME field with the specified manufacturer
 * code. This allows the sensor to impersonate devices from different
 * manufacturers (e.g., Garmin, Raymarine, Simrad).
 *
 * @param sensorNum The sensor index (0, 1, or 2)
 * @param code The 11-bit NMEA2000 manufacturer code
 */
void Menu_Controller::setManufacturerCode(int sensorNum, uint16_t code) {
    // Update the specific sensor with the new manufacturer code
    Sensor* targetSensor = nullptr;
    switch(sensorNum) {
        case 0: targetSensor = sensor1; break;
        case 1: targetSensor = sensor2; break;
        case 2: targetSensor = sensor3; break;
    }

    if(targetSensor != nullptr) {
        targetSensor->setManufacturerCode(code);
    }
}

/**
 * @brief Displays the sensor configuration screen with current settings.
 *
 * Renders a custom display showing the sensor's current configuration:
 * - Row 0: Title (e.g., "SENSOR 1")
 * - Row 2: Current manufacturer name
 * - Row 3: Current device type (with horizontal scrolling for long names)
 * - Row 4: Current sensor value (live updated)
 * - Rows 5-7: Menu options (Manufacturer, Device Type, Active)
 *
 * The currently selected menu option is highlighted with inverse font.
 *
 * @param sensorNum The sensor index (0, 1, or 2)
 */
void Menu_Controller::updateSensorConfigDisplay(int sensorNum) {
    // Custom display for sensor configuration showing current settings and menu options
    Sensor* targetSensor = nullptr;
    Menu* targetMenu = nullptr;

    switch(sensorNum) {
        case 0:
            targetSensor = sensor1;
            targetMenu = configureSensor1Menu;
            break;
        case 1:
            targetSensor = sensor2;
            targetMenu = configureSensor2Menu;
            break;
        case 2:
            targetSensor = sensor3;
            targetMenu = configureSensor3Menu;
            break;
    }

    if(targetSensor != nullptr && targetMenu != nullptr) {
        prepScreen();

        // Row 0: Title
        String title = String("SENSOR ") + String(sensorNum + 1);
        int padding = (screen->getCols() - title.length()) / 2;
        screen->drawString(padding, 0, title.c_str());

        // Row 2: Current manufacturer - uses MANUFACTURERS array from constants.h
        uint16_t currentCode = targetSensor->getManufacturerCode();
        const char* mfrNamePtr = getManufacturerName(currentCode);
        String mfrName = mfrNamePtr ? String(mfrNamePtr) : String(currentCode);
        String mfrLine = "Mfr:" + mfrName;
        if(mfrLine.length() > 16) mfrLine = mfrLine.substring(0, 16);
        screen->drawString(0, 2, mfrLine.c_str());

        // Row 3: Show current PGN type with scrolling if needed
        MessageType currentType = targetSensor->getMessageType();
        String typeName = String(getSensorDisplayName((int)currentType));
        screen->drawString(0, 3, "Type:");

        // If type name fits, just display it; otherwise scroll
        if((int)typeName.length() <= SCROLL_VISIBLE_CHARS) {
            screen->drawString(5, 3, typeName.c_str());
        } else {
            // Add padding for smooth scroll wrap-around
            String scrollText = typeName + "   " + typeName;
            String visible = scrollText.substring(typeScrollOffset, typeScrollOffset + SCROLL_VISIBLE_CHARS);
            // Pad to clear any leftover characters
            while((int)visible.length() < SCROLL_VISIBLE_CHARS) visible += " ";
            screen->drawString(5, 3, visible.c_str());
        }

        // Row 4: Current value
        screen->drawString(0, 4, "Value:");
        String valueStr = String(targetSensor->getRawValue());
        if(valueStr.length() > 9) {
            valueStr = valueStr.substring(0, 9);
        }
        screen->drawString(6, 4, valueStr.c_str());

        // Rows 5, 6, 7: Menu options (Manufacturer, Device Type, Active)
        // Option 0 - Manufacturer
        if(targetMenu->curr_option == 0) {
            screen->setInverseFont(1);
        } else {
            screen->setInverseFont(0);
        }
        screen->drawString(0, 5, " * Manufacturer ");

        // Option 1 - Device Type
        if(targetMenu->curr_option == 1) {
            screen->setInverseFont(1);
        } else {
            screen->setInverseFont(0);
        }
        screen->drawString(0, 6, " * Device Type  ");

        // Option 2 - Active toggle - read from menu option text
        if(targetMenu->curr_option == 2) {
            screen->setInverseFont(1);
        } else {
            screen->setInverseFont(0);
        }

        // Display the active state from the menu option
        String activeText = targetMenu->options[2].name;
        if(activeText.length() > 13) {
            activeText = activeText.substring(0, 13);
        }
        String displayLine = String(" * ") + activeText;
        while(displayLine.length() < 16) displayLine += " ";
        screen->drawString(0, 7, displayLine.c_str());

        screen->setInverseFont(0);
    }
}

/**
 * @brief Updates only the sensor value on the configuration screen.
 *
 * Performs an efficient partial update that only redraws the value
 * line (row 4) without redrawing the entire screen. This is called
 * periodically from update() for live value display.
 *
 * @param sensorNum The sensor index (0, 1, or 2)
 */
void Menu_Controller::updateSensorValueOnly(int sensorNum) {
    // Only update the value display, not the whole screen
    Sensor* targetSensor = nullptr;

    switch(sensorNum) {
        case 0: targetSensor = sensor1; break;
        case 1: targetSensor = sensor2; break;
        case 2: targetSensor = sensor3; break;
    }

    if(targetSensor != nullptr) {
        // Clear the value area and redraw (row 4, starting at col 6)
        screen->drawString(6, 4, "          ");  // Clear old value

        String valueStr = String(targetSensor->getRawValue());
        if(valueStr.length() > 9) {
            valueStr = valueStr.substring(0, 9);
        }
        screen->drawString(6, 4, valueStr.c_str());
    }
}
