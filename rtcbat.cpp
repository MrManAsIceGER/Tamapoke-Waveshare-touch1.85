
#include "rtcbat.h"
#include "pin_config.h"

#include <Wire.h>
#include <time.h>
#include <RtcDrv.hpp>

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
// Waveshare ESP32-S3-Touch-LCD-1.85 Rev 2.0:
//   BAT_ADC liegt auf GPIO8.
//   200 kOhm / 100 kOhm Spannungsteiler: VBAT = VADC * 3.
//   Der ADC liefert nur eine spannungsbasierte SOC-Schaetzung.
//   ETA6098-STAT treibt die separate Lade-LED und ist nicht am ESP32
//   angeschlossen; der Software-Ladezustand bleibt daher unbekannt.
// ------------------------------------------------------------

static bool batReady = false;
static bool batHasSample = false;
static uint32_t batLastSampleMs = 0;
static uint32_t batPollIntervalMs = 2000;
static uint16_t batLastMv = 0;
static int batLastPercent = -1;

static constexpr uint8_t BAT_ADC_SAMPLES = 8;
static constexpr uint16_t BAT_PRESENT_MIN_MV = 1500;
static constexpr uint16_t BAT_ADC_DIVIDER = 3;

struct BatteryCurvePoint {
  uint16_t millivolts;
  uint8_t percent;
};

// Grobe 1S-Li-Ion-Leerlaufkurve. Unter Last und waehrend des Ladens
// kann die angezeigte Schaetzung von der tatsaechlichen Kapazitaet abweichen.
static const BatteryCurvePoint BATTERY_CURVE[] = {
  { 3300,   0 },
  { 3500,   5 },
  { 3600,  10 },
  { 3700,  20 },
  { 3750,  30 },
  { 3800,  40 },
  { 3850,  50 },
  { 3900,  60 },
  { 4000,  75 },
  { 4100,  90 },
  { 4200, 100 }
};

static int batEstimatePercent(uint16_t millivolts) {
  constexpr size_t POINT_COUNT =
      sizeof(BATTERY_CURVE) / sizeof(BATTERY_CURVE[0]);

  if (millivolts <= BATTERY_CURVE[0].millivolts) return 0;
  if (millivolts >= BATTERY_CURVE[POINT_COUNT - 1].millivolts) return 100;

  for (size_t i = 1; i < POINT_COUNT; ++i) {
    const BatteryCurvePoint &lo = BATTERY_CURVE[i - 1];
    const BatteryCurvePoint &hi = BATTERY_CURVE[i];

    if (millivolts <= hi.millivolts) {
      const uint32_t spanMv = hi.millivolts - lo.millivolts;
      const uint32_t offsetMv = millivolts - lo.millivolts;
      const uint32_t spanPct = hi.percent - lo.percent;
      return lo.percent + (int)((offsetMv * spanPct + spanMv / 2) / spanMv);
    }
  }

  return 100;
}

static void batSampleVoltage() {
  uint32_t adcMvSum = 0;

  for (uint8_t i = 0; i < BAT_ADC_SAMPLES; ++i) {
    adcMvSum += analogReadMilliVolts(BAT_ADC_PIN);
  }

  const uint32_t adcMv = (adcMvSum + BAT_ADC_SAMPLES / 2) / BAT_ADC_SAMPLES;
  const uint32_t batteryMv = adcMv * BAT_ADC_DIVIDER;

  batLastSampleMs = millis();
  batHasSample = true;

  // Ohne angeschlossenen Akku liegt BAT_ADC durch den unteren Widerstand
  // nahe 0 V. Dann wird das Batteriesymbol wie bisher ausgeblendet.
  if (batteryMv < BAT_PRESENT_MIN_MV) {
    batLastMv = 0;
    batLastPercent = -1;
    return;
  }

  batLastMv = batteryMv > UINT16_MAX ? UINT16_MAX : (uint16_t)batteryMv;
  batLastPercent = batEstimatePercent(batLastMv);
}

static void batUpdateIfDue() {
  if (!batReady) return;

  const uint32_t now = millis();
  if (!batHasSample || now - batLastSampleMs >= batPollIntervalMs) {
    batSampleVoltage();
  }
}

bool batBegin() {
  pinMode(BAT_ADC_PIN, INPUT);
  analogReadResolution(12);
  analogSetPinAttenuation(BAT_ADC_PIN, ADC_11db);

  batReady = true;
  batHasSample = false;
  batSampleVoltage();

  Serial.printf("[BAT] BAT_ADC GPIO%d bereit: %d mV, %d%%\n",
                BAT_ADC_PIN, batLastMv, batLastPercent);
  return true;
}

void pmuEnablePanel() {
  // Nicht benoetigt: die Panelversorgung wird nicht ueber einen AXP2101
  // oder einen separaten PMU-Ausgang geschaltet.
}

void batSetPollMs(uint32_t ms) {
  // Mindestens 250 ms Abstand, damit ADC-Messungen nie den Renderloop fluten.
  batPollIntervalMs = (ms < 250) ? 250 : ms;
}

int batPercent() {
  batUpdateIfDue();
  return batLastPercent;
}

bool batCharging() {
  // ETA6098-STAT ist nur mit der Hardware-Lade-LED verbunden.
  // Ein Software-Ladesymbol waere ohne zusaetzliche Leitung geraten.
  return false;
}

int batMillivolts() {
  batUpdateIfDue();
  return batLastPercent >= 0 ? batLastMv : 0;
}

bool batShouldShutdown() {
  // Keine automatische harte Abschaltung auf Basis der nur geschaetzten
  // Zellspannung: dafuer waeren eine validierte Schwelle und USB-Erkennung noetig.
  return false;
}

bool usbPresent() {
  // Konservativer Rueckgabewert: verhindert eine unbeabsichtigte Abschaltung,
  // solange das Board keine separate Software-USB-Erkennung bereitstellt.
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
