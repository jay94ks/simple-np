#include "cdc_message.h"
#include "../usbd.h"
#include "../../kbd/kbd.h"
#include "../../kbd/scancode.h"
#include "../../kbd/handlers/userfn.h"
#include "../../tft/tft.h"
#include "pico/bootrom.h"
#include <string.h>

#define UsbdTransmitReply(data) Usbd::get()->transmit(makeEcho(_cmd | ECMD_REPLY_FLAG, 5, data))
#define UsbdTransmitEchoReply(data, len) Usbd::get()->transmit(makeEcho(_cmd | ECMD_REPLY_FLAG, len, data))

UsbdCdcMessage::UsbdCdcMessage() {
    _state = 0;
    _stx = 0x02;
    _etx = 0x03;
    _cmd = 0;
    _len = _rlen = 0;
    _chk = 0;
}

UsbdCdcMessage UsbdCdcMessage::makeEcho(uint8_t cmd, uint8_t len, uint8_t* data) {
    UsbdCdcMessage message;
    if (len >= 16) {
        len = 16;
    }

    message._cmd = cmd; // --> NOP.
    message._len = message._rlen = len;

    memcpy(message._data, data, len);
    message._chk = message.checksum();

    return message;
}

UsbdCdcMessage UsbdCdcMessage::makeNotify(EKey key, EKeyState state) {
    UsbdCdcMessage message;
    
    message._cmd = ECMD_NOTIFY_KEY; // --> NOTIFY_KEY.
    message._len = message._rlen = 2;
    message._data[0] = key;
    message._data[1] = state;
    message._chk = message.checksum();

    return message;
}

bool UsbdCdcMessage::decode(uint8_t byte) {
    switch(_state) {
        case WAIT_STX:
            if (byte != 0x02) {
                return true;
            }

            _state++;
            return true;

        case WAIT_CMD:
            _cmd = byte;
            _state++;
            return true;

        case WAIT_LEN:
            if ((_len = byte) == 0) {
                _state++;
            }

            _state++;
            return true;

        case WAIT_DATA:
            if (_rlen < _len) {
                _data[_rlen++] = byte;
            }

            if (_rlen == _len) {
                _state++;
            }

            return true;

        case WAIT_ETX:
            _etx = byte;
            _state++;
            return true;

        case WAIT_CHK:
            _chk = byte;
            _state++;
            return true;

        default:
            break;
    }

    return false;
}

void UsbdCdcMessage::invoke() {
    if (checksum() != _chk) {
        return; // --> ignore checksum mismatch.
    }

    Kbd* kbd = Kbd::get();
    switch(_cmd) {
        case ECMD_NOP: // --> NOP.
            onCmdNop();
            break;

        case ECMD_GET_UFN: // --> GET_UFN:
            onGetUfn();
            break;

        case ECMD_SET_UFN: // --> SET_UFN:
            onSetUfn();
            break;

        case ECMD_RESET_UFN: // --> RESET_UFN:
            onResetUfn();
            break;

        case ECMD_FLASH_MODE: // --> FLASH_MODE:
            onFlashMode();
            break;

        default:
            return;
    }

}

uint32_t UsbdCdcMessage::encode(uint8_t* buf, uint16_t len) const {
    uint8_t temp[BUF_LEN];
    uint8_t pos = 0;

    temp[pos++] = _stx;
    temp[pos++] = _cmd;
    temp[pos++] = _len;

    memcpy(temp + pos, _data, _len);
    pos += _len;

    temp[pos++] = _etx;
    temp[pos++] = _chk;

    // min(pos, len).
    if (pos > len) {
        pos = len;
    }

    memcpy(buf, temp, pos);
    return pos;
}

uint8_t UsbdCdcMessage::checksum() const {
    uint16_t sum = _stx + _cmd + _len + _etx;

    for(uint8_t i = 0; i < _len; ++i) {
        sum += _data[i];
    }

    return uint8_t(sum & 0xff);
}

void UsbdCdcMessage::onCmdNop() {
    tty_print("NOP\n");
    UsbdTransmitEchoReply(_data, _len);
}

/**
 *  set the reply manually.
 */
