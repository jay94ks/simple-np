#ifndef __BOARD_LEDCTL_H__
#define __BOARD_LEDCTL_H__

#include <stdint.h>

/**
 * LED bit-indices.
 */
enum ELED {
    ELED_NUMLOCK = 0,
    ELED_UFN_1,
    ELED_UFN_2,
    ELED_UFN_3,
    ELED_UFN_4,
    ELED_UFN_5,
    ELED_MREC,
    ELED_MPL,

    LED_INV = 0xff
};

/**
 * LED indicators.
 * (HC595D driver)
 */
class Ledctl {
private:
    uint8_t _next;
    uint8_t _prev;

private:
    Ledctl();

public:
    /* get the singleton instance. */
    static Ledctl* get();

public:
    /* update LED states. */
    void updateOnce();

    /* get the current LED state. */
    bool get(ELED led);

    /* set the LED state. */
    void set(ELED led, bool state);

    /* toggle the LED state and return final state. */
    bool toggle(ELED led);
};

#endif