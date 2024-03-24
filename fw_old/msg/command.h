#ifndef __MSG_COMMAND_H__
#define __MSG_COMMAND_H__

#include <stdint.h>
#include "frame.h"
#include "../kbd/keymap.h"

typedef void (* msg_frame_handler)(const SMsgFrame* frame);
struct SMsgCmdTbl {
    uint8_t cmd;
    msg_frame_handler handler;
};

/**
 * get the message handler for the specified command.
 */
msg_frame_handler msg_handler_get(uint8_t cmd);

enum {
    ECMD_NOP     = 0x00,
    ECMD_GET_UFN = 0x01,
    ECMD_SET_UFN = 0x02,
    ECMD_RESET_UFN = 0x03,
    ECMD_FLASH_MODE = 0x7f,

    // -- notifications.
    ECMD_NOTIFY_KEY = 0xfe,
};

enum {
    ECERR_SUCCESS = 0,
    ECERR_INV_LEN = 1,
    ECERR_INV_UFN = 2,
    ECERR_INV_KEY = 3,  // invalid scan code.
    ECERR_INV_TM  = 4,  // invalid toggle mode.
};

/**
 * emit a reply about command execution.
 */
void msg_handler_reply(const SMsgFrame* frame, const uint8_t* data, uint16_t len);

/**
 * emit a key notification.
 */
void msg_cmd_notify_key(EKey key, EKeyState state);

#ifdef __MSG_COMMAND_CPP__
void on_msgh_nop(const SMsgFrame* frame);
void on_msgh_get_ufn(const SMsgFrame* frame);
void on_msgh_set_ufn(const SMsgFrame* frame);
void on_msgh_reset_ufn(const SMsgFrame* frame);
void on_msgh_flash_mode(const SMsgFrame* frame);
#endif
#endif