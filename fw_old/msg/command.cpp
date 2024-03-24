#define __MSG_COMMAND_CPP__
#include "command.h"
#include "../kbd/toggle.h"
#include "../usb/usbd.h"
#include "pico/bootrom.h"
#include "pico/stdlib.h"
#include "../tft/tft.h"
#include <string.h>

const SMsgCmdTbl g_msg_cmd_tbl[] = {
    { ECMD_NOP, on_msgh_nop },                   // --> No operation, this is used for connectivity test.
    { ECMD_GET_UFN, on_msgh_get_ufn },           // --> Get the user function mapping.
    { ECMD_SET_UFN, on_msgh_set_ufn },           // --> Set the user function mapping.
    { ECMD_RESET_UFN, on_msgh_reset_ufn },        // --> Reset all user function mappings.
    { ECMD_FLASH_MODE, on_msgh_flash_mode },     // --> Enter to flash mode, aka, DFU mode.
};

// --> size of message command table.
#define MAX_MSG_CMD (sizeof(g_msg_cmd_tbl) / sizeof(SMsgCmdTbl))
#define __MATH_MIN(x, mv)   ((x) > (mv) ? (mv) : (x))

#define MAX_UFN_KEYS    5
const EKey g_msg_ufns[MAX_UFN_KEYS] = {
    EKEY_UFN_1, EKEY_UFN_2, EKEY_UFN_3, EKEY_UFN_4, EKEY_UFN_5
};

msg_frame_handler msg_handler_get(uint8_t cmd) {
    for(uint8_t i = 0; i < MAX_MSG_CMD; ++i) {
        const SMsgCmdTbl& each = g_msg_cmd_tbl[i];
        if (each.cmd == cmd) {
            return each.handler;
        }
    }

    return nullptr;
}

void msg_handler_reply(const SMsgFrame* frame, const uint8_t* data, uint16_t len) {
    SMsgFrame reply = *frame; // --> copy frame.
    uint8_t buf[FRAME_BUFFER_LEN];

    reply.cmd |= 0x80; // --> set reply bit.
    reply.len = __MATH_MIN(len, FRAME_MAX_LEN);

    if (reply.len > 0) {
        memcpy(reply.data, data, reply.len);
    }

    reply.chk = msg_frame_checksum(&reply);
    uint32_t rlen = msg_frame_encode(buf, sizeof(buf), &reply);

    // --> emit reply through CDC.
    usbd_cdc_transmit(buf, rlen);
}

void msg_cmd_notify_key(EKey key, EKeyState state) {
    SMsgFrame frame;
    uint8_t buf[FRAME_BUFFER_LEN];

    frame.stx = FRAME_STX;
    frame.etx = FRAME_ETX;
    frame.cmd = ECMD_NOTIFY_KEY;
    frame.len = 2;

    frame.data[0] = key;
    frame.data[1] = state;

    frame.chk = msg_frame_checksum(&frame);
    uint32_t rlen = msg_frame_encode(buf, sizeof(buf), &frame);

    // --> emit reply through CDC.
    usbd_cdc_transmit(buf, rlen);
    usbd_cdc_flush();
}

// ---
void on_msgh_nop(const SMsgFrame* frame) {
    // nop --> just echo.
    msg_handler_reply(frame, frame->data, frame->len);
    tft_print("OP: NOP.\n");
}

/**
 *  set the reply manually.
 */
void on_msgh_set_key_reply(uint8_t data[5], uint8_t err, 
    uint8_t ufn = 0xFF, uint8_t scan = KC_INV, uint8_t mod = 0, uint8_t toggle = 0)
{
    data[0] = err;
    data[1] = ufn;

    if (err == ECERR_SUCCESS) {
        data[2] = scan;
        data[3] = mod;
        data[4] = toggle ? 1 : 0;
    }

    else {
        data[2] = KC_INV;
        data[3] = KM_NONE;
        data[4] = 0xFF; // --> invalid.
    }
}

