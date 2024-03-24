#ifndef __KBD_MACRO_H__
#define __KBD_MACRO_H__

#include <stdint.h>

/**
 * recorded key item.
 */
struct SKeyRecord {
    uint8_t dm;  // --> delay (ms) from previous record.
    uint8_t ls;  // --> level state must be.
    uint8_t key; // --> key code. (EKey)
};

/**
 * macro state.
 */
enum EMacroState {
    EKMS_NONE = 0,
    EKMS_RECORD,    // --> recording.
    EKMS_PLAY       // --> playing.
};

void kbd_macro_init();
EMacroState kbd_macro_state();

// --> macro completion callback.
typedef void(*kbd_macro_cb)();

// --> play recorded macro.
bool kbd_macro_play(kbd_macro_cb cb);

// --> stop the macro player.
bool kbd_macro_stop();

// --> start recording macro.
bool kbd_macro_record(kbd_macro_cb cb);

// --> stop the macro recoder.
bool kbd_macro_record_stop();

/**
 * get the emulated key-scan bits for restoring current frame.
 */
bool kbd_macro_get_current(uint8_t* keyscan, uint8_t rows);

#endif