#ifndef __MODE_NUMPAD_H__
#define __MODE_NUMPAD_H__

#include "mode.h"

/**
 * Numberpad mode.
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

public:
    /* called from main loop. */
    virtual void stepOnce() override;
};

#endif