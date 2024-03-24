#ifndef __KBD_SCANNER_BASIC_H__
#define __KBD_SCANNER_BASIC_H__

#include "../kbd.h"

/**
 * Basic key scanner.
 */
class KbdBasicScanner : public IKeyScanner {
private:
    static constexpr uint8_t GPIO_DELAY = 5;
    static constexpr uint32_t MAX_ROWS = 5;
    static constexpr uint32_t MAX_COLS = 5;

private:
    static const uint8_t ROW_PINS[5];
    static const uint8_t COL_PINS[5];

private:
    /* key state bitmap. */
    uint8_t _prevRows[MAX_ROWS];
    uint8_t _nextRows[MAX_ROWS];
    uint8_t _empty;

private:
    KbdBasicScanner();

public:
    /* get the singleton instance. */
    static KbdBasicScanner* instance();

public:
    /* scan once. */
    virtual bool scanOnce();

    /* test whether no scanning result changes or not. */
    virtual bool isEmpty() const;

    /* take the latest scanned state and return true if key presents. */
    virtual bool takeState(EKey key, bool& prevOut, bool& nextOut) const;
};

#endif