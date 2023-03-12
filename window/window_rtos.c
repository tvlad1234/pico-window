
#include "pico/stdlib.h"
#include "window_rtos.h"
#include "window.h"
#include "gfx.h"
#include "ps2.h"

#include "freertos.h"
#include "task.h"
#include "semphr.h"

TaskHandle_t keyScanHandle;
SemaphoreHandle_t keySemaphore;

TaskHandle_t infoBarHandle;

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

// Key scanning task
void keyScan(void *p)
{
    char c;

    while (true)
    {
        while (!PS2_keyAvailable() || activeWindow->numKeys == KEY_BUF_LEN)
            Window_delay(10);
        takeKeySemaphore();
        c = PS2_readKey();
        switch (c)
        {
        case PS2_SHIFT_TAB:
            Window_nextWindow();
            break;

        default:
            if (c == '\r')
                c = '\n';
            activeWindow->keyBuf[activeWindow->numKeys++] = c;
            activeWindow->keyBuf[activeWindow->numKeys] = '\0';

            if(activeWindow->enableEcho)
                Window_write(activeWindow, c);

            break;
        }
        giveKeySemaphore();
    }
}

// Info bar task
void infoBar(void *p)
{
    GFX_setTextSize(1);
    GFX_fillRect(0, 0, 640, 11, WHITE);
    while (true)
    {
        GFX_fillRect(0, 0, 160, 11, WHITE);
        GFX_setCursor(3, 2);
        GFX_setTextColor(BLACK);

        GFX_printf("Free memory: %.2f kB", xPortGetFreeHeapSize() / 1000.0f);

        Window_delay(1000);
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
/// @param windowParam Pointer to a user specified parameter, used for passing data to the task
void Window_createTaskWithWindow(TaskFunction_t taskFunc, uint xPos, uint yPos, uint xSize, uint ySize, char name[], uint8_t borderCol, void *windowParam)
{
    TermWindow *w = Window_createWindow(xPos, yPos, xSize, ySize, name, borderCol);
    w->miscParam = windowParam;
    xTaskCreate(taskFunc, name, 512, w, 1, &windowTaskList[numCreatedTasks++]);
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

    // Key scanning task
    xTaskCreate(keyScan, "KeyScan", 128, NULL, 2, &keyScanHandle);

    // Info bar task
    xTaskCreate(infoBar, "InfoBar", 256, NULL, 2, &infoBarHandle);

    // Start FreeRTOS kernel
    vTaskStartScheduler();
    panic("RTOS kernel not running!"); // we shouldn't get here
}