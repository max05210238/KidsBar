/*
 * Time Setup UI - Interactive time setting screen
 * Uses rotary encoder for navigation and setting
 */

#ifndef TIME_SETUP_H
#define TIME_SETUP_H

#include <Arduino.h>
#include <GxEPD2_BW.h>
#include "rtc_manager.h"
#include "encoder_pcnt.h"

class TimeSetup {
public:
  // Run interactive time setup and return true if time was set
  bool run(GxEPD2_BW<GxEPD2_290_BS, GxEPD2_290_BS::HEIGHT>& display,
           RTCManager& rtc,
           volatile int& encStepAccum,
           portMUX_TYPE& encMux);

private:
  enum Field {
    FIELD_YEAR,
    FIELD_MONTH,
    FIELD_DAY,
    FIELD_HOUR,
    FIELD_MINUTE,
    FIELD_CONFIRM,
    FIELD_COUNT
  };

  void drawSetupScreen(GxEPD2_BW<GxEPD2_290_BS, GxEPD2_290_BS::HEIGHT>& display,
                       int year, int month, int day, int hour, int minute,
                       Field selectedField);

  int clampValue(int value, int min, int max);
};

#endif // TIME_SETUP_H
