#ifndef __MODE_WELCOME_H__
#define __MODE_WELCOME_H__

#include "mode.h"

// --> forward decls.
class WelcomeMode;

/* callback definition to member of `WelcomeMode` class. */
typedef void (WelcomeMode::* FWSchFrameCb)(const void* user, float progress);

/**
 * Frame schedule to display something.
 */
struct SWSchFrame {
    uint32_t duration;
    FWSchFrameCb callback;
    const void* user;
};

/**
 * typography frame.
 */
struct SWSchTypographyFrame {
    const char* text;
    uint8_t posX, posY;
    uint8_t size;
};

/**
 * Welcome mode.
 * This mode is for displaying `Welcome` like `hello world!`.
 */
class WelcomeMode : public IMode {
    DECLARE_SINGLETON_MODE(WelcomeMode);

private:
    static const SWSchTypographyFrame FD_0_TYPO;
    static const SWSchTypographyFrame FD_1_TYPO;
    static const SWSchFrame FRAMES[];

protected:
    WelcomeMode();

public:
    virtual ~WelcomeMode() { }

private:
    uint8_t _enter;
    uint32_t _tick;
    uint32_t _timer;

public:
    /* enter to this mode. */
    virtual bool enter() override;

    /* leave from this mode. */
    virtual void leave() override;

    /* called from main loop. */
    virtual void stepOnce() override;

private:
    void updateTimer();
    const SWSchFrame* getCurrentFrame(bool& ended, float& progressOut) const;

private:
    void frameForTypography(const void* user, float progress);
};

#endif