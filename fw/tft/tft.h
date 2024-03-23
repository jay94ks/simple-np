#ifndef __TFT_TFT_H__
#define __TFT_TFT_H__

#include <stdint.h>
#include "../lib/st7735/ST7735_TFT.hpp"

enum {
    ECOL_BLACK = ST7735_WHITE,
    ECOL_WHITE = ST7735_BLACK,
};

void tft_init();

/**
 * set the brightness of backlight. 
 */
void tft_set_backlight(float brightness);

#endif // __TFT_TFT_H__
