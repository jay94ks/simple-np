#include "numpad.h"
#include "board/usbd.h"

DEFINE_SINGLETON_MODE(NumpadMode);

bool NumpadMode::enter() {
    // --> enable HID.
    Usbd::get()->enableHid();
    return true;
}

void NumpadMode::leave() {
    // --> disable HID.
    Usbd::get()->disableHid();
}

void NumpadMode::stepOnce() {

}