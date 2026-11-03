
#include <Arduino.h>
#include <Wire.h>
#include <Arduino_GFX_Library.h>

#include "pin_config.h"
#include "tca9554.h"

// --------------------------------------------------
// Display: Waveshare ESP32-S3-Touch-LCD-1.85 Rev 2.0
// Controller: ST77916
// Aufloesung: 360 x 360
// Schnittstelle: QSPI
// --------------------------------------------------

Arduino_DataBus *bus = new Arduino_ESP32QSPI(
    LCD_CS,
    LCD_SCLK,
    LCD_SDIO0,
    LCD_SDIO1,
    LCD_SDIO2,
    LCD_SDIO3,
    false
);

Arduino_GFX *gfx = new Arduino_ST77916(
    bus,
    -1,       // Reset erfolgt ueber TCA9554 EXIO2
    0,        // Rotation
    true,     // IPS
    360,      // Breite
    360       // Hoehe
);

// RGB565-Farben
#define COLOR_BLACK   0x0000
#define COLOR_WHITE   0xFFFF
#define COLOR_RED     0xF800
#define COLOR_GREEN   0x07E0
#define COLOR_BLUE    0x001F
#define COLOR_YELLOW  0xFFE0
#define COLOR_CYAN    0x07FF
#define COLOR_MAGENTA 0xF81F

void showColorTest()
{
    Serial.println("Teste Vollbildfarben ...");

    const uint16_t colors[] = {
        COLOR_RED,
        COLOR_GREEN,
        COLOR_BLUE,
        COLOR_WHITE,
        COLOR_BLACK
    };

    const char *names[] = {
        "ROT",
        "GRUEN",
        "BLAU",
        "WEISS",
        "SCHWARZ"
    };

    for (int i = 0; i < 5; i++) {
        Serial.printf("Zeige Farbe: %s\n", names[i]);

        gfx->fillScreen(colors[i]);
        delay(1200);
    }
}

void showGraphicsTest()
{
    Serial.println("Teste Grafik und Text ...");

    gfx->fillScreen(COLOR_BLACK);

    // Rahmen
    gfx->drawRect(0, 0, 360, 360, COLOR_WHITE);
    gfx->drawRect(5, 5, 350, 350, COLOR_CYAN);

    // Farbige Balken
    gfx->fillRect(20, 25, 320, 35, COLOR_RED);
    gfx->fillRect(20, 65, 320, 35, COLOR_GREEN);
    gfx->fillRect(20, 105, 320, 35, COLOR_BLUE);

    // Kreis und Mittelpunkt
    gfx->drawCircle(180, 235, 75, COLOR_YELLOW);
    gfx->fillCircle(180, 235, 8, COLOR_MAGENTA);

    // Diagonalen
    gfx->drawLine(20, 150, 340, 320, COLOR_WHITE);
    gfx->drawLine(340, 150, 20, 320, COLOR_WHITE);

    // Text
    gfx->setTextColor(COLOR_WHITE);
    gfx->setTextSize(2);
    gfx->setCursor(75, 165);
    gfx->print("ST77916 OK");

    gfx->setTextSize(1);
    gfx->setCursor(115, 335);
    gfx->print("360 x 360 QSPI");
}

void setup()
{
    Serial.begin(115200);
    delay(1500);

    Serial.println();
    Serial.println("================================");
    Serial.println(" TamaPoke - ST77916 Display-Test");
    Serial.println(" Waveshare ESP32-S3 Touch LCD");
    Serial.println(" Rev 2.0 - 360 x 360");
    Serial.println("================================");

    // Display-Hintergrundbeleuchtung ausschalten
    pinMode(LCD_BL, OUTPUT);
    digitalWrite(LCD_BL, LOW);

    // I2C fuer den TCA9554 starten
    Serial.println("Starte I2C ...");

    Wire.begin(IIC_SDA, IIC_SCL);
    Wire.setTimeOut(50);

    delay(100);

    // TCA9554 initialisieren
    Serial.println("Initialisiere TCA9554 ...");

    if (!tca9554Begin(Wire)) {
        Serial.println("FEHLER: TCA9554 nicht erreichbar!");
        return;
    }

    Serial.println("TCA9554 OK");

    // LCD ueber EXIO2 zuruecksetzen
    Serial.println("Setze LCD zurueck ...");

    if (!tca9554PulseReset(EXIO_LCD_RESET, 20, 120)) {
        Serial.println("FEHLER: LCD-Reset fehlgeschlagen!");
        return;
    }

    Serial.println("LCD-Reset abgeschlossen");

    // Display initialisieren
    Serial.println("Initialisiere ST77916 ...");

    if (!gfx->begin()) {
        Serial.println("FEHLER: Display-Initialisierung fehlgeschlagen!");
        return;
    }

    Serial.println("ST77916 erfolgreich initialisiert");

    // Hintergrundbeleuchtung einschalten
    digitalWrite(LCD_BL, HIGH);
    delay(200);

    Serial.println("Display-Hintergrundbeleuchtung eingeschaltet");

    // Farbtest
    showColorTest();

    // Grafiktest
    showGraphicsTest();

    Serial.println();
    Serial.println("================================");
    Serial.println(" Display-Test abgeschlossen");
    Serial.println("================================");
}

void loop()
{
    delay(1000);
}
