/**
 * \file Menu.cpp
 * \brief Implementation of the Menu class for OLED display navigation.
 *
 * This file contains the implementation of the Menu class, providing a
 * flexible menu system for SH1106-based OLED displays. The implementation
 * handles menu rendering, navigation, text scrolling, and display management.
 */

#include <Menu.h>

/************************************************************************//**
 * \brief Constructs a new Menu object with the specified parameters.
 *
 * Initializes all internal state variables for the menu system including:
 * - Display driver reference and screen dimensions
 * - Menu title and options array
 * - Navigation state (current option, row, column)
 * - Scrolling parameters for long text animation
 *
 * \param u8x8 Pointer to the U8x8 display driver instance for rendering.
 * \param Title Title string displayed at the top of the menu.
 * \param userOptions Pointer to array of FunctionStruct menu options.
 * \param usrNumChoices Number of options in the userOptions array.
 * \param isMenu Operating mode flag: 1 for interactive menu, 0 for text display.
 */
Menu::Menu(U8X8_SH1106_128X64_NONAME_HW_I2C* u8x8, String Title, FunctionStruct* userOptions, int usrNumChoices, int isMenu){
    // Store display driver reference
    screen = u8x8;

    // Initialize menu content
    menu_title = Title;
    options = userOptions;
    num_choices = usrNumChoices;
    is_menu = isMenu;

    // Query display dimensions from driver
    num_rows = screen -> getRows();
    num_cols = screen -> getCols();

    // Set default menu operand (selection indicator prefix)
    menu_operand = " * ";

    // Initialize scrolling state
    scrollOffset = 0;
    lastScrollTime = 0;
    scrollDelay = 300;  // 300ms between scroll steps for readable animation

    // Initialize navigation state
    curr_option = 0;  // Start at first option
    curr_row = 0;
    curr_col = 0;
}

/************************************************************************//**
 * \brief Truncates a string to fit within the specified maximum length.
 *
 * Handles string truncation for display on the OLED screen. If the input
 * string exceeds the maximum length, it is truncated and an ellipsis (...)
 * is appended to indicate continuation.
 *
 * \param str The input string to truncate.
 * \param maxLen Maximum allowed length for the output string.
 * \return Truncated string that fits within maxLen characters.
 *         Returns original string if already within limits.
 */
String Menu::truncateString(String str, int maxLen) {
    // Return unchanged if within length limit
    if(str.length() <= maxLen) {
        return str;
    }

    // Truncate and add ellipsis if there's room for it
    if(maxLen > 3) {
        return str.substring(0, maxLen - 3) + "...";
    }

    // For very short maxLen, just truncate without ellipsis
    return str.substring(0, maxLen);
}

/************************************************************************//**
 * \brief Renders the complete menu to the OLED display.
 *
 * Draws the menu interface including:
 * - Centered title at the top of the screen
 * - List of menu options with selection indicator prefix
 * - Inverted highlighting on the currently selected option
 *
 * The menu supports up to 6 visible items (rows 2-7 on an 8-row screen).
 * Items exceeding the available width are truncated with ellipsis for
 * non-selected items, or scrolled for selected items via updateScrollingText().
 *
 * \return 0 on success.
 */
