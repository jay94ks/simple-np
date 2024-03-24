#include "ledctl.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include <bsp/board.h>
#include <string.h>

constexpr uint32_t HC595_SER = 10;
constexpr uint32_t HC595_CLK = 11;
constexpr uint32_t HC595_LAT = 12;

static uint8_t g_ledctl_next = 0xffu;
static uint8_t g_ledctl_cur = 0x00u;

static uint8_t g_ledctl_rsv_map;
static uint32_t g_ledctl_rsv_off[8];

void ledctl_init() {
    gpio_init(HC595_SER);
    gpio_init(HC595_CLK);
    gpio_init(HC595_LAT);

    gpio_set_dir(HC595_SER, GPIO_OUT);
    gpio_set_dir(HC595_CLK, GPIO_OUT);
    gpio_set_dir(HC595_LAT, GPIO_OUT);

    g_ledctl_rsv_map = 0;
    memset(g_ledctl_rsv_off, 0, sizeof(g_ledctl_rsv_off));

    // --> clear HC595 state.
    ledctl_task();   
}

/**
 * delay 3 cycle of MCU.
 */
void ledctl_ndelay() {
    asm volatile(
        "nop\n"
        "nop\n"
        "nop\n"
    );

    sleep_us(1);
}

void ledctl_task() {
    if (g_ledctl_rsv_map) {
        const uint32_t now_ms = board_millis();
        for(uint8_t i = 0; i < 8; ++i) {
            const uint8_t mask = 1 << (7 - i);

            // --> test whether there is `off` planed or not.
            if ((g_ledctl_rsv_map & mask) != 0) {
                const uint32_t target_ms = g_ledctl_rsv_off[i];
                if (target_ms < now_ms) { // --> off
                    g_ledctl_rsv_map &= ~mask;
                    g_ledctl_next &= ~mask;
                }
            }
        }
    }

    if (g_ledctl_cur != g_ledctl_next) {
        const uint8_t next = g_ledctl_next;

        gpio_put(HC595_LAT, 0);
        gpio_put(HC595_CLK, 0);
        ledctl_ndelay();

        for (uint8_t i = 0; i < 8; i++) {
            const uint8_t mask = 1 << (7 - i);
            gpio_put(HC595_CLK, 0);
            ledctl_ndelay();

            gpio_put(HC595_SER, (next & mask) ? 1 : 0);
            ledctl_ndelay();

            gpio_put(HC595_CLK, 1);
            ledctl_ndelay();
        }
        
        gpio_put(HC595_CLK, 0);
        ledctl_ndelay();

        gpio_put(HC595_LAT, 1);
        ledctl_ndelay();

        gpio_put(HC595_LAT, 0);
        ledctl_ndelay();

        // --> update current state.
        g_ledctl_cur = next;
    }
}

void ledctl_set(uint8_t led, uint8_t state, uint32_t ms) {
    if (led >= 8) {
        return;
    }

    // --> active low.
    const uint8_t mask = 1 << led;
    if (state) {
        g_ledctl_next &= ~mask;
    }
    
    else {
        g_ledctl_next |= mask;
    }

    // --> put `turn-off` plan.
    if (ms != LEDCTL_PERMANENT_ON) {
        const uint32_t now_ms = board_millis();
        uint32_t fin_ms = now_ms + ms;

        // --> may overflow.
        if (fin_ms < now_ms) {
            fin_ms = 0xffffffffu;
        }
        
        g_ledctl_rsv_map |= mask;
        g_ledctl_rsv_off[led] = fin_ms;
    }

    else {
        g_ledctl_rsv_map &= ~mask;
        g_ledctl_rsv_off[led] = 0;
    }
}

uint8_t ledctl_get(uint8_t led) {
    if (led >= 8) {
        return 0;
    }

    return (g_ledctl_next & (1 << led)) ? 1 : 0;
}

void ledctl_toggle(uint8_t led) {
    if (ledctl_get(led)) {
        ledctl_set(led, 0);
        return;
    }

    ledctl_set(led, 1);
}