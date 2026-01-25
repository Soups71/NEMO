/**
 * \file Menu.h
 * \brief Menu system for OLED display navigation and interaction.
 *
 * This file provides a flexible menu system designed for use with SH1106-based
 * OLED displays via the U8x8 library. The Menu class supports both interactive
 * menu navigation with selectable options and static text display modes.
 */

#ifndef MENU_H
#define MENU_H

#include <Arduino.h>
#include <U8x8lib.h>

/**
 * \brief Function pointer type for menu item callbacks.
 *
 * This typedef defines a pointer to a void function with no parameters.
 * Used as callback functions that are executed when menu items are selected.
 */
typedef void (*FunctionPointer)();

/**
 * \brief Structure representing a menu option with associated callback.
 *
 * Each menu item consists of a display name shown on the OLED screen
 * and an optional function pointer that is called when the item is selected.
 */
typedef struct {
    String name;            ///< Display name of the menu option shown on screen
    FunctionPointer func;   ///< Callback function executed on selection (can be NULL)
} FunctionStruct;

/**
 * \brief Menu class for OLED display navigation and interaction.
 *
 * The Menu class provides a complete menu system for SH1106-based OLED displays.
 * It supports two primary modes of operation:
 *
 * 1. **Menu Mode**: Interactive navigation with selectable options, highlighting,
 *    and callback execution on selection.
 *
 * 2. **Text Mode**: Static text display for information screens without
 *    interactive selection.
 *
 * The class handles display rendering, cursor navigation, text scrolling for
 * long menu items, and callback management for selected options.
 *
 * \note This class is designed specifically for the U8X8_SH1106_128X64_NONAME_HW_I2C
 *       display driver but could be adapted for other U8x8-compatible displays.
 */
class Menu {
private:
    /**
     * \brief Truncates a string to fit within the specified maximum length.
     *
     * If the input string exceeds the maximum length, it is truncated
     * to ensure proper display on the OLED screen without overflow.
     *
     * \param str The input string to truncate.
     * \param maxLen Maximum allowed length for the output string.
     * \return Truncated string that fits within maxLen characters.
     */
    String truncateString(String str, int maxLen);

public:
    int curr_option;        ///< Index of the currently selected menu option (0-based)
    int num_choices;        ///< Total number of menu options available
    int is_menu;            ///< Flag indicating mode: 1 for menu mode, 0 for text mode
    int num_rows;           ///< Number of display rows available for menu items
    int num_cols;           ///< Number of display columns (character width)
    int curr_row;           ///< Current cursor row position on display
    int curr_col;           ///< Current cursor column position on display
    String menu_operand;    ///< Optional operand string for menu operations
    FunctionStruct* options; ///< Pointer to array of menu options (FunctionStruct)
    String menu_title;      ///< Title string displayed at the top of the menu
    U8X8_SH1106_128X64_NONAME_HW_I2C* screen; ///< Pointer to the U8x8 display driver instance

    // Scrolling support
    int scrollOffset;               ///< Current horizontal scroll offset for long text
    unsigned long lastScrollTime;   ///< Timestamp of last scroll update (milliseconds)
    int scrollDelay;                ///< Delay between scroll steps (milliseconds)

    /**
     * \brief Constructs a new Menu object.
     *
     * Initializes the menu system with the specified display, title, options,
     * and operating mode. Sets up internal state for navigation and rendering.
     *
     * \param u8x8 Pointer to the U8x8 display driver instance.
     * \param Title Title string to display at the top of the menu.
     * \param userOptions Pointer to array of FunctionStruct menu options.
     * \param usrNumChoices Number of options in the userOptions array.
     * \param isMenu Operating mode flag: 1 for interactive menu, 0 for text display.
     */
    Menu(U8X8_SH1106_128X64_NONAME_HW_I2C* u8x8,
         String Title,
         FunctionStruct* userOptions,
         int usrNumChoices,
         int isMenu);

    /**
     * \brief Renders the menu to the OLED display.
     *
     * Draws the menu title, all visible menu options, and highlights
     * the currently selected option. Handles pagination if there are
     * more options than visible rows.
     *
     * \return 0 on success, non-zero on error.
     */
    int printMenu();

    /**
     * \brief Renders static text to the OLED display.
     *
     * Used in text mode to display information without interactive
     * selection capability. Renders the menu title and option names
     * as static text lines.
     *
     * \return 0 on success, non-zero on error.
     */
    int printText();

    /**
     * \brief Moves the selection cursor down to the next option.
     *
     * Advances the current selection to the next menu item. If at the
     * last option, the selection wraps around to the first option.
     *
     * \return The new current option index after the move.
     */
    int downOption();

    /**
     * \brief Moves the selection cursor up to the previous option.
     *
     * Moves the current selection to the previous menu item. If at the
     * first option, the selection wraps around to the last option.
     *
     * \return The new current option index after the move.
     */
    int upOption();

    /**
     * \brief Scrolls the menu view upward.
     *
     * Scrolls the visible portion of the menu up to show previous
     * options that were not visible. Used for menus with more options
     * than can fit on screen at once.
     *
     * \return 0 on success, non-zero if already at top.
     */
    int scrollUp();

    /**
     * \brief Scrolls the menu view downward.
     *
     * Scrolls the visible portion of the menu down to show additional
     * options that were not visible. Used for menus with more options
     * than can fit on screen at once.
     *
     * \return 0 on success, non-zero if already at bottom.
     */
    int scrollDown();

    /**
     * \brief Updates the display to reflect the current option selection.
     *
     * Redraws the menu highlighting to show the newly selected option.
     * Should be called after upOption() or downOption() to update the display.
     *
     * \return 0 on success, non-zero on error.
     */
    int updateOption();

    /**
     * \brief Sets the display width for a text line.
     *
     * Configures the column width used for rendering the specified line
     * of text on the display.
     *
     * \param curr_line The text line to measure and set width for.
     * \return The calculated width in characters.
     */
    int setw(String curr_line);

    /**
     * \brief Resets the menu to its initial state.
     *
     * Returns the menu to its default state with the first option selected,
     * scroll position at the top, and display cleared for fresh rendering.
     *
     * \return 0 on success, non-zero on error.
     */
    int reset();

    /**
     * \brief Updates the horizontal scrolling animation for long text.
     *
     * Called periodically to animate text scrolling for menu items that
     * are too long to fit on screen. Handles timing and scroll offset
     * updates based on scrollDelay setting.
     */
    void updateScrollingText();
};

#endif // MENU_H
