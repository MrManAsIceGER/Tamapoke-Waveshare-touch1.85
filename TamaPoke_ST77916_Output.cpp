#include "TamaPoke_ST77916_Output.h"

#include "Display_ST77916.h"

/*
 * TamaPoke continues to use its original 466 x 466 coordinate system.
 * The physical Waveshare display has a resolution of 360 x 360 pixels.
 *
 * During flush(), Arduino_Canvas passes the complete 466x466 RGB565 frame.
 * This adapter scales it to 360x360 using nearest-neighbor sampling and sends
 * at most 22 rows per DMA transfer (360 * 22 * 2 = 15,840 bytes).
 *
 * LCD_addWindow() swaps the byte order in the buffer in-place.
 * Therefore, each chunk is copied into its own DMA buffer.
 */
namespace {
constexpr int16_t LCD_W = EXAMPLE_LCD_WIDTH;
constexpr int16_t LCD_H = EXAMPLE_LCD_HEIGHT;
constexpr int16_t GAME_W = 466;
constexpr int16_t GAME_H = 466;
constexpr int16_t TRANSFER_ROWS = 22;

static uint16_t dmaBuffer[LCD_W * TRANSFER_ROWS];
}

TamaPoke_ST77916_Output::TamaPoke_ST77916_Output()
    : Arduino_G(LCD_W, LCD_H) {
}

bool TamaPoke_ST77916_Output::begin(int32_t speed) {
    (void)speed;  // The QSPI clock frequency is configured in the Waveshare driver.

    if (panel_handle == nullptr) {
        LCD_Init();
    }

    return panel_handle != nullptr;
}

// Arduino_Canvas calls draw16bitRGBBitmap() during flush().
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

    // This output adapter is intended for the complete TamaPoke canvas.
    // Other/partial bitmaps are not silently scaled incorrectly.
    if (x != 0 || y != 0 || w != GAME_W || h != GAME_H) {
        Serial.printf("ST77916 Output: unerwartete Bitmap %dx%d bei %d,%d\n",
                      w, h, x, y);
        return;
    }

    for (int16_t rowStart = 0; rowStart < LCD_H; rowStart += TRANSFER_ROWS) {
        const int16_t rowsThisTransfer =
            (rowStart + TRANSFER_ROWS <= LCD_H)
                ? TRANSFER_ROWS
                : (LCD_H - rowStart);

        for (int16_t row = 0; row < rowsThisTransfer; ++row) {
            const int16_t dstY = rowStart + row;

            // Map pixel centers so the final source row/column is also
            // included in the scaling.
            const int16_t srcY =
                static_cast<int16_t>(((2 * dstY + 1) * GAME_H) / (2 * LCD_H));
            const uint16_t *srcRow = bitmap + static_cast<int32_t>(srcY) * w;
            uint16_t *dstRow = dmaBuffer + static_cast<int32_t>(row) * LCD_W;

            for (int16_t dstX = 0; dstX < LCD_W; ++dstX) {
                const int16_t srcX =
                    static_cast<int16_t>(((2 * dstX + 1) * GAME_W) / (2 * LCD_W));
                dstRow[dstX] = srcRow[srcX];
            }
        }

        LCD_addWindow(
            0,
            static_cast<uint16_t>(rowStart),
            static_cast<uint16_t>(LCD_W - 1),
            static_cast<uint16_t>(rowStart + rowsThisTransfer - 1),
            dmaBuffer
        );

        // LCD_addWindow now waits for the actual DMA completion.
        // No fixed delay is needed after each stripe.
    }
}
