#include "pico/stdlib.h"
#include "stdarg.h"
#include "stdio.h"
#include "string.h"

#include "FreeRTOS.h"
#include "task.h"

#include "window.h"
#include "window_rtos.h"

#include "vga.h"
#include "gfx.h"
#include "ps2.h"

TermWindow *activeWindow = NULL;
TermWindow *windowCarousel[MAX_WINDOWS];
uint nrWindows = 0;
uint activeNr;

/// @brief Sets focus to the specified window
/// @param w pointer to the window to focus to
void Window_setActiveWindow(TermWindow *w)
{
    if (activeWindow != NULL)
        GFX_fillCircle(activeWindow->xPos + activeWindow->xRes - 10, activeWindow->yPos - 7, 3, WHITE);
    activeWindow = w;
    GFX_fillCircle(activeWindow->xPos + activeWindow->xRes - 10, activeWindow->yPos - 7, 3, GREEN);
}

/// @brief Initializes an already existing window
/// @param w Pointer to the window to be initialized
/// @param xPos X coordinate of the window on screen
/// @param yPos Y coordinate of the window on screen
/// @param xSize Horizontal size of the window
/// @param ySize Vertical size of the window
/// @param name Name of the window
/// @param borderCol Border colour of the window
void Window_initWindow(TermWindow *w, uint xPos, uint yPos, uint xSize, uint ySize, char name[], uint8_t borderCol)
{
    // Align everything to even collumns, to make shifting lines easier (because the framebuffer is 3bpp, 1 byte = 2 pixels)
    if (xPos % 2)
        xPos++;
    if (xSize % 2)
        xSize++;

    w->xPos = xPos + 2;
    w->yPos = yPos + 2;
    w->xRes = xSize;
    w->yRes = ySize;
    Window_setTextSize(w, 1);

    w->borderCol = borderCol;
    GFX_drawRect(xPos, yPos, xSize + 4, ySize + 4, borderCol);
    GFX_fillRect(w->xPos - 2, yPos - 10, xSize + 4, 10, WHITE);

    GFX_setCursor(xPos + 1, yPos - 9);
    GFX_setTextColor(BLACK);
    GFX_printf(name);

    Window_setTextColour(w, WHITE);
    Window_setCursor(w, 0, 0);

    activeNr = nrWindows;
    windowCarousel[nrWindows++] = w;
    Window_setActiveWindow(w);
}

/// @brief Creates and initalizes a window
/// @param xPos X coordinate of the window on screen
/// @param yPos Y coordinate of the window on screen
/// @param xSize Horizontal size of the window
/// @param ySize Vertical size of the window
/// @param name Name of the task and window
/// @param borderCol Border colour of the window
/// @return Pointer to the created window
TermWindow *Window_createWindow(uint xPos, uint yPos, uint xSize, uint ySize, char name[], uint8_t borderCol)
{
    TermWindow *w = pvPortMalloc(sizeof(TermWindow));
    Window_initWindow(w, xPos, yPos, xSize, ySize, name, borderCol);
    return w;
}

/// @brief Shifts focus to the next window
void Window_nextWindow()
{
    if (activeNr < nrWindows - 1)
        activeNr++;
    else
        activeNr = 0;
    kbKeys = 0; // clear keypress buffer when switching focus
    Window_setActiveWindow(windowCarousel[activeNr]);
}

/// @brief Returns number of text rows in specified window
/// @param w window
/// @return number of rows
uint Window_getRows(TermWindow *w)
{
    return w->term_rows;
}

/// @brief Returns number of collumns in specified window
/// @param w window
/// @return number of collumns
uint Window_getCols(TermWindow *w)
{
    return w->term_cols;
}

void Window_splash()
{
    GFX_setCursor(10, 170);
    GFX_setTextSize(3);
    GFX_printf("Welcome to the");
    GFX_setTextSize(4);
    GFX_setTextColor(RED);
    GFX_printf("\n   Pico ");
    GFX_setTextColor(GREEN);
    GFX_printf("Windowing ");
    GFX_setTextColor(BLUE);
    GFX_printf("System");
    GFX_setTextSize(2);
    GFX_setCursor(0, 237);
    GFX_setTextColor(YELLOW);
    GFX_printf("\n\n\n\n\n\nver. %s\n", WINDOW_VER);
    GFX_setTextColor(WHITE);
    GFX_printf("Based on FreeRTOS %s\n", tskKERNEL_VERSION_NUMBER);

    while (!PS2_keyAvailable())
        ;
    char c = PS2_readKey();
}

/// @brief Initializes the PS2 keyboard and VGA display
/// @param d PS2 data pin
/// @param c PS2 clock pin
/// @param vsync_pin VGA VSync pin
/// @param hsync_pin VGA HSync pin
/// @param r_pin  VGA Red pin (or blue if using BGR)
void Window_initIO(uint d, uint c, uint vsync_pin, uint hsync_pin, uint r_pin)
{
    PS2_init(d, c);
    VGA_initDisplay(vsync_pin, hsync_pin, r_pin);
    Window_splash();
    VGA_fillScreen(BLACK);
}
