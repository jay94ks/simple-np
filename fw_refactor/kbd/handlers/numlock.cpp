#include "numlock.h"
#ifdef __INTELLISENSE__
#include "../../tusb_config.h"
#endif
#include "../../board/ledctl.h"
#include <tusb.h>

KbdNumlockHandler* KbdNumlockHandler::instance() {
    static KbdNumlockHandler _handler;
    return &_handler;
}

bool KbdNumlockHandler::onKeyUpdated(Kbd* kbd, EKey key, EKeyState state) {
    if (key != EKEY_NUMLOCK) {
        return false;
    }
    
    SKey* numlock = kbd->getKeyPtr(key);
    switch(state) {
        case EKLS_FALL: // --> updating on `RELEASING` state.
            numlock->ts = ~numlock->ts;
            break;

        default:
            break;
    }

    // --> successfully handled.
    return true;
}

void KbdNumlockHandler::onKeyNotify(const Kbd* kbd, EKey key, EKeyState state) {
    if (key != EKEY_NUMLOCK) {
        return;
    }
    
    // --> apply toggle state.
    SKey* numlock = kbd->getKeyPtr(key);
    Ledctl::get()->set(ELED_NUMLOCK, numlock->ts);
}

CFG_TUD_EXTERN void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {
  (void) instance;

    if (report_type == HID_REPORT_TYPE_OUTPUT && report_id == RID_KEYBOARD) {
        if (bufsize < 1) {
            return;
        }

        Kbd* kbd = Kbd::get();
        SKey* numlock = kbd->getKeyPtr(EKEY_NUMLOCK);
        if (numlock == nullptr) {
            return;
        }

        // --> receive keyboard LED status.
        const uint8_t kbd_leds = buffer[0];

        // --> set the received toggle state.
        if ((kbd_leds & KEYBOARD_LED_NUMLOCK) != 0) {
            numlock->ts = 0xff;
        }

        else {
            numlock->ts = 0x00;
        }
        
        KbdNumlockHandler::instance()
            ->onKeyNotify(kbd, EKEY_NUMLOCK, EKeyState(numlock->ls));
    }
}

CFG_TUD_EXTERN uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen) {
  (void) instance;
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) reqlen;
  return 0;
}
