#include "receiver.h"
#include "command.h"
#include "frame.h"
#include "../usb/usbd.h"

SMsgDecodeCtx g_msg_decoder;

void msg_frame_receiver_reset() {
    msg_frame_init_decode_ctx(&g_msg_decoder);
}

// --> called from `tud_cdc_rx_cb` at `usbd_event.cpp`
void msg_frame_receive(const uint8_t* buf, uint32_t len) {
    while (len) {
        uint32_t handled = msg_frame_decode(buf, len, &g_msg_decoder);
        if (handled > len) {
            // --> error.
            msg_frame_receiver_reset();
            break;
        }

        // --> subtract handled bytes.
        len -= handled; buf += handled;

        // --> test whether the decoder is done or not.
        if (msg_frame_decode_is_done(&g_msg_decoder)) {
            if (g_msg_decoder.stage == EFDS_ERROR) {
                msg_frame_init_decode_ctx(&g_msg_decoder);
                continue;
            }

            const SMsgFrame frame = g_msg_decoder.frame;
            msg_frame_init_decode_ctx(&g_msg_decoder);
        
            // --> get the handler for the command,
            //   : and then invoke it to handle the command.
            const auto handler = msg_handler_get(frame.cmd);
            if (handler) {
                handler(&frame);
            }
        }
    }
}