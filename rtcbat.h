
#pragma once
#include <Arduino.h>

// RTC PCF85063: Uhrzeit bleibt mit angeschlossener RTC-Batterie erhalten.
bool rtcBegin();
uint32_t rtcEpoch();       // Unix-Sekunden; 0 wenn die RTC-Zeit ungültig ist
void rtcSetEpoch(uint32_t e);

// Akku-/Ladefunktionen:
// Das Board besitzt keinen vom Originalcode unterstützten AXP2101.
// Die Funktionen bleiben aus Kompatibilitätsgründen erhalten.
bool batBegin();
void pmuEnablePanel();     // Auf diesem Board nicht erforderlich
void batSetPollMs(uint32_t ms);
int batPercent();          // -1 = keine Messung verfügbar
bool batCharging();        // Ladezustand nicht auslesbar
int batMillivolts();       // 0 = keine Spannungsmessung verfügbar
bool batShouldShutdown();  // automatische Akkuabschaltung nicht verfügbar
void batPowerOff();        // keine programmatische PMU-Abschaltung
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
