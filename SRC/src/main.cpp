/**
 * \file main.cpp
 * \brief Main entry point for NEMO (NMEA Educational Maritime Observatory) firmware.
 *
 *
 * The device uses two CAN interfaces:
 * - CAN1: Transmits simulated sensor data as multiple NMEA2000 devices
 * - CAN2: Listens to NMEA2000 traffic for monitoring and analysis
 *
 * \author Soups71
 
 */

#include<Arduino.h>
#include <NMEA2000.h>
#include <N2kMessages.h>
#include <NMEA2000_Teensyx.h>
#include "constants.h"
#include <PGN_Helpers.h>
#include <U8x8lib.h>
#include <U8g2lib.h>
#include <Menu.h>
#include <N2K_Monitor.h>
#include <Attack_Controller.h>
#include <Menu_Controller.h>
#include <Splash_Screen.h>
#include <Sensor.h>



//Primary CAN interface for transmitting simulated sensor data
tNMEA2000_Teensyx NMEA2000_CAN1(tNMEA2000_Teensyx::CAN1);

// Secondary CAN interface for listening to NMEA2000 traffic.
tNMEA2000_Teensyx NMEA2000_CAN2(tNMEA2000_Teensyx::CAN2);



//Interval in milliseconds between sensor updates.
#define UPDATE_INTERVAL 1000

//NMEA2000 manufacturer code for simulated devices.
#define DEVICE_MANUFACTURER_CODE 2046

//NMEA2000 device class for simulated devices.
#define DEVICE_CLASS 25

//Number of simulated devices on the NMEA2000 bus.
#define NUM_DEVICES 3

//Sequence counter for fast-packet frame fragmentation.
static uint8_t fastPacketSequence = 0;


//Timestamps of last button press for each button.
unsigned long lastButtonPress[4] = {0, 0, 0, 0};

//Debounce delay in milliseconds.
unsigned long debounceDelay = 250;

//Timestamp of last sensor update.
unsigned long lastUpdateTime = 0;

//Text-mode display driver for menu system.
U8X8_SH1106_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);

// Graphics-mode display driver for splash screen.
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

//Simulated Engine RPM sensor (Device 0).
Sensor sensor1(SENSOR_PIN_1, MSG_ENGINE_RPM, &NMEA2000_CAN1, 0);

//Simulated Water Depth sensor (Device 1).
Sensor sensor2(SENSOR_PIN_2, MSG_WATER_DEPTH, &NMEA2000_CAN1, 1);

//Simulated Heading sensor (Device 2).
Sensor sensor3(SENSOR_PIN_3, MSG_HEADING, &NMEA2000_CAN1, 2);


//NMEA2000 network monitor instance
N2K_Monitor* n2kMonitor;

//Attack demonstration controller instance.
Attack_Controller* attackController;

//Menu system controller instance.
Menu_Controller* menuController;


/**
 * \brief Handles incoming NMEA2000 messages from CAN2 interface.
 * \param N2kMsg Reference to the received NMEA2000 message.
 */
void HandleNMEA2000Msg(const tN2kMsg &N2kMsg);


/**
 * \brief Configures the NMEA2000 CAN1 interface and simulated devices.
 */
void setupNMEA2000();


/**
 * \brief Prints a single CAN frame in candump format to Serial.
 * \param canId The 29-bit extended CAN identifier.
 * \param len The number of data bytes in the frame (0-8).
 * \param data Pointer to the array of data bytes.
 */
void PrintCanFrame(uint32_t canId, uint8_t len, const uint8_t *data);

/**
 * \brief Prints an NMEA2000 message in candump format, handling fast-packet fragmentation.
 * \param N2kMsg Reference to the NMEA2000 message to print.
 */
void PrintCandumpFormat(const tN2kMsg &N2kMsg);

/**
 * \brief Maps a button pin to its debounce array index.
 * \param button The button pin number (BUTTON_UP, BUTTON_DOWN, BUTTON_LEFT, or BUTTON_RIGHT).
 * \return The array index (0-3) corresponding to the button.
 */
int getButtonIndex(int button);

/**
 * \brief Checks if a button has been pressed with debounce filtering.
 * \param button The button pin number to check.
 * \return 1 if button is pressed and debounce time has elapsed, 0 otherwise.
 */
short buttonPressed(int button);



/**
 * \brief Arduino setup function - initializes all hardware and software components.
 *
 * This function is called once at startup and performs complete system
 * initialization:
 *
 * Hardware initialization:
 * - Serial communication at 115200 baud
 * - Sensor input pins (analog)
 * - Button input pins with internal pull-up resistors
 *
 * NMEA2000 initialization:
 * - CAN1 interface for sensor transmission (via setupNMEA2000())
 * - CAN2 interface for listening/monitoring
 *
 * Display initialization:
 * - Splash screen animation on startup
 * - OLED display for menu system
 *
 * Controller initialization:
 * - N2K_Monitor for NMEA2000 traffic analysis
 * - Attack_Controller for educational demonstrations
 * - Menu_Controller for user interface
 */
