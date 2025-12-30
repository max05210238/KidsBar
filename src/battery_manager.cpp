/*
 * Battery Manager implementation
 */

#include "battery_manager.h"

void BatteryManager::begin(uint8_t adcPin, float dividerRatio) {
  _adcPin = adcPin;
  _dividerRatio = dividerRatio;

  // Configure ADC
  analogReadResolution(12);  // 12-bit ADC (0-4095)
  analogSetAttenuation(ADC_11db);  // 0-3.3V range

  Serial.printf("[Battery] Initialized on pin %d, divider ratio: %.1f\n", adcPin, dividerRatio);
}

float BatteryManager::readSmoothedVoltage() {
  float sum = 0;
  for (int i = 0; i < SAMPLES; i++) {
    int adc = analogRead(_adcPin);
    float voltage = (adc / 4095.0) * 3.3 * _dividerRatio;
    sum += voltage;
    delay(1);
  }
  return sum / SAMPLES;
}

float BatteryManager::getVoltage() {
  return readSmoothedVoltage();
}

int BatteryManager::getPercentage() {
  float voltage = getVoltage();

  // Clamp voltage to valid range
  if (voltage >= VOLTAGE_MAX) return 100;
  if (voltage <= VOLTAGE_MIN) return 0;

  // Linear mapping (simple but works)
  float percentage = (voltage - VOLTAGE_MIN) / (VOLTAGE_MAX - VOLTAGE_MIN) * 100.0;

  // More accurate LiPo discharge curve (optional, uncomment for better accuracy)
  /*
  if (voltage > 4.1) percentage = 90 + (voltage - 4.1) / 0.1 * 10;
  else if (voltage > 4.0) percentage = 70 + (voltage - 4.0) / 0.1 * 20;
  else if (voltage > 3.9) percentage = 50 + (voltage - 3.9) / 0.1 * 20;
  else if (voltage > 3.8) percentage = 30 + (voltage - 3.8) / 0.1 * 20;
  else if (voltage > 3.7) percentage = 15 + (voltage - 3.7) / 0.1 * 15;
  else if (voltage > 3.5) percentage = 5 + (voltage - 3.5) / 0.2 * 10;
  else percentage = (voltage - 3.0) / 0.5 * 5;
  */

  return (int)percentage;
}

bool BatteryManager::isCharging() {
  // TODO: Implement if charge detect pin is available
  return false;
}

String BatteryManager::getStatusString() {
  float voltage = getVoltage();
  int percentage = getPercentage();

  char buf[32];
  snprintf(buf, sizeof(buf), "%.2fV (%d%%)", voltage, percentage);
  return String(buf);
}
