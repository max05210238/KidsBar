/*
 * KidsBar Tamagotchi
 * Clean rewrite based on ArduinoGotchi + TamaLib
 *
 * Hardware: ESP32-S3 + E-ink 2.9" + Rotary Encoder
 */

#include <Arduino.h>
#include <GxEPD2_BW.h>
#include <Preferences.h>

#include "config.h"
#include "encoder_pcnt.h"
#include "led_status.h"
#include "bitmaps.h"

extern "C" {
  #include "tamalib.h"
  #include "hw.h"
  #include "hal.h"
}

#include "savestate.h"

// ==================== HARDWARE ====================

GxEPD2_BW<GxEPD2_290_BS, GxEPD2_290_BS::HEIGHT> display(
  GxEPD2_290_BS(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY)
);

// ==================== STATE ====================

// Tamagotchi state
static cpu_state_t g_cpu_state;
static bool_t g_matrix[LCD_HEIGHT][LCD_WIDTH/8] = {{0}};
static bool_t g_icons[ICON_NUM] = {0};

// Encoder input state
volatile int g_encStepAccum = 0;
portMUX_TYPE g_encMux = portMUX_INITIALIZER_UNLOCKED;

// Button state - use int to allow -1 for "none"
static int g_currentBtn = -1;  // -1 = none, 0 = BTN_LEFT, 1 = BTN_MIDDLE, 2 = BTN_RIGHT
static unsigned long g_lastBtnTime = 0;

// Auto-save
static unsigned long g_lastSave = 0;
#define AUTO_SAVE_INTERVAL_MS (5 * 60 * 1000UL)

// ==================== HAL IMPLEMENTATION ====================

static void hal_halt(void) {
  Serial.println(F("HALT"));
}

static void hal_log(log_level_t level, char *msg, ...) {
  // Silent
}

static void hal_sleep_until(timestamp_t ts) {
  // Not used (CPU_SPEED_RATIO = 0)
}

static timestamp_t hal_get_timestamp(void) {
  // Return milliseconds - matches ts_freq=1000 passed to tamalib_init()
  return millis();
}

static void hal_set_lcd_matrix(u8_t x, u8_t y, bool_t val) {
  // Buffer pixel changes - will be rendered on next update_screen()
  if (x >= LCD_WIDTH || y >= LCD_HEIGHT) return;

  uint8_t byte_idx = x / 8;
  uint8_t bit_idx = x % 8;
  uint8_t mask = 0x80 >> bit_idx;

  if (val) {
    g_matrix[y][byte_idx] |= mask;
  } else {
    g_matrix[y][byte_idx] &= ~mask;
  }
}

static void hal_set_lcd_icon(u8_t icon, bool_t val) {
  if (icon < ICON_NUM) {
    g_icons[icon] = val;
  }
}

static void hal_set_frequency(u32_t freq) {
  // No sound yet
}

static void hal_play_frequency(bool_t en) {
  // No sound yet
}

static int hal_handler(void) {
  // Update button state for TamaLib
  hw_set_button(BTN_LEFT,   g_currentBtn == (int)BTN_LEFT   ? BTN_STATE_PRESSED : BTN_STATE_RELEASED);
  hw_set_button(BTN_MIDDLE, g_currentBtn == (int)BTN_MIDDLE ? BTN_STATE_PRESSED : BTN_STATE_RELEASED);
  hw_set_button(BTN_RIGHT,  g_currentBtn == (int)BTN_RIGHT  ? BTN_STATE_PRESSED : BTN_STATE_RELEASED);

  return 0; // Continue
}

static void hal_update_screen(void) {
  // Actually update the E-ink display
  // This is called by TamaLib when enough time has elapsed (based on framerate)
  static uint32_t update_count = 0;

  // Count pixels in buffer
  uint16_t pixel_count = 0;
  for (uint8_t y = 0; y < LCD_HEIGHT; y++) {
    for (uint8_t x = 0; x < LCD_WIDTH/8; x++) {
      uint8_t byte_val = g_matrix[y][x];
      for (uint8_t bit = 0; bit < 8; bit++) {
        if (byte_val & (0x80 >> bit)) pixel_count++;
      }
    }
  }

  Serial.printf("Screen update #%u, pixels=%u, icons=[", ++update_count, pixel_count);
  for (uint8_t i = 0; i < 8; i++) {
    Serial.printf("%d", g_icons[i] ? 1 : 0);
  }
  Serial.println("]");

  display.setPartialWindow(0, 0, display.width(), display.height());
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);

    // Draw Tamagotchi LCD (32x16 pixels, scaled 3x = 96x48)
    for (uint8_t y = 0; y < LCD_HEIGHT; y++) {
      for (uint8_t x = 0; x < LCD_WIDTH; x++) {
        uint8_t byte_idx = x / 8;
        uint8_t bit_idx = x % 8;
        uint8_t mask = 0x80 >> bit_idx;

        if (g_matrix[y][byte_idx] & mask) {
          int16_t screen_x = 16 + (x * 3);
          int16_t screen_y = 20 + (y * 3);
          display.fillRect(screen_x, screen_y, 3, 3, GxEPD_BLACK);
        }
      }
    }

    // Draw icon menu at bottom
    for (uint8_t i = 0; i < 8; i++) {
      int16_t icon_x = i * 16 + 4;
      int16_t icon_y = 90;

      // Selection triangle
      if (g_icons[i]) {
        display.drawLine(icon_x + 6, icon_y + 1, icon_x + 10, icon_y + 1, GxEPD_BLACK);
        display.drawLine(icon_x + 7, icon_y + 2, icon_x + 9,  icon_y + 2, GxEPD_BLACK);
        display.drawPixel(icon_x + 8, icon_y + 3, GxEPD_BLACK);
      }

      // Icon bitmap
      display.drawBitmap(icon_x, icon_y + 6, bitmaps + i * 18, 16, 9, GxEPD_BLACK);
    }
  } while (display.nextPage());
}

