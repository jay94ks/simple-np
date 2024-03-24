#ifndef __BOARD_USBD_H__
#define __BOARD_USBD_H__

#include <stdint.h>
#include "usbd/cdc_message.h"

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
    static constexpr uint32_t MAX_RBUF = 1024;

private:
    Usbd();
    ~Usbd() { }

public:
    static Usbd* get();

public:
    bool init();

private:
    uint8_t _init;
    uint8_t _mounted;
    uint8_t _rbuf[MAX_RBUF];
    uint16_t _rpos, _wpos, _rlen;

    uint8_t _decodeBuf;
    uint8_t _decodeLen;
    UsbdCdcMessage _decoder;

public:
    /* get the HID notifier. */
    UsbdHidNotifier* getNotifier() const;

    /* test whether the USBD is mounted or not. */
    bool isMounted() const {
        return _mounted != 0;
    }

public:
    bool enableHid();
    bool disableHid();

public:
    void stepOnce();
    
public:
    /* set usb mounted, called from TUD callback. */
    void setMounted(bool value);

    /* push bytes into rbuf, called from TUD callback. */
    void pushRbuf(uint8_t* buf, uint32_t len);

private:
    /* read a byte from rbuf.*/
    int16_t dequeueRbuf();

public:
    /* transmit a message. */
    void transmit(const UsbdCdcMessage& msg);

    /* notify key state. */
    void notify(EKey key, EKeyState state);
};

#endif
