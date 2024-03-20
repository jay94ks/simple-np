#include "frame.h"
#include <string.h>

uint8_t msg_frame_checksum(const SMsgFrame* frame) {
    const uint8_t len
        = frame->len > FRAME_MAX_LEN
        ? FRAME_MAX_LEN : frame->len;

    uint16_t sum = 0;
    for(uint8_t i = 0; i < len; ++i) {
        sum += frame->data[i];
    }

    sum += frame->stx + frame->etx + frame->cmd + len;
    return uint8_t(sum & 0xff);
}

uint32_t msg_frame_encode(uint8_t* buf, uint32_t max, const SMsgFrame* frame) {
    if (!frame) {
        return 0;
    }

    const uint8_t len
        = frame->len > FRAME_MAX_LEN
        ? FRAME_MAX_LEN : frame->len;

    uint8_t temp[FRAME_BUFFER_LEN];
    uint8_t pos = 0;

    temp[pos++] = frame->stx;
    temp[pos++] = frame->cmd;
    temp[pos++] = len;

    if (len) {
        memcpy(temp + pos, frame->data, len);
        pos += len;
    }

    temp[pos++] = frame->etx;
    temp[pos++] = frame->chk;
    
    if (max > pos) {
        max = pos;
    }

    if (buf) {
        memcpy(buf, temp, max);
    }

    return pos;
}

uint8_t msg_frame_dequeue_buf(const uint8_t** bufptr, uint32_t& len) {
    const uint8_t* buf = *bufptr;
    const uint8_t ret = *buf;

    *bufptr = ++buf;
    len--;

    return ret;
}

void msg_frame_init_decode_ctx(SMsgDecodeCtx* context) {
    memset(context, 0, sizeof(SMsgDecodeCtx));
    context->stage = EFDS_WAIT_STX;
}

// --> goto next stage for context.
#define msg_frame_context_next_stage(context) context->stage++

uint32_t msg_frame_decode(const uint8_t* buf, uint32_t len, SMsgDecodeCtx* context) {
    const uint32_t initial_len = len;

    // --> error state.
    if (context->stage == EFDS_ERROR || context->stage == EFDS_DONE) {
        return 0;
    }

    while(true) {
        const uint8_t stage = context->stage;
        SMsgFrame* frame = &context->frame;

        switch(context->stage) {
            case EFDS_NONE:
                memset(frame, 0, sizeof(SMsgFrame));
                msg_frame_context_next_stage(context);
                break;

            case EFDS_WAIT_STX:
                while(len) {
                    if (*buf != FRAME_STX) {
                        msg_frame_dequeue_buf(&buf, len);
                        continue;
                    }

                    // found.
                    frame->stx = FRAME_STX;
                    msg_frame_context_next_stage(context);
                    break;
                }
                break;

            case EFDS_WAIT_CMD:
                if (len) {
                    frame->cmd = msg_frame_dequeue_buf(&buf, len);
                    msg_frame_context_next_stage(context);
                }
                break;

            case EFDS_WAIT_LEN:
                if (len) {
                    frame->len = 0;

                    // --> length.
                    context->len = msg_frame_dequeue_buf(&buf, len);
                    msg_frame_context_next_stage(context);
                }
                break;

            case EFDS_WAIT_DATA:
                while (len && context->len > frame->len) {
                    frame->data[frame->len++] = msg_frame_dequeue_buf(&buf, len);
                }

                // --> more bytes required.
                if (context->len != frame->len) {
                    break;
                }

                // --> done.
                msg_frame_context_next_stage(context);
                break;

            case EFDS_WAIT_ETX:
                if (len) {
                    frame->etx = msg_frame_dequeue_buf(&buf, len);
                    msg_frame_context_next_stage(context);
                }
                break;

            case EFDS_WAIT_CHK:
                if (len) {
                    frame->chk = msg_frame_dequeue_buf(&buf, len);
                    msg_frame_context_next_stage(context);
                }
                break;

            case EFDS_DONE:
                {
                    const uint8_t new_chk = msg_frame_checksum(frame);
                    if (new_chk != frame->chk) {
                        context->stage = EFDS_ERROR; // --> error.
                    }
                }
                break;

            case EFDS_ERROR:
                break;
        }

        if (context->stage != stage) {
            continue;
        }

        break;
    }

    return initial_len - len;
}