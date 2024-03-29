#include "numpad.h"
#include "../board/usbd.h"
#include "../tft/tft.h"
#include "../kbd/kbd.h"

DEFINE_SINGLETON_MODE(NumpadMode);

NumpadMode::NumpadMode() {
    
}

bool NumpadMode::enter() {
    // --> enable HID.
    Usbd::get()->enableHid();

    // --> set the TFT mode to TTY.
    Tft::get()->mode(ETFTM_TTY);
    Tft::get()->clear();

    // --> enable KBD.
    Kbd::get()->enable();    
    return true;
}

void NumpadMode::leave() {
    // --> disable HID.
    Usbd::get()->disableHid();

    // --> clear the TFT display.
    //Tft::get()->clear();

    // --> disable KBD.
    Kbd::get()->disable();
}