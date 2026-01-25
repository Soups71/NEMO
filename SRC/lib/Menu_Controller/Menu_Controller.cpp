/**
 * @file Menu_Controller.cpp
 * @brief Core functionality for the Menu_Controller class including constructor,
 *        destructor, and menu initialization.
 *
 * This file contains the main entry points for the Menu_Controller system,
 * which manages the user interface for the NEMO NMEA2000 research device.
 * The Menu_Controller coordinates between the OLED display, button inputs,
 * sensors, network monitoring, and attack functionality.
 */

#include "Menu_Controller.h"

// Initialize static instance pointer
Menu_Controller* Menu_Controller::instance = nullptr;

/**
 * @brief Constructs a new Menu_Controller object.
 *
 * Initializes all member variables, sets up references to external components
 * (display, buttons, sensors, monitor, attack controller), and creates the
 * menu hierarchy. The static instance pointer is set for callback functions.
 *
 * @param u8x8 Pointer to the OLED display driver
 * @param upBtn Pin number for the up navigation button
 * @param downBtn Pin number for the down navigation button
 * @param leftBtn Pin number for the left/back navigation button
 * @param rightBtn Pin number for the right/select navigation button
 * @param s1 Pointer to Sensor 1 object
 * @param s2 Pointer to Sensor 2 object
 * @param s3 Pointer to Sensor 3 object
 * @param mon Pointer to the N2K_Monitor for network monitoring
 * @param attk Pointer to the Attack_Controller for attack functionality
 */
Menu_Controller::Menu_Controller(U8X8_SH1106_128X64_NONAME_HW_I2C* u8x8,
                                 int upBtn, int downBtn, int leftBtn, int rightBtn,
                                 Sensor* s1, Sensor* s2, Sensor* s3,
                                 N2K_Monitor* mon, Attack_Controller* attk) {
    // Screen and buttons
    screen = u8x8;
    btnUp = upBtn;
    btnDown = downBtn;
    btnLeft = leftBtn;
    btnRight = rightBtn;

    // Sensors and external libraries
    sensor1 = s1;
    sensor2 = s2;
    sensor3 = s3;
    monitor = mon;
    attackController = attk;

    // Menu state
    currentMenuID = MENU_MAIN;
    menuStackPointer = 0;
    inSpecialMode = false;
    currentSensorBeingConfigured = 0;

    // Device/PGN navigation state
    selectedDeviceIndex = 0;
    selectedPGNIndex = 0;
    detailScrollOffset = 0;
    currentDeviceAddress = 0;
    currentPGN = 0;
    lastPGNUpdate = 0;
    detailViewInitialized = false;
    impFieldSelectInitialized = false;
    viewingPGNDetail = false;
    lastDisplayedValue = 0.0;

    // Scrolling state
    typeScrollOffset = 0;
    lastScrollUpdate = 0;
    deviceListScrollOffset = 0;
    lastDeviceScrollUpdate = 0;
    pgnFieldScrollOffset = 0;
    lastPGNFieldScrollUpdate = 0;

    // Manufacturer selection
    selectedManufacturerIndex = 0;

    // Impersonate attack navigation state
    impDeviceScrollIndex = 0;
    impPGNScrollIndex = 0;

    // About menu navigation state
    aboutPGNScrollIndex = 0;

    // Display update tracking for spam attack
    lastSpamDisplayUpdate = 0;
    spamActiveInitialized = false;

    // Display update tracking for attack status screen
    attackStatusInitialized = false;
    attackStatusScrollOffset = 0;
    lastAttackStatusScrollUpdate = 0;

    // Set static instance for callbacks
    instance = this;

    // Initialize displayed lines tracking
    for (int i = 0; i < 8; i++) {
        displayedLines[i] = "";
    }

    initializeMenus();
}

/**
 * @brief Destroys the Menu_Controller object and frees allocated memory.
 *
 * Cleans up all dynamically allocated Menu objects created during initialization.
 * This includes the main menu, sensor configuration menus, attack menus, and
 * PGN type selection menus.
 */
Menu_Controller::~Menu_Controller() {
    // Clean up dynamically allocated menus
    delete mainMenu;
    delete sensorReadingsMenu;
    delete configureMenu;
    delete configureSensor1Menu;
    delete configureSensor2Menu;
    delete configureSensor3Menu;
    delete attacksMenu;
    delete aboutMenu;
    delete deviceConfigMenu;
    delete manufacturerMenu;
    for(int i = 0; i < 3; i++) {
        delete pgnTypeMenus[i];
    }
}

/**
 * @brief Prepares the screen for drawing by clearing and resetting font settings.
 *
 * Clears the display, moves cursor to home position, sets the default font,
 * and disables inverse font mode. Should be called before drawing a new screen.
 */
void Menu_Controller::prepScreen(){
    screen->clear();
    screen->home();
    screen->setFont(u8x8_font_artossans8_r);
    screen->setInverseFont(0);
}

/**
 * @brief Initializes all menu structures and their options.
 *
 * Creates and configures all Menu objects for the user interface including:
 * - Main menu with Live Data, Attacks, Configure, and About options
 * - Configure menu with sensor and device configuration options
 * - Device configuration menu for stale cleanup settings
 * - Manufacturer selection menu populated from MANUFACTURERS constant
 * - Attacks menu with DOS Attack and Impersonate options
 * - About menu with Info and Supported PGNs options
 * - Sensor configuration menus for all three sensors
 * - PGN type selection menus populated from SENSOR_DEFS constant
 * - Sensor readings menu (populated dynamically)
 *
 * Each menu option is associated with a callback function for selection handling.
 */
