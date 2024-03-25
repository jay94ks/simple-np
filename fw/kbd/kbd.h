#ifndef __KBD_H__
#define __KBD_H__

#include <stdint.h>
#include <vector>
#include "keys.h"

// --> forward decls.
class IKeyScanner;
class IKbdState;
class IKeyHandler;
class IKeyListener;

/**
 * Keyboard class. 
 */
class Kbd {
public:
    // --> shortcuts.
    using FScannerList = std::vector<IKeyScanner*>;
    using FHandlerList = std::vector<IKeyHandler*>;
    using FListenerList = std::vector<IKeyListener*>;

private:
    static const SKeyChar KEY_MAP[EKEY_MAX];

public:
    /* get the keyboard instance. */
    static Kbd* get();

private:
    /* key configurations. */
    mutable SKey _keys[EKEY_MAX];
    EKey _orderedKeys[EKEY_MAX];
    
    /* key handlers. */
    FScannerList _scanners;
    FHandlerList _handlers;
    FListenerList _listeners;

    /* enable/disable states. */
    uint8_t _enabled, _reserved;
    
private:
    Kbd();

public:
    /* push a key scanner instance. */
    bool push(IKeyScanner* scanner);

    /* pop the key scanner instance. */
    bool pop(IKeyScanner* scanner);

    /* push a key handler instance. */
    bool push(IKeyHandler* handler);

    /* pop the key handler instance. */
    bool pop(IKeyHandler* handler);

    /* add the key listener instance. */
    bool listen(IKeyListener* listener);

    /* remove the key listener instance. */
    bool unlisten(IKeyListener* listener);

private:
    /* invoke handler for the specified key. */
    bool handle(EKey key) const;

public:
    /* scan all key states and update. */
    void scanOnce();

private:
    /* update all key states. */
    void updateOnce(const FScannerList& scanners);

    /* trigger handlers for keys. */
    void trigger();

public:
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

    /* test whether the key is down or not. */
    bool isKeyDown(EKey key) const;

    /* test whether the key is up or not. */
    bool isKeyUp(EKey key) const;

    /* check the key state matches or not. */
    bool checkKeyState(EKey key, EKeyState state) const;

    /* get the last key number in the state. */
    EKey getRecentKey(EKeyState state) const;

    /* get pressing keys based on order value. */
    uint8_t getPressingKeys(EKey* outKeys, uint8_t max) const;

    /* set the key state forcibly. */
    bool forceKeyState(EKey key, EKeyState state);

private:
    /* get all state listeners. */
    void getStateListeners(std::vector<IKbdState*>& listeners);

public:
    /* test whether the keyboard update enabled or not. */
    bool isEnabled() const { return _enabled != 0; }

    /* enable the keyboard updates. */
    bool enable();

    /* disable the keyboard updates. */
    bool disable();

};

/**
 * Key scanner.
 */
class IKeyScanner {
public:
    virtual ~IKeyScanner() { }

public:
    /* scan once. */
    virtual bool scanOnce() = 0;

    /* test whether no scanning result changes or not. */
    virtual bool isEmpty() const = 0;

    /* take the latest scanned state and return true if key presents. */
    virtual bool takeState(EKey key, bool& nextOut) const = 0;
};

/**
 * Key handler/listener's base interface.
 * This is used to get `enable`, `disable` event.
*/
class IKbdState {
public:
    virtual ~IKbdState() { }

public:
    /* called when the kbd is enabled. */
    virtual void onEnabled(const Kbd* kbd) { }

    /* called when the kbd is disabled. */
    virtual void onDisabled(const Kbd* kbd) { }
};

/**
 * Keyboard handler interface.
 */
class IKeyHandler : public IKbdState {
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

/**
 * Keyboard listener interface. 
 */
class IKeyListener : public IKbdState {
public:
    virtual ~IKeyListener() { }

public:
    /**
     * called when key state updated.
     */
    virtual void onKeyNotify(const Kbd* kbd, EKey key, EKeyState state) = 0;

    /**
     * called after all `Key-Notify` cycle completed. 
     */
    virtual void onPostKeyNotify(const Kbd* kbd) { }

public:
    /**
     * called when the listener is registered on the kbd instance.
     */
    virtual void onListen() { }

    /**
     * called when the listener is unregistered from the kbd instance.
     */
    virtual void onUnlisten() { }
};

#endif