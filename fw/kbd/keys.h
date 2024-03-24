#ifndef __KBD_KEYS_H__
#define __KBD_KEYS_H__

#include <stdint.h>

/**
 * key definitions. 
 */
enum EKey {
    EKEY_UFN_1 = 0,     // --> SW 2
    EKEY_SLASH = 1,     // --> SW 7
    EKEY_ASTEROID = 2,  // --> SW 12
    EKEY_MINUS = 3,     // --> SW 16
    EKEY_NUMLOCK = 4,   // --> SW 21
    
    EKEY_UFN_2 = 5,     // --> SW 3
    EKEY_NUM_7 = 6,     // --> SW 8
    EKEY_NUM_8 = 7,     // --> SW 13
    EKEY_NUM_9 = 8,     // --> SW 17
    EKEY_PLUS = 9,      // --> SW 22.

    EKEY_UFN_3 = 10,    // --> SW 4
    EKEY_NUM_4 = 11,    // --> SW 9
    EKEY_NUM_5 = 12,    // --> SW 14
    EKEY_NUM_6 = 13,    // --> SW 18
    EKEY_ENTER = 14,    // --> SW 23.

    EKEY_UFN_4 = 15,    // --> SW 5
    EKEY_NUM_1 = 16,    // --> SW 10
    EKEY_NUM_2 = 17,    // --> SW 15
    EKEY_NUM_3 = 18,    // --> SW 19
    
    EKEY_UFN_5 = 20,    // --> SW 6
    EKEY_NUM_0 = 21,    // --> SW 11
    EKEY_DOT = 23,      // --> SW 20

    EKEY_MREC = 19,     // --> SW 24, GP13
    EKEY_MPLAY  = 24,   // --> SW 25, GP22

    /* Maximum count of registrations of keys. */
    EKEY_MAX = 25,
    EKEY_HIDDEN = 22,
    EKEY_INV = 0xff,
};

/**
 * key state definitions. 
 */
enum EKeyState {
    EKLS_LOW = 0,
    EKLS_RISE = 1,
    EKLS_HIGH = 2,
    EKLS_FALL = 3
};

/**
 * key character. 
 */
struct SKeyChar {
    char def;       // --> default value.
    char alt;       // --> alternative value.
    uint8_t kc;     // --> scan code, key code.
    uint8_t mod;    // --> scan code, modifier value.
};

/**
 * toggle mode. 
 */
enum EKeyToggle {
    EKTG_NONE = 0,              // --> normal keys: utilize EKLS_*. 
    EKTG_TOGGLE,                // --> toggle keys: numlock, ufn_*.
    EKTG_ONESHOT,               // --> oneshot toggle, 
    EKTG_MAX_VALUE,
    EKTG_INVALID = 0xff         // --> invalid.
};

/**
 * handler triggering state. 
 */
enum EKeyTrigger {
    EKHT_TRIGGERED = 0,
    EKHT_PENDING
};

// --> test whether the key is pending or not.
#define KBD_IS_PENDING_TRIGGER(x)   ((x) == EKHT_PENDING)

/**
 * key map entity. 
 */
struct SKey {
    uint8_t order;  // --> level state order, 0: not needed, 1: pending.
    uint8_t ht;     // --> handler triggered or not.
    uint32_t ms;    // --> timestamp when key state changed.
    uint8_t ls;     // --> key state.
    uint8_t ts;     // --> toggle state.
    uint8_t tm;     // --> toggle mode.
    SKeyChar ch;    // --> key character.
};

#endif