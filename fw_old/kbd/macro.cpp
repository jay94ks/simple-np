#include "macro.h"
#include "keymap.h"
#include "bsp/board.h"
#include <string.h>

// --> 2048 slots to store macro records.
constexpr uint16_t MAX_MACRO_SLOTS = 2048;
SKeyRecord g_macro_slot[MAX_MACRO_SLOTS];
uint16_t g_macro_max;
uint8_t g_macro_state;
uint32_t g_macro_ts;

kbd_macro_cb g_macro_cb;
kbd_keymap_cb g_macro_rec_kmcb;

void kbd_macro_init() {
    memset(g_macro_slot, 0, sizeof(g_macro_slot));
    g_macro_state = EKMS_NONE;
    g_macro_max = 0;
}

EMacroState kbd_macro_state() {
    return EMacroState(g_macro_state);
}

bool kbd_macro_play(kbd_macro_cb cb) {
    if (g_macro_state != EKMS_NONE) {
        return false;
    }

    g_macro_cb = cb;
    g_macro_state = EKMS_PLAY;
    g_macro_ts = board_millis();
    return true;
}

bool kbd_macro_stop() {
    if (g_macro_state != EKMS_PLAY) {
        return false;
    }

    if (g_macro_cb) {
        g_macro_cb();
    }

    g_macro_cb = nullptr;
    g_macro_state = EKMS_NONE;
    g_macro_ts = 0;
    return true;
}

void kbd_macro_record_cb(EKey key, SKeyMap* map);
bool kbd_macro_record(kbd_macro_cb cb) {
    if (g_macro_state != EKMS_NONE) {
        return false;
    }

    g_macro_max = 0;
    g_macro_cb = cb;
    g_macro_state = EKMS_RECORD;
    g_macro_ts = board_millis();
    g_macro_rec_kmcb = kbd_keymap_set_cb(kbd_macro_record_cb);
    return true;
}

bool kbd_macro_record_stop() {
    if (g_macro_state != EKMS_RECORD) {
        return false;
    }

    if (g_macro_cb) {
        g_macro_cb();
    }

    g_macro_cb = nullptr;
    g_macro_state = EKMS_NONE;
    g_macro_ts = 0;
    g_macro_rec_kmcb = nullptr;
    return true;
}

bool kbd_macro_get_current(uint8_t* keyscan, uint8_t rows) {
    if (g_macro_state != EKMS_PLAY) {
        return false;
    }

    const uint32_t now_ms = board_millis();
    const uint32_t elapsed = now_ms - g_macro_ts;
    uint32_t frame = 0, index = 0;

    for (;index < g_macro_max; ++index) {
        const SKeyRecord& record = g_macro_slot[index];

        if ((frame += record.dm) > elapsed) {
            break;
        }

        kbd_keyscan_set(
            keyscan, rows, 
            EKey(record.key), 
            EKeyState(record.ls));
    }

    // --> if reached to end of records.
    if (index >= g_macro_max) {
        kbd_macro_stop();
    }

    return true;
}

void kbd_macro_record_cb(EKey key, SKeyMap* map) {
    const uint32_t now_ms = board_millis();
    const uint32_t delay = now_ms - g_macro_ts;

    // --> bypass key once.
    if (g_macro_rec_kmcb) {
        g_macro_rec_kmcb(key, map);
    }

    // --> simplify level state.
    EKeyState state = 
        (map->ls == EKLS_RISE || map->ls == EKLS_HIGH)
        ? EKLS_HIGH : EKLS_LOW;
    
    // --> find state duplicates.
    for(int16_t i = g_macro_max - 1; i >= 0; --i) {
        const SKeyRecord& record = g_macro_slot[i];

        if (record.key == key) {
            if (record.ls == state) {
                // --> same state exists.
                return;
            }

            // --> different state.
            break;
        }
    }

    // --> store record item.
    g_macro_slot[g_macro_max].key = key;
    g_macro_slot[g_macro_max].dm = delay;
    g_macro_slot[g_macro_max].ls = state;
    
    // --> update timestamp.
    g_macro_ts = now_ms;
    g_macro_max++;
    
    // --> if max slots reached, stop macro recorder.
    if (g_macro_max >= MAX_MACRO_SLOTS) {
        kbd_macro_record_stop();
    }
}