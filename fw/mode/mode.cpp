#include "mode.h"
#include "welcome.h"

IMode* IMode::CURRENT = nullptr;

bool IMode::trySetCurrent(IMode* mode) {
    IMode* prevMode = CURRENT;

    // --> if nullptr, treat it as wanna enter default.
    if (!mode) {
        mode = WelcomeMode::mode();
    }

    // --> same mode: ignore.
    if (prevMode == mode) {
        return false;
    }

    // --> if failed to enter to new mode: false.
    if (!mode->enter()) {
        return false;
    }

    if (prevMode) {
        prevMode->leave();
    }

    CURRENT = mode;
    return true;
}

IMode* IMode::getCurrent() {
    if (!CURRENT) {
        // --> set default mode if nullptr.
        trySetCurrent(nullptr);
    }

    return CURRENT;
}