#ifndef __BOARD_USBD_CDC_MESSAGE_H__
#define __BOARD_USBD_CDC_MESSAGE_H__

#include <stdint.h>
#include "../../kbd/kbd.h"

enum {
    ECMD_NOP     = 0x00,
    ECMD_GET_UFN = 0x01,
    ECMD_SET_UFN = 0x02,
    ECMD_RESET_UFN = 0x03,
    ECMD_FLASH_MODE = 0x7f,

    // -- notifications.
    ECMD_NOTIFY_KEY = 0xfe,
    ECMD_REPLY_FLAG = 0x80,
};

enum {
    ECERR_SUCCESS = 0,
    ECERR_INV_LEN = 1,
    ECERR_INV_UFN = 2,
    ECERR_INV_KEY = 3,  // invalid scan code.
    ECERR_INV_TM  = 4,  // invalid toggle mode.
};

/**
 * CDC message.
*/
class UsbdCdcMessage {
public:
    UsbdCdcMessage();

public:
    /* buffer length to decode/encode message. */
    static constexpr uint32_t BUF_LEN = 21;

private:
    uint8_t _state;
    uint8_t _stx;
    uint8_t _cmd;
    uint8_t _len;
    uint8_t _data[16];
    uint8_t _etx;
    uint8_t _chk;
    uint8_t _rlen;

private:
    enum {
        WAIT_STX = 0,
        WAIT_CMD,
        WAIT_LEN,
        WAIT_DATA,
        WAIT_ETX,
        WAIT_CHK,
        DONE
    };

public:
    /* make echo message.*/
    static UsbdCdcMessage makeEcho(uint8_t cmd, uint8_t len, uint8_t* data);

    /* make notification message. */
    static UsbdCdcMessage makeNotify(EKey key, EKeyState state);

public:
    /* test whether `DONE` state reached or not. */
    bool isDone() const {
        return _state == DONE;
    }

    /* push a byte and increment decoding state. */
    bool decode(uint8_t byte);

    /* invoke the received message. */
    void invoke();
    
    /* encode message into buffer. */
    uint32_t encode(uint8_t* buf, uint16_t len) const;

private:
    uint8_t checksum() const;

    void onCmdNop();
    void onGetUfn();
    void onSetUfn();
    void onResetUfn();
    void onFlashMode();
};

#endif