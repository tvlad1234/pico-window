#include "pico/stdlib.h"
#include "stdarg.h"
#include "stdio.h"
#include "string.h"

#include "window.h"
#include "window_rtos.h"

/// @brief Returns number of keypresses present in the buffer for the specified window
/// @param w Window for which to get number of keypresses
/// @return number of keypresses
uint8_t Window_pressedKey(TermWindow *w)
{
    return w->numKeys;
}

/// @brief Gets a single character from specified window input, blocks if there are no characters to be read or if the window is not active
/// @param w Window from which to get input
/// @return Read character
char Window_getchar(TermWindow *w)
{
    while (!(w == activeWindow && w->numKeys))
        Window_taskYield();

    takeKeySemaphore();
    char c = w->keyBuf[0];
    w->numKeys--;
    memcpy(w->keyBuf, w->keyBuf + 1, w->numKeys);
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
    while (c != '\n')
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
int Window_scanf(TermWindow *w, const char *format, ...)
{
    va_list args;
    va_start(args, format);

     w->enableEcho = true;

    while(w->numKeys == 0)
        Window_taskYield();
    
    while(w->keyBuf[w->numKeys-1] != '\n')
        Window_taskYield();

    w->enableEcho = false;

    w->keyBuf[w->numKeys-1] = '\0';
    int r = vsscanf(w->keyBuf, format, args);
    w->numKeys = 0;
    
    va_end(args);
    return r;
}