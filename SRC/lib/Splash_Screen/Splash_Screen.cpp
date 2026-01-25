/**
 * @file Splash_Screen.cpp
 * @brief Implementation of the NEMO boot animation and splash screen display.
 *
 * This file implements the splash screen functionality for the NEMO
 * (NMEA2000 Education & Maritime Operations) platform. It provides an
 * animated boot sequence featuring a swimming clownfish with bubbles,
 * followed by a logo reveal animation with fade effects.
 *
 * The animation sequence consists of three phases:
 * - Phase 1: Animated clownfish swimming across the screen with trailing bubbles
 * - Phase 2: NEMO logo reveal with line-by-line building effect
 * - Phase 3: Fade-out effect with progressive pixel removal
 */

#include "Splash_Screen.h"

/**
 * @brief Draw the animated clownfish at a specific position and frame.
 *
 * Renders the NEMO clownfish sprite using U8g2 drawing primitives. The fish
 * is drawn facing right (swimming direction) with the tail on the left side
 * and head/eye on the right side. The sprite includes:
 * - Animated tail fin (moves up/down based on frame)
 * - Filled ellipse body
 * - Two white stripes (drawn as black lines to create gaps)
 * - Dorsal fin (top)
 * - Pectoral fin (side)
 * - Eye with pupil
 * - Mouth
 *
 * @param u8g2  Pointer to the U8G2 display driver instance.
 * @param x     X coordinate (horizontal position) for the fish's left edge.
 * @param y     Y coordinate (vertical position) for the fish's top edge.
 * @param frame Animation frame number (0-2) controlling tail position:
 *              - 0: Tail angled upward (-3 pixel offset)
 *              - 1: Tail in middle position (0 pixel offset)
 *              - 2: Tail angled downward (+3 pixel offset)
 */
void Splash_Screen::drawFish(U8G2_SH1106_128X64_NONAME_F_HW_I2C* u8g2, int x, int y, int frame) {
    // Fish dimensions: ~40 wide x 24 tall
    // Body center is at (x+25, y+12)

    // Calculate the center point of the fish body ellipse
    int bodyX = x + 25;   // Horizontal center of body ellipse
    int bodyY = y + 12;   // Vertical center of body ellipse

    int tailOffset = 0;
    switch (frame % 3) {
        case 0: tailOffset = -3; break;  // Tail swings upward
        case 1: tailOffset = 0; break;   // Tail returns to center
        case 2: tailOffset = 3; break;   // Tail swings downward
    }

    // Draw tail as filled triangle on the left side of the fish
    // Triangle vertices: top-left, bottom-left, and point connecting to body
    u8g2->drawTriangle(x, y + 4 + tailOffset, x, y + 20 + tailOffset, x + 12, y + 12);

    u8g2->drawFilledEllipse(bodyX, bodyY, 14, 10, U8G2_DRAW_ALL);

    // White stripes - characteristic clownfish markings
    // Drawn as black (color 0) lines to create gaps in the orange body
    // Two stripes are drawn to match NEMO's appearance
    u8g2->setDrawColor(0);  // Switch to black to erase/create stripe gaps

    // First stripe - positioned behind the head (toward front of fish)
    u8g2->drawLine(bodyX + 5, bodyY - 9, bodyX + 5, bodyY + 9);
    u8g2->drawLine(bodyX + 6, bodyY - 9, bodyX + 6, bodyY + 9);

    // Second stripe - positioned in the middle of the body
    u8g2->drawLine(bodyX - 4, bodyY - 10, bodyX - 4, bodyY + 10);
    u8g2->drawLine(bodyX - 5, bodyY - 10, bodyX - 5, bodyY + 10);

    u8g2->setDrawColor(1);  // Restore white drawing color

    
    // Dorsal fin - triangular fin on top of the fish
    
    u8g2->drawTriangle(bodyX - 5, bodyY - 10, bodyX + 5, bodyY - 10, bodyX, bodyY - 16);

    
    // Pectoral fin - small side fin for steering
    
    u8g2->drawTriangle(bodyX - 2, bodyY + 2, bodyX + 4, bodyY + 6, bodyX - 2, bodyY + 8);

    
    // Eye - circular eye with darker pupil for depth
    
    // Outer eye (white filled circle)
    u8g2->drawDisc(bodyX + 10, bodyY - 2, 2, U8G2_DRAW_ALL);

    // Pupil (single black pixel for detail)
    u8g2->setDrawColor(0);
    u8g2->drawPixel(bodyX + 11, bodyY - 2);
    u8g2->setDrawColor(1);

    
    // Mouth - simple line to suggest the fish's mouth
    
    u8g2->drawLine(bodyX + 13, bodyY + 1, bodyX + 14, bodyY + 2);
}

