
#include "rtcbat.h"
#include "pin_config.h"

#include <Wire.h>
#include <time.h>
#include <SensorPCF85063.hpp>

// ------------------------------------------------------------
// RTC PCF85063
// ------------------------------------------------------------

static SensorPCF85063 rtc;
static bool rtcOk = false;

bool rtcBegin() {
  rtcOk = rtc.begin(Wire, IIC_SDA, IIC_SCL);

  if (!rtcOk) {
    Serial.println("PCF85063 nicht erkannt");
  } else {
    Serial.println("PCF85063 RTC bereit");
  }

  return rtcOk;
}

uint32_t rtcEpoch() {
  if (!rtcOk) return 0;

  RTC_DateTime t = rtc.getDateTime();

  // Plausibilitätsprüfung der gespeicherten Uhrzeit
  if (t.getYear() < 2025 || t.getYear() > 2120) {
    return 0;
  }

  struct tm tmv = {};

  tmv.tm_year = t.getYear() - 1900;
  tmv.tm_mon  = t.getMonth() - 1;
  tmv.tm_mday = t.getDay();
  tmv.tm_hour = t.getHour();
  tmv.tm_min  = t.getMinute();
  tmv.tm_sec  = t.getSecond();

  // Die RTC wird in UTC geführt.
  // Das vermeidet Abweichungen durch Sommer-/Winterzeit.
  time_t e = mktime(&tmv);

  return e > 0 ? (uint32_t)e : 0;
}

void rtcSetEpoch(uint32_t e) {
  if (!rtcOk) return;

  time_t tt = e;
  struct tm tmv;

  gmtime_r(&tt, &tmv);

  rtc.setDateTime(
    RTC_DateTime(
      tmv.tm_year + 1900,
      tmv.tm_mon + 1,
      tmv.tm_mday,
      tmv.tm_hour,
      tmv.tm_min,
      tmv.tm_sec
    )
  );
}

// ------------------------------------------------------------
// Akku- und Ladeverwaltung
//
// Das Original verwendet einen AXP2101.
// Dieser ist auf dem ESP32-S3-Touch-LCD-1.85 Rev 2.0
// nicht vorhanden.
//
// Deshalb keine AXP2101-I2C-Zugriffe und keine erfundenen
// Akku-Spannungs- oder Ladezustandswerte.
// ------------------------------------------------------------

bool batBegin() {
  Serial.println(
    "Akku-Monitor: keine kompatible Messhardware vorhanden"
  );

  return false;
}

void pmuEnablePanel() {
  // Nicht benötigt:
  // Die Displayversorgung wird auf diesem Board nicht
  // über den BLDO1-Ausgang eines AXP2101 geschaltet.
}

void batSetPollMs(uint32_t ms) {
  // Keine Akku-Messung vorhanden.
  (void)ms;
}

int batPercent() {
  return -1;
}

bool batCharging() {
  return false;
}

int batMillivolts() {
  return 0;
}

bool batShouldShutdown() {
  // Ohne zuverlässige Spannungsmessung darf das Spiel
  // nicht aufgrund eines erfundenen Akkuwertes abschalten.
  return false;
}

void batPowerOff() {
  // Keine programmatische PMU-Abschaltung verfügbar.
}

bool usbPresent() {
  // Konservativer Rückgabewert:
  // Verhindert, dass die vorhandene Spiel-Logik wegen
  // eines nicht messbaren USB-Zustands abschaltet.
  return true;
}

bool usbPresentNow() {
  return true;
}

// ------------------------------------------------------------
// Power-Taste
//
// Die AXP2101-Power-Key-IRQ-Funktionen des Originalboards
// stehen hier nicht zur Verfügung.
// ------------------------------------------------------------

void pwrSetup() {
  // Keine AXP2101-Power-Key-Konfiguration.
}

bool pwrShortPressed() {
  return false;
}
