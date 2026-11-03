
#include "TamaPoke_ST77916_Output.h"

#include <algorithm>
#include <cstring>

#include "Display_ST77916.h"

/*
 * 360 Pixel x 22 Zeilen x 2 Byte = 15.840 Byte.
 * Der Waveshare-Treiber ist auf maximal 16 KB Transfergroesse konfiguriert.
 *
 * Der Puffer liegt statisch im internen RAM und ist damit fuer SPI-DMA
 * geeignet. LCD_addWindow() tauscht die Byte-Reihenfolge im uebergebenen
 * Puffer in-place. Deshalb kopieren wir jeden Teilframe zuerst hierher:
 * der originale Arduino_Canvas-Framebuffer bleibt unveraendert.
 */
namespace {
constexpr int16_t LCD_W = EXAMPLE_LCD_WIDTH;
constexpr int16_t LCD_H = EXAMPLE_LCD_HEIGHT;
constexpr int16_t TRANSFER_ROWS = 22;

static uint16_t dmaBuffer[LCD_W * TRANSFER_ROWS];
}

TamaPoke_ST77916_Output::TamaPoke_ST77916_Output()
    : Arduino_G(LCD_W, LCD_H) {
}

bool TamaPoke_ST77916_Output::begin(int32_t speed) {
    (void)speed;  // Die Waveshare-QSPI-Taktfrequenz ist im Treiber festgelegt.

    if (panel_handle == nullptr) {
        LCD_Init();
    }

    return panel_handle != nullptr;
}

// Diese Funktionen sind hier nicht erforderlich: Arduino_Canvas verwendet
// beim Flush ausschliesslich draw16bitRGBBitmap().
void TamaPoke_ST77916_Output::drawBitmap(
    int16_t, int16_t, uint8_t *, int16_t, int16_t, uint16_t, uint16_t) {
}

void TamaPoke_ST77916_Output::drawIndexedBitmap(
    int16_t, int16_t, uint8_t *, uint16_t *,
    int16_t, int16_t, int16_t) {
}

void TamaPoke_ST77916_Output::draw3bitRGBBitmap(
    int16_t, int16_t, uint8_t *, int16_t, int16_t) {
}

void TamaPoke_ST77916_Output::draw24bitRGBBitmap(
    int16_t, int16_t, uint8_t *, int16_t, int16_t) {
}

void TamaPoke_ST77916_Output::draw16bitRGBBitmap(
    int16_t x, int16_t y, uint16_t *bitmap,
    int16_t w, int16_t h) {

    if (panel_handle == nullptr || bitmap == nullptr || w <= 0 || h <= 0) {
        return;
    }

    // Zielrechteck auf die physische Displayflaeche begrenzen.
    const int16_t x0 = std::max<int16_t>(0, x);
    const int16_t y0 = std::max<int16_t>(0, y);
    const int16_t x1 = std::min<int16_t>(LCD_W, x + w);
    const int16_t y1 = std::min<int16_t>(LCD_H, y + h);

    if (x0 >= x1 || y0 >= y1) {
        return;
    }

    const int16_t clippedW = x1 - x0;
    const int16_t clippedH = y1 - y0;
    const int16_t sourceX = x0 - x;
    const int16_t sourceY = y0 - y;

    for (int16_t rowStart = 0; rowStart < clippedH; rowStart += TRANSFER_ROWS) {
        const int16_t rowsThisTransfer =
            std::min<int16_t>(TRANSFER_ROWS, clippedH - rowStart);

        // Kontiguierlichen RGB565-Teilframe in den DMA-Puffer kopieren.
        for (int16_t row = 0; row < rowsThisTransfer; ++row) {
            const uint16_t *src =
                bitmap + (sourceY + rowStart + row) * w + sourceX;
            uint16_t *dst = dmaBuffer + row * clippedW;
            std::memcpy(dst, src, clippedW * sizeof(uint16_t));
        }

        // LCD_addWindow() verwendet inklusive Endkoordinaten und fuehrt
        // den vom Waveshare-Treiber erwarteten Byte-Tausch selbst aus.
        LCD_addWindow(
            static_cast<uint16_t>(x0),
            static_cast<uint16_t>(y0 + rowStart),
            static_cast<uint16_t>(x1 - 1),
            static_cast<uint16_t>(y0 + rowStart + rowsThisTransfer - 1),
            dmaBuffer
        );

        // Der funktionierende Test uebergab den naechsten DMA-Puffer erst
        // nach 5 ms. Das gleiche sichere Timing nutzen wir auch hier.
        delay(5);
    }
}
