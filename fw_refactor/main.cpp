#include "kbd/kbd.h"
#include "pico/stdlib.h"
#include "pico/multicore.h"

void core_main() {
    // todo: TFT display.
    multicore_fifo_push_blocking(1);
    while(1) {
        sleep_ms(10);
    }
}

int main(void) {
    multicore_launch_core1(core_main);

    // --> wait for core_main.
    multicore_fifo_pop_blocking(); 
    
    Kbd* kbd = Kbd::get();
    while(true) {
        kbd->scanOnce();

        
    }
}