/**
 * set the reply from UFN mapping data.
 */
void UsbdCdcMakeUfnReply(uint8_t data[5], uint8_t ufn) {
    uint8_t err = ufn < 5 ? ECERR_SUCCESS : ECERR_INV_UFN;
    uint8_t kc = EKEY_INV;     // --> key scan code.
    uint8_t km = 0;            // --> key modifier mask.
    uint8_t tm = ETGM_INVALID; // --> toggle mode.

    if (ufn < MAX_UFN_KEYS) {
        const EKey key = g_msg_ufns[ufn];
        const SKeyMap* map = kbd_get_ptr(key);

        if (map) {
            kc = map->ch.scd;
            km = map->ch.mod;
            tm = map->tm;
        }
        else {
            err = ECERR_INV_KEY;
        }
    }

    else {
        err = ECERR_INV_UFN;
        ufn = EKEY_INV;
    }

    on_msgh_set_key_reply(data, err, ufn, kc, km, tm);
}

void on_msgh_get_ufn(const SMsgFrame* frame) {
    uint8_t data[5] = { 0, };   // --> ERROR_CODE, UFN_NO, SCAN_CODE, SCAN_MOD, TOGGLE
    if (frame->len < 1) {
        // --> error: lack of argument.
        on_msgh_set_key_reply(data, ECERR_INV_LEN);
        msg_handler_reply(frame, data, sizeof(data));
        return;
    }
    
    const uint8_t ufn_no = frame->data[0];

    UsbdCdcMakeUfnReply(data, ufn_no);
    msg_handler_reply(frame, data, sizeof(data));
}

void on_msgh_set_ufn(const SMsgFrame* frame) {
    uint8_t data[5] = { 0, };   // --> ERROR_CODE, UFN_NO, SCAN_CODE, SCAN_MOD
    if (frame->len < 4) {
        data[0] = ECERR_INV_LEN;
        msg_handler_reply(frame, data, sizeof(data));
        return;
    }
    
    // --> unpack data.
    const uint8_t ufn_no = frame->data[0];
    uint8_t kc = frame->data[1];
    uint8_t km = frame->data[2];
    uint8_t tm = frame->data[3];
    uint8_t err = 0;

    if (ufn_no < MAX_UFN_KEYS) {
        // --> scan code.
        if (kc == KC_INV) {
            err = ECERR_INV_KEY;
        }

        else if (tm >= ETGM_MAX_VALUE) {
            err = ECERR_INV_TM;
        }

        else {
            const EKey key = g_msg_ufns[ufn_no];
            SKeyMap* map = kbd_get_ptr(key);
            
            if (!map) {
                err = ECERR_INV_KEY;
            }

            else {
                // --> copy key properties.
                map->ch.scd = kc;
                map->ch.mod = km;
                map->tm = tm;
            }
        }
    }

    else {
        err = ECERR_INV_UFN;
    }

    if (err == 0) {
        UsbdCdcMakeUfnReply(data, ufn_no);
    }

    else {
        on_msgh_set_key_reply(data, err, ufn_no, kc, km, tm);
    }

    msg_handler_reply(frame, data, sizeof(data));
}

void on_msgh_reset_ufn(const SMsgFrame* frame) {
    // --> reset all user fn keys.
    for(uint8_t i = 0; i < MAX_UFN_KEYS; ++i) {
        const EKey key = g_msg_ufns[i];
        SKeyMap* map = kbd_get_ptr(key);

        if (map) {
            map->ch.scd = 0;
            map->ch.mod = 0;
            map->tm = 0;
        }
    }

    
    on_msgh_nop(frame);
}

void on_msgh_flash_mode(const SMsgFrame* frame) {
    on_msgh_nop(frame);
    sleep_ms(100);

#ifndef __INTELLISENSE__
    reset_usb_boot(0, 0);
#endif
}
