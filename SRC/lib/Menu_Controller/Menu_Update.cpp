/**
 * @file Menu_Update.cpp
 * @brief Main update loop for the Menu_Controller class.
 *
 * This file implements the update() method which is called regularly from the
 * main loop to handle real-time display updates. It manages screen refreshes
 * for all menu states including device lists, PGN details, attack screens,
 * sensor configuration, and text scrolling animations.
 *
 * The update function uses a state machine approach based on currentMenuID
 * to determine what updates are needed for each screen type. It employs
 * efficient partial screen updates where possible to minimize flicker and
 * reduce CPU overhead.
 *
 * MIT License
 *
 * Copyright (c) 2024-2025 NEMO Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "Menu_Controller.h"

/**
 * @brief Main update loop for real-time display and system updates.
 *
 * This method should be called regularly from the main loop (typically every
 * iteration). It handles:
 *
 * - Network monitor updates (stale device cleanup)
 * - Attack controller updates (DOS/impersonate message transmission)
 * - Screen refresh for current menu state
 * - Text scrolling animations for long content
 * - Live value updates for sensor readings and attack statistics
 *
 * Update frequencies vary by screen type:
 * - Attack screens: 100ms for live statistics
 * - PGN detail: 250ms for field values
 * - Device/PGN lists: 500ms-1s for new entries
 * - Text scrolling: SCROLL_DELAY_MS (defined in constants.h)
 *
 * The function uses early returns after handling specialized screens to
 * prevent redundant processing. Screen-specific initialization flags
 * (spamActiveInitialized, impFieldSelectInitialized, etc.) track whether
 * a full redraw is needed or just a partial update.
 *
 * @note This function modifies display state and should not be called
 *       from interrupt context.
 */
