cmake_minimum_required(VERSION 3.13)

# PICO_SDK_PATH required.
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(simple_np C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()
add_executable(simple_np
    main.cpp
    kbd/kbd.cpp
)

pico_enable_stdio_usb(simple_np 0)
target_link_libraries(simple_np
    pico_stdlib
    pico_unique_id
    pico_bootrom
    pico_multicore
    tinyusb_device
    hardware_gpio
    hardware_spi
    hardware_pwm
)

# --> to make TinyUSB to pick up tusb_config.h file.
target_include_directories(simple_np
PRIVATE 
    .
    #$ENV{PICO_SDK_PATH}/lib/tinyusb/hw
    #$ENV{PICO_SDK_PATH}/lib/tinyusb/src
)

pico_add_extra_outputs(simple_np)
    