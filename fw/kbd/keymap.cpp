#include "keymap.h"
#include "toggle.h"
#include "macro.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "../usb/usbd.h"
#include <string.h>
#include <stdio.h>

constexpr uint8_t KEY_MAX_ROWS = 5;
constexpr uint8_t KEY_MAX_COLS = 5;
constexpr uint8_t KEY_MAX_KEYS = 5 * 5;

const uint8_t g_key_rowpin[] = { 0, 1, 2, 3, 4 };
const uint8_t g_key_colpin[] = { 9, 8, 7, 6, 5 };

SKeyMap g_keymap[EKEY_MAX];
uint8_t g_keyscan_prev[KEY_MAX_ROWS];
uint8_t g_keyscan_next[KEY_MAX_ROWS];

// --> L, R, H, F (KeyState value, offset: 1)
uint8_t g_keymap_last[4];

// --> global keymap callback.
kbd_keymap_cb g_keymap_cb;

// --> make column bitmask.
#define KEYMAP_COL_MASK(n)       (1 << (n))
#define KEYMAP_KEY_NUM(row, col) (((row) * KEY_MAX_COLS) + (col))

/**
 * delay 1 microseconds.
 */
#define kbd_keymap_udelay() sleep_us(10)

/**
 * store previous status. 
 */
#define kbd_keymap_store_prev() memcpy(g_keyscan_prev, g_keyscan_next, sizeof(g_keyscan_next))

/**
 * set the key-scan level state.
 */
void kbd_keyscan_set(uint8_t* keyscan, uint8_t rows, EKey key, EKeyState state) {
    if (key == EKEY_INV || key >= EKEY_MAX) {
        return;
    }

    const uint8_t row_no = key / 5;
    const uint8_t col_no = key % 5;
    if (rows <= row_no) {
        return;
    }

    if (state == EKLS_HIGH || state == EKLS_RISE) {
        keyscan[row_no] |= KEYMAP_COL_MASK(col_no);
    }
    else {
        keyscan[row_no] &= ~KEYMAP_COL_MASK(col_no);
    }
}

/**
 * initialize GPIO pins for the key-scan lines. 
 */
void kbd_keymap_gpio_init(const uint8_t* pins, uint8_t n, bool dir) {
    for(uint8_t i = 0; i < n; ++i) {
        const uint8_t pin_no = pins[i];

        gpio_init(pin_no);
        gpio_set_dir(pin_no, dir);

        if (dir == GPIO_IN) {
            gpio_pull_down(pin_no);
        }
    }
}

/**
 * initialize the keyboard `key-mapping` subsystem. 
 */
void kbd_keymap_init() {
    g_keymap_cb = kbd_keymap_cb_defimpl;

    memset(g_keymap, 0, sizeof(g_keymap));
    memset(g_keyscan_prev, 0, sizeof(g_keyscan_prev));
    memset(g_keyscan_next, 0, sizeof(g_keyscan_next));

    for(uint8_t i = 0; i < sizeof(g_keymap_last); ++i) {
        g_keymap_last[i] = EKEY_INV;
    }

    kbd_keymap_gpio_init(g_key_rowpin, KEY_MAX_ROWS, GPIO_OUT);
    kbd_keymap_gpio_init(g_key_colpin, KEY_MAX_COLS, GPIO_IN);

    kbd_toggle_init();
    kbd_set_scancodes();
    kbd_macro_init();
}

/**
 * scan keys and store them to `g_keyscan_next` as bitmap. 
 */
void kbd_keymap_scan() {
    memset(g_keyscan_next, 0, sizeof(g_keyscan_next));

    // --> now playing.
    if (kbd_macro_state() == EKMS_PLAY) {
        kbd_macro_get_current(g_keyscan_next, KEY_MAX_ROWS);
        return;
    }

    // --> scan each rows.
    for(uint8_t row = 0; row < KEY_MAX_ROWS; ++row) {
        gpio_put(g_key_rowpin[row], 1);
        kbd_keymap_udelay();

        for(uint8_t col = 0; col < KEY_MAX_COLS; ++col) {
            if (gpio_get(g_key_colpin[col])) {
                g_keyscan_next[row] |= KEYMAP_COL_MASK(col);
            }
        }

        gpio_put(g_key_rowpin[row], 0);
        kbd_keymap_udelay();
    }
}

