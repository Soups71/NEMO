/**
 * @file Menu_Display.cpp
 * @brief Display functions for the Menu_Controller class.
 *
 * This file implements all display rendering functions for the NEMO menu system,
 * including device lists, PGN lists, PGN detail views, device configuration,
 * manufacturer selection, and about screens. It handles text scrolling for
 * long content and efficient partial screen updates.
 */

#include "Menu_Controller.h"

/**
 * @brief Displays the list of detected network devices.
 *
 * Shows all NMEA2000 devices detected on the network with their names and
 * PGN counts. Supports vertical scrolling through the list and horizontal
 * scrolling for long device names. The currently selected device is
 * highlighted with inverse font.
 *
 * Display format:
 * - Row 0: Title "NETWORK DEVICES"
 * - Rows 2-6: Device entries with format "DeviceName (N)" where N is PGN count
 * - Row 7: Navigation hints "< BACK    SEL >"
 *
 * If no devices are detected, shows "Scanning..." message.
 */
void Menu_Controller::displayDeviceList() {
    prepScreen();

    // Title - centered (16 char width, 15 char title = offset 0)
    screen->drawString(0, 0, "NETWORK DEVICES");

    std::vector<uint8_t>& deviceList = monitor->getDeviceList();
    std::map<uint8_t, DeviceInfo>& devices = monitor->getDevices();

    if(deviceList.empty()) {
        screen->drawString(0, 3, "Scanning...");
        screen->drawString(0, 4, "No devices yet");
        screen->drawString(0, 7, "< BACK");
        return;
    }

    // Display devices with scrolling
    int row = 2;
    int startIdx = selectedDeviceIndex > 3 ? selectedDeviceIndex - 3 : 0;

    for(int i = startIdx; i < (int)deviceList.size() && row < 7; i++) {
        uint8_t addr = deviceList[i];
        bool isSelected = (i == selectedDeviceIndex);

        if(isSelected) {
            screen->setInverseFont(1);
        } else {
            screen->setInverseFont(0);
        }

        String deviceName = "";
        int pgnCount = 0;

        if(devices.find(addr) != devices.end()) {
            DeviceInfo& dev = devices[addr];
            deviceName = dev.name;
            pgnCount = dev.pgns.size();
        }

        // If no name, use address
        if(deviceName.length() == 0) {
            deviceName = "Device " + String(addr);
        }

        // Format: "DeviceName (N)" where N is PGN count
        String pgnSuffix = " (" + String(pgnCount) + ")";
        int maxNameLen = 16 - pgnSuffix.length();  // Leave room for PGN count

        String displayName;
        if(isSelected && (int)deviceName.length() > maxNameLen) {
            // Scrolling for selected item with long name
            String scrollText = deviceName + "   " + deviceName;
            displayName = scrollText.substring(deviceListScrollOffset, deviceListScrollOffset + maxNameLen);
        } else if((int)deviceName.length() > maxNameLen) {
            // Truncate non-selected long names
            displayName = deviceName.substring(0, maxNameLen - 2) + "..";
        } else {
            displayName = deviceName;
        }

        // Pad to fill space before PGN count
        while((int)displayName.length() < maxNameLen) {
            displayName += " ";
        }

        String line = displayName + pgnSuffix;
        screen->drawString(0, row, line.c_str());
        row++;
    }

    screen->setInverseFont(0);
    screen->drawString(0, 7, "< BACK    SEL >");
}

/**
 * @brief Displays the list of PGNs for the currently selected device.
 *
 * Shows all PGNs transmitted by the device at currentDeviceAddress.
 * Supports vertical scrolling through the PGN list. The currently
 * selected PGN is highlighted with inverse font.
 *
 * Display format:
 * - Row 0: Title "DEV [address] PGNs"
 * - Rows 2-6: PGN names (truncated to 16 chars or with "..." suffix)
 * - Row 7: Navigation hints "< BACK    SEL >"
 *
 * Shows "No PGNs yet" if the device has no detected PGNs.
 */
