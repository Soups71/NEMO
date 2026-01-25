/**
 * @file Splash_Screen.h
 * @brief Splash screen display and boot animation for NEMO device.
 *
 * This file provides the splash screen functionality for the NEMO
 * (NMEA Educational Maritime Observer) device. It displays an animated
 * clownfish swimming across the screen during boot, along with the
 * NEMO logo text.
 *
 * The splash screen uses the U8g2 graphics library for rendering
 * bitmap graphics and animations on the SH1106 128x64 OLED display.
 */

#ifndef SPLASH_SCREEN_H
#define SPLASH_SCREEN_H

#include <Arduino.h>
#include <U8g2lib.h>

/**
 * @class Splash_Screen
 * @brief Handles the boot splash screen animation for the NEMO device.
 *
 * This class provides static methods for displaying an animated splash
 * screen on the OLED display during device startup. The animation features
 * a swimming clownfish (representing NEMO) with bubbles and the NEMO logo.
 *
 * The splash screen is designed to be displayed using U8g2 graphics mode,
 * and after completion, control is handed off to U8x8 text mode for menu
 * navigation.
 */
class Splash_Screen {
public:
    /**
     * @brief Display the splash screen animation sequence.
     *
     * This method runs the complete boot animation sequence which includes:
     * - Animated clownfish swimming across the screen
     * - Rising bubble effects
     * - NEMO logo display
     *
     * The animation uses U8g2 graphics mode for smooth bitmap rendering.
     * After the animation completes, the display can be switched to U8x8
     * text mode for menu navigation.
     *
     * @param u8g2 Pointer to the U8G2 display driver instance.
     *             Must be initialized before calling this method.
     *
     * @return void
     */
    static void show(U8G2_SH1106_128X64_NONAME_F_HW_I2C* u8g2);

private:
    /**
     * @brief Draw the animated clownfish at a specific position and frame.
     *
     * Renders one frame of the swimming fish animation at the specified
     * screen coordinates. The fish bitmap changes based on the frame
     * parameter to create a tail-wagging animation effect.
     *
     * @param u8g2  Pointer to the U8G2 display driver instance.
     * @param x     X coordinate (horizontal position) for the fish.
     * @param y     Y coordinate (vertical position) for the fish.
     * @param frame Animation frame number (0-2) controlling tail position:
     *              - 0: Tail up position
     *              - 1: Tail middle position
     *              - 2: Tail down position
     *
     * @return void
     */
    static void drawFish(U8G2_SH1106_128X64_NONAME_F_HW_I2C* u8g2, int x, int y, int frame);

    /**
     * @brief Draw a bubble at the specified position.
     *
     * Renders a circular bubble graphic at the given coordinates.
     * Bubbles are used to enhance the underwater theme of the
     * splash screen animation.
     *
     * @param u8g2 Pointer to the U8G2 display driver instance.
     * @param x    X coordinate (horizontal position) for the bubble center.
     * @param y    Y coordinate (vertical position) for the bubble center.
     * @param size Diameter of the bubble in pixels.
     *
     * @return void
     */
    static void drawBubble(U8G2_SH1106_128X64_NONAME_F_HW_I2C* u8g2, int x, int y, int size);

    /**
     * @brief Draw the NEMO logo text.
     *
     * Renders the "NEMO" letters using custom bitmap fonts for
     * a distinctive appearance. The logo is centered on the display
     * and uses the letter bitmaps defined in this header.
     *
     * @param u8g2 Pointer to the U8G2 display driver instance.
     *
     * @return void
     */
    static void drawNemoLogo(U8G2_SH1106_128X64_NONAME_F_HW_I2C* u8g2);
};

/**
 * @brief Clownfish animation frame 1 - tail up position.
 *
 * First frame of the swimming animation with the tail fin
 * angled upward. Size: 32x24 pixels (96 bytes).
 */
