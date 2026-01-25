/**
 * @file Menu_Controller.h
 * @brief Menu controller for the NEMO device user interface.
 *
 * This file defines the Menu_Controller class which manages all menu navigation,
 * display rendering, and user interaction for the NEMO NMEA2000 research device.
 * The controller handles multiple menu types including standard lists, PGN selectors,
 * toggle menus, information displays, and real-time PGN monitors.
 */

#ifndef MENU_CONTROLLER_H
#define MENU_CONTROLLER_H

#include <Arduino.h>
#include <U8x8lib.h>
#include <Menu.h>
#include <N2kMessages.h>
#include <Sensor.h>
#include <NMEA2000_Teensyx.h>
#include "constants.h"
#include <PGN_Helpers.h>
#include <N2K_Monitor.h>
#include <Attack_Controller.h>

/*
 *                              Forward Declarations
*/

class Sensor;
class tNMEA2000_Teensyx;

/*
 *                              Enumerations
*/

/**
 * @enum MenuType
 * @brief Defines the different behavioral types of menus in the system.
 *
 * Each menu type determines how the menu responds to user input and
 * how its content is rendered on the display.
 */
enum MenuType {
    MENU_LIST,           ///< Standard menu with selectable options
    MENU_PGN_SELECTOR,   ///< Special menu for selecting PGN type from available sensors
    MENU_TOGGLE,         ///< Toggle menu switching between Active/Inactive states
    MENU_INFO_DISPLAY,   ///< Display-only menu for static information (e.g., About screen)
    MENU_PGN_MONITOR     ///< Real-time PGN value display with live updates
};

/**
 * @enum MenuID
 * @brief Unique identifiers for each menu screen in the navigation system.
 *
 * These IDs are used by the menu stack to track navigation history and
 * enable proper back navigation throughout the menu hierarchy.
 */
enum MenuID {
    MENU_MAIN,                  ///< Main menu - top level navigation
    MENU_SENSOR_READINGS,       ///< Display current sensor values
    MENU_DEVICE_LIST,           ///< List of devices discovered on the NMEA2000 network
    MENU_DEVICE_PGNS,           ///< PGNs received from the selected device
    MENU_PGN_DETAIL,            ///< Detailed view of selected PGN with field values
    MENU_CONFIGURE,             ///< Configuration submenu for sensors and device
    MENU_CONFIGURE_SENSOR1,     ///< Configuration options for sensor 1
    MENU_CONFIGURE_SENSOR2,     ///< Configuration options for sensor 2
    MENU_CONFIGURE_SENSOR3,     ///< Configuration options for sensor 3
    MENU_ATTACKS,               ///< Attack demonstration menu
    MENU_ABOUT,                 ///< About menu with device information
    MENU_SENSOR1_PGN_TYPE,      ///< PGN type selection for sensor 1
    MENU_SENSOR1_ACTIVE,        ///< Active/Inactive toggle for sensor 1
    MENU_SENSOR2_PGN_TYPE,      ///< PGN type selection for sensor 2
    MENU_SENSOR2_ACTIVE,        ///< Active/Inactive toggle for sensor 2
    MENU_SENSOR3_PGN_TYPE,      ///< PGN type selection for sensor 3
    MENU_SENSOR3_ACTIVE,        ///< Active/Inactive toggle for sensor 3
    MENU_SPAM_CONFIG,           ///< Configure spam attack device count
    MENU_SPAM_ACTIVE,           ///< Spam attack running status display
    MENU_IMP_DEVICE_SELECT,     ///< Select device to impersonate
    MENU_IMP_PGN_SELECT,        ///< Select PGN to spoof from target device
    MENU_IMP_FIELD_SELECT,      ///< Select and modify field values for impersonation
    MENU_DEVICE_CONFIG,         ///< Device-level configuration options
    MENU_STALE_CLEANUP,         ///< Toggle for automatic stale device cleanup
    MENU_MANUFACTURER_SELECT,   ///< Manufacturer code selection for sensors
    MENU_ABOUT_INFO,            ///< About information page with device details
    MENU_ABOUT_PGNS,            ///< List of supported PGNs
    MENU_ATTACK_STATUS          ///< Shows active attack status with stop option
};

/*
 *                              Menu_Controller Class
*/

