/*
 * Battery Manager - ADC-based battery voltage and percentage monitoring
 * Uses voltage divider circuit to measure LiPo battery voltage
 */

#ifndef BATTERY_MANAGER_H
#define BATTERY_MANAGER_H

#include <Arduino.h>

class BatteryManager {
public:
  // Initialize with ADC pin and voltage divider ratio
  void begin(uint8_t adcPin = 34, float dividerRatio = 2.0);

  // Get battery voltage (V)
  float getVoltage();

  // Get battery percentage (0-100%)
  int getPercentage();

  // Check if battery is charging (if charge detect pin is configured)
  bool isCharging();

  // Get battery status string
  String getStatusString();

private:
  uint8_t _adcPin;
  float _dividerRatio;

  // LiPo voltage curve (3.0V = 0%, 4.2V = 100%)
  static constexpr float VOLTAGE_MIN = 3.0;
  static constexpr float VOLTAGE_MAX = 4.2;

  // Smoothing
  static const int SAMPLES = 10;
  float readSmoothedVoltage();
};

#endif // BATTERY_MANAGER_H
