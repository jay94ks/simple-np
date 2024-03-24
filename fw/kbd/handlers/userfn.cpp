#include "userfn.h"
#include "../../board/ledctl.h"

KbdUserFnHandler* KbdUserFnHandler::instance() {
    static KbdUserFnHandler _handler;
    return &_handler;
}

int8_t KbdUserFnHandler::indexOf(EKey key) {
    switch(key) {
        case EKEY_UFN_1: return 0;
        case EKEY_UFN_2: return 1;
        case EKEY_UFN_3: return 2;
        case EKEY_UFN_4: return 3;
        case EKEY_UFN_5: return 4;
        default: break;
    }

    return -1;
}

bool KbdUserFnHandler::onKeyUpdated(Kbd* kbd, EKey key, EKeyState state) {
    int8_t ufn = indexOf(key);
    if (ufn < 0) {
        return false;
    }

    Ledctl* ledctl = Ledctl::get();
    ELED led = ELED(ufn + 1);
    switch(state) {
        case EKLS_FALL:
            ledctl->set(led, false);
            break;

        case EKLS_RISE:
            ledctl->set(led, true);
            break;

        default:
            break;
    }

    // TODO: handles user function key.
    return true;
}