/**
 * @class Menu_Controller
 * @brief Central controller for the NEMO device menu system and user interface.
 *
 * The Menu_Controller manages all aspects of the user interface including:
 * - Menu navigation with history stack for back navigation
 * - Display rendering on the SH1106 OLED screen
 * - Button input handling for navigation
 * - Real-time updates for live data displays
 * - Integration with sensors, network monitor, and attack controller
 *
 * The controller uses a singleton-like pattern with a static instance pointer
 * to enable static callback functions required by the Menu library's function
 * pointer interface.
 *
 * @note The controller assumes a 128x64 pixel OLED display organized as 8 rows
 *       of 8-pixel-high characters.
 */
class Menu_Controller {

/*
 *                              Private Members
*/
private:
    /* ------------------------------------------------------------------------
     * Display and Menu State
     * ------------------------------------------------------------------------ */

    U8X8_SH1106_128X64_NONAME_HW_I2C* screen;  ///< Pointer to the OLED display driver
    Menu* currentMenu;                          ///< Currently active menu object
    MenuID currentMenuID;                       ///< ID of the current menu
    MenuID previousMenuID;                      ///< ID of the previous menu (for transitions)

    /* ------------------------------------------------------------------------
     * Navigation Stack
     * ------------------------------------------------------------------------ */

    static const int MAX_MENU_DEPTH = 10;       ///< Maximum depth of menu navigation stack
    MenuID menuStack[MAX_MENU_DEPTH];           ///< Stack storing menu navigation history
    int menuStackPointer;                       ///< Current position in the menu stack

    /* ------------------------------------------------------------------------
     * Menu Objects
     * ------------------------------------------------------------------------ */

    Menu* mainMenu;              ///< Main menu object
    Menu* sensorReadingsMenu;    ///< Sensor readings display menu
    Menu* configureMenu;         ///< Configuration submenu
    Menu* configureSensor1Menu;  ///< Sensor 1 configuration menu
    Menu* configureSensor2Menu;  ///< Sensor 2 configuration menu
    Menu* configureSensor3Menu;  ///< Sensor 3 configuration menu
    Menu* attacksMenu;           ///< Attack demonstrations menu
    Menu* aboutMenu;             ///< About information menu
    Menu* deviceConfigMenu;      ///< Device configuration menu
    Menu* manufacturerMenu;      ///< Manufacturer selection menu

    Menu* pgnTypeMenus[3];       ///< PGN type selection menus (one per sensor)

    /* ------------------------------------------------------------------------
     * Menu Choice Counts
     * ------------------------------------------------------------------------ */

    const static int mainChoicesNum = 4;              ///< Number of main menu options
    const static int configureChoicesNum = 4;         ///< Number of configure menu options (Sensor1, Sensor2, Sensor3, Device Config)
    const static int sensorConfigChoicesNum = 3;      ///< Number of sensor 1 config options (Change Type, Active, Manufacturer)
    const static int sensor2ConfigChoicesNum = 3;     ///< Number of sensor 2 config options
    const static int sensor3ConfigChoicesNum = 3;     ///< Number of sensor 3 config options
    const static int attacksChoicesNum = 2;           ///< Number of attack menu options
    const static int aboutChoicesNum = 2;             ///< Number of about menu options
    const static int pgnTypeChoicesNum = SENSOR_COUNT; ///< Number of PGN types (from constants.h)
    const static int deviceConfigChoicesNum = 1;      ///< Number of device config options (stale cleanup toggle)
    const static int manufacturerChoicesNum = MANUFACTURER_COUNT; ///< Number of manufacturer options

    /* ------------------------------------------------------------------------
     * Menu Choice Arrays
     * ------------------------------------------------------------------------ */

    FunctionStruct mainChoices[mainChoicesNum];               ///< Main menu function structures
    FunctionStruct configureChoices[configureChoicesNum];     ///< Configure menu function structures
    FunctionStruct sensorConfigChoices[sensorConfigChoicesNum];   ///< Sensor 1 config function structures
    FunctionStruct sensor2ConfigChoices[sensor2ConfigChoicesNum]; ///< Sensor 2 config function structures
    FunctionStruct sensor3ConfigChoices[sensor3ConfigChoicesNum]; ///< Sensor 3 config function structures
    FunctionStruct attacksChoices[attacksChoicesNum];         ///< Attacks menu function structures
    FunctionStruct aboutChoices[aboutChoicesNum];             ///< About menu function structures
    FunctionStruct pgnTypeChoices[pgnTypeChoicesNum];         ///< PGN type selection function structures
    FunctionStruct deviceConfigChoices[deviceConfigChoicesNum];   ///< Device config function structures
    FunctionStruct manufacturerChoices[manufacturerChoicesNum];   ///< Manufacturer selection function structures

