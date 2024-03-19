#include "toggle.h"
#include "macro.h"
#include "../ledctl.h"
#include "../usb/usbd.h"

/**
 * toggle flags.
*/
enum EToggle {
    EKTF_NUMLOCK = 1,
    EKTF_UFN_1 = 2,
    EKTF_UFN_2 = 4,
    EKTF_UFN_3 = 8,
    EKTF_UFN_4 = 16,
    EKTF_UFN_5 = 32,
    EKTF_MREC = 64,
    EKTF_MPLAY = 128,
};

/**
 * Key-Flag-LED mapping entity.
 */
struct SKeyToggleMap {
    EKey key;
    EToggle flag;
    uint8_t led;
};

// --> toggle key mappings.
const SKeyToggleMap g_key_toggle_map[] = {
    { EKEY_NUMLOCK, EKTF_NUMLOCK, LED_NUMLOCK },
    { EKEY_UFN_1,   EKTF_UFN_1,   LED_USRFN_1 },
    { EKEY_UFN_2,   EKTF_UFN_2,   LED_USRFN_2 },
    { EKEY_UFN_3,   EKTF_UFN_3,   LED_USRFN_3 },
    { EKEY_UFN_4,   EKTF_UFN_4,   LED_USRFN_4 },
    { EKEY_UFN_5,   EKTF_UFN_5,   LED_USRFN_5 },
    { EKEY_MREC,    EKTF_MREC,    LED_MREC },
    { EKEY_MPLAY,   EKTF_MPLAY,   LED_MPL }
};

#define MAX_KEY_TOGGLES (sizeof(g_key_toggle_map) / sizeof(SKeyToggleMap))

void kbd_toggle_keymap_cb(EKey key, SKeyMap* map);
void kbd_toggle_init() {

    for (uint8_t i = 0; i < MAX_KEY_TOGGLES; ++i) {
        const SKeyToggleMap& current = g_key_toggle_map[i];

        if (SKeyMap* map = kbd_get_ptr(current.key)) {
            map->cb = kbd_toggle_keymap_cb;
            map->tf = current.flag;
            map->led = current.led;
        }
    }
}

void kbd_toggle_set(EKey key, bool state) {
    if (SKeyMap* map = kbd_get_ptr(key)) {
        if (!map->tf) {
            return;
        }

        kbd_toggle_trigger(key, map);
    }
}

bool kbd_toggle_get(EKey key) {
    if (SKeyMap* map = kbd_get_ptr(key)) {
        if (!map->tf) {
            return false;
        }

        return map->ts != 0;
    }
    
    return false;
}

void kbd_toggle_on_mplay(EKey key, SKeyMap* map);
void kbd_toggle_trigger(EKey key, SKeyMap* map) {
    if (key == EKEY_INV || !map) {
        return;
    }

    if (map->tf) {
        if (key == EKEY_MPLAY) {
            kbd_toggle_on_mplay(key, map);
            return;
        }

        const bool prev = map->ts != 0;
        ledctl_set(map->led, map->ts = !prev);

        if (map->tcb) {
            map->tcb(key, map);
        }

        if (key == EKEY_NUMLOCK) {
            if (map->ls == EKLS_RISE || map->ls == EKLS_FALL) {
                usbd_hid_add_key_oneshot(key);
            }

            return;
        }

        // --> handle the default key actions.
        kbd_keymap_cb_defimpl(key, map);
    }
}

void kbd_toggle_keymap_cb(EKey key, SKeyMap* map) {
    if (map->ls != EKLS_RISE) {
        return;
    }

    kbd_toggle_trigger(key, map);
}

void kbd_toggle_on_mrecord(EKey key, SKeyMap* map) {
    const bool prev = map->ts != 0;
    bool state = false;

    if (map->ts == false) {
        // --> start recording keys.
        map->ts = kbd_macro_record([]() {
            if (SKeyMap* map = kbd_get_ptr(EKEY_MREC)) {
                ledctl_set(map->led, map->ts = false);
            }
        });
        
        ledctl_set(map->led, map->ts);
    }

    else {
        // --> stop the macro, registered callback will manipulate LED and state.
        kbd_macro_record_stop();
    }
}

void kbd_toggle_on_mplay(EKey key, SKeyMap* map) {
    const bool prev = map->ts != 0;
    bool state = false;

    if (map->ts == false) {
        // --> play the macro.
        map->ts = kbd_macro_play([]() {
            if (SKeyMap* map = kbd_get_ptr(EKEY_MPLAY)) {
                ledctl_set(map->led, map->ts = false);
            }
        });
        
        ledctl_set(map->led, map->ts);
    }

    else {
        // --> stop the macro, registered callback will manipulate LED and state.
        kbd_macro_stop();
    }
}