/**
 * @brief Draw a bubble at the specified position.
 *
 * Renders a circular bubble outline at the given screen coordinates.
 * Bubbles are used to create an underwater atmosphere in the splash
 * screen animation, trailing behind the swimming fish.
 *
 * @param u8g2 Pointer to the U8G2 display driver instance.
 * @param x    X coordinate (horizontal position) for the bubble center.
 * @param y    Y coordinate (vertical position) for the bubble center.
 * @param size Radius of the bubble in pixels.
 */
void Splash_Screen::drawBubble(U8G2_SH1106_128X64_NONAME_F_HW_I2C* u8g2, int x, int y, int size) {
    u8g2->drawCircle(x, y, size, U8G2_DRAW_ALL);
}

/**
 * @brief Draw the NEMO logo text using bitmap letters.
 *
 * Renders the "NEMO" letters using pre-defined XBM bitmap data.
 * Each letter is 16 pixels wide and 24 pixels tall, positioned
 * with configurable spacing for visual appeal.
 *
 * @param u8g2 Pointer to the U8G2 display driver instance.
 */
void Splash_Screen::drawNemoLogo(U8G2_SH1106_128X64_NONAME_F_HW_I2C* u8g2) {
    // Draw "NEMO" using the bitmap letters
    // Each letter is 16 pixels wide, 24 pixels tall
    int startX = 10;           // Left margin for logo positioning
    int startY = 20;           // Top margin for logo positioning
    int letterSpacing = 4;     // Pixels between each letter

    // Draw each letter using XBM bitmap format
    u8g2->drawXBMP(startX, startY, 16, 24, letter_N);
    u8g2->drawXBMP(startX + 16 + letterSpacing, startY, 16, 24, letter_E);
    u8g2->drawXBMP(startX + 32 + letterSpacing * 2, startY, 16, 24, letter_M);
    u8g2->drawXBMP(startX + 48 + letterSpacing * 3, startY, 16, 24, letter_O);
}

/**
 * @brief Display the complete splash screen animation sequence.
 *
 * This method executes the full NEMO boot animation in three distinct phases:
 *
 * **Phase 1: Fish Swimming Animation**
 * - Clownfish enters from left side of screen
 * - Fish swims across with animated tail movement
 * - Bubbles spawn periodically and float upward
 * - Animation continues until fish exits right side
 *
 * **Phase 2: Logo Reveal**
 * - Screen clears with brief pause
 * - NEMO logo builds line-by-line (top to bottom)
 * - Platform subtitle appears in three lines with delays
 * - Logo holds on screen for viewing
 *
 * **Phase 3: Fade Out**
 * - Progressive pixel removal creates fade effect
 * - Uses modulo-based pixel skipping pattern
 * - Screen clears before handing off to menu system
 *
 * @param u8g2 Pointer to the U8G2 display driver instance.
 *             Must be properly initialized before calling.
 */