void Menu_Controller::displayDevicePGNs() {
    prepScreen();

    std::map<uint8_t, DeviceInfo>& devices = monitor->getDevices();
    if(devices.find(currentDeviceAddress) == devices.end()) {
        screen->drawString(0, 0, "Device not found");
        screen->drawString(0, 7, "< BACK");
        return;
    }

    DeviceInfo& device = devices[currentDeviceAddress];

    // Title - show device address
    String title = "DEV " + String(currentDeviceAddress) + " PGNs";
    screen->drawString(0, 0, title.c_str());

    if(device.pgns.empty()) {
        screen->drawString(0, 3, "No PGNs yet");
        screen->drawString(0, 7, "< BACK");
        return;
    }

    // Build ordered list of PGNs
    std::vector<uint32_t> pgnList;
    for(auto& pair : device.pgns) {
        pgnList.push_back(pair.first);
#if DEBUG
        Serial.println(pair.first);
#endif
    }

    // Display PGNs with scrolling
    int row = 2;
    int startIdx = selectedPGNIndex > 3 ? selectedPGNIndex - 3 : 0;

    for(int i = startIdx; i < (int)pgnList.size() && row < 7; i++) {
        uint32_t pgn = pgnList[i];
        if(i == selectedPGNIndex) {
            screen->setInverseFont(1);
        } else {
            screen->setInverseFont(0);
        }

        String line = monitor->getPGNName(pgn);
        if(line.length() > 16) line = line.substring(0, 13) + "...";

        screen->drawString(0, row, line.c_str());
        row++;
    }

    screen->setInverseFont(0);
    screen->drawString(0, 7, "< BACK    SEL >");
}

/**
 * @brief Displays the detailed view for the currently selected PGN.
 *
 * Shows all fields of the PGN with their names, values, and units.
 * Supports vertical scrolling through fields and horizontal scrolling
 * for long field values. Uses the drawLine() helper for efficient
 * partial updates.
 *
 * Display format:
 * - Row 0: PGN name (scrolls if longer than 16 chars)
 * - Row 1: "PGN [number]"
 * - Rows 2-6: Field entries with format "Name: Value Unit"
 * - Row 7: Navigation hints with scroll indicators (^ v)
 *
 * Sets detailViewInitialized flag after first complete draw.
 */
void Menu_Controller::displayPGNDetail() {
    prepScreen();

    // Reset displayed lines tracking
    for(int i = 0; i < 8; i++) {
        displayedLines[i] = "";
    }

    std::map<uint8_t, DeviceInfo>& devices = monitor->getDevices();
    if(devices.find(currentDeviceAddress) == devices.end() ||
       devices[currentDeviceAddress].pgns.find(currentPGN) == devices[currentDeviceAddress].pgns.end()) {
        drawLine(0, "PGN not found");
        drawLine(7, "< BACK");
        detailViewInitialized = true;
        return;
    }

    PGNData& pgnData = devices[currentDeviceAddress].pgns[currentPGN];

    // Title - PGN name (row 0) - truncate initially, scrolling happens in update()
    String title = pgnData.name;
    if((int)title.length() > 16) {
        title = title.substring(0, 16);
    }
    drawLine(0, title);

    // Show PGN number (row 1)
    drawLine(1, "PGN " + String(currentPGN));

    // Display fields (rows 2-6) - truncate initially, scrolling happens in update()
    int maxRows = 5;
    int totalFields = pgnData.fields.size();

    // Clamp scroll offset
    if(detailScrollOffset > totalFields - maxRows) {
        detailScrollOffset = max(0, totalFields - maxRows);
    }
    if(detailScrollOffset < 0) detailScrollOffset = 0;

    int row = 2;
    for(int i = detailScrollOffset; i < totalFields && row < 7; i++) {
        PGNField& field = pgnData.fields[i];

        // Build line with label fixed, value truncated if needed
        String label = "";
        String valueWithUnit = field.value;
        int labelWidth = 0;

        if(field.name.length() > 0) {
            label = field.name + ": ";
            labelWidth = label.length();
        }
        if(field.unit.length() > 0) {
            valueWithUnit += " " + field.unit;
        }

        // Calculate available width for value
        int valueAreaWidth = 16 - labelWidth;

        // Truncate value if needed - scrolling happens in update()
        String displayValue = valueWithUnit;
        if((int)valueWithUnit.length() > valueAreaWidth && valueAreaWidth > 0) {
            displayValue = valueWithUnit.substring(0, valueAreaWidth);
        }

        String line = label + displayValue;
        // Pad to 16 chars
        while((int)line.length() < 16) line += " ";

        drawLine(row, line);
        row++;
    }

    // Clear any unused rows
    while(row < 7) {
        drawLine(row, "");
        row++;
    }

    // Show scroll indicators (row 7)
    String navLine = "< BACK";
    if(totalFields > maxRows) {
        if(detailScrollOffset > 0) navLine += " ^";
        if(detailScrollOffset < totalFields - maxRows) navLine += " v";
    }
    drawLine(7, navLine);

    detailViewInitialized = true;
}