// --> set the last key number for each state.
#define kbd_keymap_set_last_ls(key_no)  \
    g_keymap_last[g_keymap[(key_no)].ls] = (key_no)

/**
 * update level state of each keys.
 * and call keymap callbacks.
 */
void kbd_keymap_update_ls() {
    for(uint8_t row = 0; row < KEY_MAX_ROWS; ++row) {
        const uint8_t prev_scan = g_keyscan_prev[row];
        const uint8_t next_scan = g_keyscan_next[row];

        for(uint8_t col = 0; col < KEY_MAX_COLS; ++col) {
            const uint8_t key_no = KEYMAP_KEY_NUM(row, col);
            const uint8_t col_mask = KEYMAP_COL_MASK(col);

            const uint8_t prev_bit = prev_scan & col_mask;
            const uint8_t next_bit = next_scan & col_mask;

            SKeyMap& key = g_keymap[key_no];
            kbd_keymap_cb callback = key.cb;

            if (!callback) {
                 callback = g_keymap_cb;
            }

            if (prev_bit == next_bit) {
                if (key.ls == EKLS_RISE) {
                    // --> high level.
                    key.ls = EKLS_HIGH;
                    kbd_keymap_set_last_ls(key_no);
                    if (callback) {
                        callback(EKey(key_no), &key);
                    }
                }

                else if(key.ls == EKLS_FALL) {
                    // --> low level.
                    key.ls = EKLS_LOW;
                    kbd_keymap_set_last_ls(key_no);
                    if (callback) {
                        callback(EKey(key_no), &key);
                    }
                }

                continue;
            }

            if (next_bit) {
                // --> rising edge.
                key.ls = EKLS_RISE;
            } else {
                // --> falling edge.
                key.ls = EKLS_FALL;
            }
            
            kbd_keymap_set_last_ls(key_no);
            if (callback) {
                callback(EKey(key_no), &key);
            }
        }
    }
}

/**
 * scan keys.
 */
void kbd_keymap_task() {
    // --> circuit-level scanning.
    kbd_keymap_store_prev();
    kbd_keymap_scan();

    // --> logic-level update.
    kbd_keymap_update_ls();
}

/**
 * set the keymap callback.
 */
kbd_keymap_cb kbd_keymap_set_cb(kbd_keymap_cb cb) {
    kbd_keymap_cb prev = g_keymap_cb;
    if (cb) {
        g_keymap_cb = cb;
    }

    else {
        g_keymap_cb = kbd_keymap_cb_defimpl;
    }

    return prev;
}

SKeyMap* kbd_get_ptr(EKey key) {
    if (key >= EKEY_MAX) {
        return nullptr;
    }

    return &g_keymap[key];
}

void kbd_key_set_sc(EKey key, uint16_t scd, char def, char alt) {
    if (SKeyMap* ptr = kbd_get_ptr(key)) {
        ptr->ch.def = def;
        ptr->ch.alt = alt;
        ptr->ch.scd = scd;
    }
}

EKey kbd_get_last(EKeyState state) {
    const EKey last_key = EKey(g_keymap_last[state]);
    if (last_key == EKEY_INV || g_keymap[last_key].ls != state) {
        return EKEY_INV;
    }

    return last_key;
}

char kbd_get_ch() {
    EKey key = kbd_get_last(EKLS_FALL);
    if (key == EKEY_INV) {
        return 0;
    }

    const bool numlock = kbd_toggle_get(EKEY_NUMLOCK);
    const SKeyChar ch = g_keymap[key].ch;
    if (numlock && ch.alt) {
        return ch.alt;
    }

    return ch.def;
}

/**
 * default behaviour for keymap callback.
 */
void kbd_keymap_cb_defimpl(EKey key, SKeyMap* map) {
    if (map->ch.scd) {
        // --> scan code registered.
        // todo: emit scan code sequence.
        char temp[4] = {0, };

        switch(map->ls) {
            case EKLS_HIGH: // --> pressed.
            case EKLS_LOW:  // --> released.
                break;

            case EKLS_RISE: // --> pressing.
                usbd_hid_add_key(key);
                break;

            case EKLS_FALL: // --> releasing.
                usbd_hid_remove_key(key);
                break;
        }
    }

    else {
        // --> no scan code registered.
        // todo: handle the user-fn keys.
    }
}
