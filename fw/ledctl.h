#ifndef __LEDCTL_H__
#define __LEDCTL_H__

#include <stdint.h>

/**
 * LED controller.
 * This is based on 74HC595D.
 * -------------
 * SER pin: 10
 * CLK pin: 11
 * RCLK pin: 12
 */

enum {
    LED_NUMLOCK = 0,
    LED_USRFN_1,
    LED_USRFN_2,
    LED_USRFN_3,
    LED_USRFN_4,
    LED_USRFN_5,
    LED_MREC,
    LED_MPL,

    LED_INV = 0xff
};

/**
 * initialize a LEDCTL.
 */
void ledctl_init();

/**
 * delay 3 cycle of MCU.
 */
void ledctl_ndelay();

/**
 * this emit current state to HC595.
 */
void ledctl_task();

/**
 * set the LED state.
 */
void ledctl_set(uint8_t led, uint8_t state);

/**
 * get the LED state.
 */
uint8_t ledctl_get(uint8_t led);

/**
 * toggle LED state.
 */
void ledctl_toggle(uint8_t led);

#endif
