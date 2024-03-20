#ifndef __KBD_TOGGLE_H__
#define __KBD_TOGGLE_H__

#include "keymap.h"

enum {
    ETGM_TOGGLE = 0,
    ETGM_NOT_TOGGLE = 1,
    ETGM_ONESHOT = 2,
    ETGM_MAX_VALUE = 3,
    ETGM_INVALID = 0xFF
};

void kbd_toggle_init();

/**
 * set the toggle state by key.
 */
void kbd_toggle_set(EKey key, bool state);
bool kbd_toggle_get(EKey key);

/**
 * trigger the toggle callback for the key.
 */
void kbd_toggle_trigger(EKey key, SKeyMap* map);

#endif // __KBD_TOGGLE_H__