/*
 * Time Setup UI implementation
 */

#include "time_setup.h"
#include "config.h"

bool TimeSetup::run(GxEPD2_BW<GxEPD2_290_BS, GxEPD2_290_BS::HEIGHT>& display,
                     RTCManager& rtc,
                     volatile int& encStepAccum,
                     portMUX_TYPE& encMux) {

  // Default values (2024-01-01 12:00)
  int year = 2024;
  int month = 1;
  int day = 1;
  int hour = 12;
  int minute = 0;

  Field currentField = FIELD_YEAR;
  bool needsRedraw = true;

  Serial.println(F("[TimeSetup] Starting interactive time setup"));

  while (true) {
    // Draw screen if needed
    if (needsRedraw) {
      drawSetupScreen(display, year, month, day, hour, minute, currentField);
      needsRedraw = false;
    }

    // Poll encoder
    encoderPcntPoll(true, &encStepAccum, &encMux);

    // Read rotation
    int steps = 0;
    portENTER_CRITICAL(&encMux);
    steps = encStepAccum;
    encStepAccum = 0;
    portEXIT_CRITICAL(&encMux);

    // Adjust selected field
    if (steps != 0) {
      switch (currentField) {
        case FIELD_YEAR:   year = clampValue(year + steps, 2024, 2099); break;
        case FIELD_MONTH:  month = clampValue(month + steps, 1, 12); break;
        case FIELD_DAY:    day = clampValue(day + steps, 1, 31); break;
        case FIELD_HOUR:   hour = clampValue(hour + steps, 0, 23); break;
        case FIELD_MINUTE: minute = clampValue(minute + steps, 0, 59); break;
        case FIELD_CONFIRM: break;  // No adjustment for confirm field
      }
      needsRedraw = true;
    }

    // Read button press
    if (digitalRead(ENC_SW_PIN) == LOW) {
      delay(50);  // Debounce
      while (digitalRead(ENC_SW_PIN) == LOW) delay(10);  // Wait for release

      if (currentField == FIELD_CONFIRM) {
        // Confirm and exit
        rtc.setTime(year, month, day, hour, minute, 0);
        Serial.println(F("[TimeSetup] Time confirmed and set"));
        return true;
      } else {
        // Move to next field
        currentField = (Field)((currentField + 1) % FIELD_COUNT);
        needsRedraw = true;
      }
    }

    delay(10);
  }
}

void TimeSetup::drawSetupScreen(GxEPD2_BW<GxEPD2_290_BS, GxEPD2_290_BS::HEIGHT>& display,
                                 int year, int month, int day, int hour, int minute,
                                 Field selectedField) {
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.setFont();

    // Title
    display.setCursor(90, 10);
    display.print("Set Time");

    // Date
    char dateBuf[32];
    snprintf(dateBuf, sizeof(dateBuf), "%04d-%02d-%02d", year, month, day);
    display.setCursor(70, 40);
    display.print(dateBuf);

    // Time
    char timeBuf[16];
    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", hour, minute);
    display.setCursor(100, 60);
    display.print(timeBuf);

    // Field indicators (underline selected field)
    int y = 80;
    display.setCursor(30, y);
    display.print("Year   Month  Day    Hour   Min");

    y += 15;
    int x = 30;
    if (selectedField == FIELD_YEAR) {
      display.fillRect(x, y, 30, 2, GxEPD_BLACK);
    }
    x += 42;
    if (selectedField == FIELD_MONTH) {
      display.fillRect(x, y, 30, 2, GxEPD_BLACK);
    }
    x += 42;
    if (selectedField == FIELD_DAY) {
      display.fillRect(x, y, 30, 2, GxEPD_BLACK);
    }
    x += 42;
    if (selectedField == FIELD_HOUR) {
      display.fillRect(x, y, 30, 2, GxEPD_BLACK);
    }
    x += 42;
    if (selectedField == FIELD_MINUTE) {
      display.fillRect(x, y, 30, 2, GxEPD_BLACK);
    }

    // Instructions
    display.setCursor(30, 110);
    display.print("Rotate: Adjust");
    display.setCursor(30, 120);
    display.print("Press: Next Field");

    // Confirm button
    int btnY = 105;
    if (selectedField == FIELD_CONFIRM) {
      display.fillRoundRect(190, btnY - 10, 80, 20, 3, GxEPD_BLACK);
      display.setTextColor(GxEPD_WHITE);
      display.setCursor(200, btnY);
      display.print("CONFIRM");
      display.setTextColor(GxEPD_BLACK);
    } else {
      display.drawRoundRect(190, btnY - 10, 80, 20, 3, GxEPD_BLACK);
      display.setCursor(200, btnY);
      display.print("CONFIRM");
    }

  } while (display.nextPage());
}

int TimeSetup::clampValue(int value, int min, int max) {
  if (value < min) return min;
  if (value > max) return max;
  return value;
}