static const unsigned char fish_frame1[] PROGMEM = {
    0x00, 0x00, 0xF0, 0x01,  //                     #####
    0x00, 0x00, 0xFC, 0x07,  //                   #########
    0x00, 0x00, 0xFE, 0x0F,  //                  ###########
    0x00, 0x00, 0xFF, 0x1F,  //                 #############
    0x00, 0x80, 0xFF, 0x3F,  //                ##############
    0x06, 0xC0, 0x9F, 0x7E,  //      ##  ##    ######  ######
    0x0F, 0xE0, 0x0F, 0x7C,  //     ####  ###   ####    #####
    0x1F, 0xF0, 0x07, 0xFC,  //    #########    ###    ######
    0x3F, 0xF8, 0x07, 0xFC,  //   ##########    ###    ######
    0x7F, 0xFC, 0x87, 0xFE,  //  ############   ###   ####### o
    0x7F, 0xFE, 0xC7, 0xFE,  //  #############  ###   ####### oo
    0xFF, 0xFF, 0xE7, 0xFF,  // ###############  ##   ########
    0xFF, 0xFF, 0xFF, 0xFF,  // ################################
    0xFF, 0xFF, 0xE7, 0xFF,  // ###############  ##   ########
    0x7F, 0xFE, 0xC7, 0xFE,  //  #############  ###   #######
    0x7F, 0xFC, 0x87, 0xFE,  //  ############   ###   #######
    0x3F, 0xF8, 0x07, 0xFC,  //   ##########    ###    ######
    0x1F, 0xF0, 0x07, 0xFC,  //    #########    ###    ######
    0x0F, 0xE0, 0x0F, 0x7C,  //     ####  ###   ####    #####
    0x06, 0xC0, 0x9F, 0x7E,  //      ##  ##    ######  ######
    0x00, 0x80, 0xFF, 0x3F,  //                ##############
    0x00, 0x00, 0xFF, 0x1F,  //                 #############
    0x00, 0x00, 0xFE, 0x0F,  //                  ###########
    0x00, 0x00, 0xF8, 0x03   //                    #######
};

/**
 * @brief Clownfish animation frame 2 - tail middle position.
 *
 * Second frame of the swimming animation with the tail fin
 * in a neutral/middle position. Size: 32x24 pixels (96 bytes).
 */
static const unsigned char fish_frame2[] PROGMEM = {
    0x00, 0x00, 0xF0, 0x01,  //                     #####
    0x00, 0x00, 0xFC, 0x07,  //                   #########
    0x00, 0x00, 0xFE, 0x0F,  //                  ###########
    0x00, 0x00, 0xFF, 0x1F,  //                 #############
    0x00, 0x80, 0xFF, 0x3F,  //                ##############
    0x03, 0xC0, 0x9F, 0x7E,  //       ####     ######  ######
    0x07, 0xE0, 0x0F, 0x7C,  //      ######    ####    #####
    0x0F, 0xF0, 0x07, 0xFC,  //     ########   ###    ######
    0x1F, 0xF8, 0x07, 0xFC,  //    ##########  ###    ######
    0x3F, 0xFC, 0x87, 0xFE,  //   ###########  ###   ####### o
    0x7F, 0xFE, 0xC7, 0xFE,  //  #############  ###   ####### oo
    0xFF, 0xFF, 0xE7, 0xFF,  // ###############  ##   ########
    0xFF, 0xFF, 0xFF, 0xFF,  // ################################
    0xFF, 0xFF, 0xE7, 0xFF,  // ###############  ##   ########
    0x7F, 0xFE, 0xC7, 0xFE,  //  #############  ###   #######
    0x3F, 0xFC, 0x87, 0xFE,  //   ###########  ###   #######
    0x1F, 0xF8, 0x07, 0xFC,  //    ##########  ###    ######
    0x0F, 0xF0, 0x07, 0xFC,  //     ########   ###    ######
    0x07, 0xE0, 0x0F, 0x7C,  //      ######    ####    #####
    0x03, 0xC0, 0x9F, 0x7E,  //       ####     ######  ######
    0x00, 0x80, 0xFF, 0x3F,  //                ##############
    0x00, 0x00, 0xFF, 0x1F,  //                 #############
    0x00, 0x00, 0xFE, 0x0F,  //                  ###########
    0x00, 0x00, 0xF8, 0x03   //                    #######
};

/**
 * @brief Clownfish animation frame 3 - tail down position.
 *
 * Third frame of the swimming animation with the tail fin
 * angled downward. Size: 32x24 pixels (96 bytes).
 */