/**
 * @brief Updates only the value portions of the PGN detail view.
 *
 * Performs an efficient partial update of the PGN detail screen,
 * only redrawing field values that have changed. Skips fields that
 * need horizontal scrolling (handled separately in update()).
 *
 * This avoids full screen redraws and reduces flicker during
 * live data updates.
 */
void Menu_Controller::updatePGNDetailValues() {
    if(!detailViewInitialized) {
        displayPGNDetail();
        return;
    }

    std::map<uint8_t, DeviceInfo>& devices = monitor->getDevices();
    if(devices.find(currentDeviceAddress) == devices.end() ||
       devices[currentDeviceAddress].pgns.find(currentPGN) == devices[currentDeviceAddress].pgns.end()) {
        return;
    }

    PGNData& pgnData = devices[currentDeviceAddress].pgns[currentPGN];

    // Only update the value rows (2-6), leave title and navigation alone
    int totalFields = pgnData.fields.size();

    int row = 2;
    for(int i = detailScrollOffset; i < totalFields && row < 7; i++) {
        PGNField& field = pgnData.fields[i];

        // Build line with label fixed, value truncated if needed
        String label = "";
        String valueWithUnit = field.value;
        int labelWidth = 0;

        if(field.name.length() > 0) {
            label = field.name + ": ";
            labelWidth = label.length();
        }
        if(field.unit.length() > 0) {
            valueWithUnit += " " + field.unit;
        }

        // Calculate available width for value
        int valueAreaWidth = 16 - labelWidth;

        // Skip fields that need scrolling - the scroll code in update() handles those
        if((int)valueWithUnit.length() > valueAreaWidth && valueAreaWidth > 0) {
            row++;
            continue;
        }

        // Only update fields that fit without scrolling
        String displayValue = valueWithUnit;

        String line = label + displayValue;
        // Pad to 16 chars
        while((int)line.length() < 16) line += " ";

        drawLine(row, line);
        row++;
    }
}

/**
 * @brief Draws a line of text only if it has changed from the cached value.
 *
 * Compares the new text against the cached displayedLines array and only
 * performs a screen draw if the content is different. Text is padded to
 * 16 characters to clear any previous content.
 *
 * @param row The display row (0-7)
 * @param text The text to display (will be padded/truncated to 16 chars)
 */
void Menu_Controller::drawLine(int row, const String& text) {
    if(row < 0 || row >= 8) return;

    // Pad text to 16 chars to clear any old content
    String paddedText = text;
    while(paddedText.length() < 16) paddedText += " ";
    if(paddedText.length() > 16) paddedText = paddedText.substring(0, 16);

    // Only draw if different from what's currently displayed
    if(displayedLines[row] != paddedText) {
        displayedLines[row] = paddedText;
        screen->drawString(0, row, paddedText.c_str());
    }
}

/**
 * @brief Gets the number of fields in the currently displayed PGN.
 *
 * @return int The number of fields, or 0 if PGN not found
 */
int Menu_Controller::getPGNFieldCount() {
    std::map<uint8_t, DeviceInfo>& devices = monitor->getDevices();
    if(devices.find(currentDeviceAddress) == devices.end() ||
       devices[currentDeviceAddress].pgns.find(currentPGN) == devices[currentDeviceAddress].pgns.end()) {
        return 0;
    }
    return devices[currentDeviceAddress].pgns[currentPGN].fields.size();
}
/**
 * @brief Displays the legacy PGN list screen.
 *
 * Shows detected PGNs in a simple list format. This is the legacy display
 * method; the new device-centric view (displayDeviceList/displayDevicePGNs)
 * is preferred.
 *
 * Display format:
 * - Row 0: Title "LIVE NMEA DATA" (centered)
 * - Rows 2-7: PGN entries with " * " prefix
 *
 * Shows "Waiting for network data..." if no PGNs detected.
 */
void Menu_Controller::displayPGNList() {
    prepScreen();

    // Title
    int titleLen = String("LIVE NMEA DATA").length();
    int padding = (screen->getCols() - titleLen) / 2;
    screen->drawString(padding, 0, "LIVE NMEA DATA");

    // Check if any PGNs have been detected
    std::vector<PGNInfo>& detectedPGNs = monitor->getDetectedPGNs();
    if(detectedPGNs.empty()) {
        // No PGNs detected yet
        screen->drawString(0, 3, "Waiting for");
        screen->drawString(0, 4, "network data...");
        screen->drawString(0, 7, "< BACK");
        return;
    }

    // Display PGNs
    int row = 2;
    int startIdx = selectedPGNIndex > 3 ? selectedPGNIndex - 3 : 0;

    for(int i = startIdx; i < (int)detectedPGNs.size() && row < 8; i++) {
        if(i == selectedPGNIndex) {
            screen->setInverseFont(1);
        } else {
            screen->setInverseFont(0);
        }

        // Truncate name to fit (leave room for " * " = 3 chars, so max 13 chars)
        String line = detectedPGNs[i].name;
        if(line.length() > 13) {
            line = line.substring(0, 10) + "...";
        }

        screen->drawString(0, row, " * ");
        screen->drawString(3, row, line.c_str());
        row++;
    }

    screen->setInverseFont(0);
}

