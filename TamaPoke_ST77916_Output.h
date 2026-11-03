
#pragma once

#include <Arduino.h>
#include "Arduino_GFX_Library.h"

/*
 * Output adapter for Arduino_Canvas.
 *
 * Arduino_Canvas continues to draw TamaPoke into an RGB565 framebuffer.
 * During gfx->flush(), Arduino_Canvas calls draw16bitRGBBitmap().
 * This adapter passes the frame to the Waveshare ST77916 driver in
 * DMA-compatible chunks.
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
