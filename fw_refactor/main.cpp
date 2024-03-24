#include "kbd/kbd.h"
#include "task/taskqueue.h"
#include "pico/stdlib.h"

int main(void) {
    TaskQueue::prepare();
    Kbd* kbd = Kbd::get();

    while(true) {
        kbd->scanOnce();

        
    }
}