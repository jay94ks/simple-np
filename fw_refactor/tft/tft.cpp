#include "tft.h"
#include "hardware/pwm.h"

#define TFT_SCREEN_COLOR    ST7735_WHITE

Tft::Tft() {
    _backlight = 1.0f;
    _pwmValue = 0;

    setupGpio();
}

void Tft::setupGpio() {
    // --> PWM init.
    gpio_set_function(28, GPIO_FUNC_PWM);

    // --> setup duty cycle.
    uint32_t slice = pwm_gpio_to_slice_num(28);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 4.f);
    pwm_init(slice, &config, true);
    
    // --> TFT library init.
    _tft.TFTSetupGPIO(21, 20, 17, 18, 19);
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
        pwm_set_gpio_level(28, newPwmValue);
        _pwmValue = newPwmValue;
    }
}