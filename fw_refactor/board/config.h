#ifndef __BOARD_CONFIG_H__
#define __BOARD_CONFIG_H__

enum {

    GPIO_KBD_ROW_1 = 0,
    GPIO_KBD_ROW_2 = 1,
    GPIO_KBD_ROW_3 = 2,
    GPIO_KBD_ROW_4 = 3,
    GPIO_KBD_ROW_5 = 4,

    GPIO_KBD_COL_5 = 5,
    GPIO_KBD_COL_4 = 6,
    GPIO_KBD_COL_3 = 7,
    GPIO_KBD_COL_2 = 8,
    GPIO_KBD_COL_1 = 9,

    GPIO_HC595_SER = 10,
    GPIO_HC595_CLK = 11,
    GPIO_HC595_LAT = 12,

    GPIO_TFT_RST = 21,
    GPIO_TFT_DC = 20,
    GPIO_TFT_CS = 17,
    GPIO_TFT_CLK = 18,
    GPIO_TFT_DAT = 19,

    GPIO_TFT_BACKLIGHT_PWM = 28,
};

#endif