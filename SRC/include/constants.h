/**
 * \file constants.h
 * \brief Hardware configuration and system constants for the NEMO Project.
 *
 * This file contains all hardware pin definitions, timing constants, and
 * configuration values used throughout the NEMO NMEA2000 research platform.
 * These constants define the interface between the Teensy 4.0 microcontroller
 * and connected peripherals including sensors, buttons, and display.
 *
 * \note PGN, Manufacturer, and Sensor definitions are in the
 *       PGN_Helpers library. Include <PGN_Helpers.h> to access those definitions.
 */

#ifndef CONSTANTS_H
#define CONSTANTS_H

#pragma once
#include <Arduino.h>

/*
 * Debug Configuration
  */

/**
 * \def DEBUG
 * \brief Enable or disable verbose debug output.
 *
 * When set to \c true, enables verbose debug output to the serial console.
 * When set to \c false, only candump-style output is displayed.
 *
 * \note Set to \c false for production builds to reduce serial traffic.
 */
#define DEBUG false

/*
 * Hardware Pin Definitions - Sensor Inputs
*/

/**
 * \def SENSOR_PIN_1
 * \brief GPIO pin number for the first sensor input.
 *
 * This pin is used for reading analog or digital sensor data on the Teensy 4.0.
 * Connected to GPIO 16 on the Teensy 4.0 board.
 */
#define SENSOR_PIN_1 16

/**
 * \def SENSOR_PIN_2
 * \brief GPIO pin number for the second sensor input.
 *
 * This pin is used for reading analog or digital sensor data on the Teensy 4.0.
 * Connected to GPIO 15 on the Teensy 4.0 board.
 */
#define SENSOR_PIN_2 15

/**
 * \def SENSOR_PIN_3
 * \brief GPIO pin number for the third sensor input.
 *
 * This pin is used for reading analog or digital sensor data on the Teensy 4.0.
 * Connected to GPIO 14 on the Teensy 4.0 board.
 */
#define SENSOR_PIN_3 14

/*
 * Hardware Pin Definitions - Navigation Buttons
*/

/**
 * \def BUTTON_UP
 * \brief GPIO pin number for the UP navigation button.
 *
 * Used for menu navigation and scrolling up through lists.
 * Connected to GPIO 5 on the Teensy 4.0 board.
 */
#define BUTTON_UP 5

/**
 * \def BUTTON_DOWN
 * \brief GPIO pin number for the DOWN navigation button.
 *
 * Used for menu navigation and scrolling down through lists.
 * Connected to GPIO 3 on the Teensy 4.0 board.
 */
#define BUTTON_DOWN 3

/**
 * \def BUTTON_LEFT
 * \brief GPIO pin number for the LEFT navigation button.
 *
 * Used for menu navigation, going back, and decrementing values.
 * Connected to GPIO 2 on the Teensy 4.0 board.
 */
#define BUTTON_LEFT 2

/**
 * \def BUTTON_RIGHT
 * \brief GPIO pin number for the RIGHT navigation button.
 *
 * Used for menu navigation, selection, and incrementing values.
 * Connected to GPIO 4 on the Teensy 4.0 board.
 */
#define BUTTON_RIGHT 4

/*
 * Timing Constants
*/

/**
 * \brief Timeout for marking device data as stale (in milliseconds).
 *
 * When no updates are received from a device within this timeout period,
 * the device data is considered stale and may be cleaned up. This helps
 * manage memory and ensure the display shows only active network devices.
 *
 * Default value: 60000 ms (1 minute)
 */
inline constexpr unsigned long STALE_TIMEOUT_MS = 60000;

/**
 * \brief Delay between display scroll updates (in milliseconds).
 *
 * Controls the speed at which text scrolls horizontally on the display
 * when content exceeds the visible width. Lower values result in faster
 * scrolling, higher values result in slower, more readable scrolling.
 *
 * Default value: 400 ms
 */
inline constexpr unsigned long SCROLL_DELAY_MS = 400;

/*
 * Display Configuration Constants
*/

/**
 * \brief Number of visible characters for scrolling text display.
 *
 * Defines the maximum number of characters visible at one time when
 * displaying scrolling text on the LCD or OLED display. Text longer
 * than this value will scroll horizontally.
 *
 * Default value: 10 characters
 */
inline constexpr int SCROLL_VISIBLE_CHARS = 10;

/**
 * \brief Number of visible characters for device name display.
 *
 * Defines the maximum number of characters visible when displaying
 * device names on the screen. Device names longer than this value
 * will be truncated or scrolled.
 *
 * Default value: 12 characters
 */
inline constexpr int DEVICE_NAME_VISIBLE_CHARS = 12;

/*
 * Attack Controller Constants
*/

/**
 * \brief Maximum number of impersonate fields that can be locked.
 *
 * Defines the upper limit on the number of fields that can be locked
 * during an impersonation attack demonstration. This limit helps manage
 * memory usage and ensures predictable behavior during attack simulations.
 *
 * Default value: 16 fields
 */
inline constexpr int MAX_IMP_FIELDS = 16;
 
#endif /* CONSTANTS_H */
