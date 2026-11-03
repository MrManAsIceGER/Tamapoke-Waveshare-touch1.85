
#pragma once

#include <Arduino.h>
#include "Arduino_GFX_Library.h"

/*
 * Ausgabeadapter fuer Arduino_Canvas.
 *
 * Arduino_Canvas zeichnet TamaPoke wie bisher in einen RGB565-Framebuffer.
 * Beim gfx->flush() ruft Arduino_Canvas draw16bitRGBBitmap() auf.
 * Dieser Adapter uebergibt den Frame in DMA-tauglichen Teilbildern an
 * den erfolgreich getesteten Waveshare-ST77916-Treiber.
 */

class TamaPoke_ST77916_Output : public Arduino_G {
public:
    TamaPoke_ST77916_Output();

    bool begin(int32_t speed = GFX_NOT_DEFINED) override;

    void drawBitmap(
        int16_t x, int16_t y, uint8_t *bitmap,
        int16_t w, int16_t h, uint16_t color, uint16_t bg
    ) override;

    void drawIndexedBitmap(
        int16_t x, int16_t y, uint8_t *bitmap,
        uint16_t *color_index, int16_t w, int16_t h,
        int16_t x_skip = 0
    ) override;

    void draw3bitRGBBitmap(
        int16_t x, int16_t y, uint8_t *bitmap,
        int16_t w, int16_t h
    ) override;

    void draw16bitRGBBitmap(
        int16_t x, int16_t y, uint16_t *bitmap,
        int16_t w, int16_t h
    ) override;

    void draw24bitRGBBitmap(
        int16_t x, int16_t y, uint8_t *bitmap,
        int16_t w, int16_t h
    ) override;
};
