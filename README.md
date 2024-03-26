# Simple Number Pad
![](hw/RP2040_KBD_LCD.jpg)

### License
* This repository: MIT, partially GPL-v3 due to TFT library.
* TFT library: GPL-v3, [ST7735_TFT_PICO](https://github.com/gavinlyonsrepo/ST7735_TFT_PICO)

### Video
![](video.gif)

### Schematic
![](hw/RP2040_KBD-MCU.png)
See `hw/pdf/*.pdf` files to read more details.

### Features
* MCU: RP2040, 133MHz Dual core.
* 5 keys for user-defined functions.
* 17 keys for number pad.
* `TFT` display for calculator if no `HID` attached.
* 2 macro control keys, `RECORD` and `PLAY`.

### Plans for future features
* A flash memory to store user configurations? (currently, they're stored at `SRAM` of MCU only, so, volatile)
* I2C or SPI extension port?
* A little game that runs on the `TFT` display?
* Anything else...

### Firmware
Current planed features are implemented.
Now, calculator mode will be implemented soon.

### `tinyusb` RX callback.
Don't use `0.15.0` distribution that included on `pico-sdk`.
Update its branch to above version, then it works perfectly.