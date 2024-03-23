#ifndef __TFT_TFT_H__
#define __TFT_TFT_H__

#include <stdint.h>
#include "../lib/st7735/ST7735_TFT.hpp"

enum {
    ECOL_BLACK = ST7735_WHITE,
    ECOL_WHITE = ST7735_BLACK,
};

void tft_init();
void tft_task();

/**
 * set the brightness of backlight. 
 */
void tft_set_backlight(float brightness);

/**
 * clear all characters in TFT console.
*/
void tft_clear();

/**
 * scroll `n` lines.
 */
void tft_scroll(uint8_t n = 1);

/**
 * print characters to tft console. 
 */
void tft_print(const char* text, uint16_t max = 0xffff);

#endif // __TFT_TFT_H__