void Menu_Controller::update() {
    // Called regularly from main loop for real-time updates
    unsigned long currentTime = millis();

    // Update monitor (handles stale cleanup)
    monitor->update();

    // Update attack controller (handles spam/impersonate attacks)
    attackController->update();

    // -------------------------------------------------------------------------
    // Attack Status Screen Updates
    // -------------------------------------------------------------------------
    // Shows overview of currently running attack with live statistics
    if(currentMenuID == MENU_ATTACK_STATUS) {
        if(currentTime - lastSpamDisplayUpdate > 100) {
            lastSpamDisplayUpdate = currentTime;
            if(!attackStatusInitialized) {
                displayAttackStatus();
            } else {
                updateAttackStatusDisplay();
            }
        }
        return;  // Skip other updates during attack status screen
    }

    // -------------------------------------------------------------------------
    // DOS/Spam Attack Active Screen Updates
    // -------------------------------------------------------------------------
    // Updates message count display during active DOS attack
    if(attackController->isSpamActive() && currentMenuID == MENU_SPAM_ACTIVE) {
        if(currentTime - lastSpamDisplayUpdate > 100) {
            lastSpamDisplayUpdate = currentTime;
            if(!spamActiveInitialized) {
                // First time - do full draw
                displaySpamActive();
            } else {
                // Subsequent updates - only update message count and address
                updateSpamActiveValue();
            }
        }
        return;  // Skip other updates during spam attack screen
    }

    // -------------------------------------------------------------------------
    // Impersonate Field Select Screen Updates
    // -------------------------------------------------------------------------
    // Updates field value display during active impersonation attack
    // Only updates when on the field select screen, not when attack runs in background
    if(attackController->isImpersonateActive() && currentMenuID == MENU_IMP_FIELD_SELECT) {
        if(!impFieldSelectInitialized) {
            // First time - do full draw
            displayImpFieldSelect();
            impFieldSelectInitialized = true;
        } else {
            // Subsequent updates - only update the value line
            updateImpFieldSelectValue();
        }
        return;
    }

    // -------------------------------------------------------------------------
    // Device List Screen Updates
    // -------------------------------------------------------------------------
    // Handles device list refresh and name scrolling for the Live Data view
    if(currentMenuID == MENU_DEVICE_LIST) {
        // Refresh device list periodically to show new devices
        std::vector<uint8_t>& deviceList = monitor->getDeviceList();
        std::map<uint8_t, DeviceInfo>& devices = monitor->getDevices();
        if(currentTime - lastPGNUpdate > 1000) {  // Update every 1 second
            lastPGNUpdate = currentTime;
            static size_t lastDeviceCount = 0;
            if(deviceList.size() != lastDeviceCount) {
                lastDeviceCount = deviceList.size();
                displayDeviceList();
            }
        }

        // Handle horizontal scrolling for selected device name
        if(currentTime - lastDeviceScrollUpdate > SCROLL_DELAY_MS) {
            lastDeviceScrollUpdate = currentTime;

            // Check if selected device name needs scrolling
            if(!deviceList.empty() && selectedDeviceIndex < (int)deviceList.size()) {
                uint8_t addr = deviceList[selectedDeviceIndex];
                if(devices.find(addr) != devices.end()) {
                    String deviceName = devices[addr].name;
                    if(deviceName.length() == 0) {
                        deviceName = "Device " + String(addr);
                    }

                    // Calculate max name length (16 chars - " (N)" suffix)
                    int pgnCount = devices[addr].pgns.size();
                    String pgnSuffix = " (" + String(pgnCount) + ")";
                    int maxNameLen = 16 - pgnSuffix.length();

                    if((int)deviceName.length() > maxNameLen) {
                        // Advance scroll position
                        deviceListScrollOffset++;
                        // Reset when we've scrolled through full text + padding
                        if(deviceListScrollOffset >= (int)deviceName.length() + 3) {
                            deviceListScrollOffset = 0;
                        }

                        // Redraw the device list to show scrolled name
                        displayDeviceList();
                    }
                }
            }
        }
    }
    // -------------------------------------------------------------------------
    // Device PGN List Screen Updates
    // -------------------------------------------------------------------------
    // Refreshes PGN list when new PGNs are detected for the selected device
    else if(currentMenuID == MENU_DEVICE_PGNS) {
        std::map<uint8_t, DeviceInfo>& devices = monitor->getDevices();
        if(currentTime - lastPGNUpdate > 500) {
            lastPGNUpdate = currentTime;
            if(devices.find(currentDeviceAddress) != devices.end()) {
                static size_t lastPGNCount = 0;
                size_t currentPGNCount = devices[currentDeviceAddress].pgns.size();
                if(currentPGNCount != lastPGNCount) {
                    lastPGNCount = currentPGNCount;
                    displayDevicePGNs();
                }
            }
        }
    }
    // -------------------------------------------------------------------------
    // PGN Detail Screen Updates
    // -------------------------------------------------------------------------
    // Handles live field value updates and horizontal text scrolling
    else if(currentMenuID == MENU_PGN_DETAIL) {
        // Refresh PGN detail view periodically for live data
        // Only update lines that have actually changed to avoid flicker
        if(currentTime - lastPGNUpdate > 250) {  // Update every 250ms for live feel
            lastPGNUpdate = currentTime;
            updatePGNDetailValues();  // Only update changed values, no full redraw
        }

        // Handle horizontal scrolling for long field text - update only scrolling lines
        // Scrolls until text is off screen, pauses, then resets to beginning
        if(currentTime - lastPGNFieldScrollUpdate > SCROLL_DELAY_MS) {
            lastPGNFieldScrollUpdate = currentTime;

            std::map<uint8_t, DeviceInfo>& devices = monitor->getDevices();
            if(devices.find(currentDeviceAddress) != devices.end() &&
               devices[currentDeviceAddress].pgns.find(currentPGN) != devices[currentDeviceAddress].pgns.end()) {
                PGNData& pgnData = devices[currentDeviceAddress].pgns[currentPGN];
                bool anyScrolled = false;
                int maxScrollNeeded = 0;

                // Check if PGN title needs scrolling
                if((int)pgnData.name.length() > 16) {
                    int titleMaxScroll = pgnData.name.length();  // Scroll until off screen
                    if(titleMaxScroll > maxScrollNeeded) maxScrollNeeded = titleMaxScroll;
                }

                // Check if any field values need scrolling
                int row = 2;
                for(int i = detailScrollOffset; i < (int)pgnData.fields.size() && row < 7; i++) {
                    PGNField& field = pgnData.fields[i];
                    String valueWithUnit = field.value;
                    int labelWidth = (field.name.length() > 0) ? field.name.length() + 2 : 0;
                    if(field.unit.length() > 0) valueWithUnit += " " + field.unit;
                    int valueAreaWidth = 16 - labelWidth;
                    if((int)valueWithUnit.length() > valueAreaWidth && valueAreaWidth > 0) {
                        int fieldMaxScroll = valueWithUnit.length();  // Scroll until off screen
                        if(fieldMaxScroll > maxScrollNeeded) maxScrollNeeded = fieldMaxScroll;
                    }
                    row++;
                }

                // Only scroll if something needs scrolling
                if(maxScrollNeeded > 0) {
                    // Add pause at start (3 steps) when text is fresh
                    int pauseSteps = 3;
                    int totalCycle = maxScrollNeeded + pauseSteps;
                    int rawPos = pgnFieldScrollOffset % totalCycle;

                    // First few steps are pause (show position 0), then scroll
                    int scrollPos = (rawPos < pauseSteps) ? 0 : (rawPos - pauseSteps);

                    // Scroll title if needed (row 0)
                    if((int)pgnData.name.length() > 16) {
                        int titleScrollPos = min(scrollPos, (int)pgnData.name.length());
                        String title = pgnData.name.substring(titleScrollPos);
                        while((int)title.length() < 16) title += " ";
                        title = title.substring(0, 16);
                        screen->drawString(0, 0, title.c_str());
                        anyScrolled = true;
                    }

                    // Scroll field value portion only - keep labels fixed (rows 2-6)
                    row = 2;
                    for(int i = detailScrollOffset; i < (int)pgnData.fields.size() && row < 7; i++) {
                        PGNField& field = pgnData.fields[i];

                        // Calculate label portion and value portion
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

                        // Calculate available width for value after label
                        int valueAreaWidth = 16 - labelWidth;

                        // Only scroll if value portion is longer than available space
                        if((int)valueWithUnit.length() > valueAreaWidth && valueAreaWidth > 0) {
                            anyScrolled = true;
                            int fieldScrollPos = min(scrollPos, (int)valueWithUnit.length());

                            // Get remaining text after scroll position, pad with spaces
                            String scrolledValue = valueWithUnit.substring(fieldScrollPos);
                            while((int)scrolledValue.length() < valueAreaWidth) scrolledValue += " ";
                            scrolledValue = scrolledValue.substring(0, valueAreaWidth);

                            // Draw label at column 0 (fixed), then scrolled value
                            screen->drawString(0, row, label.c_str());
                            screen->drawString(labelWidth, row, scrolledValue.c_str());
                        }
                        row++;
                    }

                    // Increment offset for next cycle
                    if(anyScrolled) pgnFieldScrollOffset++;
                }
            }
        }
    }
    // -------------------------------------------------------------------------
    // Legacy PGN Monitoring Mode Updates
    // -------------------------------------------------------------------------
    // Handles the old-style PGN list and detail views (maintained for compatibility)
    else if(inSpecialMode && currentMenuID == MENU_SENSOR_READINGS) {
        std::vector<PGNInfo>& detectedPGNs = monitor->getDetectedPGNs();
        if(viewingPGNDetail) {
            // Viewing PGN detail - update values and display if changed
            if(currentTime - lastPGNUpdate > 100) {  // Update every 100ms
                lastPGNUpdate = currentTime;

                // Check if the displayed value changed
                if(selectedPGNIndex < (int)detectedPGNs.size()) {
                    double currentValue = detectedPGNs[selectedPGNIndex].value;
                    if(currentValue != lastDisplayedValue) {
                        lastDisplayedValue = currentValue;
                        // Only update the value line, not the whole screen
                        updatePGNValueDisplay(selectedPGNIndex);
                    }
                }
            }
        } else {
            // Viewing PGN list - check if new PGNs arrived
            if(currentTime - lastPGNUpdate > 500) {  // Check every 500ms
                lastPGNUpdate = currentTime;

                // Check if list size changed (new PGN detected)
                static size_t lastKnownSize = 0;
                if(detectedPGNs.size() != lastKnownSize) {
                    lastKnownSize = detectedPGNs.size();
                    displayPGNList();  // Redraw list with new PGN
                }
            }
        }
    }
    // -------------------------------------------------------------------------
    // Sensor Configuration Screen Updates
    // -------------------------------------------------------------------------
    // Handles live sensor value display and type name scrolling
    else if(currentMenuID == MENU_CONFIGURE_SENSOR1 ||
              currentMenuID == MENU_CONFIGURE_SENSOR2 ||
              currentMenuID == MENU_CONFIGURE_SENSOR3) {
        // Update sensor config display in real-time
        static unsigned long lastSensorUpdate = 0;

        // Update the sensor value display every 100ms
        if(currentTime - lastSensorUpdate > 100) {
            lastSensorUpdate = currentTime;
            int sensorNum = (currentMenuID == MENU_CONFIGURE_SENSOR1) ? 0 :
                           (currentMenuID == MENU_CONFIGURE_SENSOR2) ? 1 : 2;
            updateSensorValueOnly(sensorNum);
        }

        // Handle scrolling for long type names
        if(currentTime - lastScrollUpdate > SCROLL_DELAY_MS) {
            lastScrollUpdate = currentTime;

            // Get current sensor to check type name length
            Sensor* targetSensor = (currentMenuID == MENU_CONFIGURE_SENSOR1) ? sensor1 :
                                   (currentMenuID == MENU_CONFIGURE_SENSOR2) ? sensor2 : sensor3;

            if(targetSensor != nullptr) {
                String typeName = String(getSensorDisplayName((int)targetSensor->getMessageType()));

                if((int)typeName.length() > SCROLL_VISIBLE_CHARS) {
                    // Advance scroll position
                    typeScrollOffset++;
                    // Reset when we've scrolled through full text + padding
                    if(typeScrollOffset >= (int)typeName.length() + 3) {
                        typeScrollOffset = 0;
                    }

                    // Update just the type text line (row 3, column 5)
                    String scrollText = typeName + "   " + typeName;
                    String visible = scrollText.substring(typeScrollOffset, typeScrollOffset + SCROLL_VISIBLE_CHARS);
                    while((int)visible.length() < SCROLL_VISIBLE_CHARS) visible += " ";
                    screen->drawString(5, 3, visible.c_str());
                }
            }
        }
    }
    // -------------------------------------------------------------------------
    // Standard Menu Updates
    // -------------------------------------------------------------------------
    // Handles text scrolling for regular menu items
    else if(!inSpecialMode && currentMenu != nullptr) {
        currentMenu->updateScrollingText();
    }

    // -------------------------------------------------------------------------
    // Main Menu Attack Indicator
    // -------------------------------------------------------------------------
    // Shows persistent "!Attack Active" banner on main menu when attack is running
    if(currentMenuID == MENU_MAIN && attackController->isAttackActive()) {
        static unsigned long lastAttackIndicatorUpdate = 0;
        if(currentTime - lastAttackIndicatorUpdate > 500) {
            lastAttackIndicatorUpdate = currentTime;
            screen->setInverseFont(1);
            screen->drawString(0, 7, "!Attack Active  ");
            screen->setInverseFont(0);
        }
    }
}
