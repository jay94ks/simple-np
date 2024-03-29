#include "tft.h"
#include "../board/config.h"
#include "../task/task.h"
#include "hardware/pwm.h"
#include <stdio.h>
#include <stdarg.h>

Tft::Tft() {
    _backlight = 1.0f;
    _pwmValue = 0;
    _mode = ETFTM_TTY;

    for(uint16_t i = 0; i < MAX_BUF; ++i) {
        _ttyBuf[i].fg = TFT_FONT_COLOR;
        _ttyBuf[i].bg = TFT_SCREEN_COLOR;
        _ttyBuf[i].ch = ' ';
    }

    _ttyPos = 0;
    _ttyFg = TFT_FONT_COLOR;
    _ttyBg = TFT_SCREEN_COLOR;

    setupGpio();
}

void Tft::setupGpio() {
    // --> PWM init.
    gpio_set_function(GPIO_TFT_BACKLIGHT_PWM, GPIO_FUNC_PWM);

    // --> setup duty cycle.
    uint32_t slice = pwm_gpio_to_slice_num(GPIO_TFT_BACKLIGHT_PWM);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 4.f);
    pwm_init(slice, &config, true);
    
    // --> TFT library init.
    _tft.TFTSetupGPIO(
        GPIO_TFT_RST, GPIO_TFT_DC, GPIO_TFT_CS,
        GPIO_TFT_CLK, GPIO_TFT_DAT);
        
    _tft.TFTInitScreenSize(26, 1, 80, 160);
    _tft.TFTInitPCBType(ST7735_TFT::TFT_ST7735S_Black);
    _tft.TFTsetRotation(ST7735_TFT::TFT_Degrees_270);
    _tft.TFTfillScreen(TFT_SCREEN_COLOR);
    _tft.TFTFontNum(ST7735_TFT::TFTFont_Default);

    // --> apply PWM state.
    applyPwm();
}

Tft* Tft::get() {
    static Tft _tft;
    return &_tft;
}

void Tft::applyPwm() {
    _backlight = _backlight < 0.0f ? 0.0f : _backlight;
    _backlight = _backlight > 1.0f ? 1.0f : _backlight;

    uint8_t newPwmValue = uint16_t(_backlight * 255.0f);
    if (newPwmValue != _pwmValue) {
        pwm_set_gpio_level(GPIO_TFT_BACKLIGHT_PWM, newPwmValue << 8);
        _pwmValue = newPwmValue;
    }
}

void Tft::redraw() {
    uint8_t mode = _mode;
    if (_prevMode != mode) {
        _prevMode = mode;
        _dirty = 1;
        _tft.TFTfillScreen(TFT_SCREEN_COLOR);
    }

    if (_dirty == 0) {
        return;
    }

    _dirty = 0;

    if (mode != ETFTM_TTY) {
        drawGrp();
        return;
    }

    //_tft.TFTdrawText(10, 10, "hello", TFT_FONT_COLOR, TFT_SCREEN_COLOR, 2);
    drawTty();
}

void Tft::drawGrp() {
    _tft.TFTdrawBitmap16Data(0, 0,
        (uint8_t*) _graphicBuf,
        MAX_GRP_COL, MAX_GRP_ROW);
}

void Tft::drawTty() {
    for(uint8_t row = 0; row < MAX_ROW; ++row) {
        const uint32_t offset = MAX_COL * row;

        for(uint8_t col = 0; col < MAX_COL; ++col) {
            const STftChar& ch = _ttyBuf[offset + col];
            const char value = ch.ch ? ch.ch : ' ';

            _tft.TFTdrawChar(
                col * 11, row * 20,
                value, ch.fg, ch.bg, 2
            );
        }
    }
}

void Tft::mode(uint8_t mode) {
    if (mode > ETFTM_GRAPHIC) {
        return;
    }

    if (_mode != mode) {
        _mode = mode;
        _dirty = 1;
    }
}

