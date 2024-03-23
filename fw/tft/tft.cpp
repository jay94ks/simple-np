#include "tft.h"
#include "../lib/st7735/ST7735_TFT.hpp"
#include "hardware/pwm.h"

ST7735_TFT g_main_tft;

void tft_draw_test(int x, int y) {
    for(uint8_t lx = 0; lx < x; lx++) {
        for(uint8_t ly = 0; ly < y; ly++) {
            g_main_tft.TFTdrawText(lx * 10, 10 * ly, "hello", ECOL_WHITE, ECOL_BLACK, 3);
        }
    }
}

void tft_init() {
    gpio_set_function(28, GPIO_FUNC_PWM);

    // --> PWM init.
    uint32_t slice = pwm_gpio_to_slice_num(28);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 4.f);
    pwm_init(slice, &config, true);

    g_main_tft.TFTSetupGPIO(21, 20, 17, 18, 19);
    tft_set_backlight(100);

    g_main_tft.TFTInitScreenSize(26, 1, 80, 160);
    g_main_tft.TFTInitPCBType(ST7735_TFT::TFT_ST7735S_Black);
    g_main_tft.TFTsetRotation(ST7735_TFT::TFT_Degrees_270);
    g_main_tft.TFTfillScreen(ECOL_BLACK);

    g_main_tft.TFTFontNum(g_main_tft.TFTFont_Default);
    //g_main_tft.setTextSize(16);
    //g_main_tft.setTextColor(ST7735_BLUE);
    tft_draw_test(1, 1);
}

void tft_set_backlight(float brightness) {
    brightness = (brightness > 100) ? 100 : brightness;
    brightness = (brightness < 0) ? 0 : brightness;
    uint16_t value = uint16_t(brightness / 100.0f * 255.0f);
    pwm_set_gpio_level(28, value << 8);
}