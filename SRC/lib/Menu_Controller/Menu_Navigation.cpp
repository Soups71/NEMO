/**
 * @file Menu_Navigation.cpp
 * @brief Navigation handlers for the Menu_Controller class.
 *
 * This file implements all navigation functionality for the NEMO menu system,
 * including up/down scrolling, back navigation, item selection, and menu
 * stack management. Navigation behavior is context-sensitive based on the
 * current menu state and active mode.
 */

#include "Menu_Controller.h"

/**
 * @brief Handles upward navigation in the menu system.
 */
void Menu_Controller::navigateUp() {
    // Attack status screen has no up/down navigation
    if(currentMenuID == MENU_ATTACK_STATUS) return;

    // Attack menu navigation
    if(currentMenuID == MENU_SPAM_CONFIG) {
        // Spam device count is no longer configurable - we claim all known devices
        displaySpamConfig();
        return;
    }
    if(currentMenuID == MENU_SPAM_ACTIVE) {
        return;  // No navigation during active attack
    }
    if(currentMenuID == MENU_IMP_DEVICE_SELECT) {
        if(impDeviceScrollIndex > 0) {
            impDeviceScrollIndex--;
            displayImpDeviceSelect();
        }
        return;
    }
    if(currentMenuID == MENU_IMP_PGN_SELECT) {
        if(impPGNScrollIndex > 0) {
            impPGNScrollIndex--;
            displayImpPGNSelect();
        }
        return;
    }
    if(currentMenuID == MENU_ABOUT_INFO) {
        return;  // No scrolling on info page
    }
    if(currentMenuID == MENU_ABOUT_PGNS) {
        if(aboutPGNScrollIndex > 0) {
            aboutPGNScrollIndex--;
            displaySupportedPGNs();
        }
        return;
    }
    if(currentMenuID == MENU_IMP_FIELD_SELECT) {
        // Move to previous field (each field has its own lock state)
        int fieldIdx = attackController->getImpSelectedFieldIndex();
        if(fieldIdx > 0) {
            attackController->setImpSelectedFieldIndex(fieldIdx - 1);
            impFieldSelectInitialized = false;  // Force full redraw for new field
            displayImpFieldSelect();
            impFieldSelectInitialized = true;
        }
        return;
    }

    // Stale cleanup screen has no up/down navigation
    if(currentMenuID == MENU_STALE_CLEANUP) {
        return;
    }

    // Manufacturer selection navigation
    if(currentMenuID == MENU_MANUFACTURER_SELECT) {
        if(selectedManufacturerIndex > 0) {
            selectedManufacturerIndex--;
            displayManufacturerSelect();
        }
        return;
    }

    if(currentMenuID == MENU_DEVICE_LIST) {
        // Device list navigation
        if(selectedDeviceIndex > 0) {
            selectedDeviceIndex--;
            deviceListScrollOffset = 0;  // Reset scroll when selection changes
            displayDeviceList();
        }
    } else if(currentMenuID == MENU_DEVICE_PGNS) {
        // PGN list navigation
        if(selectedPGNIndex > 0) {
            selectedPGNIndex--;
            displayDevicePGNs();
        }
    } else if(currentMenuID == MENU_PGN_DETAIL) {
        // Scroll up in PGN detail view
        if(detailScrollOffset > 0) {
            detailScrollOffset--;
            displayPGNDetail();
        }
    } else if(viewingPGNDetail) {
        // Legacy: Don't navigate when viewing PGN detail
        return;
    } else if(inSpecialMode) {
        // Legacy: In PGN monitoring mode
        std::vector<PGNInfo>& detectedPGNs = monitor->getDetectedPGNs();
        if(!detectedPGNs.empty() && selectedPGNIndex > 0) {
            selectedPGNIndex--;
            displayPGNList();
        }
    } else if(currentMenuID == MENU_CONFIGURE_SENSOR1 ||
              currentMenuID == MENU_CONFIGURE_SENSOR2 ||
              currentMenuID == MENU_CONFIGURE_SENSOR3) {
        // In sensor config menu - use custom navigation with bounds checking
        if(currentMenu != nullptr && currentMenu->curr_option > 0) {
            currentMenu->upOption();
            int sensorNum = (currentMenuID == MENU_CONFIGURE_SENSOR1) ? 0 :
                           (currentMenuID == MENU_CONFIGURE_SENSOR2) ? 1 : 2;
            updateSensorConfigDisplay(sensorNum);
        }
    } else {
        if(currentMenu != nullptr) {
            currentMenu->upOption();
        }
    }
}

