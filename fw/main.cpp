#include "kbd/kbd.h"
#include "tft/tft.h"
#include "board/ledctl.h"
#include "task/taskqueue.h"
#include "pico/stdlib.h"

int main(void) {
    TaskQueue::prepare();
    tty_print("tft: init.\n");

    Kbd* kbd = Kbd::get();
    tty_print("kbd: init.\n");

    Ledctl* led = Ledctl::get();
    tty_print("led: init.\n");


    while(true) {
        kbd->scanOnce();
        led->updateOnce();

        
    }
}