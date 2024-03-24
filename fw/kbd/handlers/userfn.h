#ifndef __KBD_HANDLERS_USERFN_H__
#define __KBD_HANDLERS_USERFN_H__

#include "../kbd.h"

/**
 * user function key handler.
 */
class KbdUserFnHandler : public IKeyHandler {
public:
    ~KbdUserFnHandler() { }

private:
    KbdUserFnHandler() { }
    
public:
    /* get the singleton instance. */
    static KbdUserFnHandler* instance();

    /* get the index of user function key. */
    static int8_t indexOf(EKey key);

public:
    /**
     * called when key state updated. 
     * this will be called after applying orders.
     * if this returns false for the key, it will yield process to other listener.
     */
    virtual bool onKeyUpdated(Kbd* kbd, EKey key, EKeyState state);
};

#endif