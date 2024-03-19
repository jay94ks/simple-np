#include "scancode.h"
#include "keymap.h"

struct SKeyScanMap {
    EKey key;
    uint16_t scd;
    char def;
    char alt;
};

const SKeyScanMap g_key_scantbl[] = {
    { EKEY_NUMLOCK, KC_NUM_LOCK,           0, 0},
    { EKEY_ENTER,   KC_KEYPAD_ENTER,       '\n', '\n'},
    { EKEY_DOT,     KC_KEYPAD_DECIMAL,     'B', '.'},  // B: Backspace.
    { EKEY_PLUS,    KC_KEYPAD_ADD,         '+', '+'},
    { EKEY_MINUS,   KC_KEYPAD_SUBTRACT,    '-', '-'},
    { EKEY_ASTEROID,KC_KEYPAD_MULTIPLY,    '*', '*'},
    { EKEY_SLASH,   KC_KEYPAD_DIVIDE,      '/', '/'},
    { EKEY_NUM_1,   KC_KEYPAD_1,           'E', '1'},  // E: End.
    { EKEY_NUM_2,   KC_KEYPAD_2,           'D', '2'},  // D: Arrow down.
    { EKEY_NUM_3,   KC_KEYPAD_3,           'S', '3'},  // S: Page up.
    { EKEY_NUM_4,   KC_KEYPAD_4,           'L', '4'},  // L: Arrow left.
    { EKEY_NUM_5,   KC_KEYPAD_5,           ' ', '5'},  // Space, 5.
    { EKEY_NUM_6,   KC_KEYPAD_6,           'R', '6'},  // R: Arrow right.
    { EKEY_NUM_7,   KC_KEYPAD_7,           'H', '7'},  // H: Home.
    { EKEY_NUM_8,   KC_KEYPAD_8,           'U', '8'},  // U: Arrow up.
    { EKEY_NUM_9,   KC_KEYPAD_9,           'P', '9'},  // P: Page down.
    { EKEY_NUM_0,   KC_KEYPAD_0,           'I', '0'},  // I: Insert.
};

#define MAX_KEY_SCANTBL (sizeof(g_key_scantbl) / sizeof(SKeyScanMap))

void kbd_set_scancodes() {
    for(uint8_t i = 0; i < MAX_KEY_SCANTBL; ++i) {
        const SKeyScanMap& current = g_key_scantbl[i];
        kbd_key_set_sc(current.key, current.scd, current.def, current.alt);
    }
}