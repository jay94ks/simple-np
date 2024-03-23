#include "toggle.h"
#include "macro.h"
#include "../ledctl.h"
#include "../usb/usbd.h"
#include "../tft/tft.h"

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

void kbd_toggle_trigger(EKey key, SKeyMap* map);
void kbd_toggle_init() {

    for (uint8_t i = 0; i < MAX_KEY_TOGGLES; ++i) {
        const SKeyToggleMap& current = g_key_toggle_map[i];

        if (SKeyMap* map = kbd_get_ptr(current.key)) {
            map->cb = kbd_toggle_trigger;
            map->tf = current.flag;
            map->led = current.led;
        }
    }
}

void kbd_toggle_on_numlock(EKey key, SKeyMap* map);
void kbd_toggle_reported(bool numlock) {
    SKeyMap* map = kbd_get_ptr(EKEY_NUMLOCK);
    const uint8_t old_ts = map->ts;

    if (numlock) {
        ledctl_set(LED_NUMLOCK, true);
        map->ts = 1;
    } else {
        ledctl_set(LED_NUMLOCK, false);
        map->ts = 0;
    }

    if (old_ts != map->ts) {
        kbd_toggle_on_numlock(EKEY_NUMLOCK, map);
    }
}

// void kbd_toggle_set(EKey key, bool state, bool trigger) {
//     if (SKeyMap* map = kbd_get_ptr(key)) {
//         if (!map->tf) {
//             return;
//         }

//         kbd_toggle_trigger(key, map);
//     }
// }

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
void kbd_toggle_on_mrecord(EKey key, SKeyMap* map);

void kbd_toggle_numlock(EKey key, SKeyMap* map) {
    const bool prev = map->ts != 0;
    
    ledctl_set(map->led, map->ts = !prev);
    kbd_toggle_on_numlock(key, map);

    if (map->ls == EKLS_RISE || map->ls == EKLS_FALL) {
        usbd_hid_add_key_oneshot(key);
    }
}

void kbd_toggle_on_numlock(EKey key, SKeyMap* map) {
    if (map->tcb) {
        map->tcb(key, map);
    }

    if (map->ts) {
        tft_print("[NUMLOCK] ON\n");
    } else {
        tft_print("[NUMLOCK] OFF\n");
    }
}

#define KBD_TOGGLE_IGNORE_NON_RISE() \
    if (map->ls != EKLS_RISE) { \
        return; \
    }

void kbd_toggle_trigger(EKey key, SKeyMap* map) {
    if (key == EKEY_INV || !map) {
        return;
    }

    if (map->tf) {
        if (key == EKEY_MREC) {
            KBD_TOGGLE_IGNORE_NON_RISE();
            kbd_toggle_on_mrecord(key, map);
        }

        if (key == EKEY_MPLAY) {
            KBD_TOGGLE_IGNORE_NON_RISE();
            kbd_toggle_on_mplay(key, map);
            return;
        }

        if (key == EKEY_NUMLOCK) {
            KBD_TOGGLE_IGNORE_NON_RISE();
            kbd_toggle_numlock(key, map);
            return;
        }

        switch (map->tm) {
            case ETGM_TOGGLE:
            case ETGM_ONESHOT: {
                const bool prev = map->ts != 0;
                ledctl_set(map->led, map->ts = !prev);
                break;
            }
                
            case ETGM_NOT_TOGGLE:
            default: 
                // --> turn the led on only until 150 ms.
                ledctl_set(map->led, true, 150);
                break;
        }

        if (map->tcb) {
            map->tcb(key, map);
        }

        // --> handle the default key actions.
        kbd_keymap_cb_defimpl(key, map);
    }
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

            tft_print("[MACRREC] OFF\n");
        });
        
        ledctl_set(map->led, map->ts);
        if (map->ts) {
            tft_print("[MACRREC] ON\n");
        }
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