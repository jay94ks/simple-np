#ifndef __KBD_HANDLERS_NUMLOCK_H__
#define __KBD_HANDLERS_NUMLOCK_H__

#include "../kbd.h"

/**
 * numlock handler. 
 */
class KbdNumlockHandler : public IKeyHandler, public IKeyListener {
public:
    ~KbdNumlockHandler() { }

private:
    KbdNumlockHandler() { }

public:
    /* get the singleton instance. */
    static KbdNumlockHandler* instance();

public:
    /**
     * called when key state updated. 
     * this will be called after applying orders.
     * if this returns false for the key, it will yield process to other listener.
     */
    virtual bool onKeyUpdated(Kbd* kbd, EKey key, EKeyState state);

    /* called on listener event. */
    virtual void onKeyNotify(const Kbd* kbd, EKey key, EKeyState state);
};

#endif
