#include "usbd.h"
#include "../kbd/keymap.h"
#include <tusb.h>
#include <stdlib.h>

extern "C" void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {
  (void) instance;
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) bufsize;
}

extern "C" uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen) {
  (void) instance;
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) reqlen;
  return 0;
}

uint8_t g_usbd_hid_dirty;
uint8_t g_usbd_hid_key_prev[6];
uint8_t g_usbd_hid_key_next[6];
uint8_t g_usbd_hid_key_pos;

uint8_t g_usbd_hid_key_rmp[6];


void usbd_hid_init() {
    g_usbd_hid_dirty = 0;
    g_usbd_hid_key_pos = 0;

    for(uint8_t i = 0; i < 6; ++i) {
        g_usbd_hid_key_prev[i] = EKEY_INV;
        g_usbd_hid_key_next[i] = EKEY_INV;
        g_usbd_hid_key_rmp[i] = EKEY_INV;
    }
}

// --> reset the HID state.
void usbd_hid_reset() {
    g_usbd_hid_dirty = 1;
}

void usbd_hid_task() {
    const uint32_t ts = usbd_get_tick();
    int32_t diff = memcmp(
        &g_usbd_hid_key_prev, &g_usbd_hid_key_next, 
        sizeof(g_usbd_hid_key_prev));

    if (g_usbd_hid_dirty || diff) {
        if (!tud_hid_ready()) {
            return;
        }

        uint8_t buf[6] = {0, };
        uint8_t buf_pos = 0;

        for (uint8_t i = 0; i < 6; ++i) {
            const EKey key = EKey(g_usbd_hid_key_next[i]);
            if (key == EKEY_INV) {
                continue;
            }

            const SKeyMap* ptr = kbd_get_ptr(key);
            if (ptr) {
                buf[buf_pos++] = ptr->ch.scd;
                if (buf_pos >= 6) {
                    break;
                }
            }
        }

        g_usbd_hid_dirty = 0;
        memcpy(
            &g_usbd_hid_key_prev, &g_usbd_hid_key_next, 
            sizeof(g_usbd_hid_key_next));

        tud_hid_keyboard_report(RID_KEYBOARD, 0, buf);

        for(uint8_t i = 0; i < 6; ++i) {
            if (g_usbd_hid_key_rmp[i] != EKEY_INV) {
                usbd_hid_remove_key(g_usbd_hid_key_rmp[i]);
            }
        }
    }
}

// --> add a key to report @ usbd_hid.cpp
void usbd_hid_add_key(uint8_t key) {
    usbd_hid_remove_key(key);

    if (g_usbd_hid_key_pos >= 6) {
        memmove(g_usbd_hid_key_next, g_usbd_hid_key_next + 1, 5);
        g_usbd_hid_key_pos--;
    }

    g_usbd_hid_key_next[g_usbd_hid_key_pos++] = key;
}

// --> add a key as oneshot @ usbd_hid.cpp
void usbd_hid_add_key_oneshot(uint8_t key) {
    usbd_hid_add_key(key);
    
    // --> store the key as remove pending.
    for(uint8_t i = 0; i < 6; ++i) {
        if (g_usbd_hid_key_rmp[i] == EKEY_INV) {
            g_usbd_hid_key_rmp[i] = key;
            break;
        }
    }
}

// --> remove a key from report @ usbd_hid.cpp
void usbd_hid_remove_key(uint8_t key) {
    for(uint8_t i = 0; i < g_usbd_hid_key_pos; ++i) {
        if (g_usbd_hid_key_next[i] == key) {
            for(uint8_t j = i; j < g_usbd_hid_key_pos - 1; ++j) {
                g_usbd_hid_key_next[j] = g_usbd_hid_key_next[j + 1];
            }

            g_usbd_hid_key_pos--;
            g_usbd_hid_key_next[g_usbd_hid_key_pos] = EKEY_INV;
            break;
        }
    }

    // --> remove the key from remove pending.
    for(uint8_t i = 0; i < 6; ++i) {
        if (g_usbd_hid_key_rmp[i] == key) {
            g_usbd_hid_key_rmp[i] = EKEY_INV;
        }
    }
}