    /* ------------------------------------------------------------------------
     * Sensor References
     * ------------------------------------------------------------------------ */

    Sensor* sensor1;  ///< Pointer to sensor 1 object (from main.cpp)
    Sensor* sensor2;  ///< Pointer to sensor 2 object (from main.cpp)
    Sensor* sensor3;  ///< Pointer to sensor 3 object (from main.cpp)

    /* ------------------------------------------------------------------------
     * External Controller References
     * ------------------------------------------------------------------------ */

    N2K_Monitor* monitor;              ///< NMEA2000 network monitor for device/PGN tracking
    Attack_Controller* attackController; ///< Attack controller for research demonstrations

    /* ------------------------------------------------------------------------
     * Device/PGN Navigation State
     * ------------------------------------------------------------------------ */

    int selectedDeviceIndex;            ///< Index of selected device in device list
    int selectedPGNIndex;               ///< Index of selected PGN in PGN list
    int detailScrollOffset;             ///< Scroll position in PGN detail view
    uint8_t currentDeviceAddress;       ///< NMEA2000 address of currently selected device
    uint32_t currentPGN;                ///< Currently selected PGN number
    unsigned long lastPGNUpdate;        ///< Timestamp of last PGN value update

    /* ------------------------------------------------------------------------
     * Display Optimization State
     * ------------------------------------------------------------------------ */

    String displayedLines[8];           ///< Cache of currently displayed text on each row
    bool detailViewInitialized;         ///< Flag indicating if detail view has been fully drawn
    bool impFieldSelectInitialized;     ///< Flag indicating if impersonate field select is drawn

    /* ------------------------------------------------------------------------
     * Legacy PGN Display State
     * ------------------------------------------------------------------------ */

    bool viewingPGNDetail;              ///< Flag tracking if viewing a PGN detail screen
    double lastDisplayedValue;          ///< Last displayed PGN value for change detection

    /* ------------------------------------------------------------------------
     * Button Configuration
     * ------------------------------------------------------------------------ */

    int btnUp;     ///< GPIO pin number for up button
    int btnDown;   ///< GPIO pin number for down button
    int btnLeft;   ///< GPIO pin number for left/back button
    int btnRight;  ///< GPIO pin number for right/select button

    /* ------------------------------------------------------------------------
     * Menu State Tracking
     * ------------------------------------------------------------------------ */

    int currentSensorBeingConfigured;  ///< Index of sensor being configured (0, 1, or 2)
    bool inSpecialMode;                ///< Flag indicating special mode (PGN monitor or attack)

    /* ------------------------------------------------------------------------
     * Sensor Config Display Scrolling
     * ------------------------------------------------------------------------ */

    int typeScrollOffset;              ///< Current scroll position for sensor type name
    unsigned long lastScrollUpdate;    ///< Timestamp of last scroll position change

    /* ------------------------------------------------------------------------
     * Device List Display Scrolling
     * ------------------------------------------------------------------------ */

    int deviceListScrollOffset;            ///< Current scroll position for device name
    unsigned long lastDeviceScrollUpdate;  ///< Timestamp of last device scroll change

    /* ------------------------------------------------------------------------
     * PGN Detail Field Scrolling
     * ------------------------------------------------------------------------ */

    int pgnFieldScrollOffset;              ///< Current scroll position for PGN field text
    unsigned long lastPGNFieldScrollUpdate; ///< Timestamp of last PGN field scroll change

    /* ------------------------------------------------------------------------
     * Manufacturer Configuration State
     * ------------------------------------------------------------------------ */

    int selectedManufacturerIndex;     ///< Currently selected manufacturer in menu

    /* ------------------------------------------------------------------------
     * Impersonate Attack Navigation State
     * ------------------------------------------------------------------------ */

