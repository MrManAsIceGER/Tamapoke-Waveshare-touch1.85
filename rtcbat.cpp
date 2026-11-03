
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
// PWR-Taste / Batterie-Haltefunktion
//
// Waveshare ESP32-S3-Touch-LCD-1.85:
//   GPIO6 = Key BAT (PWR-Taste, aktiv LOW)
//   GPIO7 = BAT Control (HIGH haelt die Batterie-Versorgung aktiv)
//
// Die Werte stammen aus dem offiziellen Board-Schaltplan. GPIO7 muss
// moeglichst frueh auf HIGH gesetzt werden, damit das Board nach dem
// Einschalten ueber PWR weiter mit Batteriespannung versorgt bleibt.
// ------------------------------------------------------------

static bool pwrReady = false;
static bool pwrRawPressed = false;
static bool pwrStablePressed = false;
static bool pwrIgnoreNextRelease = false;
static bool pwrLongReported = false;
static bool pwrShortPending = false;
static bool pwrLongPending = false;
static uint32_t pwrLastChangeMs = 0;
static uint32_t pwrPressedAtMs = 0;

static constexpr uint32_t PWR_DEBOUNCE_MS = 35;
static constexpr uint32_t PWR_SHORT_MIN_MS = 60;
static constexpr uint32_t PWR_LONG_MS = 4000;

void pwrSetup() {
  // Erst die Stromversorgung halten, danach den Taster konfigurieren.
  pinMode(BAT_CTRL_PIN, OUTPUT);
  digitalWrite(BAT_CTRL_PIN, HIGH);

  pinMode(PWR_KEY_PIN, INPUT_PULLUP);

  const bool pressed = (digitalRead(PWR_KEY_PIN) == LOW);
  pwrRawPressed = pressed;
  pwrStablePressed = pressed;
  pwrIgnoreNextRelease = pressed;  // Einschalt-Druck nicht als Spielaktion werten
  pwrLongReported = false;
  pwrShortPending = false;
  pwrLongPending = false;
  pwrLastChangeMs = millis();
  pwrPressedAtMs = pwrLastChangeMs;
  pwrReady = true;
}

static void pwrUpdateButton() {
  if (!pwrReady) return;

  const uint32_t now = millis();
  const bool rawPressed = (digitalRead(PWR_KEY_PIN) == LOW);

  if (rawPressed != pwrRawPressed) {
    pwrRawPressed = rawPressed;
    pwrLastChangeMs = now;
    return;
  }

  if (now - pwrLastChangeMs < PWR_DEBOUNCE_MS) return;

  if (rawPressed != pwrStablePressed) {
    pwrStablePressed = rawPressed;

    if (pwrStablePressed) {
      pwrPressedAtMs = now;
      pwrLongReported = false;
      Serial.println("[PWR] Taste gedrueckt");
    } else {
      const uint32_t duration = now - pwrPressedAtMs;
      Serial.printf("[PWR] Taste losgelassen nach %lu ms\n",
                    (unsigned long)duration);

      if (pwrIgnoreNextRelease) {
        pwrIgnoreNextRelease = false;
      } else if (duration >= PWR_LONG_MS) {
        if (!pwrLongReported) pwrLongPending = true;
            } else if (duration >= PWR_SHORT_MIN_MS &&
                 duration < PWR_LONG_MS) {
        pwrShortPending = true;
      }
    }
  }

  // Lange Tastendruecke bereits waehrend des Haltens erkennen.
  // Der Sonderfall eines beim Booten gehaltenen PWR-Tasters bleibt ignoriert.
  if (pwrStablePressed && !pwrIgnoreNextRelease && !pwrLongReported &&
      now - pwrPressedAtMs >= PWR_LONG_MS) {
    pwrLongReported = true;
    pwrLongPending = true;
  }
}

PwrButtonEvent pwrPoll() {
  pwrUpdateButton();

  if (pwrLongPending) {
    pwrLongPending = false;
    Serial.println("[PWR] langer Tastendruck");
    return PWR_EVENT_LONG;
  }

  if (pwrShortPending) {
    pwrShortPending = false;
    Serial.println("[PWR] kurzer Tastendruck");
    return PWR_EVENT_SHORT;
  }

  return PWR_EVENT_NONE;
}

void batPowerOff() {
  Serial.println("[PWR] BAT Control LOW - Batterie-Versorgung wird getrennt");
  digitalWrite(BAT_CTRL_PIN, LOW);
  delay(100);
}
