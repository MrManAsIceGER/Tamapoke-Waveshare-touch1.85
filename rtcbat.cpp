
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

  // Plausibility check for the stored time
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

  // The RTC uses UTC.
  // This avoids daylight-saving-time changes.
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
// Battery and charging management
//
// Waveshare ESP32-S3-Touch-LCD-1.85 Rev 2.0:
// BAT_ADC is connected to GPIO8.
// 200 kOhm / 100 kOhm voltage divider: VBAT = VADC * 3.
// The ADC provides only a voltage-based state-of-charge estimate.
// ETA6098-STAT drives the separate charge LED and is not
// connected to the ESP32; software charging status is therefore unavailable.
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

// Approximate 1-cell Li-ion open-circuit curve. Under load and while charging,
// the displayed estimate may differ from the actual capacity.
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

  // Without a connected battery, the lower resistor pulls BAT_ADC
  // close to 0 V. The battery icon is then hidden as before.
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
  // Not required: the panel supply is not controlled by an AXP2101
  // or a separate PMU output.
}

void batSetPollMs(uint32_t ms) {
  // At least 250 ms between samples, so ADC reads never flood the render loop.
  batPollIntervalMs = (ms < 250) ? 250 : ms;
}

int batPercent() {
  batUpdateIfDue();
  return batLastPercent;
}

bool batCharging() {
  // ETA6098-STAT is connected only to the hardware charge LED.
  // A software charging icon would be guesswork without an additional signal wire.
  return false;
}

int batMillivolts() {
  batUpdateIfDue();
  return batLastPercent >= 0 ? batLastMv : 0;
}

bool batShouldShutdown() {
  // No automatic hard shutdown based on the estimated
  // cell voltage: a validated threshold and USB detection would be required.
  return false;
}

bool usbPresent() {
  // Conservative return value to prevent an unintended shutdown,
  // since the board does not provide separate software USB detection.
  return true;
}

bool usbPresentNow() {
  return true;
}

// ------------------------------------------------------------
// PWR button / battery power hold
//
// Waveshare ESP32-S3-Touch-LCD-1.85:
// GPIO6 = Key BAT (PWR button, active LOW)
// GPIO7 = BAT Control (HIGH keeps battery power enabled)
//
// Values are taken from the official board schematic. GPIO7 must
// be driven HIGH as early as possible so the board remains
// powered from the battery after startup via PWR.
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
  // Enable the power supply first, then configure the button.
  pinMode(BAT_CTRL_PIN, OUTPUT);
  digitalWrite(BAT_CTRL_PIN, HIGH);

  pinMode(PWR_KEY_PIN, INPUT_PULLUP);

  const bool pressed = (digitalRead(PWR_KEY_PIN) == LOW);
  pwrRawPressed = pressed;
  pwrStablePressed = pressed;
  pwrIgnoreNextRelease = pressed;  // Do not treat the power-on press as a game action
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

  // Detect long button presses while the button is still held.
  // A PWR button held during boot is still ignored.
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
