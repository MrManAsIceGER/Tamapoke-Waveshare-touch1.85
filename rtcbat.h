
#pragma once
#include <Arduino.h>

// RTC PCF85063: Uhrzeit bleibt mit angeschlossener RTC-Batterie erhalten.
bool rtcBegin();
uint32_t rtcEpoch();       // Unix-Sekunden; 0 wenn die RTC-Zeit ungültig ist
void rtcSetEpoch(uint32_t e);

// Akku-/Ladefunktionen:
// Batteriespannung ueber BAT_ADC (GPIO8, Spannungsteiler 1:3).
// batPercent() ist eine grobe Schaetzung aus der Zellspannung.
// Ein Software-Ladestatus ist mangels ETA6098-STAT-GPIO nicht verfuegbar.
bool batBegin();
void pmuEnablePanel();     // Auf diesem Board nicht erforderlich
void batSetPollMs(uint32_t ms);
int batPercent();          // -1 = kein Akku erkannt; sonst spannungsbasierte Schaetzung
bool batCharging();        // false: ETA6098-STAT ist nicht am ESP32 verfuegbar
int batMillivolts();       // gemessene/hochgerechnete Akkuspannung; 0 ohne Akku
bool batShouldShutdown();  // automatische Akkuabschaltung nicht verfügbar
void batPowerOff();        // GPIO7 LOW trennt die Batterie-Versorgung
bool usbPresent();         // konservativ: Versorgung als vorhanden behandeln
bool usbPresentNow();

// Hardware-Power-Taste des Waveshare 1.85 Rev 2.0.
// Kurz: Display ein/aus. Lang (4 s): Batterie-Versorgung abschalten.
enum PwrButtonEvent : uint8_t {
  PWR_EVENT_NONE = 0,
  PWR_EVENT_SHORT,
  PWR_EVENT_LONG
};
void pwrSetup();
PwrButtonEvent pwrPoll();
