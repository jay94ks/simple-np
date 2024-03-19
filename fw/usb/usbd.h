#ifndef __USB_USBD_H__
#define __USB_USBD_H__

#include <stdint.h>

enum {
    RID_KEYBOARD = 1,
};

enum {
    EPNUM_HID       = 0x81,
    EPNUM_CDC_OUT   = 0x02,
    EPNUM_CDC_IN    = 0x83,
    EPNUM_CDC_NOTIF = 0x84,
};

#define MAX_UID_BYTES 8

// --> usbd_uid.cpp.
void usbd_get_uid(char* buf, uint32_t len);

// --> usbd_event.cpp, called from usbd_init@usbd.cpp.
void usbd_event_init();

// --> usbd.cpp.
void usbd_init();
void usbd_task();

// --> get USBD tick in millis, usbd_event.cpp.
uint32_t usbd_get_tick();

// --> HID init & task, called from usbd_init@usbd.cpp.
void usbd_hid_init();
void usbd_hid_reset();
void usbd_hid_task();

// --> test whether the CDC is connected or not.
bool usbd_cdc_connected();

// --> test whether the CDC read channel is available or not.
bool usbd_cdc_read_avail();

// --> transmit bytes through CDC write channel and returns transmitted bytes.
uint32_t usbd_cdc_transmit(const uint8_t* buf, uint32_t len);

// --> flush transmit-pending bytes.
uint32_t usbd_cdc_flush();

// --> add a key to report @ usbd_hid.cpp
void usbd_hid_add_key(uint8_t key);

// --> add a key as oneshot @ usbd_hid.cpp
void usbd_hid_add_key_oneshot(uint8_t key);

// --> remove a key from report @ usbd_hid.cpp
void usbd_hid_remove_key(uint8_t key);

#endif // __USB_USBD_H__