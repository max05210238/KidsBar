// KidsBar - Hardware Configuration
// GPIO pin definitions for ESP32-S3 DevKit
#pragma once

#include <Arduino.h>

// ==================== Hardware Pin Assignments ====================

// E-ink Display Pins (2.9" E-paper)
#define EPD_BUSY 4      // Busy signal input
#define EPD_RST 16      // Reset pin
#define EPD_DC 17       // Data/Command select
#define EPD_CS 10       // SPI Chip Select
#define EPD_SCK 12      // SPI Clock
#define EPD_MOSI 11     // SPI Data Out

// Rotary Encoder Pins
// Note: GPIO 1/2 are PCNT-compatible on ESP32-S3
// DO NOT use GPIO 5/6 (reserved for SPI Flash on ESP32-S3)
#define ENC_CLK_PIN 2   // Encoder CLK (Phase A) - PCNT compatible
#define ENC_DT_PIN 1    // Encoder DT (Phase B) - PCNT compatible
#define ENC_SW_PIN 21   // Encoder push button (active low)

// WS2812B RGB LED Pins
#define NEOPIXEL_PIN 15      // External WS2812B data pin
#define NEOPIXEL_COUNT 1     // Number of LEDs

#define BOARD_RGB_PIN 48     // Onboard WS2812B (ESP32-S3 DevKit)
#define BOARD_RGB_COUNT 1    // Onboard LED count

// ==================== Application Settings ====================

// Virtual Pet Update Intervals
#define PET_UPDATE_INTERVAL_MS (60000UL)     // Update pet state every minute
#define ANIMATION_FRAME_MS (500UL)           // Animation frame delay
#define IDLE_SLEEP_TIMEOUT_MS (300000UL)     // Sleep after 5 minutes idle

// Display Settings
#define SCREEN_WIDTH 296
#define SCREEN_HEIGHT 128

// NVS Storage Keys
#define NVS_NAMESPACE "kidsbar"
#define NVS_KEY_PET_HUNGER "hunger"
#define NVS_KEY_PET_HAPPINESS "happiness"
#define NVS_KEY_PET_HEALTH "health"
#define NVS_KEY_BRIGHTNESS "brightness"
