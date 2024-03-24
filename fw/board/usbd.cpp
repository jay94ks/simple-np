#define __VISIBLE_TUSB__
#include "usbd.h"
#include "usbd/hid_notifier.h"
#include "../kbd/kbd.h"

Usbd::Usbd() {
    _init = 0;
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
    tud_task();
}