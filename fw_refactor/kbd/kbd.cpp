#include "kbd.h"
#include "scancode.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include <string.h>
#include <vector>

#define KEY_MAP_UNMAPPED { 0, 0, 0, 0 }

const uint8_t Kbd::ROW_PINS[5] = { 0, 1, 2, 3, 4 };
const uint8_t Kbd::COL_PINS[5] = { 9, 8, 7, 6, 5 };
const SKeyChar Kbd::KEY_MAP[EKEY_MAX] = {
    KEY_MAP_UNMAPPED,                        // EKEY_UFN_1
    { '/', '/',     KC_KEYPAD_DIVIDE,   0 }, // EKEY_SLASH
    { '*', '*',     KC_KEYPAD_MULTIPLY, 0 }, // EKEY_ASTEROID
    { '-', '-',     KC_KEYPAD_SUBTRACT, 0 }, // EKEY_MINUS
    { 0, 0,         KC_NUM_LOCK,        0 }, // EKEY_NUMLOCK
    KEY_MAP_UNMAPPED,                        // EKEY_UFN_2
    { '7', 'H',     KC_KEYPAD_7,        0 }, // EKEY_NUM_7
    { '8', 'U',     KC_KEYPAD_8,        0 }, // EKEY_NUM_8
    { '9', 'P',     KC_KEYPAD_9,        0 }, // EKEY_NUM_9
    { '+', '+',     KC_KEYPAD_ADD,      0 }, // EKEY_PLUS
    KEY_MAP_UNMAPPED,                        // EKEY_UFN_3
    { '4', 'L',     KC_KEYPAD_4,        0 }, // EKEY_NUM_4
    { '5', ' ',     KC_KEYPAD_5,        0 }, // EKEY_NUM_5
    { '6', 'R',     KC_KEYPAD_6,        0 }, // EKEY_NUM_6
    { '\n', '\n',   KC_KEYPAD_ENTER,    0 }, // EKEY_ENTER
    KEY_MAP_UNMAPPED,                        // EKEY_UFN_4
    { '1', 'E',     KC_KEYPAD_1,        0 }, // EKEY_NUM_1
    { '2', 'D',     KC_KEYPAD_2,        0 }, // EKEY_NUM_2
    { '3', 'S',     KC_KEYPAD_3,        0 }, // EKEY_NUM_3
    KEY_MAP_UNMAPPED,                        // EKEY_HIDDEN
    KEY_MAP_UNMAPPED,                        // EKEY_UFN_5
    { '0', 'I',     KC_KEYPAD_0,        0 }, // EKEY_NUM_0
    { '.', 'B',     KC_KEYPAD_DECIMAL,  0 }, // EKEY_DOT
    KEY_MAP_UNMAPPED,                        // EKEY_MREC
    KEY_MAP_UNMAPPED                         // EKEY_MPLAY
};

Kbd* Kbd::get() {
    static Kbd _kbd;
    return &_kbd;
}

