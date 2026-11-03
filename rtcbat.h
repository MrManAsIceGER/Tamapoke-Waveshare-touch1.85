
#pragma once
#include <Arduino.h>

// The PCF85063 RTC retains the time when a backup battery is connected.
bool rtcBegin();
uint32_t rtcEpoch();       // Unix seconds; 0 if the RTC time is invalid
void rtcSetEpoch(uint32_t e);

// Battery/charging functions:
// Battery voltage via BAT_ADC (GPIO8, 1:3 voltage divider).
// batPercent() is a rough estimate based on cell voltage.
// Software charging status is unavailable because ETA6098-STAT is not connected to an ESP32 GPIO.
bool batBegin();
void pmuEnablePanel();     // Not required on this board
void batSetPollMs(uint32_t ms);
int batPercent();          // -1 = no battery detected; otherwise a voltage-based estimate
bool batCharging();        // false: ETA6098-STAT is not available to the ESP32
int batMillivolts();       // Measured/scaled battery voltage; 0 if no battery is detected
bool batShouldShutdown();  // Automatic battery shutdown is not available
void batPowerOff();        // GPIO7 LOW disconnects battery power
bool usbPresent();         // Conservatively treat the power supply as present
bool usbPresentNow();

// Hardware PWR button on the Waveshare 1.85 Rev 2.0.
// Short press: display on/off. Long press (4 s): disconnect battery power.
enum PwrButtonEvent : uint8_t {
  PWR_EVENT_NONE = 0,
  PWR_EVENT_SHORT,
  PWR_EVENT_LONG
};
void pwrSetup();
PwrButtonEvent pwrPoll();
