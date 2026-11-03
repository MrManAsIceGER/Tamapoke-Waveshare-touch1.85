Waveshare ST77916 Standalone Test
=================================

Board: Waveshare ESP32-S3-Touch-LCD-1.85 Rev 2.0
Arduino IDE: 2.x
ESP32 Arduino Core: 3.x (tested configuration reported by user: 3.3.12)
No Arduino_GFX or LVGL library is required by this standalone sketch.

HOW TO USE
----------
1. Extract this ZIP.
2. Open ST77916_Waveshare_Test.ino in Arduino IDE.
3. Select ESP32S3 Dev Module.
4. Set USB CDC On Boot = Enabled.
5. Set Flash Size = 16MB and PSRAM = OPI PSRAM.
6. Select the correct COM port.
7. Upload and open Serial Monitor at 115200 baud.

The required source files are all in the sketch root. Keep the .c file as .c.
The display implementation is based on the Waveshare files supplied by the user.
The TCA9554 adapter uses the already-tested EXIO1/EXIO2 functionality, leaving
other expander pins untouched as inputs/configured by the existing expander state.

IMPORTANT IMPLEMENTATION DETAILS
--------------------------------
- QSPI pins: SCK 40, D0 46, D1 45, D2 42, D3 41, CS 21.
- LCD reset: TCA9554 EXIO2.
- Backlight: GPIO5 using LEDC PWM.
- The Waveshare init test sends a 15,840-byte DMA block, so the bus max transfer
  size is set to 16 KB. The later color test writes two rows at a time (1,440 bytes).
- LCD_addWindow swaps RGB565 bytes in-place, so the test refills the line buffer
  before every draw operation.

The source package is prepared for the user's reported core version, but has not
been compiled against the user's local Arduino installation or tested on their
physical board in this environment. If compilation fails, send the complete
compiler output.