/**
 * @brief Handles downward navigation in the menu system.
 */
void Menu_Controller::navigateDown() {
    // Attack status screen has no up/down navigation
    if(currentMenuID == MENU_ATTACK_STATUS) return;

    // Attack menu navigation
    if(currentMenuID == MENU_SPAM_CONFIG) {
        // Spam device count is no longer configurable - we claim all known devices
        displaySpamConfig();
        return;
    }
    if(currentMenuID == MENU_SPAM_ACTIVE) {
        return;  // No navigation during active attack
    }
    if(currentMenuID == MENU_IMP_DEVICE_SELECT) {
        // Use filtered impDeviceList (built in displayImpDeviceSelect)
        if(!impDeviceList.empty() && impDeviceScrollIndex < (int)impDeviceList.size() - 1) {
            impDeviceScrollIndex++;
            displayImpDeviceSelect();
        }
        return;
    }
    if(currentMenuID == MENU_IMP_PGN_SELECT) {
        std::vector<uint32_t>& impPGNList = attackController->getImpPGNList();
        if(!impPGNList.empty() && impPGNScrollIndex < (int)impPGNList.size() - 1) {
            impPGNScrollIndex++;
            displayImpPGNSelect();
        }
        return;
    }
    if(currentMenuID == MENU_ABOUT_INFO) {
        return;  // No scrolling on info page
    }
    if(currentMenuID == MENU_ABOUT_PGNS) {
        // Use IMPERSONATABLE_PGN_COUNT from constants.h
        if(aboutPGNScrollIndex < IMPERSONATABLE_PGN_COUNT - 1) {
            aboutPGNScrollIndex++;
            displaySupportedPGNs();
        }
        return;
    }
    if(currentMenuID == MENU_IMP_FIELD_SELECT) {
        // Move to next field (each field has its own lock state)
        uint32_t impTargetPGN = attackController->getImpTargetPGN();
        int numFields = attackController->getEditableFieldCount(impTargetPGN);
        int fieldIdx = attackController->getImpSelectedFieldIndex();
        if(fieldIdx < numFields - 1) {
            attackController->setImpSelectedFieldIndex(fieldIdx + 1);
            impFieldSelectInitialized = false;  // Force full redraw for new field
            displayImpFieldSelect();
            impFieldSelectInitialized = true;
        }
        return;
    }

    // Stale cleanup screen has no up/down navigation
    if(currentMenuID == MENU_STALE_CLEANUP) {
        return;
    }

    // Manufacturer selection navigation
    if(currentMenuID == MENU_MANUFACTURER_SELECT) {
        if(selectedManufacturerIndex < MANUFACTURER_COUNT - 1) {
            selectedManufacturerIndex++;
            displayManufacturerSelect();
        }
        return;
    }

    if(currentMenuID == MENU_DEVICE_LIST) {
        // Device list navigation
        std::vector<uint8_t>& deviceList = monitor->getDeviceList();
        if(!deviceList.empty() && selectedDeviceIndex < (int)deviceList.size() - 1) {
            selectedDeviceIndex++;
            deviceListScrollOffset = 0;  // Reset scroll when selection changes
            displayDeviceList();
        }
    } else if(currentMenuID == MENU_DEVICE_PGNS) {
        // PGN list navigation
        std::map<uint8_t, DeviceInfo>& devices = monitor->getDevices();
        if(devices.find(currentDeviceAddress) != devices.end()) {
            int pgnCount = devices[currentDeviceAddress].pgns.size();
            if(selectedPGNIndex < pgnCount - 1) {
                selectedPGNIndex++;
                displayDevicePGNs();
            }
        }
    } else if(currentMenuID == MENU_PGN_DETAIL) {
        // Scroll down in PGN detail view
        int fieldCount = getPGNFieldCount();
        int maxRows = 5;
        if(detailScrollOffset < fieldCount - maxRows) {
            detailScrollOffset++;
            displayPGNDetail();
        }
    } else if(viewingPGNDetail) {
        // Legacy: Don't navigate when viewing PGN detail
        return;
    } else if(inSpecialMode) {
        // Legacy: In PGN monitoring mode
        std::vector<PGNInfo>& detectedPGNs = monitor->getDetectedPGNs();
        if(!detectedPGNs.empty() && selectedPGNIndex < (int)detectedPGNs.size() - 1) {
            selectedPGNIndex++;
            displayPGNList();
        }
    } else if(currentMenuID == MENU_CONFIGURE_SENSOR1 ||
              currentMenuID == MENU_CONFIGURE_SENSOR2 ||
              currentMenuID == MENU_CONFIGURE_SENSOR3) {
        // In sensor config menu - use custom navigation with bounds checking
        if(currentMenu != nullptr && currentMenu->num_choices > 0 &&
           currentMenu->curr_option < currentMenu->num_choices - 1) {
            currentMenu->downOption();
            int sensorNum = (currentMenuID == MENU_CONFIGURE_SENSOR1) ? 0 :
                           (currentMenuID == MENU_CONFIGURE_SENSOR2) ? 1 : 2;
            updateSensorConfigDisplay(sensorNum);
        }
    } else {
        if(currentMenu != nullptr) {
            currentMenu->downOption();
        }
    }
}