    int impDeviceScrollIndex;              ///< Scroll position for device selection
    int impPGNScrollIndex;                 ///< Scroll position for PGN selection
    std::vector<uint8_t> impDeviceList;    ///< Filtered list of devices with impersonatable PGNs

    /* ------------------------------------------------------------------------
     * About Menu Navigation State
     * ------------------------------------------------------------------------ */

    int aboutPGNScrollIndex;           ///< Scroll position for supported PGNs list

    /* ------------------------------------------------------------------------
     * Spam Attack Display State
     * ------------------------------------------------------------------------ */

    unsigned long lastSpamDisplayUpdate;   ///< Timestamp of last spam display update
    bool spamActiveInitialized;            ///< Flag indicating if spam active screen is drawn

    /* ------------------------------------------------------------------------
     * Attack Status Display State
     * ------------------------------------------------------------------------ */

    bool attackStatusInitialized;              ///< Flag indicating if attack status screen is drawn
    int attackStatusScrollOffset;              ///< Scroll position for long text in attack status
    unsigned long lastAttackStatusScrollUpdate; ///< Timestamp of last scroll change

    /* ========================================================================
     *                          Private Helper Methods
     * ======================================================================== */

    /* ------------------------------------------------------------------------
     * Initialization and Display Helpers
     * ------------------------------------------------------------------------ */

    /**
     * @brief Initializes all menu objects and their option arrays.
     *
     * Creates menu structures, populates choice arrays with labels and
     * callbacks, and establishes the menu hierarchy.
     */
    void initializeMenus();

    /**
     * @brief Prepares the screen for drawing.
     *
     * Clears the display and resets display state variables.
     */
    void prepScreen();

    /* ------------------------------------------------------------------------
     * Navigation Stack Helpers
     * ------------------------------------------------------------------------ */

    /**
     * @brief Pushes a menu ID onto the navigation stack.
     * @param menuID The menu ID to push onto the stack.
     *
     * Enables back navigation by tracking menu history.
     */
    void pushMenu(MenuID menuID);

    /**
     * @brief Pops and returns the top menu ID from the navigation stack.
     * @return The menu ID that was on top of the stack.
     */
    MenuID popMenu();

    /**
     * @brief Changes to a new menu and updates the display.
     * @param newMenuID The ID of the menu to change to.
     *
     * Handles all state changes required for menu transitions.
     */
    void changeMenu(MenuID newMenuID);

    /* ------------------------------------------------------------------------
     * Sensor Configuration Display Helpers
     * ------------------------------------------------------------------------ */

    /**
     * @brief Displays the configuration screen for a sensor.
     * @param sensorNum The sensor number (1, 2, or 3) to display config for.
     */
    void displaySensorConfig(int sensorNum);

    /**
     * @brief Updates the sensor configuration display with current values.
     * @param sensorNum The sensor number (1, 2, or 3) to update.
     */
    void updateSensorConfigDisplay(int sensorNum);

    /**
     * @brief Updates only the sensor value portion of the display.
     * @param sensorNum The sensor number (1, 2, or 3) to update.
     *
     * Optimized update that avoids redrawing static elements.
     */
    void updateSensorValueOnly(int sensorNum);

    /* ------------------------------------------------------------------------
     * Device-Centric Live Data Display Methods
     * ------------------------------------------------------------------------ */

    /**
     * @brief Displays the list of discovered NMEA2000 network devices.
     */
    void displayDeviceList();

    /**
     * @brief Displays PGNs received from the selected device.
     */
    void displayDevicePGNs();

    /**
     * @brief Displays detailed field values for the selected PGN.
     */
    void displayPGNDetail();

    /**
     * @brief Updates only changed values in the PGN detail view.
     *
     * Optimized update that avoids full screen redraw.
     */
    void updatePGNDetailValues();

    /**
     * @brief Draws a line of text only if it differs from what is displayed.
     * @param row The display row (0-7) to draw on.
     * @param text The text to display.
     *
     * Prevents flicker by only updating changed content.
     */
    void drawLine(int row, const String& text);

    /**
     * @brief Gets the number of fields in the current PGN.
     * @return The number of fields in the currently selected PGN.
     */
    int getPGNFieldCount();