int Menu::printMenu(){
    // Clear display and reset state
    reset();
    curr_option = 0;  // Reset to first option when menu is printed

    // Draw centered title on first row
    setw(menu_title);

    // Menu options begin on row 2 (after title with blank line)
    curr_row = 2;
    scrollOffset = 0;  // Reset scroll when redrawing menu

    // Calculate available width for option text after operand prefix
    int maxOptionLen = num_cols - menu_operand.length();

    // Limit visible items to 6 (leaves room for title and spacing)
    int maxVisibleItems = 6;
    int itemsToShow = (num_choices < maxVisibleItems) ? num_choices : maxVisibleItems;

    // Render each visible menu option
    for(int i = 0; i < itemsToShow; i++){
        // Highlight currently selected option with inverse font
        if(curr_option == i){
            screen -> setInverseFont(1);
        }else{
            screen -> setInverseFont(0);
        }

        // Draw selection indicator prefix
        screen -> drawString(curr_col, curr_row, menu_operand.c_str());
        curr_col += menu_operand.length();

        // Prepare option text for display
        String optionText = options[i].name;

        if(curr_option == i && optionText.length() > maxOptionLen) {
            // Selected item is too long - show beginning for now
            // (will scroll via updateScrollingText())
            optionText = optionText.substring(0, maxOptionLen);
        } else if(optionText.length() > maxOptionLen) {
            // Non-selected items: truncate with ellipsis
            optionText = truncateString(optionText, maxOptionLen);
        }

        // Draw the option text
        screen -> drawString(curr_col, curr_row, optionText.c_str());

        // Move to next row, reset column
        curr_row++;
        curr_col = 0;
    }

    // Restore normal font mode
    screen -> setInverseFont(0);
    return 0;
}

/************************************************************************//**
 * \brief Updates the horizontal scrolling animation for long text.
 *
 * This function should be called periodically (e.g., in the main loop)
 * to animate text scrolling for the currently selected menu item when
 * its text is too long to fit on screen.
 *
 * The scrolling behavior:
 * - Only scrolls the currently selected item if it exceeds maxOptionLen
 * - Waits scrollDelay milliseconds between scroll steps
 * - Resets to beginning when scroll reaches the end of the text
 * - Redraws only the affected display row for efficiency
 */
void Menu::updateScrollingText() {
    // Check if enough time has passed since last scroll update
    unsigned long currentTime = millis();
    if(currentTime - lastScrollTime < scrollDelay) {
        return;  // Not time to scroll yet
    }

    // Update timestamp for next scroll interval
    lastScrollTime = currentTime;

    // Calculate maximum text length for option display area
    int maxOptionLen = num_cols - menu_operand.length();

    // Check if current option needs scrolling
    if(curr_option < num_choices) {
        String optionText = options[curr_option].name;

        if(optionText.length() > maxOptionLen) {
            // This option text exceeds display width - scroll it
            scrollOffset++;

            // Reset scroll offset when reaching end of text
            if(scrollOffset > optionText.length() - maxOptionLen) {
                scrollOffset = 0;
            }

            // Calculate the display row for the selected item
            int displayRow = 2 + curr_option;

            // Clear the option text area (fill with spaces)
            screen->setInverseFont(1);
            screen->drawString(menu_operand.length(), displayRow, "             ");

            // Extract and draw the scrolled portion of text
            String scrolledText = optionText.substring(scrollOffset, scrollOffset + maxOptionLen);
            screen->drawString(menu_operand.length(), displayRow, scrolledText.c_str());

            // Restore normal font mode
            screen->setInverseFont(0);
        }
    }
}

/************************************************************************//**
 * \brief Moves the selection cursor down to the next option.
 *
 * Advances the current selection to the next menu item if not already
 * at the last option. After updating the selection index, triggers a
 * display refresh via updateOption().
 *
 * \return 0 on success.
 */
int Menu::downOption(){
    // Only advance if not at last option (no wrap-around)
    if(curr_option < num_choices-1){
        curr_option++;
    }

    // Refresh display to show new selection
    updateOption();
    return 0;
}

/************************************************************************//**
 * \brief Moves the selection cursor up to the previous option.
 *
 * Moves the current selection to the previous menu item if not already
 * at the first option. After updating the selection index, triggers a
 * display refresh via updateOption().
 *
 * \return 0 on success.
 */
int Menu::upOption(){
    // Only move back if not at first option (no wrap-around)
    if(curr_option > 0){
        curr_option--;
    }

    // Refresh display to show new selection
    updateOption();
    return 0;
}

