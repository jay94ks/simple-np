#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
//#include "tusb.h"
//#include "bsp/board.h"
#include "kbd/keymap.h"
#include "kbd/toggle.h"
#include "ledctl.h"

#include "usb/usbd.h"

// void core_main();
int main(void) {
    //stdio_init_all(); // --> debug only.
    //board_init();
    
    ledctl_init();
    kbd_keymap_init();
    usbd_init();
    
    // --> key handling.
    while(1) {
        kbd_keymap_task();
        ledctl_task();
        usbd_task();
    }

    return 0;
}