/**
 * @brief Displays the value detail view for a legacy PGN.
 *
 * Shows detailed information for a single PGN including its name,
 * PGN number, and current value. This is the legacy detail view;
 * displayPGNDetail() provides a more comprehensive field-by-field view.
 *
 * @param pgnIndex Index into the detectedPGNs vector
 */
void Menu_Controller::displayPGNValue(int pgnIndex) {
    std::vector<PGNInfo>& detectedPGNs = monitor->getDetectedPGNs();
    if(pgnIndex >= (int)detectedPGNs.size()) return;

    prepScreen();

    PGNInfo& info = detectedPGNs[pgnIndex];

    // Truncate name if needed (max 16 chars)
    String displayName = info.name;
    if(displayName.length() > 16) {
        displayName = displayName.substring(0, 13) + "...";
    }

    screen->drawString(0, 0, displayName.c_str());
    screen->drawString(0, 2, "PGN:");
    screen->drawString(5, 2, String(info.pgn).c_str());
    screen->drawString(0, 4, "Value:");

    // Format value to fit on screen
    String valueStr = String(info.value, 2);
    if(valueStr.length() > 16) {
        valueStr = valueStr.substring(0, 16);
    }
    screen->drawString(0, 5, valueStr.c_str());
    screen->drawString(0, 7, "< BACK");
}

/**
 * @brief Updates only the value line in the legacy PGN value display.
 *
 * Efficiently updates just the value portion of the PGN detail screen
 * without redrawing the entire display.
 *
 * @param pgnIndex Index into the detectedPGNs vector
 */
void Menu_Controller::updatePGNValueDisplay(int pgnIndex) {
    std::vector<PGNInfo>& detectedPGNs = monitor->getDetectedPGNs();
    if(pgnIndex >= (int)detectedPGNs.size()) return;

    PGNInfo& info = detectedPGNs[pgnIndex];

    // Only update the value line (row 5)
    // Clear the value line first by drawing spaces
    screen->drawString(0, 5, "                ");

    // Format and display new value
    String valueStr = String(info.value, 2);
    if(valueStr.length() > 16) {
        valueStr = valueStr.substring(0, 16);
    }
    screen->drawString(0, 5, valueStr.c_str());
}

/**
 * @brief Displays the device configuration screen for stale cleanup toggle.
 *
 * Shows the current state of the stale cleanup feature which automatically
 * removes devices and PGNs that haven't been seen recently.
 *
 * Display format:
 * - Row 0: Title "STALE CLEANUP"
 * - Rows 2-3: Description "Remove stale devices/PGNs:"
 * - Row 5: Current state "ENABLED" or "DISABLED" (inverse font)
 * - Row 7: Navigation hints "< BACK  TOGGLE>"
 */
void Menu_Controller::displayDeviceConfig() {
    prepScreen();

    // Title
    screen->drawString(0, 0, "STALE CLEANUP");

    // Current state
    screen->drawString(0, 2, "Remove stale");
    screen->drawString(0, 3, "devices/PGNs:");

    screen->setInverseFont(1);
    if(monitor->isStaleCleanupEnabled()) {
        screen->drawString(0, 5, "  ENABLED     ");
    } else {
        screen->drawString(0, 5, "  DISABLED    ");
    }
    screen->setInverseFont(0);

    screen->drawString(0, 7, "< BACK  TOGGLE>");
}

/**
 * @brief Displays the manufacturer selection screen.
 *
 * Shows a scrollable list of NMEA2000 manufacturers from the MANUFACTURERS
 * constant array. The current manufacturer for the sensor being configured
 * is shown at the top, and the currently highlighted selection is shown
 * with inverse font.
 *
 * Display format:
 * - Row 0: Title "SENSOR [N] MFR"
 * - Row 1: Current manufacturer "Cur: [name]"
 * - Rows 3-6: Manufacturer list entries
 * - Row 7: Navigation hints "< BACK    SEL >"
 */
