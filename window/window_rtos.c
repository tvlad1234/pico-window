
#include "pico/stdlib.h"
#include "window_rtos.h"
#include "window.h"
#include "ps2.h"

#include "freertos.h"
#include "task.h"
#include "semphr.h"

char kbBuf[50];
uint kbKeys = 0;

TaskHandle_t keyScanHandle;
SemaphoreHandle_t keySemaphore;

TaskHandle_t windowTaskList[MAX_WINDOWS];
uint numCreatedTasks = 0;

void giveKeySemaphore()
{
    xSemaphoreGive(keySemaphore);
}

void takeKeySemaphore()
{
    xSemaphoreTake(keySemaphore, portMAX_DELAY);
}

/// @brief Yields CPU time to other tasks
void Window_taskYield()
{
    taskYIELD();
}

void keyScan(void *p)
{
    char c;

    while (true)
    {
        while (!PS2_keyAvailable())
            Window_taskYield();
        takeKeySemaphore();
        c = PS2_readKey();
        switch (c)
        {
        case PS2_SHIFT_TAB:
            Window_nextWindow();
            break;

        default:
            kbBuf[kbKeys++] = c;
            break;
        }
        giveKeySemaphore();
    }
}

/// @brief Creates a task and an associated window. The pointer to the created window gets transmitted to the task.
/// @param taskFunc Function which the task will use
/// @param xPos X coordinate of the window on screen
/// @param yPos Y coordinate of the window on screen
/// @param xSize Horizontal size of the window
/// @param ySize Vertical size of the window
/// @param name Name of the task and window
/// @param borderCol Border colour of the window
void Window_createTaskWithWindow(TaskFunction_t taskFunc, uint xPos, uint yPos, uint xSize, uint ySize, char name[], uint8_t borderCol)
{
    TermWindow *w = Window_createWindow(xPos, yPos, xSize, ySize, name, borderCol);
    xTaskCreate(taskFunc, name, 2048, w, 1, &windowTaskList[numCreatedTasks++]);
}

/// @brief Blocks calling task for some amount of milliseconds
/// @param ms how many milliseconds the task should sleep
void Window_delay(uint ms)
{
    vTaskDelay(ms / portTICK_PERIOD_MS);
}

/// @brief Starts the FreeRTOS scheduler
void Window_startRTOS()
{
    keySemaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(keySemaphore);
    xTaskCreate(keyScan, "KeyScan", 128, NULL, 1, &keyScanHandle);

    // Start FreeRTOS kernel
    vTaskStartScheduler();
    panic("RTOS kernel not running!"); // we shouldn't get here
}