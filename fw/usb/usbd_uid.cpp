#include <stdint.h>
#include <pico/unique_id.h>
#include "usbd.h"

void usbd_get_uid(char* buf, uint32_t len) {
    pico_get_unique_board_id_string(buf, len);
}