#include "tft.h"
#include "../lib/st7735/ST7735_TFT.hpp"
#include "hardware/pwm.h"
#include <string.h>

ST7735_TFT g_main_tft;

constexpr int32_t MAX_TFT_ROW = 4;
constexpr int32_t MAX_TFT_COL = 14;
constexpr int32_t MAX_TFT_BUF = MAX_TFT_ROW * MAX_TFT_COL;

char g_tft_buf[MAX_TFT_BUF];
char g_tft_prev[MAX_TFT_BUF];
uint32_t g_tftcol_buf[MAX_TFT_BUF];
uint32_t g_tftcol_prev[MAX_TFT_BUF];

uint32_t g_tft_color = 0;
uint32_t g_tft_pos = 0;

#define TFT_GET_BG(x)             uint16_t((x) >> 16)
#define TFT_GET_FG(x)             uint16_t((x) & 0xffff)
#define TFT_MAKE_COLOR(fg, bg)    (uint32_t(fg) | (uint32_t((bg)) << 16))

void tft_draw_test(int x, int y) {
    for(uint8_t lx = 0; lx < x; lx++) {
        for(uint8_t ly = 0; ly < y; ly++) {
            g_main_tft.TFTdrawText(lx * 10, 20 * ly, "hello", ECOL_WHITE, ECOL_BLACK, 2);
        }
    }
}

void tft_backlight_init() {
    // --> PWM init.
    gpio_set_function(28, GPIO_FUNC_PWM);

    // --> setup duty cycle.
    uint32_t slice = pwm_gpio_to_slice_num(28);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 4.f);
    pwm_init(slice, &config, true);
    
    // --> set the backlight to 100% brightness.
    tft_set_backlight(100);
}

void tft_init() {
    tft_backlight_init();
    
    g_main_tft.TFTSetupGPIO(21, 20, 17, 18, 19);
    g_main_tft.TFTInitScreenSize(26, 1, 80, 160);
    g_main_tft.TFTInitPCBType(ST7735_TFT::TFT_ST7735S_Black);
    g_main_tft.TFTsetRotation(ST7735_TFT::TFT_Degrees_270);
    g_main_tft.TFTfillScreen(ECOL_BLACK);
    g_main_tft.TFTFontNum(g_main_tft.TFTFont_Default);
    
    // --> prepare console buffer.
    memset(g_tft_prev, 0, sizeof(g_tft_prev));
    memset(g_tft_buf, 0, sizeof(g_tft_buf));

    g_tft_color = TFT_MAKE_COLOR(ECOL_WHITE, ECOL_BLACK);
    for(uint16_t n = 0; n < MAX_TFT_BUF; ++n) {
        g_tftcol_prev[n] = g_tftcol_buf[n] = g_tft_color;
    }

}

void tft_set_backlight(float brightness) {
    brightness = (brightness > 100) ? 100 : brightness;
    brightness = (brightness < 0) ? 0 : brightness;
    uint16_t value = uint16_t(brightness / 100.0f * 255.0f);
    pwm_set_gpio_level(28, value << 8);
}

void tft_task() {
    for(uint8_t row = 0; row < MAX_TFT_ROW; ++row) {
        for(uint8_t col = 0; col < MAX_TFT_COL; ++col) {
            const uint16_t cur = row * MAX_TFT_COL + col;
            const uint32_t color = g_tftcol_buf[cur];
            char ch = g_tft_buf[row * MAX_TFT_COL + col];

            const uint32_t color_p = g_tftcol_prev[cur];
            const char ch_p = g_tft_prev[cur];

            // --> if nothfing changed, skip current row.
            if (color_p == color && ch_p == ch) {
                continue;
            }

            const uint16_t bg = TFT_GET_BG(color);
            const uint16_t fg = TFT_GET_FG(color);

            if (ch == 0) {
                ch = ' ';
            }

            g_main_tft.TFTfillRect(col * 11, 20 * row, 11, 20, bg);
            g_main_tft.TFTdrawChar(col * 11, 20 * row, ch, fg, bg, 2);

            g_tft_prev[cur] = g_tft_buf[cur];
            g_tftcol_prev[cur] = g_tftcol_buf[cur];
        }
    }
}

void tft_clear() {
    // --> prepare console buffer.
    memset(g_tft_buf, 0, sizeof(g_tft_buf));
    for(uint16_t n = 0; n < MAX_TFT_BUF; ++n) {
        g_tftcol_buf[n] = ECOL_BLACK;
    }

    g_tft_pos = 0;
}

void tft_scroll(uint8_t n) {
    if (n > MAX_TFT_ROW) {
        n = MAX_TFT_ROW;
    }

    if (n <= 0 || n == MAX_TFT_ROW) {
        tft_clear();
        return;
    }

    if (n == 1) {
        constexpr uint32_t LINES_TO_MOVE = MAX_TFT_COL * (MAX_TFT_ROW - 1);

        memmove(&g_tft_buf[0], &g_tft_buf[MAX_TFT_COL], LINES_TO_MOVE * sizeof(char));
        memmove(&g_tftcol_buf[0], &g_tftcol_buf[MAX_TFT_COL], LINES_TO_MOVE * sizeof(uint32_t));

        memset(&g_tft_buf[LINES_TO_MOVE], 0, MAX_TFT_COL);
        for(uint16_t n = LINES_TO_MOVE; n < MAX_TFT_BUF; ++n) {
            g_tftcol_buf[n] = ECOL_BLACK;
        }

        g_tft_pos = LINES_TO_MOVE;
        return;
    }

    for(uint8_t i = 0; i < n; ++i) {
        tft_scroll(1);
    }

    g_tft_pos = (MAX_TFT_ROW - n) * MAX_TFT_COL;
}

void tft_print(const char* text, uint16_t max) {
    while (*text && max) {
        const char ch = *text++;
        max--;

        if (ch == '\n') {
            uint8_t row = (g_tft_pos / MAX_TFT_COL) + 1;
            g_tft_pos = row * MAX_TFT_COL;
            continue;
        }

        if (g_tft_pos >= MAX_TFT_BUF) {
            tft_scroll(1);
        }

        const auto pos = g_tft_pos++;

        g_tft_buf[pos] = ch;
        g_tftcol_buf[pos] = g_tft_color;

        const auto rem = MAX_TFT_BUF - (pos + 1);
        memset(&g_tft_buf[pos + 1], 0, rem * sizeof(char));
        for(uint32_t i = pos + 1; i < MAX_TFT_BUF; ++i) {
            g_tftcol_buf[i] = g_tft_color;
        }
    }
    //g_tft_pos

}