    /**
     * @brief Displays the device configuration screen.
     */
    void displayDeviceConfig();

    /**
     * @brief Displays the manufacturer selection screen.
     */
    void displayManufacturerSelect();

    /**
     * @brief Sets the manufacturer code for a specific sensor.
     * @param sensorNum The sensor number (0, 1, or 2).
     * @param code The NMEA2000 manufacturer code to set.
     */
    void setManufacturerCode(int sensorNum, uint16_t code);

    /* ------------------------------------------------------------------------
     * Attack Display Methods
     * ------------------------------------------------------------------------ */

    /**
     * @brief Displays the spam attack configuration screen.
     */
    void displaySpamConfig();

    /**
     * @brief Displays the spam attack active status screen.
     */
    void displaySpamActive();

    /**
     * @brief Updates only changing values on the spam active screen.
     *
     * Optimized update for real-time display without full redraw.
     */
    void updateSpamActiveValue();

    /**
     * @brief Displays device selection for impersonation attack.
     */
    void displayImpDeviceSelect();

    /**
     * @brief Displays PGN selection for impersonation attack.
     */
    void displayImpPGNSelect();

    /**
     * @brief Displays field selection for impersonation attack.
     */
    void displayImpFieldSelect();

    /**
     * @brief Updates only changing values on the impersonate field select screen.
     *
     * Optimized update for real-time display without full redraw.
     */
    void updateImpFieldSelectValue();

    /**
     * @brief Displays the active attack status with stop option.
     */
    void displayAttackStatus();

    /**
     * @brief Updates the attack status display screen.
     */
    void updateAttackStatusDisplay();

    /* ------------------------------------------------------------------------
     * About Display Methods
     * ------------------------------------------------------------------------ */

    /**
     * @brief Displays the about information page with device details.
     */
    void displayAboutInfo();

    /**
     * @brief Displays the list of supported PGNs.
     */
    void displaySupportedPGNs();

    /* ------------------------------------------------------------------------
     * Legacy PGN Display Methods
     * ------------------------------------------------------------------------ */

    /**
     * @brief Displays the legacy PGN list view.
     */
    void displayPGNList();

    /**
     * @brief Displays a specific PGN value in legacy format.
     * @param pgnIndex Index of the PGN to display.
     */
    void displayPGNValue(int pgnIndex);

    /**
     * @brief Updates the legacy PGN value display.
     * @param pgnIndex Index of the PGN to update.
     */
    void updatePGNValueDisplay(int pgnIndex);

    /**
     * @brief Updates the legacy PGN list.
     */
    void updatePGNList();

public:

    /* ========================================================================
     *                          Constructor and Destructor
     * ======================================================================== */

    /**
     * @brief Constructs a Menu_Controller with all required dependencies.
     *
     * @param u8x8 Pointer to the OLED display driver instance.
     * @param upBtn GPIO pin number for the up navigation button.
     * @param downBtn GPIO pin number for the down navigation button.
     * @param leftBtn GPIO pin number for the left/back navigation button.
     * @param rightBtn GPIO pin number for the right/select navigation button.
     * @param s1 Pointer to sensor 1 object.
     * @param s2 Pointer to sensor 2 object.
     * @param s3 Pointer to sensor 3 object.
     * @param mon Pointer to the NMEA2000 network monitor.
     * @param attk Pointer to the attack controller.
     */
    Menu_Controller(U8X8_SH1106_128X64_NONAME_HW_I2C* u8x8,
                   int upBtn, int downBtn, int leftBtn, int rightBtn,
                   Sensor* s1, Sensor* s2, Sensor* s3,
                   N2K_Monitor* mon, Attack_Controller* attk);

    /**
     * @brief Destructor - cleans up allocated menu objects.
     */
    ~Menu_Controller();

    /* ========================================================================
     *                          Main Interface Methods
     * ======================================================================== */

    /**
     * @brief Initializes the menu controller and displays the main menu.
     *
     * Must be called after construction and before any other methods.
     * Initializes the display, creates all menus, and shows the main menu.
     */
    void begin();

    /**
     * @brief Handles button input for menu navigation.
     *
     * Polls the configured button pins and processes navigation actions.
     * Should be called regularly in the main loop.
     */
    void handleInput();