void Splash_Screen::show(U8G2_SH1106_128X64_NONAME_F_HW_I2C* u8g2) {
    // Initialize the display and set default font
    u8g2->begin();
    u8g2->setFont(u8g2_font_6x10_tf);

    
    // Phase 1: Fish swims across the screen (left to right)
    // Creates an engaging animated introduction with the NEMO mascot
    

    // Fish position and animation state variables
    int fishX = -45;  // Start off-screen left (fish is ~40 pixels wide)
    int fishY = 20;   // Vertical center position: (64 - 24) / 2 = 20
    int frame = 0;    // Current animation frame for tail movement
    int frameCount = 0;  // Total frames rendered (used for timing)

    
    // Bubble system - creates trailing bubble effect behind swimming fish
    // Uses a circular buffer of 6 bubbles for memory efficiency
    
    struct Bubble {
        int x, y, size;  // Position and radius of bubble
        bool active;     // Whether this bubble slot is currently in use
    };

    // Initialize all bubble slots as inactive
    Bubble bubbles[6] = {{0,0,0,false}, {0,0,0,false}, {0,0,0,false},
                         {0,0,0,false}, {0,0,0,false}, {0,0,0,false}};
    int nextBubble = 0;  // Index for next bubble to spawn (circular buffer)

    
    // Main swimming animation loop
    // Fish moves 4 pixels per frame at ~28 FPS until off-screen right
    
    while (fishX < 140) {
        u8g2->clearBuffer();

        // Draw fish with current animation frame
        drawFish(u8g2, fishX, fishY, frame);

        
        // Bubble spawning - create new bubble every 6 frames when fish is visible
        // Bubbles spawn near the fish's tail with randomized position/size
        
        if (frameCount % 6 == 0 && fishX > 0) {
            bubbles[nextBubble].x = fishX - 5;                      // Spawn behind fish
            bubbles[nextBubble].y = fishY + 16 + (random(10) - 5);  // Randomize vertical pos (-5 to +5)
            bubbles[nextBubble].size = 2 + random(4);               // Random size (2-5 pixels)
            bubbles[nextBubble].active = true;
            nextBubble = (nextBubble + 1) % 6;  // Advance to next slot (wraps around)
        }

        
        // Bubble physics - update positions and render active bubbles
        // Bubbles float upward and drift left to simulate rising in water
        
        for (int i = 0; i < 6; i++) {
            if (bubbles[i].active) {
                drawBubble(u8g2, bubbles[i].x, bubbles[i].y, bubbles[i].size);
                bubbles[i].y -= 2;  // Float upward (2 pixels per frame)
                bubbles[i].x -= 1;  // Drift leftward (1 pixel per frame)

                // Deactivate bubble when it floats off the top of screen
                if (bubbles[i].y < -5) {
                    bubbles[i].active = false;
                }
            }
        }

        // Send frame buffer to display
        u8g2->sendBuffer();

        
        // Update animation state for next frame
        
        fishX += 4;  // Move fish rightward (4 pixels per frame)
        frameCount++;

        // Cycle through tail animation frames every 4 render frames
        // This creates a smooth swimming motion at 1/4 the render rate
        if (frameCount % 4 == 0) {
            frame++;  // Advance to next animation frame (wraps via modulo in drawFish)
        }

        delay(35);  // ~28 FPS timing (1000ms / 35ms = ~28.5 FPS)
    }

    
    // Phase 2: Display NEMO logo with fade-in effect
    // Logo appears with a "building" animation, revealing row by row
    

    // Brief pause and clear screen for transition
    u8g2->clearBuffer();
    u8g2->sendBuffer();
    delay(300);

    
    // Logo building animation - reveals letters row by row from top to bottom
    // Each iteration reveals 2 more rows, creating a "drawing" effect
    
    for (int row = 0; row < 24; row += 2) {
        u8g2->clearBuffer();

        // Logo positioning constants
        int startX = 10;           // Left margin
        int startY = 4;            // Top margin (higher to make room for subtitle)
        int letterSpacing = 4;     // Space between letters

        // Draw each letter row by row up to current reveal point
        for (int r = 0; r <= row && r < 24; r++) {
            
            // Letter 'N' - render pixel by pixel from bitmap data
            // Each letter row is 2 bytes (16 pixels wide)
            
            for (int c = 0; c < 2; c++) {
                uint8_t byte = pgm_read_byte(&letter_N[r * 2 + c]);
                for (int b = 0; b < 8; b++) {
                    // Check each bit (MSB first) and draw pixel if set
                    if (byte & (0x80 >> b)) {
                        u8g2->drawPixel(startX + c * 8 + b, startY + r);
                    }
                }
            }

            
            // Letter 'E' - offset by one letter width plus spacing
            
            for (int c = 0; c < 2; c++) {
                uint8_t byte = pgm_read_byte(&letter_E[r * 2 + c]);
                for (int b = 0; b < 8; b++) {
                    if (byte & (0x80 >> b)) {
                        u8g2->drawPixel(startX + 16 + letterSpacing + c * 8 + b, startY + r);
                    }
                }
            }

            
            // Letter 'M' - offset by two letter widths plus spacing
            
            for (int c = 0; c < 2; c++) {
                uint8_t byte = pgm_read_byte(&letter_M[r * 2 + c]);
                for (int b = 0; b < 8; b++) {
                    if (byte & (0x80 >> b)) {
                        u8g2->drawPixel(startX + 32 + letterSpacing * 2 + c * 8 + b, startY + r);
                    }
                }
            }

            
            // Letter 'O' - offset by three letter widths plus spacing
            
            for (int c = 0; c < 2; c++) {
                uint8_t byte = pgm_read_byte(&letter_O[r * 2 + c]);
                for (int b = 0; b < 8; b++) {
                    if (byte & (0x80 >> b)) {
                        u8g2->drawPixel(startX + 48 + letterSpacing * 3 + c * 8 + b, startY + r);
                    }
                }
            }
        }

        u8g2->sendBuffer();
        delay(35);  // Brief delay between row reveals for smooth animation
    }

    // Small pause before subtitle appears
    delay(200);

    
    // Platform subtitle - displays full name across three lines
    // Each line appears with a delay for a "typewriter" effect
    
    u8g2->setFont(u8g2_font_5x7_tf);  // Smaller font for subtitle text

    // Line 1: "NMEA2000 Education &"
    u8g2->drawStr(4, 38, "NMEA2000 Education &");
    u8g2->sendBuffer();
    delay(150);

    // Line 2: "Maritime Operations"
    u8g2->drawStr(8, 48, "Maritime Operations");
    u8g2->sendBuffer();
    delay(150);

    // Line 3: "Platform" (centered below)
    u8g2->drawStr(40, 58, "Platform");
    u8g2->sendBuffer();

    // Hold the complete logo for viewing
    delay(2000);

    
    // Phase 3: Fade out effect
    // Progressively removes pixels to create a dissolve transition
    
    for (int fade = 0; fade < 8; fade++) {
        u8g2->clearBuffer();

        
        // Gradual pixel removal using modulo-based pattern
        // As fade increases, more pixels are skipped, creating dissolve effect
        // fade=0: all pixels shown
        // fade=7: almost no pixels shown (nearly blank)
        // fade=8: completely blank (loop exits)
        
        if (fade < 7) {
            int startX = 10;
            int startY = 4;
            int letterSpacing = 4;

            for (int r = 0; r < 24; r++) {
                // Skip more rows as fade progresses
                // Pattern: keep row if (r + fade) % (fade + 1) == 0
                if ((r + fade) % (fade + 1) != 0) continue;

                // Letter 'N' with pixel skipping
                for (int c = 0; c < 2; c++) {
                    uint8_t byte = pgm_read_byte(&letter_N[r * 2 + c]);
                    for (int b = 0; b < 8; b++) {
                        // Skip more pixels horizontally as fade increases
                        if ((b + fade) % (fade + 1) != 0) continue;
                        if (byte & (0x80 >> b)) {
                            u8g2->drawPixel(startX + c * 8 + b, startY + r);
                        }
                    }
                }

                // Letter 'E' with pixel skipping
                for (int c = 0; c < 2; c++) {
                    uint8_t byte = pgm_read_byte(&letter_E[r * 2 + c]);
                    for (int b = 0; b < 8; b++) {
                        if ((b + fade) % (fade + 1) != 0) continue;
                        if (byte & (0x80 >> b)) {
                            u8g2->drawPixel(startX + 16 + letterSpacing + c * 8 + b, startY + r);
                        }
                    }
                }

                // Letter 'M' with pixel skipping
                for (int c = 0; c < 2; c++) {
                    uint8_t byte = pgm_read_byte(&letter_M[r * 2 + c]);
                    for (int b = 0; b < 8; b++) {
                        if ((b + fade) % (fade + 1) != 0) continue;
                        if (byte & (0x80 >> b)) {
                            u8g2->drawPixel(startX + 32 + letterSpacing * 2 + c * 8 + b, startY + r);
                        }
                    }
                }

                // Letter 'O' with pixel skipping
                for (int c = 0; c < 2; c++) {
                    uint8_t byte = pgm_read_byte(&letter_O[r * 2 + c]);
                    for (int b = 0; b < 8; b++) {
                        if ((b + fade) % (fade + 1) != 0) continue;
                        if (byte & (0x80 >> b)) {
                            u8g2->drawPixel(startX + 48 + letterSpacing * 3 + c * 8 + b, startY + r);
                        }
                    }
                }
            }
        }

        u8g2->sendBuffer();
        delay(60);  // Slightly longer delay for visible fade steps
    }

    
    // Final cleanup - clear screen before handing control to menu system
    
    u8g2->clearBuffer();
    u8g2->sendBuffer();
    delay(100);  // Brief pause before menu appears
}
