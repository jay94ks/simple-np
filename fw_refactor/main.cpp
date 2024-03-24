#include "kbd/kbd.h"
#include "tft/tft.h"
#include "board/ledctl.h"
#include "task/taskqueue.h"
#include "pico/stdlib.h"

int main(void) {
    TaskQueue::prepare();
    tty_printf("tft: init.\n");

    Kbd* kbd = Kbd::get();
    tty_printf("kbd: init.\n");

    Ledctl* led = Ledctl::get();
    tty_printf("led: init\n");

    while(true) {
        kbd->scanOnce();
        led->updateOnce();

        
    }
}