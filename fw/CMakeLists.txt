cmake_minimum_required(VERSION 3.13)

# PICO_SDK_PATH required.
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(simple_np C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()
add_executable(simple_np
    main.cpp
    board/ledctl.cpp
    board/usbd.cpp
    board/usbd/descriptors.cpp
    board/usbd/hid_notifier.cpp
    board/usbd/cdc_message.cpp
    kbd/kbd.cpp
    kbd/scanners/basic.cpp
    kbd/handlers/numlock.cpp
    kbd/handlers/userfn.cpp
    task/task.cpp
    task/taskqueue.cpp
    tft/tft.cpp
    lib/st7735/ST7735_TFT_Bitmap_Data.cpp
    lib/st7735/ST7735_TFT_Font.cpp
    lib/st7735/ST7735_TFT_graphics.cpp
    lib/st7735/ST7735_TFT_Print.cpp
    lib/st7735/ST7735_TFT.cpp
)

pico_enable_stdio_usb(simple_np 0)
target_link_libraries(simple_np
    pico_stdlib
    pico_unique_id
    pico_bootrom
    pico_multicore
    pico_sync
    tinyusb_device
    tinyusb_board
    hardware_gpio
    hardware_spi
    hardware_pwm
)

# --> to make TinyUSB to pick up tusb_config.h file.
target_include_directories(simple_np
PRIVATE 
    ${CMAKE_CURRENT_LIST_DIR}
    #$ENV{PICO_SDK_PATH}/lib/tinyusb/hw
    #$ENV{PICO_SDK_PATH}/lib/tinyusb/src
)

pico_add_extra_outputs(simple_np)
    