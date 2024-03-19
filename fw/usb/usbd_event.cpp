#include "usbd.h"
#include "../kbd/macro.h"
#include <bsp/board.h>
#include <tusb.h>

uint32_t g_usbd_ts;
uint8_t g_usbd_mount;

void usbd_event_init() {
    g_usbd_ts = 0;
    g_usbd_mount = 0;
}

void usbd_stop_all() {
    kbd_macro_stop();
    kbd_macro_record_stop();
}

/**
 * USB mounted.
 */
extern "C" void tud_mount_cb(void) {
    g_usbd_ts = board_millis();
    g_usbd_mount = 1;
    usbd_stop_all();
    usbd_hid_reset();
}

uint32_t usbd_get_tick() {
    if (g_usbd_mount) {
        return board_millis() - g_usbd_ts;
    }

    return 0;
}

/**
 * USB unmounted.
 */
extern "C" void tud_umount_cb(void) {
    g_usbd_mount = 0;
    usbd_stop_all();
}

extern "C" void tud_cdc_rx_cb(uint8_t itf) {
    if (usbd_cdc_connected()) {
        if (!usbd_cdc_read_avail()) {
            return;
        }

        uint8_t buf[64] = {0, };
        uint32_t len = tud_cdc_n_read(itf, buf, sizeof(buf));

        if (len) {
            // TODO: handle CDC command here.
        }
    }
}