static const unsigned char fish_frame3[] PROGMEM = {
    0x00, 0x00, 0xF0, 0x01,  //                     #####
    0x00, 0x00, 0xFC, 0x07,  //                   #########
    0x00, 0x00, 0xFE, 0x0F,  //                  ###########
    0x00, 0x00, 0xFF, 0x1F,  //                 #############
    0x00, 0x80, 0xFF, 0x3F,  //                ##############
    0x01, 0xC0, 0x9F, 0x7E,  //        ###     ######  ######
    0x03, 0xE0, 0x0F, 0x7C,  //       #####    ####    #####
    0x07, 0xF0, 0x07, 0xFC,  //      #######   ###    ######
    0x0F, 0xF8, 0x07, 0xFC,  //     #########  ###    ######
    0x1F, 0xFC, 0x87, 0xFE,  //    ###########  ###   ####### o
    0x3F, 0xFE, 0xC7, 0xFE,  //   ############  ###   ####### oo
    0xFF, 0xFF, 0xE7, 0xFF,  // ###############  ##   ########
    0xFF, 0xFF, 0xFF, 0xFF,  // ################################
    0xFF, 0xFF, 0xE7, 0xFF,  // ###############  ##   ########
    0x3F, 0xFE, 0xC7, 0xFE,  //   ############  ###   #######
    0x1F, 0xFC, 0x87, 0xFE,  //    ###########  ###   #######
    0x0F, 0xF8, 0x07, 0xFC,  //     #########  ###    ######
    0x07, 0xF0, 0x07, 0xFC,  //      #######   ###    ######
    0x03, 0xE0, 0x0F, 0x7C,  //       #####    ####    #####
    0x01, 0xC0, 0x9F, 0x7E,  //        ###     ######  ######
    0x00, 0x80, 0xFF, 0x3F,  //                ##############
    0x00, 0x00, 0xFF, 0x1F,  //                 #############
    0x00, 0x00, 0xFE, 0x0F,  //                  ###########
    0x00, 0x00, 0xF8, 0x03   //                    #######
};

/**
 * @brief Letter 'N' bitmap for NEMO logo.
 *
 * 16x24 pixel bitmap of the letter N with diagonal
 * stroke design. Size: 48 bytes.
 */
static const unsigned char letter_N[] PROGMEM = {
    0xE0, 0x1C, // OOO..........OOO
    0xE0, 0x1C, // OOO..........OOO
    0xF0, 0x1C, // OOOO.........OOO
    0xF0, 0x1C, // OOOO.........OOO
    0xF8, 0x1C, // OOOOO........OOO
    0xF8, 0x1C, // OOOOO........OOO
    0xDC, 0x1C, // OO.OOO.......OOO
    0xDC, 0x1C, // OO.OOO.......OOO
    0xCE, 0x1C, // OO..OOO......OOO
    0xCE, 0x1C, // OO..OOO......OOO
    0xC7, 0x1C, // OO...OOO.....OOO
    0xC7, 0x1C, // OO...OOO.....OOO
    0xC3, 0x9C, // OO....OOO....OOO
    0xC3, 0x9C, // OO....OOO....OOO
    0xC1, 0xDC, // OO.....OOO...OOO
    0xC1, 0xDC, // OO.....OOO...OOO
    0xC0, 0xFC, // OO......OOOO.OOO
    0xC0, 0xFC, // OO......OOOO.OOO
    0xC0, 0x7C, // OO.......OOOOOOO
    0xC0, 0x7C, // OO.......OOOOOOO
    0xC0, 0x3C, // OO........OOOOOO
    0xC0, 0x3C, // OO........OOOOOO
    0xC0, 0x1C, // OO.........OOOOO
    0xC0, 0x1C  // OO.........OOOOO
};

/**
 * @brief Letter 'E' bitmap for NEMO logo.
 *
 * 16x24 pixel bitmap of the letter E with horizontal
 * bars at top, middle, and bottom. Size: 48 bytes.
 */
