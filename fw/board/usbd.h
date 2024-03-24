#ifndef __BOARD_USBD_H__
#define __BOARD_USBD_H__

#include <stdint.h>

#ifndef CFG_TUD_EXTERN
#define CFG_TUD_EXTERN  extern "C"
#endif

// --> this can be visible only with __VISIBLE_TUSB__ macro.
#ifdef __VISIBLE_TUSB__
#ifdef __INTELLISENSE__
#include "../../tusb_config.h"
#endif
#include <tusb.h>

const tusb_desc_device_t* usbdGetDevice();
#endif

// --> forward decls.
class UsbdHidNotifier;

/**
 * USB device.
*/
class Usbd {
private:
    Usbd();
    ~Usbd() { }

public:
    static Usbd* get();

public:
    bool init();

private:
    uint8_t _init;

public:
    /* get the HID notifier. */
    UsbdHidNotifier* getNotifier() const;

public:
    bool enableHid();
    bool disableHid();

public:
    void stepOnce();
    
};

#endif