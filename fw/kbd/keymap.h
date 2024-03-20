#ifndef __KBD_KEYMAP_H__
#define __KBD_KEYMAP_H__ 1

#include <stdint.h>
#include "scancode.h"

/**     
 *              MR MP
 *              19 24
 * 
 *     c1 c2 c3 c4 c5
 *  r1 00 01 02 03 04
 *  r2 05 06 07 08 09
 *  r3 10 11 12 13 14
 *  r4 15 16 17 18
 *  r5 20 21    23
 * 
 * MREC  : R4, C5 == 19.
 * MPLAY : R5, C5 == 24. 
 * 
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
    EKEY_INV = 0xff,
};

/**
 * Key Line State.
*/
enum EKeyState {
    EKLS_LOW = 0,
    EKLS_RISE = 1,
    EKLS_HIGH = 2,
    EKLS_FALL = 3
};

/**
 * set the key-scan line state.
 */
void kbd_keyscan_set(uint8_t* keyscan, uint8_t rows, EKey key, EKeyState state);

/**
 * Key control charactor.
 * '0' ~ '9': charactor as-is.
 * 'H', 'E': Home, End.
 * 'D', 'L', 'R', 'U': Arrow down, left, right, up.
 * 'S', 'P': Page up, down.
 * 'I': Insert.
 * ' ': Space.
 * 'B': Backspace.
 */
struct SKeyChar {
    char def;   // --> default value.
    char alt;   // --> alternative value. (num-lock)
    uint8_t scd; // --> scan code value.
    uint8_t mod; // --> scan code, modifier value.
};

// --> forward decl.
struct SKeyMap;

/**
 * Callback to handle key-input events.
 */
typedef void(*kbd_keymap_cb)(EKey key, SKeyMap* map);

/**
 * Key Map.
 */
struct SKeyMap {
    uint8_t ls;                 // --> level state.
    uint8_t tm;                 // --> toggle mode.
    uint8_t ts;                 // --> toggle state.
    uint8_t tf;                 // --> toggle flag.
    uint8_t led;                // --> led number.
    SKeyChar ch;                // --> char code, scan code.

    kbd_keymap_cb cb;           // --> callback.
    kbd_keymap_cb tcb;          // --> toggle callback (valid only for toggle keys).
};

/**
 * key-map task.
 */
void kbd_keymap_init();
void kbd_keymap_task();

/**
 * set the global callback to handle key-map.
 * and returns previous callback pointer.
 */
kbd_keymap_cb kbd_keymap_set_cb(kbd_keymap_cb cb);
void kbd_keymap_cb_defimpl(EKey key, SKeyMap* map);

/**
 * get the pointer to `SKeyMap` struct for the key. 
 */
SKeyMap* kbd_get_ptr(EKey key);

/**
 * set the scan code for the key.
 * @param scd scan code.
 * @param sca scan code for numlock state.
 */
void kbd_key_set_sc(EKey key, uint16_t scd, char def, char alt);

/**
 * get the key that is entered to the state as last. 
 */
EKey kbd_get_last(EKeyState state);

/**
 * get the key control charactors that pressed at last.
 */
char kbd_get_ch();

/**
 * test whether the key is on the state or not.
 */
inline bool kbd_key_test(EKey key, EKeyState state) {
    if (SKeyMap* ptr = kbd_get_ptr(key)) {
        return ptr->ls == state;
    }

    return false;
}

#define kbd_is_key_pressed(key)     kbd_key_test(key, EKLS_LOW)
#define kbd_is_key_released(key)    kbd_key_test(key, EKLS_HIGH)
#define kbd_is_key_pressing(key)    kbd_key_test(key, EKLS_RISE)
#define kbd_is_key_releasing(key)   kbd_key_test(key, EKLS_FALL)

#endif // __KBD_KEYMAP_H__
