#ifndef __KBD_H__
#define __KBD_H__

#include <stdint.h>
#include "keys.h"

// --> forward decls.
class IKeyHandler;

/**
 * Keyboard class. 
 */
class Kbd {
public:
private:
    static constexpr uint8_t GPIO_DELAY = 5;
    static constexpr uint32_t MAX_ROWS = 5;
    static constexpr uint32_t MAX_COLS = 5;
    static constexpr uint32_t MAX_HANDLERS = 16;

private:
    static const uint8_t ROW_PINS[5];
    static const uint8_t COL_PINS[5];
    static const SKeyChar KEY_MAP[EKEY_MAX];

public:
    /* get the keyboard instance. */
    static Kbd* get();

private:
    /* key configurations. */
    mutable SKey _keys[EKEY_MAX];
    EKey _orderedKeys[EKEY_MAX];
    
    /* key state bitmap. */
    uint8_t _prevRows[MAX_ROWS];
    uint8_t _nextRows[MAX_ROWS];

    /* key handlers. */
    IKeyHandler* _handlers[MAX_HANDLERS];
    uint8_t _countOfHandlers;

private:
    Kbd();

public:
    /* push a key handler instance. */
    bool push(IKeyHandler* handler);

    /* pop the key handler instance. */
    bool pop(IKeyHandler* handler);

private:
    /* invoke handler for the specified key. */
    bool handle(EKey key) const;

public:
    /* scan all key states and update. */
    void scanOnce();

    /* get the key pointer for the specified key. */
    SKey* getKeyPtr(EKey key) const;

    /* get the default key character data. */
    SKeyChar getDefaultKeyChar(EKey key) const;

    /* get the key character data for the specified key. */
    SKeyChar getKeyChar(EKey key) const;

    /* set the key character data for the specified key. */
    bool setKeyChar(EKey key, SKeyChar ch);

    /* reset all key character datas. */
    void resetKeyChars();

    /* get the last key number in the state. */
    EKey getRecentKey(EKeyState state) const;

    /* get pressing keys based on order value. */
    void getPressingKeys(EKey* outKeys, uint8_t max) const;
};


/**
 * Keyboard handler interface.
 */
class IKeyHandler {
public:
    virtual ~IKeyHandler() { }

public:
    /**
     * called when key state updated. 
     * this will be called after applying orders.
     * if this returns false for the key, it will yield process to other listener.
     */
    virtual bool onKeyUpdated(Kbd* kbd, EKey key, EKeyState state) = 0;
};

#endif