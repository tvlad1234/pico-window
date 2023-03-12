#ifndef _WINDOW_H
#define _WINDOW_H

#include "pico/stdlib.h"
#include "FreeRTOS.h"

#define VGA_BGR 0
#define MAX_WINDOWS 10

#define WINDOW_VER "1.00-dev"

#if VGA_BGR
#define BLACK 0b0
#define RED 0b100
#define GREEN 0b010
#define YELLOW 0b110
#define BLUE 0b001
#define MAGENTA 0b101
#define CYAN 0b011
#define WHITE 0b111
#else
#define BLACK 0
#define RED 1
#define GREEN 2
#define YELLOW 3
#define BLUE 4
#define MAGENTA 5
#define CYAN 6
#define WHITE 7
#endif

#define PS2_TAB 9
#define PS2_ENTER 13
#define PS2_BACKSPACE 127
#define PS2_ESC 27
#define PS2_DELETE 127
#define PS2_PAGEUP 25
#define PS2_PAGEDOWN 26
#define PS2_UPARROW 11
#define PS2_LEFTARROW 8
#define PS2_DOWNARROW 10
#define PS2_RIGHTARROW 21

#define KEY_BUF_LEN 25

typedef struct TermWindow
{
    uint xPos, yPos;
    uint xRes, yRes;
    uint term_rows, term_cols;
    uint currentRow, currentCol;
    uint textSize;

    uint bgCol;
    uint textCol;
    uint borderCol;

    uint numKeys;
    char keyBuf[KEY_BUF_LEN];

    char termPrintBuf[50];

    bool enableEcho;
    void *miscParam;
    int userIndex;

} TermWindow;

extern TermWindow *activeWindow;


void Window_initIO(uint d, uint c, uint vsync_pin, uint hsync_pin, uint r_pin);

void Window_createTaskWithWindow(TaskFunction_t taskFunc, uint xPos, uint yPos, uint xSize, uint ySize, char name[], uint8_t borderCol, void *windowParam);
void Window_startRTOS();

void Window_setUserIndex(TermWindow *w, int x);

void Window_destroy(TermWindow *w);
void Window_initWindow(TermWindow *w, uint xPos, uint yPos, uint xSize, uint ySize, char name[], uint8_t borderCol);
TermWindow *Window_createWindow(uint xPos, uint yPos, uint xSize, uint ySize, char name[], uint8_t borderCol);
void Window_setActiveWindow(TermWindow *w);
void Window_nextWindow();

uint Window_getRows(TermWindow *w);
uint Window_getCols(TermWindow *w);
void Window_setTextSize(TermWindow *w, uint s);
void Window_setCursor(TermWindow *w, int col, int row);
void Window_setTextColour(TermWindow *w, uint8_t col);

void Window_clear(TermWindow *w);
void Window_scrollLines(TermWindow *w, int linesNum);

void Window_write(TermWindow *w, unsigned char c);
void Window_printString(TermWindow *w, char s[]);
void Window_printf(TermWindow *w, const char *format, ...);

uint8_t Window_pressedKey(TermWindow *w);
char Window_getchar(TermWindow *w);
void Window_readString(TermWindow *w, char termScanBuf[]);
int Window_scanf(TermWindow *w, const char *format, ...);

void Window_taskYield();
void Window_delay(uint ms);

#endif