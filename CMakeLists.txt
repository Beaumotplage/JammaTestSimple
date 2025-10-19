# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2 CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(JammaTest2025 C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(JammaTest2025 JammaTest2025.cpp 
    pwm_audio.cpp
    hal_Video.cpp 
    hal_adc.cpp
    hal_spi.cpp
    app/app_audio.cpp 
    app/app_main.cpp 
    app/app_global_utils.cpp 
    app/app_trigtables.cpp
    app/app_polygons_fpu.cpp
    app/app_simple.cpp
    app/app_inputs.cpp 
    app/app_shapes.cpp     
    app/hxcmod.c )

# must match with pio filename and executable name from above
pico_generate_pio_header(JammaTest2025  ${CMAKE_CURRENT_LIST_DIR}/rgb.pio)
pico_generate_pio_header(JammaTest2025  ${CMAKE_CURRENT_LIST_DIR}/rgb_notransparency.pio)
pico_generate_pio_header(JammaTest2025  ${CMAKE_CURRENT_LIST_DIR}/csync.pio)
pico_set_program_name(JammaTest2025 "JammaTest2025")
pico_set_program_version(JammaTest2025 "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(JammaTest2025 0)
pico_enable_stdio_usb(JammaTest2025 0)

# Add the standard library to the build
target_link_libraries(JammaTest2025
      	pico_stdlib
	hardware_pio 
	hardware_dma 
	hardware_adc 
	hardware_irq 
	hardware_pwm 
	hardware_spi 
	hardware_interp
	pico_multicore
	)

# Add the standard include files to the build
target_include_directories(JammaTest2025 PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

pico_add_extra_outputs(JammaTest2025)