void Menu_Controller::initializeMenus() {
    // Initialize main menu - keep strings short (max ~13 chars for option text with " * " prefix)
    mainChoices[0] = {String("Live Data"), callback_SensorReadings};
    mainChoices[1] = {String("Attacks"), callback_Attacks};
    mainChoices[2] = {String("Configure"), callback_Configure};
    mainChoices[3] = {String("About"), callback_About};
    mainMenu = new Menu(screen, String("MAIN MENU"), mainChoices, mainChoicesNum, 1);

    // Initialize configure menu
    configureChoices[0] = {String("Sensor 1"), callback_ConfigSensor1};
    configureChoices[1] = {String("Sensor 2"), callback_ConfigSensor2};
    configureChoices[2] = {String("Sensor 3"), callback_ConfigSensor3};
    configureChoices[3] = {String("Device Config"), callback_DeviceConfig};
    configureMenu = new Menu(screen, String("CONFIGURE"), configureChoices, configureChoicesNum, 1);

    // Initialize device config menu
    deviceConfigChoices[0] = {String("Stale Cleanup"), callback_StaleCleanupToggle};
    deviceConfigMenu = new Menu(screen, String("DEVICE CONFIG"), deviceConfigChoices, deviceConfigChoicesNum, 1);

    // Initialize manufacturer selection menu
    for(int i = 0; i < MANUFACTURER_COUNT; i++) {
        manufacturerChoices[i] = {String(MANUFACTURERS[i].name), nullptr};
    }
    manufacturerMenu = new Menu(screen, String("MANUFACTURER"), manufacturerChoices, MANUFACTURER_COUNT, 1);

    // Initialize attacks menu
    attacksChoices[0] = {String("DOS Attack"), callback_SpamAttack};
    attacksChoices[1] = {String("Impersonate"), callback_Impersonate};
    attacksMenu = new Menu(screen, String("ATTACKS"), attacksChoices, attacksChoicesNum, 1);

    // Initialize about menu
    aboutChoices[0] = {String("Info"), callback_AboutInfo};
    aboutChoices[1] = {String("Supported PGNs"), callback_AboutPGNs};
    aboutMenu = new Menu(screen, String("ABOUT"), aboutChoices, aboutChoicesNum, 1);

    // Initialize sensor config menus (same structure for all 3)
    // Order: Manufacturer, Device Type, Active
    // Check actual sensor state for initial display
    sensorConfigChoices[0] = {String("Manufacturer"), callback_Sensor1Manufacturer};
    sensorConfigChoices[1] = {String("Device Type"), callback_Sensor1PGNType};
    sensorConfigChoices[2] = {String(sensor1->isActive() ? "Active: YES" : "Active: NO"), callback_Sensor1Active};
    configureSensor1Menu = new Menu(screen, String("SENSOR 1"), sensorConfigChoices, sensorConfigChoicesNum, 1);

    sensor2ConfigChoices[0] = {String("Manufacturer"), callback_Sensor2Manufacturer};
    sensor2ConfigChoices[1] = {String("Device Type"), callback_Sensor2PGNType};
    sensor2ConfigChoices[2] = {String(sensor2->isActive() ? "Active: YES" : "Active: NO"), callback_Sensor2Active};
    configureSensor2Menu = new Menu(screen, String("SENSOR 2"), sensor2ConfigChoices, sensor2ConfigChoicesNum, 1);

    sensor3ConfigChoices[0] = {String("Manufacturer"), callback_Sensor3Manufacturer};
    sensor3ConfigChoices[1] = {String("Device Type"), callback_Sensor3PGNType};
    sensor3ConfigChoices[2] = {String(sensor3->isActive() ? "Active: YES" : "Active: NO"), callback_Sensor3Active};
    configureSensor3Menu = new Menu(screen, String("SENSOR 3"), sensor3ConfigChoices, sensor3ConfigChoicesNum, 1);

    // Initialize PGN type selection menus - use SENSOR_DEFS from constants.h
    for(uint8_t i = 0; i < SENSOR_COUNT; i++) {
        pgnTypeChoices[i] = {String(SENSOR_DEFS[i].displayName), nullptr};
    }
    pgnTypeMenus[0] = new Menu(screen, String("SELECT PGN"), pgnTypeChoices, pgnTypeChoicesNum, 1);
    pgnTypeMenus[1] = new Menu(screen, String("SELECT PGN"), pgnTypeChoices, pgnTypeChoicesNum, 1);
    pgnTypeMenus[2] = new Menu(screen, String("SELECT PGN"), pgnTypeChoices, pgnTypeChoicesNum, 1);

    // Initialize sensor readings menu (will be populated dynamically)
    sensorReadingsMenu = new Menu(screen, String("LIVE DATA"), nullptr, 0, 1);

    // Set current menu
    currentMenu = mainMenu;
}

/**
 * @brief Starts the menu system by displaying the initial menu.
 *
 * Should be called once after construction to display the main menu
 * and begin user interaction.
 */
void Menu_Controller::begin() {
    currentMenu->printMenu();
}

/**
 * @brief Handles button input events.
 *
 * This method is called from the main loop when buttons are pressed.
 * The actual button reading and debouncing happens in main.cpp using
 * buttonPressed(), which then calls the appropriate navigation methods.
 *
 * @note This is a placeholder for potential future input handling logic.
 */
void Menu_Controller::handleInput() {
    // This will be called from main loop when buttons are pressed
    // The actual button reading happens in main.cpp using buttonPressed()
}
