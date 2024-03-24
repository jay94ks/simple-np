#include "ledctl.h"
#include "config.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include <string.h>

Ledctl::Ledctl() {
    _prev = 0xff;
    _next = 0x00;

    gpio_init(GPIO_HC595_SER);
    gpio_init(GPIO_HC595_CLK);
    gpio_init(GPIO_HC595_LAT);
    
    gpio_set_dir(GPIO_HC595_SER, GPIO_OUT);
    gpio_set_dir(GPIO_HC595_CLK, GPIO_OUT);
    gpio_set_dir(GPIO_HC595_LAT, GPIO_OUT);
}

Ledctl* Ledctl::get() {
    static Ledctl _ctl;
    return &_ctl;
}

void Ledctl::updateOnce() {
    if (_prev == _next) {
        return;
    }

    const uint8_t nextOrg = _next;
    uint8_t next = _next;

    gpio_put(GPIO_HC595_LAT, 0);
    gpio_put(GPIO_HC595_CLK, 0);
    sleep_us(1);

    for(uint8_t i = 0; i < 8; ++i) {
        const uint8_t state = (0x80 & next) != 0;
        next = next << 1;
        
        gpio_put(GPIO_HC595_CLK, 0);
        sleep_us(1);

        gpio_put(GPIO_HC595_SER, state);
        sleep_us(1);

        gpio_put(GPIO_HC595_CLK, 1);
        sleep_us(1);
    }
    
    gpio_put(GPIO_HC595_CLK, 0);
    sleep_us(1);
    
    gpio_put(GPIO_HC595_LAT, 1);
    sleep_us(1);

    gpio_put(GPIO_HC595_LAT, 0);
    _prev = nextOrg;
}

bool Ledctl::get(ELED led) {
    return (_next & (1 << led)) == 0;
}

void Ledctl::set(ELED led, bool state) {
    const uint8_t mask = 1 << led;

    // --> active low.
    if (state) {
        _next &= ~mask;
    }
    else {
        _next |= mask;
    }
}

bool Ledctl::toggle(ELED led) {
    bool ret = get(led);
    set(led, ret = !ret);
    return ret;
}