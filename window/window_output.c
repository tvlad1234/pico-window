#include "pico/stdlib.h"
#include "stdarg.h"
#include "stdio.h"
#include "string.h"

#include "vga.h"
#include "gfx.h"

#include "window.h"

/// @brief Sets the text size of specified window
/// @param w Window of which text size to set
/// @param s Text size
void Window_setTextSize(TermWindow *w, uint s)
{
    w->textSize = s;
    w->term_rows = (w->yRes - 7) / (8 * w->textSize) - 1;
    w->term_cols = w->xRes / (6 * w->textSize) - 1;
    GFX_setTextSize(s);
}

/// @brief Places the cursor of a window to specified location
/// @param w Window of which cursor to move
/// @param col Collumn to move cursor to
/// @param row Row to move cursor to
void Window_setCursor(TermWindow *w, int col, int row)
{
    w->currentCol = col;
    w->currentRow = row;
    GFX_setCursor(w->xPos + 6 * w->textSize * col + 2, w->yPos + 8 * w->textSize * row + 11);
}

void Window_CopyPixelLine(TermWindow *w, uint dst, uint src)
{
    extern unsigned char vga_data_array[TXCOUNT];

    uint8_t *realSrc = vga_data_array + (320 * (src + w->yPos + 10)) + (w->xPos / 2);
    uint8_t *realDst = vga_data_array + (320 * (dst + w->yPos + 10)) + (w->xPos / 2);
    uint transferSize = w->xRes / 2;
    dma_memcpy(realDst, realSrc, transferSize);
}

void Window_DrawLineColor(TermWindow *w, uint line, uint8_t color)
{
    extern unsigned char vga_data_array[TXCOUNT];

    uint8_t *realDst = vga_data_array + (320 * (line + 10 + w->yPos)) + (w->xPos / 2)+1;
    uint transferSize = w->xRes / 2 - 2;
    dma_memset(realDst, color, transferSize);
}

/// @brief Scroll down a number of text lines
/// @param w Window to scroll
/// @param linesNum How many text lines to scroll
void Window_scrollLines(TermWindow *w, int linesNum)
{
    uint startingLine = (8 * w->textSize * linesNum);   // How many pixel lines we wanna shift up
    uint totalLines = w->term_rows * 8 * w->textSize; // w->yRes;
    uint endingLine = totalLines - startingLine + 2;

    for (int i = 0; i < endingLine; i++)
        Window_CopyPixelLine(w, i, startingLine + i);

    for (int i = endingLine; i <= totalLines; i++)
        Window_DrawLineColor(w, i, BLACK);
}

/// @brief Clears a window and places the cursor at the beginning
/// @param w Window to clear
void Window_clear(TermWindow *w)
{
    for (int i = 10; i < w->yRes-11; i++)
        Window_DrawLineColor(w, i, BLACK);

    w->currentCol = 0;
    w->currentRow = 0;
    Window_setCursor(w, w->currentCol, w->currentRow);
    Window_setTextSize(w, 1);
}

/// @brief Writes a single character to specified window at the current cursor position
/// @param w Window to write to
/// @param c Character to write
void Window_write(TermWindow *w, unsigned char c)
{
    GFX_setTextColor(w->textCol);
    Window_setCursor(w, w->currentCol, w->currentRow);
    if (c == '\n' || c == '\r')
        w->currentCol = 0;
    if(c == '\n')
        w->currentRow++;
    else if (c == '\b' || c == PS2_BACKSPACE)
    {
        if ((w->currentRow == 0 && w->currentCol) || (w->currentRow))
        {
            if (w->currentCol > 0)
                w->currentCol--;
            else
            {
                w->currentCol = w->term_cols - 1;
                w->currentRow--;
            }
            Window_setCursor(w, w->currentCol, w->currentRow);
            GFX_write(' ');
        }
    }
    else
    {
        GFX_write(c);
        w->currentCol++;
    }

    if (w->currentCol == w->term_cols)
    {
        w->currentRow++;
        w->currentCol = 0;
    }

    if (w->currentRow == w->term_rows)
    {
        Window_scrollLines(w, 1);
        w->currentRow = w->term_rows - 1;
        w->currentCol = 0;
    }
}

/// @brief Prints a string to specified window
/// @param w Window to write to
/// @param s String to write
void Window_printString(TermWindow *w, char s[])
{
    uint8_t n = strlen(s);
    for (int i = 0; i < n; i++)
        Window_write(w, s[i]);
}

/// @brief Prints a formatted string to specified window
/// @param w Window to write to
/// @param format Format string
/// @param  
void Window_printf(TermWindow *w, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vsprintf(w->termPrintBuf, format, args);
    Window_printString(w, w->termPrintBuf);
    va_end(args);
}

/// @brief Sets the colour of text to be written to specified window
/// @param w Window
/// @param col Text colour
void Window_setTextColour(TermWindow *w, uint8_t col)
{
    GFX_setTextColor(col);
    w->textCol = col;
}
