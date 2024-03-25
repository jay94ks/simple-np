#ifndef __BOARD_USBD_HID_H__
#define __BOARD_USBD_HID_H__

#include "../../kbd/kbd.h"

// --> forward decls.
class Usbd;

/**
 * USB HID notifier.
 */
class UsbdHidNotifier : public IKeyListener {
    friend class Usbd;

private:
    static constexpr uint32_t MAX_REPORT_KEYS = 6;

public:
    ~UsbdHidNotifier() { }

private:
    UsbdHidNotifier();

protected:
    static UsbdHidNotifier* instance();

private:
    uint8_t _keycodes[MAX_REPORT_KEYS];
    uint8_t _modifier;

public:
    /* called when any key notification must be issued. */
    virtual void onKeyNotify(const Kbd* kbd, EKey key, EKeyState state) override;

    /* called when any key notification must be issued. */
    virtual void onPostKeyNotify(const Kbd* kbd) override;

public:
    /* called when the notifier disabled. */
    virtual void onUnlisten() override;

private:
    /* notify HID report. */
    void notifyHid(uint8_t keycodes[6], uint8_t modifier);
};

#endif