/**
 * @brief Handles back/left navigation in the menu system.
 */
void Menu_Controller::navigateBack() {
    // Attacks menu back handling - go to main menu
    if(currentMenuID == MENU_ATTACKS) {
        inSpecialMode = false;
        currentMenuID = MENU_MAIN;
        currentMenu = mainMenu;
        screen->clear();
        currentMenu->printMenu();
        return;
    }

    // Attack menu back handling
    if(currentMenuID == MENU_SPAM_CONFIG) {
        inSpecialMode = false;
        currentMenuID = MENU_ATTACKS;
        currentMenu = attacksMenu;
        screen->clear();
        currentMenu->printMenu();
        return;
    }
    if(currentMenuID == MENU_SPAM_ACTIVE) {
        // DON'T stop the attack - let it continue running
        // Go to main menu so user can stop via Attacks menu
        spamActiveInitialized = false;  // Reset for next time
        inSpecialMode = false;
        currentMenuID = MENU_MAIN;
        currentMenu = mainMenu;
        screen->clear();
        currentMenu->printMenu();
        return;
    }
    if(currentMenuID == MENU_ATTACK_STATUS) {
        // Go back to main menu (attack continues running)
        attackStatusInitialized = false;
        inSpecialMode = false;
        currentMenuID = MENU_MAIN;
        currentMenu = mainMenu;
        screen->clear();
        currentMenu->printMenu();
        return;
    }
    if(currentMenuID == MENU_IMP_DEVICE_SELECT) {
        inSpecialMode = false;
        currentMenuID = MENU_ATTACKS;
        currentMenu = attacksMenu;
        screen->clear();
        currentMenu->printMenu();
        return;
    }
    if(currentMenuID == MENU_IMP_PGN_SELECT) {
        currentMenuID = MENU_IMP_DEVICE_SELECT;
        impDeviceScrollIndex = 0;  // Reset scroll
        displayImpDeviceSelect();
        return;
    }
    if(currentMenuID == MENU_IMP_FIELD_SELECT) {
        // DON'T stop the attack - let it continue running
        // Go to main menu so user can view Live Data or stop via Attacks menu
        impFieldSelectInitialized = false;
        inSpecialMode = false;
        currentMenuID = MENU_MAIN;
        currentMenu = mainMenu;
        screen->clear();
        currentMenu->printMenu();
        return;
    }
    if(currentMenuID == MENU_ABOUT_INFO) {
        // Go back directly to main menu
        currentMenuID = MENU_MAIN;
        currentMenu = mainMenu;
        screen->clear();
        currentMenu->printMenu();
        return;
    }
    if(currentMenuID == MENU_ABOUT_PGNS) {
        currentMenuID = MENU_ABOUT;
        currentMenu = aboutMenu;
        screen->clear();
        currentMenu->printMenu();
        return;
    }

    // Handle new device-centric menu hierarchy
    if(currentMenuID == MENU_PGN_DETAIL) {
        // Go back from PGN detail to PGN list
        currentMenuID = MENU_DEVICE_PGNS;
        displayDevicePGNs();
        return;
    }

    if(currentMenuID == MENU_DEVICE_PGNS) {
        // Go back from PGN list to device list
        currentMenuID = MENU_DEVICE_LIST;
        displayDeviceList();
        return;
    }

    if(currentMenuID == MENU_DEVICE_LIST) {
        // Go back from device list to main menu
        inSpecialMode = false;
        currentMenuID = MENU_MAIN;
        currentMenu = mainMenu;
        screen->clear();
        currentMenu->printMenu();
        return;
    }

    if(currentMenuID == MENU_STALE_CLEANUP) {
        // Go back from stale cleanup toggle to device config menu
        // Pop the stack since changeMenu pushed MENU_DEVICE_CONFIG when entering
        if(menuStackPointer > 0) {
            popMenu();
        }
        inSpecialMode = false;
        currentMenuID = MENU_DEVICE_CONFIG;
        currentMenu = deviceConfigMenu;
        screen->clear();
        currentMenu->printMenu();
        return;
    }

    if(currentMenuID == MENU_MANUFACTURER_SELECT) {
        // Go back from manufacturer selection to sensor config menu
        // Pop the stack since changeMenu pushed the sensor config menu when entering
        if(menuStackPointer > 0) {
            popMenu();
        }
        inSpecialMode = false;
        MenuID targetMenu = (currentSensorBeingConfigured == 0) ? MENU_CONFIGURE_SENSOR1 :
                           (currentSensorBeingConfigured == 1) ? MENU_CONFIGURE_SENSOR2 : MENU_CONFIGURE_SENSOR3;
        currentMenuID = targetMenu;
        currentMenu = (currentSensorBeingConfigured == 0) ? configureSensor1Menu :
                     (currentSensorBeingConfigured == 1) ? configureSensor2Menu : configureSensor3Menu;
        typeScrollOffset = 0;
        updateSensorConfigDisplay(currentSensorBeingConfigured);
        return;
    }

    // Legacy: Return from PGN detail view to PGN list
    if(viewingPGNDetail) {
        viewingPGNDetail = false;
        displayPGNList();
        return;
    }

    if(inSpecialMode) {
        // Exit special mode - return to the menu we came from
        inSpecialMode = false;
        viewingPGNDetail = false;

        // Pop from stack to get previous menu
        if(menuStackPointer > 0) {
            MenuID prevID = popMenu();
            currentMenuID = prevID;  // Important: set this BEFORE clearing special mode flags

            screen->clear();

            // Update current menu pointer based on ID
            switch(prevID) {
                case MENU_MAIN:
                    currentMenu = mainMenu;
                    break;
                case MENU_CONFIGURE:
                    currentMenu = configureMenu;
                    break;
                case MENU_ATTACKS:
                    currentMenu = attacksMenu;
                    break;
                case MENU_ABOUT:
                    currentMenu = aboutMenu;
                    break;
                case MENU_DEVICE_CONFIG:
                    currentMenu = deviceConfigMenu;
                    break;
                default:
                    currentMenu = mainMenu;
                    break;
            }

            currentMenu->printMenu();
        }
        return;
    }

    if(menuStackPointer > 0) {
        // Pop from menu stack and go to previous menu
        MenuID prevID = popMenu();
        currentMenuID = prevID;

        screen->clear();

        // Update current menu pointer based on ID
        switch(prevID) {
            case MENU_MAIN:
                currentMenu = mainMenu;
                break;
            case MENU_CONFIGURE:
                currentMenu = configureMenu;
                break;
            case MENU_CONFIGURE_SENSOR1:
                currentMenu = configureSensor1Menu;
                typeScrollOffset = 0;
                updateSensorConfigDisplay(0);
                return;
            case MENU_CONFIGURE_SENSOR2:
                currentMenu = configureSensor2Menu;
                typeScrollOffset = 0;
                updateSensorConfigDisplay(1);
                return;
            case MENU_CONFIGURE_SENSOR3:
                currentMenu = configureSensor3Menu;
                typeScrollOffset = 0;
                updateSensorConfigDisplay(2);
                return;
            case MENU_ATTACKS:
                currentMenu = attacksMenu;
                break;
            case MENU_ABOUT:
                currentMenu = aboutMenu;
                break;
            case MENU_DEVICE_CONFIG:
                currentMenu = deviceConfigMenu;
                break;
            default:
                currentMenu = mainMenu;
                break;
        }

        currentMenu->printMenu();
    }
}

