# pico-window
Multitasking text-based windowing system for Raspberry Pi Pico, using a PS/2 keyboard, VGA monitor and FreeRTOS \
Example code can be found [here](https://github.com/tvlad1234/pico-window-example.git). 

## Requirements
- a Raspberry Pi Pico (or any other RP2040-based board)
- a VGA monitor
    - 3x 330hm resistors
- a PS/2 keyboard
    - some form of level shifting (from 5V to 3.3V)

## How it works
This library creates an environment that allows you to simultaneously run multiple windowed programs on the Pi Pico,as tasks FreeRTOS tasks. Programs can read/write characters from/into one or more windows. Keypresses are only sent to the window in focus. Focus can be switched between windows by pressing Shift+Tab.

## Building apps
In order to use *pico-window*, you will have to clone this repository with its submodules and include it as a library in your project (add the *pico-window* subdirectory into your CMakeLists and link the *window* library in *target_link_libraries*, then include *window.h* in your source files). Example code can be found [here](https://github.com/tvlad1234/pico-window-example.git). 

Projects built with this framework should respect the following flow:
-  Initialization
    - Initialize sensors, GPIO and other hardware needed for your project
    - Initialize the *pico-window* I/O (display and keyboard)
        - done by calling `Window_initIO`.
- Create FreeRTOS tasks (threads) and/or windows. 
     - The simplest way to create a task with a corresponding window is to use the `Window_createTaskWithWindow` function. It creates a FreeRTOS task and a window which is passed as a parameter to the task.
    - You can also create the tasks and windows by yourself.
- Start the FreeRTOS scheduler.
    - done by calling `Window_startRTOS`
        - this gives takes control from main and gives it to the scheduler.
        - After this, all program code will run from tasks or ISRs.

As applications are exectued as tasks in FreeRTOS, they should run in an infinite loop. The basic structure of an aplication function is described below:

```c
void myApp (void *p) {
    declare_and_init_variables();
    instructions_that_only_run_once();

    while(true)
    {
        application_loop();
    }
}
```
When creating a task with `Window_createTaskWithWindow`, the p parameter is a pointer to the window that was created and assigned to the function.

## Provided functions

### Initializing the system
- `void Window_initIO(uint d, uint c, uint vsync_pin, uint hsync_pin, uint r_pin);` initializes the PS/2 keyboard and VGA monitor. It takes the used GPIO pin numbers as parameters.

- `void Window_startRTOS();` starts the FreeRTOS scheduler

### Creating windows and tasks
- `void Window_createTaskWithWindow(TaskFunction_t taskFunc, uint xPos, uint yPos, uint xSize, uint ySize, char name[], uint8_t borderCol);` creates a task with the specified entry function and a window with the specified parameters. Using this function passes the pointer to the created window as a paramater to the task.

- `TermWindow *Window_createWindow(uint xPos, uint yPos, uint xSize, uint ySize, char name[], uint8_t borderCol);` creates a window and returns its address.

- `void Window_initWindow(TermWindow *w, uint xPos, uint yPos, uint xSize, uint ySize, char name[], uint8_t borderCol);` initializes an already existing window.

### Manipulating windows
- `void Window_setActiveWindow(TermWindow *w);` switches focus to specified window

- `void Window_nextWindow();` switches focus to the next window. The order in which they're given focus is the one in which they were initialized.

- `uint Window_getRows(TermWindow *w);` return the number of usable text rows in a window

- `uint Window_getCols(TermWindow *w);` returns the number of usable text collumns in a window
- `void Window_setTextSize(TermWindow *w, uint s);` sets the text size for specified window

- `void Window_setCursor(TermWindow *w, int col, int row);` places the cursor at the specified location for a specified window. Note that this *does not* switch focus to said window.

- `void Window_setTextColour(TermWindow *w, uint8_t col);` sets the text colour for a specified window

- `void Window_clear(TermWindow *w);` clears a window and places the cursor at the top

- `void Window_scrollLines(TermWindow *w, int linesNum);` scroll window contents down by a number of lines

### Text output
- `void Window_write(TermWindow *w, unsigned char c);` writes a character to the window, at the current cursor position

- `void Window_printString(TermWindow *w, char s[]);` prints a string to the window

- `void Window_printf(TermWindow *w, const char *format, ...);` works like a regular *printf*, except it outputs to a window

### Text input
- `char Window_getchar(TermWindow *w);` reads a character from the keyboard. It waits until the window is in focus and there are keypresses to be read.

- `void Window_readString(TermWindow *w, char termScanBuf[]);` reads a string from the keyboard, until the Return key is pressed. 

- `void Window_scanf(TermWindow *w, const char *format, ...);` works like a regular *scanf*. 

### Program control
- `void Window_taskYield();` yields processor time to other tasks

- `void Window_delay(uint ms);` blocks the program for a number of milliseconds



    
