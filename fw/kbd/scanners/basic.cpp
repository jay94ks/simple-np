#include "basic.h"
#include "../../board/config.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include <string.h>

const uint8_t KbdBasicScanner::ROW_PINS[5] = {
    GPIO_KBD_ROW_1, GPIO_KBD_ROW_2, GPIO_KBD_ROW_3, 
    GPIO_KBD_ROW_4, GPIO_KBD_ROW_5
};

const uint8_t KbdBasicScanner::COL_PINS[5] = {
    GPIO_KBD_COL_1, GPIO_KBD_COL_2, GPIO_KBD_COL_3, 
    GPIO_KBD_COL_4, GPIO_KBD_COL_5
};

KbdBasicScanner::KbdBasicScanner() {
    _empty = 1;

    memset(_prevRows, 0, sizeof(_prevRows));
    memset(_nextRows, 0, sizeof(_nextRows));

    for(uint8_t row = 0; row < MAX_ROWS; ++row) {
        const uint8_t pin = ROW_PINS[row];
        
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_OUT);
    }

    for(uint8_t col = 0; col < MAX_COLS; ++col) {
        const uint8_t pin = COL_PINS[col];
        
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_IN);
        gpio_pull_down(pin);
    }
}

KbdBasicScanner *KbdBasicScanner::instance() {
    static KbdBasicScanner _scanner;
    return &_scanner;
}

bool KbdBasicScanner::scanOnce() {
    // --> store previous states.
    memcpy(_prevRows, _nextRows, sizeof(_nextRows));
    memset(_nextRows, 0, sizeof(_nextRows));
    
    // --> scan keys and fill its state to bitmap.
    for (uint8_t row = 0; row < MAX_ROWS; ++row) {
        gpio_put(ROW_PINS[row], 1);
        sleep_us(GPIO_DELAY);

        for(uint8_t col = 0; col < MAX_COLS; ++col) {
            if (gpio_get(COL_PINS[col])) {
                _nextRows[row] |= (1 << col);
            }
        }

        gpio_put(ROW_PINS[row], 0);
        sleep_us(GPIO_DELAY);
    }

    _empty = memcmp(_prevRows, _nextRows, sizeof(_nextRows)) == 0;
    return true;
}

bool KbdBasicScanner::isEmpty() const {
    return _empty != 0;
}

bool KbdBasicScanner::takeState(EKey key, bool& prevOut, bool& nextOut) const {
    if (key == EKEY_INV || key == EKEY_HIDDEN || key >= EKEY_MAX) {
        return false;
    }
    
    const uint8_t row = key / MAX_COLS;
    const uint8_t col = key % MAX_COLS;
    
    prevOut = (_prevRows[row] & (1 << col)) != 0;
    nextOut = (_nextRows[row] & (1 << col)) != 0;
    return true;
}