/************************************************************************//**
 * \brief Updates the display to reflect the current option selection.
 *
 * Redraws the menu with proper highlighting on the newly selected option.
 * Handles vertical scrolling when the selection moves beyond the visible
 * window of 6 items:
 * - Calculates the appropriate topIndex to keep selection visible
 * - Resets horizontal scrolling state for the new selection
 * - Redraws all visible items with correct highlighting
 *
 * \return 0 on success.
 */
int Menu::updateOption(){
    // Clear display and redraw title
    reset();
    setw(menu_title);

    // Reset horizontal scroll state when selection changes
    scrollOffset = 0;
    lastScrollTime = millis();

    // Calculate top index for vertical scrolling
    // This ensures the selected item is always visible within the 6-item window
    int topIndex = (curr_option-6)+1;
    if(topIndex < 0){
        topIndex = 0;
    }

    // Calculate maximum text length for option display area
    int maxOptionLen = num_cols - menu_operand.length();

    // Menu options begin on row 2 (after title)
    curr_row = 2;

    // Render visible menu options (up to 6 items starting from topIndex)
    for(int i = topIndex; i < num_choices && i<topIndex+6; i++){
        // Highlight currently selected option with inverse font
        if(curr_option == i){
            screen -> setInverseFont(1);
        }else{
            screen -> setInverseFont(0);
        }

        // Draw selection indicator prefix
        screen -> drawString(curr_col, curr_row, menu_operand.c_str());
        curr_col += menu_operand.length();

        // Prepare option text for display
        String optionText = options[i].name;

        if(curr_option == i && optionText.length() > maxOptionLen) {
            // Selected item is too long - show beginning
            // (will scroll via updateScrollingText())
            optionText = optionText.substring(0, maxOptionLen);
        } else if(optionText.length() > maxOptionLen) {
            // Non-selected items: truncate with ellipsis
            optionText = truncateString(optionText, maxOptionLen);
        }

        // Draw the option text
        screen -> drawString(curr_col, curr_row, optionText.c_str());

        // Move to next row, reset column
        curr_row++;
        curr_col = 0;
    }

    // Restore normal font mode
    screen -> setInverseFont(0);
    return 0;
}

/************************************************************************//**
 * \brief Draws a centered text line on the current row.
 *
 * Calculates the horizontal centering offset for the given text line
 * and draws it on the display. If the text exceeds the display width,
 * it is truncated with ellipsis.
 *
 * \param curr_line The text string to display centered.
 * \return 0 on success (text fits), -1 if text was truncated.
 */
int Menu::setw(String curr_line){
    // Calculate length and remaining space
    int line_len = curr_line.length();
    int chars_left = num_cols - line_len;

    // Check if string exceeds screen width
    if(chars_left<0){
        // Text too long - truncate with ellipsis
        String truncated = truncateString(curr_line, num_cols);
        screen -> drawString(curr_col, curr_row, truncated.c_str());
        curr_row++;
        return -1;  // Indicate truncation occurred
    }else{
        // Text fits - center it by adding half the remaining space as offset
        screen -> drawString(curr_col+(chars_left/2), curr_row, curr_line.c_str());
        curr_row++;
        return 0;  // Success
    }
}

/************************************************************************//**
 * \brief Resets the display and menu state to initial values.
 *
 * Performs a complete reset of the display and internal state:
 * - Clears the OLED display buffer
 * - Moves cursor to home position
 * - Sets default font (ArtossSans 8-pixel)
 * - Disables inverse font mode
 * - Resets cursor position tracking to (0,0)
 *
 * \return 0 on success.
 */
int Menu::reset(){
    // Clear display buffer
    screen -> clear();

    // Move cursor to top-left
    screen -> home();

    // Set default font for menu rendering
    screen -> setFont(u8x8_font_artossans8_r);

    // Ensure normal (non-inverted) font mode
    screen -> setInverseFont(0);

    // Reset cursor position tracking
    curr_col = 0;
    curr_row = 0;

    return 0;
}
