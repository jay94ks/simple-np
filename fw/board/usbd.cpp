#define __VISIBLE_TUSB__
#include "usbd.h"
#include "usbd/hid_notifier.h"
#include "../kbd/kbd.h"
#include "../tft/tft.h"
#include <tusb.h>

CFG_TUD_EXTERN void tud_mount_cb(void) {
    Usbd::get()->setMounted(true);
}

CFG_TUD_EXTERN void tud_umount_cb(void) {
    Usbd::get()->setMounted(false);
}

CFG_TUD_EXTERN void tud_cdc_rx_cb(uint8_t itf) {
    uint8_t buf[64] = {0, };
    uint32_t len = tud_cdc_n_read(itf, buf, sizeof(buf));

    if (len) {
        Usbd::get()->pushRbuf(buf, len);
    }
}

Usbd::Usbd() {
    _init = 0;
    _mounted = 0;
    _rpos = _wpos = 0;
    _rlen = 0;
    _decodeBuf = 0;
    _decodeLen = 0;
}

Usbd *Usbd::get() {
    static Usbd _instance;
    return &_instance;
}

bool Usbd::init() {
    if (_init) {
        return false;
    }

    _init = 1;

    // --> to ensure it must be not optimised out.
    usbdGetDevice();
    tud_init(0);
    return true;
}

UsbdHidNotifier* Usbd::getNotifier() const {
    return UsbdHidNotifier::instance();
}

bool Usbd::enableHid() {
    Kbd* kbd = Kbd::get();
    return kbd->listen(getNotifier());
}

bool Usbd::disableHid() {
    Kbd* kbd = Kbd::get();
    return kbd->unlisten(getNotifier());
}

void Usbd::stepOnce() {
    while(1) {
        tud_task();

        if (_decoder.isDone()) {
            _decoder.invoke();

            // --> reset the decoder.
            _decoder = UsbdCdcMessage();
        }

        if (_decodeLen > 0) {
            // --> error: reset and retry.
            if (_decoder.decode(_decodeBuf) == false) {
                _decoder = UsbdCdcMessage();
                continue;
            }

            _decodeLen = 0;
        }

        int16_t ch = dequeueRbuf();
        if (ch < 0) {
            break;
        }

        if (_decoder.decode(ch) == false) {
            _decodeBuf = ch;
            _decodeLen = 1;
        }
    }
}

void Usbd::setMounted(bool value) {
    _mounted = value ? 1 : 0;

    if (value) {
        _rpos = _wpos = 0;
        _rlen = 0;
    }
}

void Usbd::pushRbuf(uint8_t* buf, uint32_t len) {
    if (_rlen >= MAX_RBUF) {
        return;
    }

    while(len > 0 && _rlen < MAX_RBUF) {
        _rbuf[_wpos++] = *buf++;
        _rlen++;

        if (_wpos >= MAX_RBUF) {
            _wpos = 0;
        }
        
        len--;
    }
}

int16_t Usbd::dequeueRbuf() {
    if (_rlen <= 0) {
        return -1;
    }

    uint8_t ret = _rbuf[_rpos++];
    _rlen--;

    if (_rpos >= MAX_RBUF) {
        _rpos = 0;
    }

    return ret;
}

void Usbd::transmit(const UsbdCdcMessage& msg) {
    uint8_t buf[UsbdCdcMessage::BUF_LEN];
    uint32_t len = msg.encode(buf, sizeof(buf));

    if (len <= 0) {
        return;
    }

    tud_cdc_write(buf, len);
    tud_cdc_write_flush();
}

void Usbd::notify(EKey key, EKeyState state) {
    transmit(UsbdCdcMessage::makeNotify(key, state));
}