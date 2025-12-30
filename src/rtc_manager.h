/*
 * RTC Manager - ESP32 internal RTC time management
 * Handles time setting, persistence, and retrieval
 */

#ifndef RTC_MANAGER_H
#define RTC_MANAGER_H

#include <Arduino.h>
#include <Preferences.h>
#include <sys/time.h>

class RTCManager {
public:
  void begin();

  // Check if time has been set
  bool isTimeSet();

  // Set time manually (year, month, day, hour, minute, second)
  void setTime(int year, int month, int day, int hour, int minute, int second);

  // Get current time
  void getTime(int &year, int &month, int &day, int &hour, int &minute, int &second);
  void getTime(int &hour, int &minute, int &second);

  // Get timestamp in milliseconds (for Tamagotchi)
  uint64_t getTimestampMs();

  // Get timestamp in seconds (Unix epoch)
  time_t getTimestampSec();

  // Format time as string
  String getTimeString();      // "14:32:05"
  String getDateString();      // "2024-12-30"
  String getDateTimeString();  // "2024-12-30 14:32:05"

private:
  Preferences prefs;
  static const uint32_t TIME_SET_MAGIC = 0x54494D45; // "TIME"
};

#endif // RTC_MANAGER_H
