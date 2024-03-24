#ifndef __TFT_TFT_H__
#define __TFT_TFT_H__

#include <stdint.h>
#include "../lib/st7735/ST7735_TFT.hpp"

/**
 * A character buffer element for TFT. 
 */
struct STftChar {
    uint16_t bg;    // --> background color.
    uint16_t fg;    // --> foreground color.
    char ch;        // --> character to display.
};

/**
 * TFT display class. 
 */
class Tft {
private:
    static constexpr uint32_t MAX_COL = 14;
    static constexpr uint32_t MAX_ROW = 4;
    static constexpr uint32_t MAX_BUF = MAX_COL * MAX_ROW;

private:
    ST7735_TFT _tft;
    float _backlight;           // --> backlight brightness, 0.0f to 1.0f.
    uint16_t _pwmValue;         // --> applied value for PWM pin.

    STftChar _prevBuf[MAX_BUF]; // --> previous buffer state.
    STftChar _nextBuf[MAX_BUF]; // --> next buffer state.

private:
    Tft();

public:
    static Tft* get();

private:
    void setupGpio();
    void applyPwm();

};

#endif // __TFT_TFT_H__