static const unsigned char letter_E[] PROGMEM = {
    0xFF, 0xFC, // OOOOOOOOOOOOOO..
    0xFF, 0xFC, // OOOOOOOOOOOOOO..
    0xFF, 0xFC, // OOOOOOOOOOOOOO..
    0xE0, 0x00, // OOO.............
    0xE0, 0x00, // OOO.............
    0xE0, 0x00, // OOO.............
    0xE0, 0x00, // OOO.............
    0xE0, 0x00, // OOO.............
    0xFF, 0xF0, // OOOOOOOOOOOO....
    0xFF, 0xF0, // OOOOOOOOOOOO....
    0xFF, 0xF0, // OOOOOOOOOOOO....
    0xFF, 0xF0, // OOOOOOOOOOOO....
    0xE0, 0x00, // OOO.............
    0xE0, 0x00, // OOO.............
    0xE0, 0x00, // OOO.............
    0xE0, 0x00, // OOO.............
    0xE0, 0x00, // OOO.............
    0xE0, 0x00, // OOO.............
    0xE0, 0x00, // OOO.............
    0xE0, 0x00, // OOO.............
    0xFF, 0xFC, // OOOOOOOOOOOOOO..
    0xFF, 0xFC, // OOOOOOOOOOOOOO..
    0xFF, 0xFC, // OOOOOOOOOOOOOO..
    0xFF, 0xFC  // OOOOOOOOOOOOOO..
};

/**
 * @brief Letter 'M' bitmap for NEMO logo.
 *
 * 16x24 pixel bitmap of the letter M with peaked
 * center design. Size: 48 bytes.
 */
static const unsigned char letter_M[] PROGMEM = {
    0xE0, 0x1C, // OOO..........OOO
    0xF0, 0x3C, // OOOO........OOOO
    0xF0, 0x3C, // OOOO........OOOO
    0xF8, 0x7C, // OOOOO......OOOOO
    0xF8, 0x7C, // OOOOO......OOOOO
    0xDC, 0xEC, // OO.OOO....OOO.OO
    0xDC, 0xEC, // OO.OOO....OOO.OO
    0xCE, 0xCC, // OO..OOO..OOO..OO
    0xCE, 0xCC, // OO..OOO..OOO..OO
    0xC7, 0x8C, // OO...OOOO....OO.
    0xC7, 0x8C, // OO...OOOO....OO.
    0xC3, 0x0C, // OO....OO.....OO.
    0xC3, 0x0C, // OO....OO.....OO.
    0xC0, 0x0C, // OO...........OO.
    0xC0, 0x0C, // OO...........OO.
    0xC0, 0x0C, // OO...........OO.
    0xC0, 0x0C, // OO...........OO.
    0xC0, 0x0C, // OO...........OO.
    0xC0, 0x0C, // OO...........OO.
    0xC0, 0x0C, // OO...........OO.
    0xC0, 0x0C, // OO...........OO.
    0xC0, 0x0C, // OO...........OO.
    0xC0, 0x0C, // OO...........OO.
    0xC0, 0x0C  // OO...........OO.
};

/**
 * @brief Letter 'O' bitmap for NEMO logo.
 *
 * 16x24 pixel bitmap of the letter O with rounded
 * oval shape. Size: 48 bytes.
 */
static const unsigned char letter_O[] PROGMEM = {
    0x0F, 0xC0, // ....OOOOOO......
    0x3F, 0xF0, // ..OOOOOOOOOO....
    0x7F, 0xF8, // .OOOOOOOOOOOO...
    0x78, 0x78, // .OOOO....OOOO...
    0xF0, 0x3C, // OOOO......OOOO..
    0xE0, 0x1C, // OOO........OOO..
    0xE0, 0x1C, // OOO........OOO..
    0xE0, 0x1C, // OOO........OOO..
    0xE0, 0x1C, // OOO........OOO..
    0xE0, 0x1C, // OOO........OOO..
    0xE0, 0x1C, // OOO........OOO..
    0xE0, 0x1C, // OOO........OOO..
    0xE0, 0x1C, // OOO........OOO..
    0xE0, 0x1C, // OOO........OOO..
    0xE0, 0x1C, // OOO........OOO..
    0xE0, 0x1C, // OOO........OOO..
    0xE0, 0x1C, // OOO........OOO..
    0xE0, 0x1C, // OOO........OOO..
    0xF0, 0x3C, // OOOO......OOOO..
    0x78, 0x78, // .OOOO....OOOO...
    0x7F, 0xF8, // .OOOOOOOOOOOO...
    0x3F, 0xF0, // ..OOOOOOOOOO....
    0x0F, 0xC0, // ....OOOOOO......
    0x00, 0x00  // ................
};

#endif // SPLASH_SCREEN_H