/**
 * @brief Handles selection/right navigation in the menu system.
 */
void Menu_Controller::navigateSelect() {
    // Attack status screen - SELECT stops the attack
    if(currentMenuID == MENU_ATTACK_STATUS) {
        // Stop whichever attack is active
        if(attackController->isSpamActive()) attackController->stopSpamAttack();
        if(attackController->isImpersonateActive()) attackController->stopImpersonate();
        attackStatusInitialized = false;
        inSpecialMode = false;
        // Go to attacks menu after stopping
        currentMenuID = MENU_ATTACKS;
        currentMenu = attacksMenu;
        screen->clear();
        currentMenu->printMenu();
        return;
    }

    // Attack menu select handling
    if(currentMenuID == MENU_SPAM_CONFIG) {

        
        attackController->startSpamAttack();
        inSpecialMode = true;
        currentMenuID = MENU_SPAM_ACTIVE;
        displaySpamActive();
        for(int i = 1; i<253; i++){
            attackController -> sendHighPriorityAddressClaim(i);
            updateSpamActiveValue();
        }
        return;
    }
    if(currentMenuID == MENU_SPAM_ACTIVE) {
        return;  // No select action during active attack
    }
    if(currentMenuID == MENU_IMP_DEVICE_SELECT) {
        // Use filtered impDeviceList (built in displayImpDeviceSelect)
        if(!impDeviceList.empty() && impDeviceScrollIndex < (int)impDeviceList.size()) {
            uint8_t targetAddr = impDeviceList[impDeviceScrollIndex];
            impPGNScrollIndex = 0;
            // Build PGN list for this device
            attackController->buildImpPGNList(targetAddr);
            currentMenuID = MENU_IMP_PGN_SELECT;
            displayImpPGNSelect();
        }
        return;
    }
    if(currentMenuID == MENU_IMP_PGN_SELECT) {
        std::vector<uint32_t>& impPGNList = attackController->getImpPGNList();
        std::map<uint8_t, DeviceInfo>& devices = monitor->getDevices();
        // Use filtered impDeviceList
        if(!impPGNList.empty() && impPGNScrollIndex < (int)impPGNList.size()) {
            uint8_t targetAddr = impDeviceList[impDeviceScrollIndex];
            uint32_t targetPGN = impPGNList[impPGNScrollIndex];

            // Check if this is one of our own sensors and track it
            String devName = devices[targetAddr].name;
            bool isOwnSensor = (devName == "Sensor 1" || devName == "Sensor 2" || devName == "Sensor 3");
            if (isOwnSensor) {
                // Determine which sensor index (0, 1, or 2)
                uint8_t sensorIdx = 0;
                if (devName == "Sensor 2") sensorIdx = 1;
                else if (devName == "Sensor 3") sensorIdx = 2;
                attackController->setImpersonatingOwnSensor(true, sensorIdx);
            } else {
                attackController->setImpersonatingOwnSensor(false, 0);
            }

            attackController->startImpersonate(targetAddr, targetPGN);
            inSpecialMode = true;
            currentMenuID = MENU_IMP_FIELD_SELECT;
            displayImpFieldSelect();
        }
        return;
    }
    if(currentMenuID == MENU_IMP_FIELD_SELECT) {
        attackController->toggleValueLock();
        impFieldSelectInitialized = false;  // Force full redraw for lock state change
        displayImpFieldSelect();
        impFieldSelectInitialized = true;
        return;
    }

    // Handle new device-centric menu hierarchy
    if(currentMenuID == MENU_DEVICE_LIST) {
        // Select a device - go to its PGN list
        std::vector<uint8_t>& deviceList = monitor->getDeviceList();
        if(!deviceList.empty() && selectedDeviceIndex < (int)deviceList.size()) {
            currentDeviceAddress = deviceList[selectedDeviceIndex];
            currentMenuID = MENU_DEVICE_PGNS;
            selectedPGNIndex = 0;
            displayDevicePGNs();
        }
        return;
    }

    if(currentMenuID == MENU_DEVICE_PGNS) {
        // Select a PGN - go to detail view
        std::map<uint8_t, DeviceInfo>& devices = monitor->getDevices();
        if(devices.find(currentDeviceAddress) != devices.end()) {
            DeviceInfo& device = devices[currentDeviceAddress];
            if(!device.pgns.empty()) {
                // Get the PGN at the selected index
                std::vector<uint32_t> pgnList;
                for(auto& pair : device.pgns) {
                    pgnList.push_back(pair.first);
                }
                if(selectedPGNIndex < (int)pgnList.size()) {
                    currentPGN = pgnList[selectedPGNIndex];
                    currentMenuID = MENU_PGN_DETAIL;
                    detailScrollOffset = 0;
                    pgnFieldScrollOffset = 0;  // Reset horizontal scroll for new PGN
                    displayPGNDetail();
                }
            }
        }
        return;
    }

    if(currentMenuID == MENU_PGN_DETAIL) {
        // Already at detail level - do nothing on select
        return;
    }

    if(currentMenuID == MENU_STALE_CLEANUP) {
        // Toggle stale cleanup setting
        monitor->setStaleCleanupEnabled(!monitor->isStaleCleanupEnabled());
        displayDeviceConfig();  // Refresh display to show new state
        return;
    }

    if(currentMenuID == MENU_MANUFACTURER_SELECT) {
        // Set the selected manufacturer code for the current sensor
        setManufacturerCode(currentSensorBeingConfigured, MANUFACTURERS[selectedManufacturerIndex].code);
        // Go back to sensor config menu
        inSpecialMode = false;
        MenuID targetMenu = (currentSensorBeingConfigured == 0) ? MENU_CONFIGURE_SENSOR1 :
                           (currentSensorBeingConfigured == 1) ? MENU_CONFIGURE_SENSOR2 : MENU_CONFIGURE_SENSOR3;
        currentMenuID = targetMenu;
        currentMenu = (currentSensorBeingConfigured == 0) ? configureSensor1Menu :
                     (currentSensorBeingConfigured == 1) ? configureSensor2Menu : configureSensor3Menu;
        typeScrollOffset = 0;
        updateSensorConfigDisplay(currentSensorBeingConfigured);
        return;
    }

    // Legacy: In PGN monitoring, show selected PGN value
    if(inSpecialMode) {
        std::vector<PGNInfo>& detectedPGNs = monitor->getDetectedPGNs();
        if(!detectedPGNs.empty() && selectedPGNIndex < (int)detectedPGNs.size()) {
            viewingPGNDetail = true;
            lastDisplayedValue = detectedPGNs[selectedPGNIndex].value;
            displayPGNValue(selectedPGNIndex);
        }
    } else if(currentMenuID == MENU_SENSOR1_PGN_TYPE ||
              currentMenuID == MENU_SENSOR2_PGN_TYPE ||
              currentMenuID == MENU_SENSOR3_PGN_TYPE) {
        // Handle PGN type selection - get selected option
        int selectedOption = currentMenu->curr_option;
        setSensorPGNType(currentSensorBeingConfigured, selectedOption);
    } else {
        // Execute the selected menu option
        int selectedOption = currentMenu->curr_option;
        if(currentMenu->options != nullptr &&
           currentMenu->options[selectedOption].func != nullptr) {
            currentMenu->options[selectedOption].func();
        }
    }
}

