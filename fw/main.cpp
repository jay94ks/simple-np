#include "kbd/kbd.h"
#include "tft/tft.h"
#include "board/ledctl.h"
#include "board/usbd.h"
#include "task/taskqueue.h"
#include "mode/mode.h"
#include "pico/stdlib.h"
#include <tusb.h>

int main(void) {
    tty_print("tft: init.\n");
    TaskQueue::prepare();

    Kbd* kbd = Kbd::get();
    tty_print("kbd: init.\n");

    Ledctl* led = Ledctl::get();
    tty_print("led: init.\n");

    Usbd* usbd = Usbd::get();
    if (!usbd->init()) {
        tty_print("usb: fatal.\n");
        
        while(true) {
            led->toggle(ELED_MREC);
            sleep_ms(250);
        }
    }

    // --> try to enter default mode.
    IMode::trySetCurrent(nullptr);
    while(true) {
        kbd->scanOnce();
        led->updateOnce();
        usbd->stepOnce();
        
        // --> calls mode-specific stepper routine.
        if (IMode* mode = IMode::getCurrent()) {
            mode->stepOnce();
        }
    }
}