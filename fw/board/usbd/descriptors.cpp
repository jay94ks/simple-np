#define __VISIBLE_TUSB__
#include "../usbd.h"
#include "../config.h"
#include "pico/unique_id.h"

// --> MAX UID bytes.
#define MAX_UID_BYTES 8

#ifndef TUD_HID_REPORT_DESC_KEYBOARD
#define TUD_HID_REPORT_DESC_KEYBOARD( ... )
#endif

enum {
    EPNUM_HID       = 0x81,
    EPNUM_CDC_NOTIF = 0x83,
    EPNUM_CDC_DATA  = 0x04,
};

enum {
    ITF_NUM_CDC = 0,
    ITF_NUM_CDC_DATA,
    ITF_NUM_HID,
    ITF_NUM_TOTAL
};

#define USB_VID             0x8857
#define USB_PID             0x0323
#define USB_BCD             0x0200

const tusb_desc_device_t g_usbd_device = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = USB_BCD,
    .bDeviceClass = 0x00,
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor = USB_VID,
    .idProduct = USB_PID,
    .bcdDevice = 0x0100,
    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x03,
    .bNumConfigurations = 0x01
};

const tusb_desc_device_t* usbdGetDevice() {
    return &g_usbd_device;
}

// -------------------- configurations.

#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN + TUD_CDC_DESC_LEN)

const uint8_t g_usbd_hid_report[] = {
    TUD_HID_REPORT_DESC_KEYBOARD( HID_REPORT_ID(RID_KEYBOARD) )
};

const uint8_t g_usbd_conf[] = {
    // --> config number, interface count, string index, total length, attribute, poower in mA.
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 200),

    // --> interface number, string index, protocol, report descriptor len, EP In address, size & polling interval
    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC, 4, EPNUM_CDC_NOTIF, 8, 0x80 | EPNUM_CDC_DATA, EPNUM_CDC_DATA, 64),

    // --> interface number, string index, protocol, report descriptor len, EP In address, size & polling interval
    TUD_HID_DESCRIPTOR(ITF_NUM_HID, 0, HID_ITF_PROTOCOL_NONE, sizeof(g_usbd_hid_report), EPNUM_HID, CFG_TUD_HID_EP_BUFSIZE, 5),

};

// --> callback to provide USB device descriptor.
CFG_TUD_EXTERN const uint8_t* tud_descriptor_device_cb() {
    return (const uint8_t*) &g_usbd_device;
}

CFG_TUD_EXTERN const uint8_t* tud_descriptor_configuration_cb(uint8_t index) {
    return g_usbd_conf;
}

CFG_TUD_EXTERN const uint8_t* tud_hid_descriptor_report_cb(uint8_t) {
    return g_usbd_hid_report;
}

// ------------------ string descriptors.
enum {
    STRID_LANGID = 0,
    STRID_MANUFACTURER,
    STRID_PRODUCT,
    STRID_SERIAL,
    STRID_MAX
};

const char g_usbd_lang_str[] = { 0x09, 0x0a };
const char* g_usbd_str_arr[] = {
    (const char*) g_usbd_lang_str,  // 0: english, 0x0409.
    "jay94ks",                      // 1: manufacturer.
    "Simple Number Pad",            // 2: product.
    nullptr,                        // 3: serial, use unique ID if possible.
    "Simple Number Pad - CDC",
};


constexpr size_t MAX_DESC_TEMP = 32;
static uint16_t g_usbd_desc_temp[MAX_DESC_TEMP + 1];

CFG_TUD_EXTERN const uint16_t* tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    (void)langid;
    size_t len = 0;

    switch(index) {
        case STRID_LANGID:
            memcpy(&g_usbd_desc_temp[1], g_usbd_str_arr[0], 2);
            len = 1;
            break;

        case STRID_SERIAL:
            {
                char temp[2 * MAX_UID_BYTES + 1];

                memset(temp, 0, sizeof(temp));
                pico_get_unique_board_id_string(temp, sizeof(temp));

                if ((len = strlen(temp)) > MAX_DESC_TEMP) {
                    len = MAX_DESC_TEMP;
                }

                for(size_t i = 0; i < len; ++i) {
                    g_usbd_desc_temp[i + 1] = temp[i];
                }
                
            }
            break;

        default:
            if (index >= STRID_MAX) {
                return nullptr;
            }

            const char* str = g_usbd_str_arr[index];
            if ((len = strlen(str)) > MAX_DESC_TEMP) {
                len = MAX_DESC_TEMP;
            }

            // --> convert to UTF-16 ascii code mapping.
            for(size_t i = 0; i < len; ++i) {
                g_usbd_desc_temp[i + 1] = str[i];
            }
            break;
    
    }

    g_usbd_desc_temp[0] = uint16_t(TUSB_DESC_STRING << 8) | (2 * len + 2);
    return g_usbd_desc_temp;
}