void setup(void)
{
  Serial.begin(115200);

  pinMode(SENSOR_PIN_1, INPUT);
  pinMode(SENSOR_PIN_2, INPUT);
  pinMode(SENSOR_PIN_3, INPUT);

  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_LEFT, INPUT_PULLUP);
  pinMode(BUTTON_RIGHT, INPUT_PULLUP);

  setupNMEA2000();
  NMEA2000_CAN2.SetMsgHandler(HandleNMEA2000Msg);
  NMEA2000_CAN2.SetMode(tNMEA2000::N2km_ListenOnly);
  NMEA2000_CAN2.SetN2kCANReceiveFrameBufSize(2048);

  // Open both CAN interfaces
  NMEA2000_CAN2.Open();

  // Show splash screen animation
  Splash_Screen::show(&u8g2);

  // Initialize display for menu system
  u8x8.begin();
  u8x8.setPowerSave(0);

  // Initialize NMEA2000 Monitor
  n2kMonitor = new N2K_Monitor();

  // Initialize Attack Controller
  attackController = new Attack_Controller(&NMEA2000_CAN1, n2kMonitor, &sensor1);

  // Initialize Menu Controller
  menuController = new Menu_Controller(&u8x8,
                                      BUTTON_UP, BUTTON_DOWN,
                                      BUTTON_LEFT, BUTTON_RIGHT,
                                      &sensor1, &sensor2, &sensor3,
                                      n2kMonitor, attackController);
  menuController->begin();

#if DEBUG
  Serial.println("System initialized");
#endif
}

/**
 * \brief Arduino main loop - executes continuously after setup().
 *
 * This function implements the main program loop with the following
 * responsibilities:
 *
 * Sensor updates (every UPDATE_INTERVAL milliseconds):
 * - Normal operation: Update and transmit all sensor values
 * - Own-sensor impersonation: Continue normal transmissions alongside attack
 * - External attack: Only update sensor1 for potentiometer control
 *
 * CAN bus processing:
 * - Parse CAN1 messages (skipped during active attacks)
 * - Parse CAN2 messages for monitoring
 *
 * User interface:
 * - Update menu controller for real-time displays
 * - Process button inputs for navigation
 *
 * Button actions:
 * - UP: Navigate up in menu
 * - DOWN: Navigate down in menu
 * - LEFT: Navigate back/cancel
 * - RIGHT: Select/enter
 */

void loop(void)
{
  unsigned long currentTime = millis();

  // Update sensors at regular intervals
  if (currentTime - lastUpdateTime >= UPDATE_INTERVAL) {
    lastUpdateTime = currentTime;

    bool attackActive = attackController->isAttackActive();
    bool impersonatingOwn = attackController->isImpersonatingOwnSensor();

    if (!attackActive) {
      // Normal operation - update and send all sensors
      sensor1.update();
      sensor2.update();
      sensor3.update();

      // Send NMEA messages
      sensor1.sendMessage();
      sensor2.sendMessage();
      sensor3.sendMessage();

    } else if (impersonatingOwn) {
      // Impersonating own sensor - continue real transmissions for all sensors
      // This creates both real data (from sensors) and spoofed data (from attack)
      // User can see effect in Live Data with interleaved real/spoofed values
      sensor1.update();
      sensor2.update();
      sensor3.update();

      // Send messages from all sensors so Live Data shows real values
      sensor1.sendMessage();
      sensor2.sendMessage();
      sensor3.sendMessage();

    } else {
      // External attack (not impersonating own sensor) - only update sensor1 for potentiometer control
      sensor1.update();
    }
  }

  // Keep parsing both buses
  // Skip CAN1 parsing during attacks to prevent library from maintaining attack state
  if (!menuController->isAttackActive()) {
    NMEA2000_CAN1.ParseMessages();
  }
  NMEA2000_CAN2.ParseMessages();

  // Update menu controller (for real-time displays)
  menuController->update();

  // Handle button inputs
  if(buttonPressed(BUTTON_UP)){
    menuController->navigateUp();
  } else if(buttonPressed(BUTTON_DOWN)){
    menuController->navigateDown();
  } else if(buttonPressed(BUTTON_LEFT)){
    menuController->navigateBack();
  } else if(buttonPressed(BUTTON_RIGHT)){
    menuController->navigateSelect();
  }
}