static hal_t g_hal_impl = {
  .halt = &hal_halt,
  .log = &hal_log,
  .sleep_until = &hal_sleep_until,
  .get_timestamp = &hal_get_timestamp,
  .update_screen = &hal_update_screen,
  .set_lcd_matrix = &hal_set_lcd_matrix,
  .set_lcd_icon = &hal_set_lcd_icon,
  .set_frequency = &hal_set_frequency,
  .play_frequency = &hal_play_frequency,
  .handler = &hal_handler,
};

hal_t *g_hal = &g_hal_impl;

// ==================== INPUT ====================

void updateInput() {
  unsigned long now = millis();

  // Debounce
  if (now - g_lastBtnTime < 50) {
    return;
  }

  // Read encoder rotation
  int steps = 0;
  portENTER_CRITICAL(&g_encMux);
  steps = g_encStepAccum;
  g_encStepAccum = 0;
  portEXIT_CRITICAL(&g_encMux);

  if (steps > 0) {
    g_currentBtn = (int)BTN_RIGHT;
    g_lastBtnTime = now;
  } else if (steps < 0) {
    g_currentBtn = (int)BTN_LEFT;
    g_lastBtnTime = now;
  }

  // Read encoder button
  static bool lastSw = HIGH;
  bool sw = digitalRead(ENC_SW_PIN);

  if (!sw && lastSw) {
    g_currentBtn = (int)BTN_MIDDLE;
    g_lastBtnTime = now;
  }
  lastSw = sw;

  // Clear button after debounce
  if (now - g_lastBtnTime > 50) {
    g_currentBtn = -1;  // None
  }
}

// ==================== SETUP ====================

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println(F("\n=== KidsBar Tamagotchi ===\n"));

  // Init display
  display.init(115200);
  display.setRotation(1);
  display.setTextColor(GxEPD_BLACK);

  // Welcome
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setFont();
    display.setCursor(60, 60);
    display.print(F("Tamagotchi"));
  } while (display.nextPage());

  // Init encoder
  encoderPcntBegin(ENC_CLK_PIN, ENC_DT_PIN);
  pinMode(ENC_SW_PIN, INPUT_PULLUP);

  // Init LED
  ledStatusBegin(NEOPIXEL_PIN, NEOPIXEL_COUNT, BOARD_RGB_PIN, BOARD_RGB_COUNT);
  ledStatusSetMasterBrightness(0.3);
  setLed(0, 255, 0);

  // Init TamaLib
  tamalib_register_hal(g_hal);
  tamalib_init(1000); // ts_freq = 1000 (milliseconds)
  tamalib_set_framerate(1); // 1 FPS for E-ink

  // Init storage
  initEEPROM();

  // Load state
  if (validEEPROM()) {
    Serial.println(F("Loading saved state..."));
    loadStateFromEEPROM(&g_cpu_state);
  } else {
    Serial.println(F("New Tamagotchi - loading egg..."));
    loadHardcodedState(&g_cpu_state);
  }

  Serial.println(F("Ready!\n"));
  setLedOff();
}

// ==================== LOOP ====================

void loop() {
  static uint32_t last_debug = 0;

  // Update input state
  updateInput();
  encoderPcntPoll(true, &g_encStepAccum, &g_encMux);

  // Run Tamagotchi
  tamalib_mainloop_step_by_step();

  // Debug output every 5 seconds
  if (millis() - last_debug >= 5000) {
    last_debug = millis();
    Serial.printf("Loop running, timestamp=%lu\n", millis());
  }

  // Auto-save
  if (millis() - g_lastSave > AUTO_SAVE_INTERVAL_MS) {
    g_lastSave = millis();
    Serial.println(F("Auto-save"));
    saveStateToEEPROM(&g_cpu_state);
  }

  // Long press reset (5 sec)
  static uint32_t resetStart = 0;
  if (digitalRead(ENC_SW_PIN) == LOW) {
    if (resetStart == 0) resetStart = millis();
    else if (millis() - resetStart > 5000) {
      Serial.println(F("RESET"));
      eraseStateFromEEPROM();
      ESP.restart();
    }
  } else {
    resetStart = 0;
  }
}
