#ifndef _WINDOW_RTOS_H
#define _WINDOW_RTOS_H

#include "pico/stdlib.h"

extern char kbBuf[50];
extern uint kbKeys;

void giveKeySemaphore();
void takeKeySemaphore();



#endif