void setupNMEA2000() {
  // Configure multi-device mode - each sensor appears as its own device on the bus
  NMEA2000_CAN1.SetDeviceCount(NUM_DEVICES);

  NMEA2000_CAN1.SetMode(tNMEA2000::N2km_NodeOnly, 22);

  // Set product info for all sensors BEFORE Open()
  // Use updateDeviceInfo() directly to avoid SendIsoAddressClaim before bus is open
  sensor1.updateDeviceInfo();
  sensor2.updateDeviceInfo();
  sensor3.updateDeviceInfo();

  // Now open - this will initialize the CAN bus
  NMEA2000_CAN1.Open();

  // Apply initial active/inactive state for each sensor
  // Sensors default to inactive, so set them to null address and disable heartbeat
  Sensor* sensors[] = {&sensor1, &sensor2, &sensor3};
  for (int i = 0; i < NUM_DEVICES; i++) {
    if (sensors[i]->isActive()) {
      // Active sensor - set custom name and broadcast info
      char name[20];
      snprintf(name, sizeof(name), "Sensor %d", i + 1);
      sensors[i]->setCustomName(name);
      delay(10);
      NMEA2000_CAN1.SendProductInformation(i);
      delay(10);
    } else {
      // Inactive sensor - set to null address (254) and disable heartbeat
      NMEA2000_CAN1.SetHeartbeatIntervalAndOffset(0, 0, i);
      NMEA2000_CAN1.SetN2kSource(254, i);  // Null address - removes from bus
    }
  }
}

void HandleNMEA2000Msg(const tN2kMsg &N2kMsg) {
#if !DEBUG
  // Print candump-style output for all received messages
  PrintCandumpFormat(N2kMsg);
#endif

  if(attackController != nullptr && attackController -> isSpamActive()) {
    attackController->attackHandler(N2kMsg);
  }
  // Handle incoming NMEA2000 messages directly
  if(n2kMonitor != nullptr && !attackController -> isSpamActive()) {
    n2kMonitor->handleN2kMessage(N2kMsg);
  }
}

short buttonPressed(int button){
  int idx = getButtonIndex(button);
  if(!digitalRead(button) && (millis() - lastButtonPress[idx]) > debounceDelay){
    lastButtonPress[idx] = millis();
    return 1;
  }else{
    return 0;
  }
}

int getButtonIndex(int button) {
  if (button == BUTTON_UP) return 0;
  if (button == BUTTON_DOWN) return 1;
  if (button == BUTTON_LEFT) return 2;
  if (button == BUTTON_RIGHT) return 3;
  return 0;
}

void PrintCandumpFormat(const tN2kMsg &N2kMsg) {
  // Reconstruct CAN ID: Priority (3 bits) | PGN (18 bits) | Source (8 bits)
  uint32_t canId = ((uint32_t)N2kMsg.Priority << 26) | ((uint32_t)N2kMsg.PGN << 8) | N2kMsg.Source;

  if (N2kMsg.DataLen <= 8) {
    // Single frame message - print directly
    PrintCanFrame(canId, N2kMsg.DataLen, N2kMsg.Data);
  } else {
    // Fast-packet message - re-fragment into 8-byte CAN frames
    uint8_t frame[8];
    int dataOffset = 0;
    int frameCount = 0;
    uint8_t seqId = (fastPacketSequence & 0x07) << 5;  // Sequence ID in upper 3 bits of counter byte

    while (dataOffset < N2kMsg.DataLen) {
      if (frameCount == 0) {
        // First frame: [SeqID + FrameNum] [TotalLen] [6 data bytes]
        frame[0] = seqId | (frameCount & 0x1F);
        frame[1] = N2kMsg.DataLen;
        int bytesToCopy = min(6, (int)N2kMsg.DataLen - dataOffset);
        for (int i = 0; i < bytesToCopy; i++) {
          frame[2 + i] = N2kMsg.Data[dataOffset + i];
        }
        // Pad remaining bytes with 0xFF
        for (int i = bytesToCopy; i < 6; i++) {
          frame[2 + i] = 0xFF;
        }
        dataOffset += bytesToCopy;
        PrintCanFrame(canId, 8, frame);
      } else {
        // Subsequent frames: [SeqID + FrameNum] [7 data bytes]
        frame[0] = seqId | (frameCount & 0x1F);
        int bytesToCopy = min(7, (int)N2kMsg.DataLen - dataOffset);
        for (int i = 0; i < bytesToCopy; i++) {
          frame[1 + i] = N2kMsg.Data[dataOffset + i];
        }
        // Pad remaining bytes with 0xFF
        for (int i = bytesToCopy; i < 7; i++) {
          frame[1 + i] = 0xFF;
        }
        dataOffset += bytesToCopy;
        PrintCanFrame(canId, 8, frame);
      }
      frameCount++;
    }

    fastPacketSequence++;
  }
}


void PrintCanFrame(uint32_t canId, uint8_t len, const uint8_t *data) {
  Serial.print("can1  ");
  if (canId < 0x10000000) Serial.print("0");
  if (canId < 0x1000000) Serial.print("0");
  if (canId < 0x100000) Serial.print("0");
  if (canId < 0x10000) Serial.print("0");
  if (canId < 0x1000) Serial.print("0");
  if (canId < 0x100) Serial.print("0");
  if (canId < 0x10) Serial.print("0");
  Serial.print(canId, HEX);

  Serial.print("   [");
  Serial.print(len);
  Serial.print("]  ");

  for (int i = 0; i < len; i++) {
    if (data[i] < 0x10) Serial.print("0");
    Serial.print(data[i], HEX);
    if (i < len - 1) Serial.print(" ");
  }
  Serial.println();
}