#include <Arduino.h>
#include <Wire.h>

#include "tca9554.h"
#include "Display_ST77916.h"

static constexpr uint8_t I2C_SDA_PIN = 11;
static constexpr uint8_t I2C_SCL_PIN = 10;
static constexpr int LCD_W = 360;
static constexpr int LCD_H = 360;

// Zwei Bildzeilen = 360 * 2 * 2 Byte = 1.440 Byte.
// Damit bleibt jeder Farbdaten-Transfer unter 2.048 Byte.
static uint16_t lineBuffer[LCD_W * 2];

static void drawSolidColor(uint16_t color)
{
    for (int y = 0; y < LCD_H; y += 2) {
        for (int i = 0; i < LCD_W * 2; ++i) {
            lineBuffer[i] = color;
        }
        LCD_addWindow(0, y, LCD_W - 1, y + 1, lineBuffer);
        delay(5); // DMA-Puffer nicht ueberschreiben, bevor die Uebertragung beendet ist.
    }
}

static void drawColorBars()
{
    const uint16_t colors[6] = {
        0xF800, // Rot
        0x07E0, // Gruen
        0x001F, // Blau
        0xFFE0, // Gelb
        0x07FF, // Cyan
        0xF81F  // Magenta
    };

    for (int y = 0; y < LCD_H; y += 2) {
        const uint16_t color = colors[(y / 60) % 6];
        for (int i = 0; i < LCD_W * 2; ++i) {
            lineBuffer[i] = color;
        }
        LCD_addWindow(0, y, LCD_W - 1, y + 1, lineBuffer);
        delay(5);
    }
}

static void drawCheckerboard()
{
    for (int y = 0; y < LCD_H; y += 2) {
        for (int row = 0; row < 2; ++row) {
            const int yy = y + row;
            for (int x = 0; x < LCD_W; ++x) {
                const bool white = (((x / 30) + (yy / 30)) & 1) != 0;
                lineBuffer[row * LCD_W + x] = white ? 0xFFFF : 0x0000;
            }
        }
        LCD_addWindow(0, y, LCD_W - 1, y + 1, lineBuffer);
        delay(5);
    }
}

void setup()
{
    Serial.begin(115200);
    delay(1500);

    Serial.println();
    Serial.println("========================================");
    Serial.println("Waveshare ST77916 QSPI Test - Rev 2.0");
    Serial.println("ESP32-S3 / 360x360 / Arduino Core 3.x");
    Serial.println("========================================");

    Serial.println("Starte I2C auf SDA=11, SCL=10 ...");
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    Wire.setTimeOut(50);
    delay(100);

    Serial.println("Initialisiere TCA9554 (nur EXIO1/EXIO2 als Ausgaenge) ...");
    if (!tca9554Begin(Wire)) {
        Serial.println("FEHLER: TCA9554 bei Adresse 0x20 nicht initialisierbar.");
        return;
    }
    Serial.println("TCA9554 OK");

    Serial.println("Initialisiere Display-Backlight PWM ...");
    Backlight_Init();

    Serial.println("Initialisiere originalen Waveshare-ST77916-Treiber ...");
    LCD_Init();
    delay(500);

    if (panel_handle == nullptr) {
        Serial.println("FEHLER: ST77916-Panelhandle ist NULL. QSPI-Initialisierung fehlgeschlagen.");
        return;
    }

    Serial.println("ST77916-Panelhandle vorhanden. Starte Farbtest.");

    const uint16_t colors[] = {0xF800, 0x07E0, 0x001F, 0xFFFF, 0x0000};
    const char *names[] = {"ROT", "GRUEN", "BLAU", "WEISS", "SCHWARZ"};

    for (size_t i = 0; i < sizeof(colors) / sizeof(colors[0]); ++i) {
        Serial.printf("Vollbild: %s\n", names[i]);
        drawSolidColor(colors[i]);
        delay(1000);
    }

    Serial.println("Zeige sechs Farbbalken ...");
    drawColorBars();
    delay(1500);

    Serial.println("Zeige schwarz-weisses Schachbrett ...");
    drawCheckerboard();

    Serial.println("DISPLAY-TEST ABGESCHLOSSEN");
}

void loop()
{
    delay(1000);
}
