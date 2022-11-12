#include "pico/stdlib.h"
#include "stdarg.h"
#include "stdio.h"
#include "string.h"

#include "window.h"
#include "window_rtos.h"

/// @brief Gets a single character from specified window input, blocks if there are no characters to be read or if the window is not active
/// @param w Window from which to get input
/// @return Read character
char Window_getchar(TermWindow *w)
{
    while (w != activeWindow || !kbKeys)
        Window_taskYield();

    takeKeySemaphore();
    char c = kbBuf[0];
    kbKeys--;
    memcpy(kbBuf, kbBuf + 1, kbKeys);
    giveKeySemaphore();

    return c;
}

/// @brief Reads a string from specified window
/// @param w Window from which to get input
/// @param termScanBuf array to read characters into
void Window_readString(TermWindow *w, char termScanBuf[])
{
    termScanBuf[0] = '\0';
    char c = Window_getchar(w);
    int i = 0;
    while (c != PS2_ENTER)
    {
        if (c == PS2_BACKSPACE)
        {
            if (i > 0)
            {
                termScanBuf[i--] = '\0';
                Window_write(w, c);
            }
        }
        else
        {
            termScanBuf[i++] = c;
            termScanBuf[i] = '\0';
            Window_write(w, c);
        }
        c = Window_getchar(w);
    }
    Window_write(w, '\n');
}

/// @brief Reads formatted data from specified window
/// @param w Window from which to get input
/// @param format Format string
/// @param
void Window_scanf(TermWindow *w, const char *format, ...)
{
    va_list args;
    va_start(args, format);

    int i = 0;
    int fcount = 0;
    while (format[i])
        if (format[i++] == '%')
            fcount++;

    Window_readString(w, w->termScanBuf);

    while (vsscanf(w->termScanBuf, format, args) != fcount)
    {
        uint16_t col = w->textCol;
        Window_setTextColour(w, RED);
        Window_printf(w, "Invalid input!\n");
        Window_setTextColour(w, col);
        Window_readString(w, w->termScanBuf);
    }
    va_end(args);
}