#ifndef __KBD_H__
#define __KBD_H__

#include <stdint.h>
#include "keys.h"

/**
 * Keyboard class. 
 */
class Kbd {
public:
private:
    static constexpr uint8_t GPIO_DELAY = 5;
    static constexpr uint32_t MAX_ROWS = 5;
    static constexpr uint32_t MAX_COLS = 5;

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

private:
    Kbd();

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

#endif