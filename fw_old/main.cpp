#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "kbd/keymap.h"
#include "kbd/toggle.h"
#include "ledctl.h"
#include "usb/usbd.h"
#include "msg/receiver.h"
#include "tft/tft.h"

void core_main() {
    tft_init();
    tft_print("tftctl: init.\n");

    multicore_fifo_push_blocking(1);
    while(1) {
        tft_task();
        sleep_ms(10);
    }
}

int main(void) {
    multicore_launch_core1(core_main);
    multicore_fifo_pop_blocking(); // --> wait for core_main.
    
    tft_print("usbd: init.\n");
    usbd_init();

    tft_print("ledctl: init.\n");
    ledctl_init();
    
    tft_print("kbd: init.\n");
    kbd_keymap_init();
    
    // --> key handling.
    while(1) {
        kbd_keymap_task();
        ledctl_task();
        usbd_task();
    }

    return 0;
}