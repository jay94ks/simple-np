# Simple Number Pad
![](hw/RP2040_KBD_LCD.jpg)

### Video
![](https://raw.githubusercontent.com/jay94ks/simple-np/main/video.mp4)

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
basic functionality implemented: keyboard input (HID), CDC tx/rx.
but, currently CDC rx is not working with unknown reason. :(
