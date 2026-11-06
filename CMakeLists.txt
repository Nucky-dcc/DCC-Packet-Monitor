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
set(sdkVersion 2.3.1)
set(toolchainVersion 15_2_Rel1)
set(picotoolVersion 2.3.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(DCC_Monitor C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(DCC_Monitor
    main.c
    dcc_rx.c
    dcc_decode.c
    dcc_packet.c
    packet_analyzer.c
    loco_analyzer.c
    accessory_analyzer.c
    service_analyzer.c
    ops_analyzer.c
    monitor_config.c
    menu.c
    statistics.c
)

pico_set_program_name(DCC_Monitor "DCC_Monitor")
pico_set_program_version(DCC_Monitor "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(DCC_Monitor 0)
pico_enable_stdio_usb(DCC_Monitor 1)

# Add the standard library to the build
target_link_libraries(DCC_Monitor
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(DCC_Monitor PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

pico_add_extra_outputs(DCC_Monitor)

