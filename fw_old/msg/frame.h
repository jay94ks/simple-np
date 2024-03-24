#ifndef __MSG_FRAME_H__
#define __MSG_FRAME_H__

#include <stdint.h>


enum {
    FRAME_STX = 0x02,
    FRAME_ETX = 0x03,
    FRAME_MAX_LEN = 8,

    FRAME_HEAD_LEN = 4, // STX, ETX, CMD, LEN.
    FRAME_TAIL_LEN = 1,
    FRAME_BUFFER_LEN = FRAME_HEAD_LEN + FRAME_MAX_LEN + FRAME_TAIL_LEN,
};

/**
 * Message frame.
 */
struct SMsgFrame {
    uint8_t stx;
    uint8_t etx;
    uint8_t cmd;        // --> rccc cccc (b) r: reply bit, c: command bits.
    uint8_t len;
    uint8_t data[FRAME_MAX_LEN];
    uint8_t chk;
};

/**
 * Message frame decode context.
 */
struct SMsgDecodeCtx {
    uint8_t stage;      // --> decode stage.
    uint8_t len;        // --> length of waiting data bytes.
    SMsgFrame frame;
};

enum {
    EFDS_NONE = 0,
    EFDS_WAIT_STX,
    EFDS_WAIT_CMD,
    EFDS_WAIT_LEN,
    EFDS_WAIT_DATA,
    EFDS_WAIT_ETX,
    EFDS_WAIT_CHK,
    EFDS_DONE,
    EFDS_ERROR
};

/**
 * compute checksum for the frame.
 */
uint8_t msg_frame_checksum(const SMsgFrame* frame);

/**
 * encode a message frame to byte array.
 * this returns required length if buf == nullptr or max is less than required.
 */
uint32_t msg_frame_encode(uint8_t* buf, uint32_t max, const SMsgFrame* frame);

/**
 * initialize the message decode context.
 */
void msg_frame_init_decode_ctx(SMsgDecodeCtx* context);

/**
 * decode a message frame from byte array.
 * this returns consumed length from specified buffer.
 * caller must check the stage in context.
 */
uint32_t msg_frame_decode(const uint8_t* buf, uint32_t len, SMsgDecodeCtx* context);

// --> test whether the context is done to decode or not.
#define msg_frame_decode_is_done(context)   \
    ((context)->stage == EFDS_DONE) || ((context)->stage == EFDS_ERROR)

#endif // __CDC_CDC_H__