/**
 * @brief Pushes the current menu ID onto the navigation stack.
 *
 * Saves the current menu state to enable back navigation. The stack has
 * a maximum depth defined by MAX_MENU_DEPTH to prevent overflow.
 *
 * @param menuID The menu ID to push onto the stack (currently unused,
 *               uses currentMenuID internally)
 */
void Menu_Controller::pushMenu(MenuID menuID) {
    if(menuStackPointer < MAX_MENU_DEPTH) {
        menuStack[menuStackPointer++] = currentMenuID;
    }
}

/**
 * @brief Pops and returns the previous menu ID from the navigation stack.
 *
 * Retrieves the most recently pushed menu ID for back navigation.
 * Returns MENU_MAIN if the stack is empty.
 *
 * @return MenuID The previous menu ID, or MENU_MAIN if stack is empty
 */
MenuID Menu_Controller::popMenu() {
    if(menuStackPointer > 0) {
        return menuStack[--menuStackPointer];
    }
    return MENU_MAIN;
}

/**
 * @brief Changes to a new menu and handles the transition.
 *
 * Manages menu transitions including:
 * - Pushing current menu to the stack (if not going back)
 * - Clearing the screen
 * - Setting up the new menu state (inSpecialMode, scroll offsets, etc.)
 * - Displaying the new menu or calling special display functions
 * - Showing attack indicator on main menu if an attack is active
 *
 * @param newMenuID The ID of the menu to switch to
 */
