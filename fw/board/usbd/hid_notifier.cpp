#define __VISIBLE_TUSB__
#include "hid_notifier.h"
#include "../config.h"
#include "../usbd.h"
#include "../../kbd/scancode.h"
#include "../../tft/tft.h"
#include <string.h>

UsbdHidNotifier::UsbdHidNotifier() {
    memset(_keycodes, 0, sizeof(_keycodes));
    _modifier = 0;
}

UsbdHidNotifier *UsbdHidNotifier::instance() {
    static UsbdHidNotifier notifier;
    return &notifier;
}

void UsbdHidNotifier::onKeyNotify(const Kbd *kbd, EKey key, EKeyState state) {
    Usbd::get()->notify(key, state);
}

void UsbdHidNotifier::onPostKeyNotify(const Kbd *kbd) {
    EKey keys[MAX_REPORT_KEYS];
    for(uint8_t i = 0; i < MAX_REPORT_KEYS; ++i) {
        keys[i] = EKEY_INV;
    }

    uint8_t keycodes[MAX_REPORT_KEYS] = {0, };
    uint8_t modifier = 0;

    // --> get current pressing keys.
    uint8_t count = kbd->getPressingKeys(keys, 6);
    uint8_t index = 0;

    for(uint8_t i = 0; i < count; ++i) {
        if (index >= 6 && keys[i] == EKEY_INV) {
            break;
        }

        const SKeyChar ch = kbd->getKeyChar(keys[i]);
        if (ch.kc != KC_NONE) {
            keycodes[index++] = ch.kc;
        }

        if (ch.mod) {
            modifier |= ch.mod;
        }
    }

    notifyHid(keycodes, modifier);
}

void UsbdHidNotifier::onUnlisten() {
    uint8_t keycodes[MAX_REPORT_KEYS] = {0, };
    uint8_t modifier = 0;

    // --> replace notification as empty.
    notifyHid(keycodes, modifier);
}

void UsbdHidNotifier::notifyHid(uint8_t keycodes[6], uint8_t modifier) {
    // --> report if any keys are changed.
    if (memcmp(_keycodes, keycodes, sizeof(_keycodes)) != 0 || _modifier != modifier) {
        memcpy(_keycodes, keycodes, sizeof(_keycodes)); _modifier = modifier;
        tud_hid_keyboard_report(RID_KEYBOARD, modifier, keycodes);
    }
}