Kbd::Kbd() {
    memset(_keys, 0, sizeof(_keys));
    memset(_prevRows, 0, sizeof(_prevRows));
    memset(_nextRows, 0, sizeof(_nextRows));
    memset(_handlers, 0, sizeof(_handlers));

    uint8_t order = 0;
    for(uint8_t i = 0; i < EKEY_MAX; ++i) {
        _keys[i].ch = KEY_MAP[i];

        if (i == EKEY_HIDDEN) {
            _keys[i].order = 24;
            continue;
        }

        _keys[i].order = order++;
        _orderedKeys[_keys[i].order] = EKey(i);
    }
    
    _orderedKeys[order++] = EKEY_HIDDEN;
    _countOfHandlers = 0;

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

IKeyHandler* Kbd::peek(uint8_t index) const {
    if (index >= _countOfHandlers) {
        return nullptr;
    }

    return _handlers[index];
}

bool Kbd::push(IKeyHandler* handler) {
    if (_countOfHandlers >= MAX_HANDLERS) {
        return false;
    }

    // --> shift all handlers to back.
    uint8_t index = _countOfHandlers;
    for(uint8_t i = 1; i < _countOfHandlers; ++i) {
        _handlers[i] = _handlers[i - 1];
    }

    _handlers[0] = handler;
    return false;
}

bool Kbd::pop(IKeyHandler* handler) {
    if (_countOfHandlers <= 0) {
        return false;
    }

    if (handler == nullptr) {
        for(uint8_t i = 0; i < _countOfHandlers - 1; ++i) {
            _handlers[i] = _handlers[i + 1];
        }

        _countOfHandlers--;
        return true;
    }

    // --> find index of handler.
    int16_t index = -1;
    for(uint8_t i = 0; i < _countOfHandlers; ++i) {
        if (handler == _handlers[i]) {
            index = i;
            break;
        }
    }

    if (index < 0) {
        return false;
    }

    // --> if the handler is last-one,
    if (index == int16_t(_countOfHandlers - 1)) {
        _countOfHandlers--;
        return true;
    }

    // --> shift all handlers to front.
    for(uint8_t i = uint8_t(index); i < _countOfHandlers - 1; ++i) {
        _handlers[i] = _handlers[i + 1];
    }

    _countOfHandlers--;
    return true;
}

bool Kbd::handle(EKey key) const {
    if (key >= EKEY_MAX|| _countOfHandlers <= 0) {
        return false;
    }

    // --> copy all key handlers to avoid handler-set manipulation.
    IKeyHandler* handlers[16];
    const uint8_t count = _countOfHandlers;
    for(uint8_t i = 0; i < count; ++i) {
        handlers[i] = _handlers[i];
    }

    // --> invoke all key handlers.
    for(uint8_t i = 0; i < count; ++i) {
        IKeyHandler* handler = handlers[i];
        const EKeyState state = EKeyState(_keys[key].ls);
        if (handler->onKeyUpdated(const_cast<Kbd*>(this), key, state)) {
            return true;
        }
    }

    return false;
}

void Kbd::scanOnce() {
    // --> store previous states.
    memcpy(_prevRows, _nextRows, sizeof(_nextRows));
    
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

    if (memcmp(_prevRows, _nextRows, sizeof(_nextRows)) == 0) {
        // --> same state: skip below.
        return;
    }

    // --> update level state and order-set.
    updateOnce();

    // --> then, trigger key handlers.
    trigger();
}

void Kbd::updateOnce() {
    uint8_t order = 0, repos = 0;
    EKey reorder[EKEY_MAX];

    for(uint8_t index = 0; index < EKEY_MAX; ++index) {
        const uint8_t row = index / MAX_COLS;
        const uint8_t col = index % MAX_COLS;

        const uint8_t prev = _prevRows[row] & (1 << col);
        const uint8_t next = _nextRows[row] & (1 << col);

        // --> current key.
        SKey& key = _keys[index];

        // --> this key has fixed order.
        if (index == EKEY_HIDDEN) {
            continue;
        }

        // --> different.
        if (prev != next) {
            key.ht = EKHT_PENDING;
            key.order = order++;
            
            if (next) {
                key.ls = EKLS_RISE;
            }

            else {
                key.ls = EKLS_FALL;
            }
        }

        else if (key.ls == EKLS_LOW || key.ls == EKLS_HIGH) {
            reorder[repos++] = EKey(index);
            continue; // --> not affected.
        }

        // --> same state.
        else {
            key.ht = EKHT_PENDING;
            key.order = order++;

            if (key.ls == EKLS_RISE) {
                key.ls == EKLS_HIGH;
            }

            else if (key.ls == EKLS_FALL) {
                key.ls = EKLS_LOW;
            }
        }

        _orderedKeys[key.order] = EKey(index);
    }
    
    // --> bubble sort.
    for(uint8_t i = 0; i < repos; ++i) {
        for(uint8_t j = 0; j < repos; ++j) {
            const uint8_t left = _keys[reorder[i]].order;
            const uint8_t right = _keys[reorder[j]].order;

            if (left > right) {
                EKey temp = reorder[i];
                reorder[i] = reorder[j];
                reorder[j] = temp;
            }
        }
    }

    // --> then assign keys to order set.
    const uint8_t offset = order;
    for(;order < EKEY_MAX; ++order) {
        const EKey key = reorder[order - offset];

        _keys[key].order = order;
        _orderedKeys[order] = key;
    }
}

void Kbd::trigger() {
    EKey orderedKeys[EKEY_MAX];
    if (_countOfHandlers <= 0) {
        return; // --> no handler exists.
    }

    // --> make snapshot to trigger.
    memcpy(orderedKeys, _orderedKeys, sizeof(orderedKeys));

    // --> invoke handlers for each keys.
    for(uint8_t i = 0; i < EKEY_MAX; ++i) {
        const EKey order = orderedKeys[i];
        if (order != EKEY_INV) {
            if (!KBD_IS_PENDING_TRIGGER(_keys[order].ht)) {
                continue;
            }

            _keys[order].ht = EKHT_TRIGGERED;
            handle(order);
        }
    }
}

SKey* Kbd::getKeyPtr(EKey key) const {
    if (key >= EKEY_MAX) {
        return nullptr;
    }

    return &_keys[key];
}

SKeyChar Kbd::getDefaultKeyChar(EKey key) const {
    if (key >= EKEY_MAX) {
        return {0, };
    }

    return KEY_MAP[key];
}

SKeyChar Kbd::getKeyChar(EKey key) const {
    if (key >= EKEY_MAX) {
        return {0, };
    }

    return _keys[key].ch;
}

bool Kbd::setKeyChar(EKey key, SKeyChar ch) {
    if (key >= EKEY_MAX) {
        return false;
    }

    _keys[key].ch = ch;
    return true;
}

void Kbd::resetKeyChars() {
    for(uint8_t i = 0; i < EKEY_MAX; ++i) {
        _keys[i].ch = KEY_MAP[i];
    }
}

EKey Kbd::getRecentKey(EKeyState state) const {
    for(uint8_t i = 0; i < EKEY_MAX; ++i) {
        const EKey key = _orderedKeys[i];

        if(_keys[key].ls == state) {
            return key;
        }
    }

    return EKEY_INV;
}

void Kbd::getPressingKeys(EKey* outKeys, uint8_t max) const {
    if (outKeys) {
        if (max > EKEY_MAX) {
            max = EKEY_MAX;
        }

        memcpy(outKeys, _orderedKeys, max);
    }
}

bool Kbd::forceKeyState(EKey key, EKeyState state) {
    if (key >= EKEY_MAX) {
        return false;
    }

    SKey& ref = _keys[key];
    if (ref.ts == state) {
        return false;
    }

    uint8_t order = _keys[key].order;

    // --> changes the key state.
    ref.ht = EKHT_PENDING;
    ref.ls = state;

    // --> shift all keys back.
    for(int8_t i = order; i > 0; --i) {
        _orderedKeys[i] = _orderedKeys[i - 1];
        
        const EKey temp = _orderedKeys[i - 1];
        _keys[temp].order = i;
    }

    // --> then, place this key to front.
    _orderedKeys[0] = key;
    _keys[key].order = 0;

    return true;
}