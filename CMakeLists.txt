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
#
# DCC Packet Monitor
#
# Real-time NMRA DCC Packet Analyzer
# for Raspberry Pi Pico (RP2040)
#
# Copyright (C) 2026 Nucky
#
# SPDX-License-Identifier: GPL-3.0-only
#

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ============================================================
# Pico SDK
# ============================================================

if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()

set(sdkVersion 2.3.1)
set(toolchainVersion 15_2_Rel1)
set(picotoolVersion 2.3.1)

set(
    picoVscode
    ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake
)

if(EXISTS ${picoVscode})
    include(${picoVscode})
endif()

# ============================================================
# Board Configuration
# ============================================================

set(PICO_BOARD pico CACHE STRING "Board type")

include(pico_sdk_import.cmake)

# ============================================================
# Project
# ============================================================

project(
    DCC_Packet_Monitor
    C
    CXX
    ASM
)

pico_sdk_init()

# ============================================================
# Executable
# ============================================================

add_executable(
    DCC_Packet_Monitor

    #
    # Main
    #
    main.c

    #
    # DCC Receive Layer
    #
    dcc_rx.c
    dcc_decode.c
    dcc_packet.c

    #
    # Analysis Layer
    #
    packet_analyzer.c
    loco_analyzer.c
    accessory_analyzer.c
    service_analyzer.c
    ops_analyzer.c

    #
    # Configuration / User Interface
    #
    monitor_config.c
    menu.c

    #
    # Statistics
    #
    statistics.c
)

# ============================================================
# Program Information
# ============================================================

pico_set_program_name(
    DCC_Packet_Monitor
    "DCC Packet Monitor"
)

pico_set_program_version(
    DCC_Packet_Monitor
    "1.0.0"
)

# ============================================================
# STDIO
# ============================================================

pico_enable_stdio_uart(
    DCC_Packet_Monitor
    0
)

pico_enable_stdio_usb(
    DCC_Packet_Monitor
    1
)

# ============================================================
# Libraries
# ============================================================

target_link_libraries(
    DCC_Packet_Monitor
    pico_stdlib
)

# ============================================================
# Include Directories
# ============================================================

target_include_directories(
    DCC_Packet_Monitor
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

# ============================================================
# Outputs
# ============================================================

pico_add_extra_outputs(
    DCC_Packet_Monitor
)