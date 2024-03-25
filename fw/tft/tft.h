#ifndef __TFT_TFT_H__
#define __TFT_TFT_H__

#include <stdint.h>
#include "../lib/st7735/ST7735_TFT.hpp"

// --> color definitions.
#define TFT_SCREEN_COLOR    ST7735_WHITE
#define TFT_FONT_COLOR      ST7735_BLACK

// --> forward decls.
class Task;

/**
 * A character buffer element for TFT. 
 */
struct STftChar {
    uint16_t bg;    // --> background color.
    uint16_t fg;    // --> foreground color.
    char ch;        // --> character to display.
};

/**
 * TFT display mode definitions. 
 */
enum ETftMode {
    ETFTM_TTY = 0,
    ETFTM_GRAPHIC = 1,
};

/**
 * TFT display class. 
 */
class Tft {
    friend class TaskQueue;

private:
    static constexpr uint32_t MAX_COL = 14;
    static constexpr uint32_t MAX_ROW = 4;
    static constexpr uint32_t MAX_BUF = MAX_COL * MAX_ROW;

    static constexpr uint32_t MAX_GRP_COL = 160;
    static constexpr uint32_t MAX_GRP_ROW = 80;
    static constexpr uint32_t MAX_GRP_BUF = MAX_GRP_COL * MAX_GRP_ROW;

private:
    ST7735_TFT _tft;
    float _backlight;                   // --> backlight brightness, 0.0f to 1.0f.
    uint16_t _pwmValue;                 // --> applied value for PWM pin.
    uint8_t _mode;                      // --> 0: TTY mode, 1: graphic mode.
    uint8_t _prevMode;                  // --> previous mode.
    uint16_t _ttyPos;                   // --> TTY position.
    uint16_t _ttyBg;
    uint16_t _ttyFg;

    uint16_t _graphicBuf[MAX_GRP_BUF];   // --> graphic buffer.
    STftChar _ttyBuf[MAX_BUF];          // --> TTY buffer.
    int32_t _dirty;

private:
    Tft();

public:
    static Tft* get();

private:
    void setupGpio();
    void applyPwm();

protected:
    void redraw();
    
private:
    void drawGrp();
    void drawTty();

public:
    /* get the raw device. */
    ST7735_TFT* raw() const { 
        return const_cast<ST7735_TFT*>(&_tft);
    }

    /* set the display mode. */
    void mode(uint8_t mode);

    /* scroll TTY buffer. */
    void scroll(uint8_t n);

    /* clear the TTY screen. */
    void clear();

    /* set the TTY cursor position. */
    void setCursor(uint8_t x, uint8_t y) {
        if (x > MAX_COL) {
            x = MAX_COL;
        }

        if (y > MAX_ROW) {
            y = MAX_ROW;
        }

        _ttyPos = x + y * MAX_COL;
    }

    /* set color, fg and bg. */
    void setColor(uint16_t fg, uint16_t bg) { _ttyFg = fg; _ttyBg = bg; }

    /* set color. */
    void setColor(uint16_t fg) { _ttyFg = fg; }

    /* print string by format. */
    void print(const char* format, ...);

    /* print string. */
    void printText(const char* text);

    /* print a character. */
    void printChar(char ch);

    /* get a pixel at position. */
    uint16_t getPixel(uint8_t x, uint8_t y);

    /* set a pixel at position. */
    void setPixel(uint8_t x, uint8_t y, uint16_t value);
    
    /* draw bitmap on pixel buffer. */
    void drawBitmap(int16_t x, int16_t y, const uint16_t* data, uint8_t w, uint8_t h);
};

/* TTY printf redirection. */
#define tty_print(x) Tft::get()->printText(x)
#define tty_printf(...) Tft::get()->print(__VA_ARGS__)

#endif // __TFT_TFT_H__
