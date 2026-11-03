
#include <Arduino.h>
#include <Wire.h>

#include "pin_config.h"
#include "tca9554.h"

void scanI2C()
{
    Serial.println();
    Serial.println("I2C-Scan gestartet ...");

    uint8_t found = 0;

    for (uint8_t address = 1; address < 127; address++) {
        Wire.beginTransmission(address);

        if (Wire.endTransmission() == 0) {
            Serial.printf("I2C-Geraet gefunden: 0x%02X\n", address);
            found++;
        }
    }

    Serial.printf("I2C-Scan beendet. Geraete gefunden: %u\n", found);
}

void testResetPin(uint8_t exio, const char *name)
{
    bool level = false;

    Serial.println();
    Serial.printf("Teste %s (EXIO%u) ...\n", name, exio);

    if (!tca9554ReadPin(exio, level)) {
        Serial.println("FEHLER: Pin konnte nicht gelesen werden.");
        return;
    }

    Serial.printf("Ausgangszustand: %s\n", level ? "HIGH" : "LOW");

    Serial.println("Setze Pin auf LOW ...");

    if (!tca9554SetPin(exio, LOW)) {
        Serial.println("FEHLER beim Schreiben von LOW.");
        return;
    }

    delay(100);

    if (!tca9554ReadPin(exio, level)) {
        Serial.println("FEHLER beim Lesen nach LOW.");
        return;
    }

    Serial.printf("Rueckmeldung: %s\n", level ? "HIGH" : "LOW");

    Serial.println("Setze Pin auf HIGH ...");

    if (!tca9554SetPin(exio, HIGH)) {
        Serial.println("FEHLER beim Schreiben von HIGH.");
        return;
    }

    delay(100);

    if (!tca9554ReadPin(exio, level)) {
        Serial.println("FEHLER beim Lesen nach HIGH.");
        return;
    }

    Serial.printf("Rueckmeldung: %s\n", level ? "HIGH" : "LOW");

    if (level) {
        Serial.printf("ERGEBNIS: %s erfolgreich.\n", name);
    } else {
        Serial.printf("WARNUNG: %s konnte nicht bestaetigt werden.\n", name);
    }
}

void setup()
{
    Serial.begin(115200);
    delay(1500);

    Serial.println();
    Serial.println("================================");
    Serial.println(" TamaPoke - TCA9554 Hardwaretest");
    Serial.println(" Waveshare ESP32-S3 Touch LCD 1.85 Rev 2.0");
    Serial.println("================================");

    Serial.println();
    Serial.println("Starte I2C-Bus ...");

    Wire.begin(IIC_SDA, IIC_SCL);
    Wire.setTimeOut(50);

    delay(100);

    scanI2C();

    Serial.println();
    Serial.println("Initialisiere TCA9554 ...");

    if (!tca9554Begin(Wire)) {
        Serial.println();
        Serial.println("FEHLER: TCA9554 konnte nicht initialisiert werden.");
        Serial.println("Pruefe I2C-Adresse, Bus und Pinbelegung.");
        return;
    }

    Serial.println("TCA9554 erfolgreich initialisiert.");

    testResetPin(EXIO_TP_RESET, "Touch-Reset");
    testResetPin(EXIO_LCD_RESET, "LCD-Reset");

    Serial.println();
    Serial.println("================================");
    Serial.println(" TCA9554-Test abgeschlossen");
    Serial.println("================================");
}

void loop()
{
    delay(1000);
}
