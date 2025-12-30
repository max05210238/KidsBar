/*
 * RTC Manager implementation
 */

#include "rtc_manager.h"

void RTCManager::begin() {
  prefs.begin("rtc", false);

  // Check if time was previously set
  if (isTimeSet()) {
    Serial.println(F("[RTC] Time was previously set"));
  } else {
    Serial.println(F("[RTC] Time not set - needs initialization"));
  }
}

bool RTCManager::isTimeSet() {
  uint32_t magic = prefs.getUInt("magic", 0);
  if (magic != TIME_SET_MAGIC) {
    return false;
  }

  // Also check if the year is reasonable (2024-2099)
  struct tm timeinfo;
  time_t now = time(nullptr);
  localtime_r(&now, &timeinfo);
  int year = timeinfo.tm_year + 1900;

  return (year >= 2024 && year < 2100);
}

void RTCManager::setTime(int year, int month, int day, int hour, int minute, int second) {
  struct tm timeinfo = {0};
  timeinfo.tm_year = year - 1900;
  timeinfo.tm_mon = month - 1;      // 0-11
  timeinfo.tm_mday = day;           // 1-31
  timeinfo.tm_hour = hour;          // 0-23
  timeinfo.tm_min = minute;         // 0-59
  timeinfo.tm_sec = second;         // 0-59

  time_t t = mktime(&timeinfo);
  struct timeval tv = { .tv_sec = t, .tv_usec = 0 };
  settimeofday(&tv, NULL);

  // Save magic to indicate time has been set
  prefs.putUInt("magic", TIME_SET_MAGIC);

  Serial.printf("[RTC] Time set to: %04d-%02d-%02d %02d:%02d:%02d\n",
                year, month, day, hour, minute, second);
}

void RTCManager::getTime(int &year, int &month, int &day, int &hour, int &minute, int &second) {
  struct tm timeinfo;
  time_t now = time(nullptr);
  localtime_r(&now, &timeinfo);

  year = timeinfo.tm_year + 1900;
  month = timeinfo.tm_mon + 1;
  day = timeinfo.tm_mday;
  hour = timeinfo.tm_hour;
  minute = timeinfo.tm_min;
  second = timeinfo.tm_sec;
}

void RTCManager::getTime(int &hour, int &minute, int &second) {
  struct tm timeinfo;
  time_t now = time(nullptr);
  localtime_r(&now, &timeinfo);

  hour = timeinfo.tm_hour;
  minute = timeinfo.tm_min;
  second = timeinfo.tm_sec;
}

uint64_t RTCManager::getTimestampMs() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (uint64_t)tv.tv_sec * 1000ULL + (uint64_t)tv.tv_usec / 1000ULL;
}

time_t RTCManager::getTimestampSec() {
  return time(nullptr);
}

String RTCManager::getTimeString() {
  int h, m, s;
  getTime(h, m, s);
  char buf[16];
  snprintf(buf, sizeof(buf), "%02d:%02d:%02d", h, m, s);
  return String(buf);
}

String RTCManager::getDateString() {
  int y, mon, d, h, m, s;
  getTime(y, mon, d, h, m, s);
  char buf[16];
  snprintf(buf, sizeof(buf), "%04d-%02d-%02d", y, mon, d);
  return String(buf);
}

String RTCManager::getDateTimeString() {
  int y, mon, d, h, m, s;
  getTime(y, mon, d, h, m, s);
  char buf[32];
  snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d", y, mon, d, h, m, s);
  return String(buf);
}
