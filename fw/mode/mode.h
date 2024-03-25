#ifndef __MODE_MODE_H__
#define __MODE_MODE_H__

#include <stdint.h>

// --> declare `mode()` method.
#define DECLARE_SINGLETON_MODE(className) \
    public: \
        static className* mode()

// --> define `mode()` method body.
#define DEFINE_SINGLETON_MODE(className) \
    className* className::mode() { \
        static className _mode; \
        return &_mode; \
    }
        
/**
 * mode interface.
 */
class IMode {
private:
    static IMode* CURRENT;

public:
    /* try to set the current mode.*/
    static bool trySetCurrent(IMode* mode);

    /* get the current mode instance. */
    static IMode* getCurrent();

public:
    virtual ~IMode() { }

public:
    /* enter to this mode. */
    virtual bool enter() { return true; }

    /* leave from this mode. */
    virtual void leave() { }

public:
    /* called from main loop. */
    virtual void stepOnce() { }
};

#endif