void Menu_Controller::displayManufacturerSelect() {
    prepScreen();

    // Title with sensor number
    String title = "SENSOR " + String(currentSensorBeingConfigured + 1) + " MFR";
    screen->drawString(0, 0, title.c_str());

    // Get current sensor's manufacturer code
    Sensor* targetSensor = (currentSensorBeingConfigured == 0) ? sensor1 :
                          (currentSensorBeingConfigured == 1) ? sensor2 : sensor3;
    uint16_t currentCode = (targetSensor != nullptr) ? targetSensor->getManufacturerCode() : 2046;

    // Show current manufacturer - uses MANUFACTURERS array from constants.h
    String currentMfr = "Cur: ";
    const char* mfrNamePtr = getManufacturerName(currentCode);
    if(mfrNamePtr) {
        currentMfr += mfrNamePtr;
    } else {
        currentMfr += String(currentCode);
    }
    if(currentMfr.length() > 16) currentMfr = currentMfr.substring(0, 16);
    screen->drawString(0, 1, currentMfr.c_str());

    // Display manufacturers with scrolling - uses MANUFACTURERS array from constants.h
    int row = 3;
    int startIdx = selectedManufacturerIndex > 2 ? selectedManufacturerIndex - 2 : 0;

    for(int i = startIdx; i < MANUFACTURER_COUNT && row < 7; i++) {
        if(i == selectedManufacturerIndex) {
            screen->setInverseFont(1);
        } else {
            screen->setInverseFont(0);
        }

        String line = String(MANUFACTURERS[i].name);
        if(line.length() > 16) line = line.substring(0, 16);
        // Pad with spaces to clear previous text
        while(line.length() < 16) line += " ";
        screen->drawString(0, row, line.c_str());
        row++;
    }

    screen->setInverseFont(0);
    screen->drawString(0, 7, "< BACK    SEL >");
}

/**
 * @brief Displays the About Info screen.
 *
 * Shows basic information about the NEMO project including version
 * number and GitHub repository URL.
 *
 * Display format:
 * - Row 0: Title "ABOUT - INFO"
 * - Row 2: "NEMO" (centered)
 * - Row 3: "Version 1.0" (centered)
 * - Rows 5-6: GitHub URL
 * - Row 7: "< BACK"
 */
void Menu_Controller::displayAboutInfo() {
    prepScreen();

    screen->drawString(0, 0, "ABOUT - INFO");

    screen->drawString(0, 2, "      NEMO");
    screen->drawString(0, 3, "   Version 1.0");
    screen->drawString(0, 5, "   github.com/");
    screen->drawString(0, 6, "   soups71/nemo");

    screen->drawString(0, 7, "< BACK");
}

/**
 * @brief Displays the list of supported PGNs.
 *
 * Shows a scrollable list of all PGNs that can be impersonated/transmitted
 * by the NEMO device. Uses IMPERSONATABLE_PGN_DEFS from constants.h.
 * The currently selected entry is indicated with a '>' prefix.
 *
 * Display format:
 * - Row 0: Title "SUPPORTED PGNs"
 * - Rows 1-6: PGN entries with ">" indicator for selection
 * - Row 7: Navigation with scroll position "[N/M]"
 */
void Menu_Controller::displaySupportedPGNs() {
    prepScreen();

    screen->drawString(0, 0, "SUPPORTED PGNs");

    // Use IMPERSONATABLE_PGN_DEFS from constants.h

    // Clamp scroll index
    if (aboutPGNScrollIndex >= IMPERSONATABLE_PGN_COUNT) {
        aboutPGNScrollIndex = IMPERSONATABLE_PGN_COUNT - 1;
    }
    if (aboutPGNScrollIndex < 0) {
        aboutPGNScrollIndex = 0;
    }

    // Display up to 6 PGNs (rows 1-6), row 7 is for navigation
    int startIdx = 0;
    if (aboutPGNScrollIndex > 5) {
        startIdx = aboutPGNScrollIndex - 5;
    }

    int row = 1;
    for (int i = startIdx; i < IMPERSONATABLE_PGN_COUNT && row < 7; i++) {
        char line[17];
        char indicator = (i == aboutPGNScrollIndex) ? '>' : ' ';
        snprintf(line, sizeof(line), "%c%s", indicator, IMPERSONATABLE_PGN_DEFS[i].shortName);
        screen->drawString(0, row, line);
        row++;
    }

    // Show scroll position
    char navHint[17];
    snprintf(navHint, sizeof(navHint), "<BACK    %2d/%2d", aboutPGNScrollIndex + 1, IMPERSONATABLE_PGN_COUNT);
    screen->drawString(0, 7, navHint);
}
