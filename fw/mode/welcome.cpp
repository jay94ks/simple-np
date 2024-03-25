#include "welcome.h"
#include "numpad.h"
#include "../board/ledctl.h"
#include "../board/usbd.h"
#include "../tft/tft.h"
#include "../kbd/kbd.h"
#include <bsp/board.h>

DEFINE_SINGLETON_MODE(WelcomeMode);

const SWSchTypographyFrame WelcomeMode::FD_0_TYPO = {
    .text = "Happy birthday",
    .posX = 0, .posY = 1,
    .size = 2,
};

const SWSchTypographyFrame WelcomeMode::FD_1_TYPO = {
    .text = "by your brother",
    .posX = 1, .posY = 2,
    .size = 2,
};

const SWSchFrame WelcomeMode::FRAMES[] = {
    // --> typography, 5 seconds. Hello.
    { 5 * 1000, &WelcomeMode::frameForTypography, &FD_0_TYPO },
    { 5 * 1000, &WelcomeMode::frameForTypography, &FD_1_TYPO },
};

// --> max welcome frames.
#define MAX_WELCOME_FRAMES \
    sizeof(WelcomeMode::FRAMES) / sizeof(SWSchFrame)

WelcomeMode::WelcomeMode() {
    _timer = 0;
    _enter = 0;
    _tick = 0;
}

bool WelcomeMode::enter() {
    // --> disable HID.
    Usbd::get()->disableHid();

    // --> set the TFT mode to graphic.
    Tft::get()->mode(ETFTM_TTY);
    Tft::get()->clear();

    // --> disable KBD.
    Kbd::get()->disable(); 

    _timer = 0;
    _enter = 1;
    _tick = board_millis();
    return true;
}

void WelcomeMode::leave() {
    _enter = 0;
    _tick = 0;
}

void WelcomeMode::stepOnce() {
    updateTimer();

    // --> trigger frames.
    bool ended = false;
    float progress = 0;
    const SWSchFrame* frame = getCurrentFrame(ended, progress);

    if (ended) {
        // --> switch to numpad mode.
        trySetCurrent(NumpadMode::mode());
        return;
    }

    if (frame) {
        auto callback = frame->callback;
        (this->*callback)(frame->user, progress);
        return;
    }
}

void WelcomeMode::updateTimer() {
    const uint32_t now = board_millis();
    if (now < _tick) {
        // --> counter overflow.
        _tick = now;
        return;
    }

    const uint32_t elapsed = now - _tick;
    if (elapsed <= 0) {
        return; // --> no time flows.
    }

    _tick = now;
    _timer += elapsed;
}

const SWSchFrame* WelcomeMode::getCurrentFrame(bool& ended, float& progressOut) const {
    const SWSchFrame* current = nullptr;
    uint32_t endAt = 0;
    uint32_t last = 0;

    for(uint32_t i = 0; i < MAX_WELCOME_FRAMES; ++i) {
        const SWSchFrame* frame = &FRAMES[i];
        const uint32_t startAt = endAt;

        // --> if timer is not reached yet.
        if (startAt > _timer) {
            break;
        }
        
        endAt += frame->duration;
        current = frame;
    }

    // --> set about current frame progress.
    if (current) {
        const uint32_t left = endAt - _timer;
        progressOut = 1.0f - (left * 1.0f / current->duration);

        if (progressOut < 0) {
            progressOut = 0;
        }

        if (progressOut > 1) {
            progressOut = 1;
        }
    }

    // --> set whether all frames are done or not.
    ended = endAt <= _timer;
    return current;
}

#define WM_MIN(x, y)    ((x) > (y) ? (y) : (x))
#define WM_MAX(x, y)    ((x) > (y) ? (x) : (y))

void WelcomeMode::frameForTypography(const void *user, float progress) {
    const SWSchTypographyFrame* frame = (const SWSchTypographyFrame*) user;
    Tft* tft = Tft::get();

    float current = WM_MIN(WM_MAX(progress - 0.1f, 0), 0.8f) / 0.8f;

    uint16_t len = strlen(frame->text);
    uint16_t visible = uint16_t(current * len);
    
    char buf[128] = { 0, };
    if (len) {
        memcpy(buf, frame->text, visible);
    }

    tft->setCursor(frame->posX, frame->posY);
    tft->printText(buf);
}
