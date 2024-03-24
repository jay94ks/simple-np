#include "kbd.h"
#include "scancode.h"
#include "scanners/basic.h"
#include "handlers/numlock.h"
#include "handlers/userfn.h"
#include "pico/stdlib.h"
#include <string.h>
#include <vector>
#include <algorithm>

#define KEY_MAP_UNMAPPED { 0, 0, 0, 0 }

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

    // --> push basic scanner here.
    push(KbdBasicScanner::instance());

    // --> push numlock handler here.
    push(KbdNumlockHandler::instance());
    listen(KbdNumlockHandler::instance());

    // --> push user-fn handler here.
    push(KbdUserFnHandler::instance());
}

bool Kbd::push(IKeyScanner* scanner) {
    if (scanner) {
        _scanners.push_back(scanner);
        return true;
    }

    return false;
}

bool Kbd::pop(IKeyScanner* scanner) {
    if (_scanners.size() <= 0) {
        return false;
    }

    if (scanner == nullptr) {
        _scanners.pop_back();
        return true;
    }

    auto iter = _scanners.begin();
    auto sel = _scanners.end();

    // --> find a last handler.
    while(iter != sel) {
        if (*iter == scanner) {
            sel = iter;
        }

        iter++;
    }

    if (sel != _scanners.end()) {
        _scanners.erase(sel);
        return true;
    }
    
    return true;
}

bool Kbd::push(IKeyHandler* handler) {
    if (handler) {
        _handlers.push_back(handler);
        return true;
    }

    return false;
}

bool Kbd::pop(IKeyHandler* handler) {
    if (_handlers.size() <= 0) {
        return false;
    }

    if (handler == nullptr) {
        _handlers.pop_back();
        return true;
    }

    auto iter = _handlers.begin();
    auto sel = _handlers.end();

    // --> find a last handler.
    while(iter != sel) {
        if (*iter == handler) {
            sel = iter;
        }

        iter++;
    }

    if (sel != _handlers.end()) {
        _handlers.erase(sel);
        return true;
    }
    
    return true;
}

bool Kbd::listen(IKeyListener* listener) {
    if (listener == nullptr) {
        return false;
    }

    for(IKeyListener* other : _listeners) {
        if (other == listener) {
            return false;
        }
    }

    _listeners.push_back(listener);
    return true;
}

bool Kbd::unlisten(IKeyListener* listener) {
    if (listener == nullptr) {
        return false;
    }

    auto iter = _listeners.begin();
    while(iter != _listeners.end()) {

        if ((*iter) == listener) {
            _listeners.erase(iter);
            return true;
        }

        iter++;
    }

    return false;
}

bool Kbd::handle(EKey key) const {
    if (key >= EKEY_MAX || _handlers.size() <= 0) {
        return false;
    }

    // --> copy all handlers to avoid handler-set manipulation.
    FHandlerList handlers(_handlers);
    FListenerList listeners(_listeners);
    
    // --> reverse copied list to invoke handlers in reverse order.
    std::reverse(handlers.begin(), handlers.end());
    std::reverse(listeners.begin(), listeners.end());

    bool retval = false;

    // --> invoke key handlers.
    for(IKeyHandler* handler: handlers) {
        const EKeyState state = EKeyState(_keys[key].ls);
        if (handler->onKeyUpdated(const_cast<Kbd*>(this), key, state)) {
            retval = true;
            break;
        }
    }

    // --> notify key state.
    for(IKeyListener* listener: listeners) {
        const EKeyState state = EKeyState(_keys[key].ls);
        listener->onKeyNotify(this, key, state);
    }

    return retval;
}

void Kbd::scanOnce() {
    FScannerList scanners;
    
    for(IKeyScanner* scanner : _scanners) {
        if (scanner->scanOnce()) {
            if (scanner->isEmpty()) {
                continue;
            }

            scanners.push_back(scanner);
        }
    }

    // --> no key level change exists.
    if (scanners.size() > 0) {
        // --> reverse pushed list to invoke scanners in reverse order.
        std::reverse(scanners.begin(), scanners.end());
    }

    // --> update level state and order-set.
    updateOnce(scanners);

    // --> then, trigger key handlers.
    trigger();
}

bool kbdGetKeyState(const Kbd::FScannerList& scanners, EKey key, bool& prevOut, bool& nextOut) {
    for(IKeyScanner* scanner: scanners) {
        if (scanner->takeState(key, prevOut, nextOut)) {
            return true;
        }
    }

    return false;
}

void Kbd::updateOnce(const FScannerList& scanners) {
    uint8_t order = 0, repos = 0;
    EKey reorder[EKEY_MAX];

    for(uint8_t index = 0; index < EKEY_MAX; ++index) {
        const EKey defKey = EKey(index);
        bool prev = false, next = false;

        if (kbdGetKeyState(scanners, defKey, prev, next) == false) {
            reorder[repos++] = defKey;
            continue;
        }

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
            reorder[repos++] = defKey;
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

        _orderedKeys[key.order] = defKey;
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
    if (_handlers.size() <= 0) {
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