    /**
     * @brief Updates the display for real-time data screens.
     *
     * Called in the main loop to refresh live data displays such as
     * PGN monitors and attack status screens.
     */
    void update();

    /* ========================================================================
     *                          Navigation Methods
     * ======================================================================== */

    /**
     * @brief Navigates up in the current menu or scrolls up in a list.
     */
    void navigateUp();

    /**
     * @brief Navigates down in the current menu or scrolls down in a list.
     */
    void navigateDown();

    /**
     * @brief Navigates back to the previous menu in the navigation stack.
     */
    void navigateBack();

    /**
     * @brief Selects the current menu item or activates a toggle.
     */
    void navigateSelect();

    /* ========================================================================
     *                          Attack State Methods
     * ======================================================================== */

    /**
     * @brief Queries whether an attack demonstration is currently active.
     * @return true if an attack is active, false otherwise.
     *
     * Delegates to the Attack_Controller.
     */
    bool isAttackActive();


    /* ========================================================================
     *                          Sensor Configuration Methods
     * ======================================================================== */

    /**
     * @brief Sets the PGN type for a sensor.
     * @param sensorNum The sensor number (0, 1, or 2).
     * @param pgnType The PGN type index to set.
     */
    void setSensorPGNType(int sensorNum, int pgnType);

    /**
     * @brief Toggles a sensor between active and inactive states.
     * @param sensorNum The sensor number (0, 1, or 2) to toggle.
     */
    void toggleSensorActive(int sensorNum);

    /* ========================================================================
     *                          Static Callback System
     * ======================================================================== */

    /**
     * @brief Static instance pointer for callback access.
     *
     * Required because the Menu library uses C-style function pointers
     * which cannot directly call member functions.
     */
    static Menu_Controller* instance;

    /** @brief Callback for navigating to Sensor Readings menu. */
    static void callback_SensorReadings();

    /** @brief Callback for navigating to Configure menu. */
    static void callback_Configure();

    /** @brief Callback for navigating to Attacks menu. */
    static void callback_Attacks();

    /** @brief Callback for navigating to About menu. */
    static void callback_About();

    /** @brief Callback for navigating to Sensor 1 configuration. */
    static void callback_ConfigSensor1();

    /** @brief Callback for navigating to Sensor 2 configuration. */
    static void callback_ConfigSensor2();

    /** @brief Callback for navigating to Sensor 3 configuration. */
    static void callback_ConfigSensor3();

    /** @brief Callback for navigating to Sensor 1 PGN type selection. */
    static void callback_Sensor1PGNType();

    /** @brief Callback for toggling Sensor 1 active state. */
    static void callback_Sensor1Active();

    /** @brief Callback for navigating to Sensor 2 PGN type selection. */
    static void callback_Sensor2PGNType();

    /** @brief Callback for toggling Sensor 2 active state. */
    static void callback_Sensor2Active();

    /** @brief Callback for navigating to Sensor 3 PGN type selection. */
    static void callback_Sensor3PGNType();

    /** @brief Callback for toggling Sensor 3 active state. */
    static void callback_Sensor3Active();

    /** @brief Callback for initiating spam attack demonstration. */
    static void callback_SpamAttack();

    /** @brief Callback for initiating impersonate attack demonstration. */
    static void callback_Impersonate();

    /**
     * @brief Callback for selecting a PGN type from the type menu.
     * @param type The PGN type index selected.
     */
    static void callback_SelectPGNType(int type);

    /** @brief Callback for navigating to Device Configuration menu. */
    static void callback_DeviceConfig();

    /** @brief Callback for toggling stale device cleanup setting. */
    static void callback_StaleCleanupToggle();

    /** @brief Callback for navigating to About Info page. */
    static void callback_AboutInfo();

    /** @brief Callback for navigating to Supported PGNs list. */
    static void callback_AboutPGNs();

    /** @brief Callback for navigating to Sensor 1 manufacturer selection. */
    static void callback_Sensor1Manufacturer();

    /** @brief Callback for navigating to Sensor 2 manufacturer selection. */
    static void callback_Sensor2Manufacturer();

    /** @brief Callback for navigating to Sensor 3 manufacturer selection. */
    static void callback_Sensor3Manufacturer();
};

#endif // MENU_CONTROLLER_H