void Menu_Controller::changeMenu(MenuID newMenuID) {
    // Only push if we're not going back
    if(newMenuID != currentMenuID) {
        pushMenu(currentMenuID);
    }

    MenuID oldMenuID = currentMenuID;
    currentMenuID = newMenuID;

    // Clear screen before changing menu
    screen->clear();

    switch(newMenuID) {
        case MENU_MAIN:
            currentMenu = mainMenu;
            inSpecialMode = false;
            // Attack indicator will be shown after printMenu()
            break;

        case MENU_DEVICE_LIST:
            // New device-centric Live Data view
            inSpecialMode = true;
            selectedDeviceIndex = 0;
            deviceListScrollOffset = 0;  // Reset scroll position
            displayDeviceList();
            return;  // Don't call printMenu for special mode

        case MENU_DEVICE_PGNS:
            // PGN list for selected device
            inSpecialMode = true;
            selectedPGNIndex = 0;
            displayDevicePGNs();
            return;

        case MENU_PGN_DETAIL:
            // Detail view for selected PGN
            inSpecialMode = true;
            detailScrollOffset = 0;
            pgnFieldScrollOffset = 0;  // Reset horizontal scroll
            displayPGNDetail();
            return;

        case MENU_SENSOR_READINGS:
            // Legacy - redirect to device list
            currentMenuID = MENU_DEVICE_LIST;
            inSpecialMode = true;
            selectedDeviceIndex = 0;
            displayDeviceList();
            return;
        case MENU_CONFIGURE:
            currentMenu = configureMenu;
            break;
        case MENU_CONFIGURE_SENSOR1:
            currentMenu = configureSensor1Menu;
            typeScrollOffset = 0;  // Reset scroll position
            updateSensorConfigDisplay(0);
            return;  // Don't call printMenu, we have custom display
        case MENU_CONFIGURE_SENSOR2:
            currentMenu = configureSensor2Menu;
            typeScrollOffset = 0;  // Reset scroll position
            updateSensorConfigDisplay(1);
            return;  // Don't call printMenu, we have custom display
        case MENU_CONFIGURE_SENSOR3:
            currentMenu = configureSensor3Menu;
            typeScrollOffset = 0;  // Reset scroll position
            updateSensorConfigDisplay(2);
            return;  // Don't call printMenu, we have custom display
        case MENU_ATTACKS:
            currentMenu = attacksMenu;
            break;
        case MENU_ABOUT:
            currentMenu = aboutMenu;
            break;
        case MENU_SENSOR1_PGN_TYPE:
            currentMenu = pgnTypeMenus[0];
            currentSensorBeingConfigured = 0;
            break;
        case MENU_SENSOR2_PGN_TYPE:
            currentMenu = pgnTypeMenus[1];
            currentSensorBeingConfigured = 1;
            break;
        case MENU_SENSOR3_PGN_TYPE:
            currentMenu = pgnTypeMenus[2];
            currentSensorBeingConfigured = 2;
            break;
        case MENU_DEVICE_CONFIG:
            currentMenu = deviceConfigMenu;
            break;
        case MENU_STALE_CLEANUP:
            // Special display for toggle state
            inSpecialMode = true;
            displayDeviceConfig();
            return;
        case MENU_MANUFACTURER_SELECT:
            // Special display for manufacturer selection
            inSpecialMode = true;
            selectedManufacturerIndex = 0;
            displayManufacturerSelect();
            return;
        default:
            currentMenu = mainMenu;
            break;
    }

    currentMenu->printMenu();

    // Show attack indicator on bottom row if attack active and on main menu
    if(newMenuID == MENU_MAIN && attackController->isAttackActive()) {
        screen->setInverseFont(1);
        screen->drawString(0, 7, "!Attack Active  ");
        screen->setInverseFont(0);
    }
}
