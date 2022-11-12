add_library(window
	window.c
	window_rtos.c
	window_input.c
	window_output.c
)

target_include_directories(window PUBLIC
	.
)

target_link_libraries(window pico_stdlib ps2 vga freertos)