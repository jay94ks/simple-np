#ifndef __MODE_NUMPAD_H__
#define __MODE_NUMPAD_H__

#include "mode.h"

/**
 * Numberpad mode.
 * This redirect all keys to HID input report.
 */
class NumpadMode : public IMode {
    DECLARE_SINGLETON_MODE(NumpadMode);

protected:
    NumpadMode();

public:
    virtual ~NumpadMode() { }    

public:
    /* enter to this mode. */
    virtual bool enter() override;

    /* leave from this mode. */
    virtual void leave() override;
};

#endif