void UsbdCdcMakeUfnReply(uint8_t data[5], uint8_t err, 
    uint8_t ufn, uint8_t scan = KC_INV, uint8_t mod = 0, uint8_t toggle = 0)
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
        data[3] = 0;
        data[4] = 0xFF; // --> invalid.
    }
}

/**
 * set the reply from UFN mapping data.
 */
void UsbdCdcMakeUfnReply(uint8_t data[5], uint8_t ufn) {
    uint8_t err = ufn < 5 ? ECERR_SUCCESS : ECERR_INV_UFN;
    uint8_t kc = EKEY_INV;     // --> key scan code.
    uint8_t km = KM_NONE;      // --> key modifier mask.
    uint8_t tm = EKTG_INVALID; // --> toggle mode.

    if (ufn < KbdUserFnHandler::MAX_UFN) {
        const EKey key = KbdUserFnHandler::keyOf(ufn);
        const SKey* map = Kbd::get()->getKeyPtr(key);

        if (map) {
            kc = map->ch.kc;
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

    UsbdCdcMakeUfnReply(data, err, ufn, kc, km, tm);
}

void UsbdCdcMessage::onGetUfn() {
    uint8_t data[5] = { 0, };   // --> ERROR_CODE, UFN_NO, SCAN_CODE, SCAN_MOD, TOGGLE

    if (_len < 1) {
        // --> invalid message: ECERR_INV_LEN.
        UsbdCdcMakeUfnReply(data, ECERR_INV_LEN, 0xFF);
        UsbdTransmitReply(data);
        return;
    }

    const uint8_t ufn = _data[0];
    UsbdCdcMakeUfnReply(data, ufn);
    UsbdTransmitReply(data);

}

void UsbdCdcMessage::onSetUfn() {
    uint8_t data[5] = { 0, };   // --> ERROR_CODE, UFN_NO, SCAN_CODE, SCAN_MOD, TOGGLE

    if (_len < 4) {
        // --> invalid message: ECERR_INV_LEN.
        UsbdCdcMakeUfnReply(data, ECERR_INV_LEN, 0xFF);
        UsbdTransmitReply(data);
        return;
    }

    constexpr uint32_t MAX_UFN = KbdUserFnHandler::MAX_UFN;
    const uint8_t ufn = _data[0];
    const uint8_t kc = _data[1];
    const uint8_t km = _data[2];
    const uint8_t tm = _data[3];
    uint8_t err = 0;

    if (ufn < MAX_UFN) {
        if (kc == KC_INV) {
            err = ECERR_INV_KEY;
        }

        else if (tm >= EKTG_MAX_VALUE) {
            err = ECERR_INV_TM;
        }
        else {
            const EKey key = KbdUserFnHandler::keyOf(ufn);
            SKey* map = Kbd::get()->getKeyPtr(key);

            if (!map) {
                err = ECERR_INV_KEY;
            }
            
            else {
                map->ch.kc = kc;
                map->ch.mod = km;
                map->tm = tm;
            }
        }
    }

    else {
        err = ECERR_INV_UFN;
    }

    if (err == 0) {
        UsbdCdcMakeUfnReply(data, ufn);
        UsbdTransmitReply(data);
        return;
    }

    UsbdCdcMakeUfnReply(data, err, ufn);
    UsbdTransmitReply(data);
}

void UsbdCdcMessage::onResetUfn() {
    constexpr uint32_t MAX_UFN = KbdUserFnHandler::MAX_UFN;
    for(uint8_t i = 0; i < MAX_UFN; ++i) {
        const EKey key = KbdUserFnHandler::keyOf(i);
        SKey* map = Kbd::get()->getKeyPtr(key);

        if (map) {
            map->ch.kc = KC_NONE;
            map->ch.mod = KM_NONE;
            map->tm = EKTG_NONE;
        }
    }

    UsbdTransmitEchoReply(_data, _len);
}

void UsbdCdcMessage::onFlashMode() {
    UsbdTransmitEchoReply(_data, _len);
    sleep_ms(100);

#ifndef __INTELLISENSE__
    reset_usb_boot(0, 0);
#endif
}