void Tft::scroll(uint8_t n) {
    if (n <= 0) {
        return;
    }

    if (n != 1) {
        for(uint8_t i = 0; i < n; ++i) {
            scroll(1);
        }

        return;
    }

    uint8_t lp = _ttyPos / MAX_COL;
    if (lp == 0) { // --> 1st line.
        for(uint8_t i = 0; i < MAX_COL; ++i) {
            _ttyBuf[i].ch = ' ';
        }

        _ttyPos = 0;
        return;
    }

    else if (lp >= MAX_ROW) {
        lp = MAX_ROW - 1;
    }
    
    // --> scroll up the buffer.
    memcpy(&_ttyBuf[0], &_ttyBuf[MAX_COL], (MAX_BUF - MAX_COL) * sizeof(STftChar));

    // --> fill empty to last line.
    for(uint8_t i = MAX_BUF - MAX_COL; i < MAX_BUF; ++i) {
        _ttyBuf[i].ch = ' ';
        _ttyBuf[i].fg = _ttyFg;
        _ttyBuf[i].bg = _ttyBg;
    }

    // --> move position to begining of line.
    _ttyPos = lp * MAX_COL;
    _dirty = 1;
}

void Tft::clear() {
    uint16_t temp[MAX_GRP_COL];

    /* set TTY buffer to default state. */
    for(uint16_t i = 0; i < MAX_BUF; ++i) {
        _ttyBuf[i].ch = ' ';
        _ttyBuf[i].fg = _ttyFg;
        _ttyBuf[i].bg = _ttyBg;
    }

    /* set graphics buffer to background color. */
    for(uint16_t i = 0; i < MAX_GRP_COL; ++i) {
        temp[i] = _ttyBg;
    }

    /* fill graphics buffer faster than individual assignment. */
    for(uint16_t i = 0; i < MAX_GRP_ROW; ++i) {
        memcpy(&_graphicBuf[i * MAX_GRP_COL], temp, sizeof(temp));
    }

    _ttyPos = 0;    
    _dirty = 1;
}

void Tft::print(const char* format, ...) {
    char buf[MAX_BUF + 1] = {0, };

    va_list arg_ptr;
    va_start(arg_ptr, format);
    vsnprintf(buf, sizeof(buf), format, arg_ptr);
    va_end(arg_ptr);

    printText(buf);
}

void Tft::printText(const char* text) {
    if (text == nullptr) {
        return;
    }

    // --> fill the buffer.
    while (*text) {
        printChar(*text++);
    }
    
    _dirty = 1;
}

void Tft::printChar(char ch) {
    if (ch == 0) {
        ch = ' ';
    }

    // --> line feed: scroll up.
    if (ch == '\n') {
        uint8_t row = (_ttyPos / MAX_COL) + 1;
        _ttyPos = row * MAX_COL;
        return;
    }

    if (_ttyPos >= MAX_BUF) {
        scroll(1); // --> scroll once.
    }

    // --> set the buffer.
    uint16_t pos = _ttyPos++;
    
    _ttyBuf[pos].ch = ch;
    _ttyBuf[pos].fg = _ttyFg;
    _ttyBuf[pos].bg = _ttyBg;

    _dirty = 1;
}

uint16_t Tft::getPixel(uint8_t x, uint8_t y) {
    if (x >= MAX_GRP_COL || y >= MAX_GRP_ROW) {
        return TFT_SCREEN_COLOR;
    }

    return _graphicBuf[uint16_t(y) * MAX_GRP_COL + x];
}

void Tft::setPixel(uint8_t x, uint8_t y, uint16_t value) {
    if (x >= MAX_GRP_COL || y >= MAX_GRP_ROW) {
        return;
    }

    _graphicBuf[uint16_t(y) * MAX_GRP_COL + x] = value;
    _dirty = 1;
}

void Tft::drawBitmap(int16_t x, int16_t y, const uint16_t* data, uint8_t w, uint8_t h) {
    if (x >= MAX_GRP_COL || y >= MAX_GRP_ROW) {
        return;
    }

    for(uint8_t py = 0; py < h; ++py) {
        const uint8_t ay = y + py;
        if (ay < 0) {
            continue;
        }

        if (ay >= MAX_GRP_ROW) {
            break;
        }

        const uint16_t* row = &data[w * py];
        const uint16_t* rowMax = row + w;
        uint16_t rowLen = w;

        if (x < 0) {
            data += -x;
            rowLen += x;
            x = 0;
        }
        
        if (row >= rowMax) {
            continue;
        }

        if (x + rowLen > MAX_GRP_COL) {
            rowLen = MAX_GRP_COL - x;

            if (!rowLen) {
                continue;
            }
        }

        uint16_t* dst = &_graphicBuf[x + ay * MAX_COL];
        memcpy(dst, row, rowLen * sizeof(uint16_t));
        _dirty = 1;
    }
    
}