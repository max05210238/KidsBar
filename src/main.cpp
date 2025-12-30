/*
 * KidsBar Tamagotchi - ArduinoGotchi adapted for ESP32-S3 + E-ink + Rotary Encoder
 *
 * Based on ArduinoGotchi by Gary Kwok (GPLv2)
 * Adapted for KidsBar hardware:
 * - ESP32-S3
 * - WaveShare 2.9" E-ink (296x128, black & white)
 * - Rotary encoder input
 * - WS2812B RGB LED status
 */

#include <Arduino.h>
#include <GxEPD2_BW.h>
#include <Adafruit_NeoPixel.h>
#include <Preferences.h>

#include "config.h"
#include "encoder_pcnt.h"
#include "led_status.h"

// TamaLib includes
extern "C" {
#include "tamalib.h"
#include "hw.h"
#include "hal.h"
}

// Savestate
#include "savestate.h"

// Bitmaps
#include "bitmaps.h"

// ==================== Hardware Configuration ====================

// E-ink Display (2.9", 296x128)
GxEPD2_BW<GxEPD2_290_BS, GxEPD2_290_BS::HEIGHT> display(
  GxEPD2_290_BS(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY)
);

// Preferences for NVS storage
Preferences prefs;

// ==================== Encoder to Button Mapping ====================
// Encoder rotation → LEFT/RIGHT buttons
// Encoder press → MIDDLE button

volatile int g_encStepAccum = 0;
portMUX_TYPE g_encMux = portMUX_INITIALIZER_UNLOCKED;

#define BTN_DEBOUNCE_MS 50
uint32_t lastBtnPress = 0;

enum ButtonState {
  BTN_LEFT_PRESSED,
  BTN_MIDDLE_PRESSED,
  BTN_RIGHT_PRESSED,
  BTN_NONE
};

ButtonState currentButton = BTN_NONE;

// ==================== TamaLib Variables ====================
// Note: LCD_HEIGHT (16) and LCD_WIDTH (32) are defined in hw.h

static uint16_t current_freq = 0;
static bool_t matrix_buffer[LCD_HEIGHT][LCD_WIDTH / 8] = {{0}};
static bool_t icon_buffer[ICON_NUM] = {0};
static cpu_state_t cpuState;
static unsigned long lastSaveTimestamp = 0;

// Display settings
#define TAMA_DISPLAY_FRAMERATE 8
#define AUTO_SAVE_MINUTES 5

// ==================== TamaLib HAL Implementation ====================

static void hal_halt(void) {
  Serial.println(F("[Tama] Halt!"));
}

static void hal_log(log_level_t level, char *buff, ...) {
  Serial.println(buff);
}

static void hal_sleep_until(timestamp_t ts) {
  // No sleep needed for ESP32
}

static timestamp_t hal_get_timestamp(void) {
  return millis() * 1000;
}

static void hal_update_screen(void);

static void hal_set_lcd_matrix(u8_t x, u8_t y, bool_t val) {
  static int pixelSetCount = 0;

  uint8_t mask;
  if (val) {
    mask = 0b10000000 >> (x % 8);
    matrix_buffer[y][x / 8] = matrix_buffer[y][x / 8] | mask;

    // Debug: Print first few pixel sets
    if (pixelSetCount < 5) {
      Serial.printf("[LCD] Set pixel (%d,%d) = ON\n", x, y);
      pixelSetCount++;
    }
  } else {
    mask = 0b01111111;
    for (byte i = 0; i < (x % 8); i++) {
      mask = (mask >> 1) | 0b10000000;
    }
    matrix_buffer[y][x / 8] = matrix_buffer[y][x / 8] & mask;
  }
}

static void hal_set_lcd_icon(u8_t icon, bool_t val) {
  icon_buffer[icon] = val;
}

static void hal_set_frequency(u32_t freq) {
  current_freq = freq;
}

static void hal_play_frequency(bool_t en) {
  // Sound disabled for now (can add buzzer later)
}

static int hal_handler(void) {
  // Handle button inputs (from encoder)

  if (currentButton == BTN_LEFT_PRESSED) {
    hw_set_button(BTN_LEFT, BTN_STATE_PRESSED);
  } else {
    hw_set_button(BTN_LEFT, BTN_STATE_RELEASED);
  }

  if (currentButton == BTN_MIDDLE_PRESSED) {
    hw_set_button(BTN_MIDDLE, BTN_STATE_PRESSED);
  } else {
    hw_set_button(BTN_MIDDLE, BTN_STATE_RELEASED);
  }

  if (currentButton == BTN_RIGHT_PRESSED) {
    hw_set_button(BTN_RIGHT, BTN_STATE_PRESSED);
  } else {
    hw_set_button(BTN_RIGHT, BTN_STATE_RELEASED);
  }

  return 0;
}

static hal_t hal = {
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

// ==================== E-ink Display Functions ====================

void drawTriangle(int16_t x, int16_t y) {
  // Small triangle for menu selection
  display.drawLine(x + 1, y + 1, x + 5, y + 1, GxEPD_BLACK);
  display.drawLine(x + 2, y + 2, x + 4, y + 2, GxEPD_BLACK);
  display.drawPixel(x + 3, y + 3, GxEPD_BLACK);
}

void drawTamaRow(uint8_t tamaLCD_y, int16_t ActualLCD_y, uint8_t pixelSize) {
  // Draw one row of Tamagotchi LCD (32 pixels wide)
  for (uint8_t i = 0; i < LCD_WIDTH; i++) {
    uint8_t mask = 0b10000000 >> (i % 8);
    if ((matrix_buffer[tamaLCD_y][i / 8] & mask) != 0) {
      // Draw pixel at position (scale up by 3x)
      int16_t x = 16 + (i * 3);  // Start at x=16, 3 pixels per Tama pixel
      display.fillRect(x, ActualLCD_y, 3, pixelSize, GxEPD_BLACK);  // Width should be 3, not 2
    }
  }
}

void drawTamaSelection(int16_t y) {
  // Draw icon menu (8 icons)
  for (uint8_t i = 0; i < 8; i++) {
    if (icon_buffer[i]) {
      drawTriangle(i * 16 + 5, y);
    }
    // Draw icon bitmap (from bitmaps.h)
    // drawBitmap expects XBM format
    display.drawBitmap(i * 16 + 4, y + 6, bitmaps + i * 18, 16, 9, GxEPD_BLACK);
  }
}

static void hal_update_screen(void) {
  static int updateCount = 0;
  updateCount++;

  // Debug: Print first few updates
  if (updateCount <= 5) {
    Serial.printf("[Display] Update #%d - Drawing Tamagotchi LCD\n", updateCount);
  }

  // Use partial update for fast refresh (no flicker)
  // E-ink will only update changed pixels
  display.setPartialWindow(0, 0, display.width(), display.height());
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);

    // Draw Tamagotchi LCD (32x16 scaled 3x = 96x48)
    for (uint8_t j = 0; j < LCD_HEIGHT; j++) {
      drawTamaRow(j, 20 + j * 3, 3);  // Start at y=20, 3x3 pixel scaling
    }

    // Draw icon menu at bottom
    drawTamaSelection(90);

  } while (display.nextPage());
}

// ==================== Input Handling ====================

void handleEncoderInput() {
  static unsigned long lastInputTime = 0;
  unsigned long now = millis();

  // Debounce
  if (now - lastInputTime < BTN_DEBOUNCE_MS) {
    return;
  }

  // Read encoder rotation
  int steps = 0;
  portENTER_CRITICAL(&g_encMux);
  steps = g_encStepAccum;
  g_encStepAccum = 0;
  portEXIT_CRITICAL(&g_encMux);

  if (steps > 0) {
    // Clockwise → RIGHT button
    currentButton = BTN_RIGHT_PRESSED;
    setLed(0, 0, 255);  // Blue flash
    lastInputTime = now;
    Serial.println(F("[Input] RIGHT"));
  } else if (steps < 0) {
    // Counter-clockwise → LEFT button
    currentButton = BTN_LEFT_PRESSED;
    setLed(255, 0, 0);  // Red flash
    lastInputTime = now;
    Serial.println(F("[Input] LEFT"));
  }

  // Read encoder button
  bool encSw = digitalRead(ENC_SW_PIN);
  static bool lastEncSw = HIGH;

  if (!encSw && lastEncSw) {
    // Button pressed → MIDDLE button
    currentButton = BTN_MIDDLE_PRESSED;
    setLed(0, 255, 0);  // Green flash
    lastInputTime = now;
    Serial.println(F("[Input] MIDDLE"));
  }

  lastEncSw = encSw;

  // Clear button state after debounce period
  if (now - lastInputTime > BTN_DEBOUNCE_MS) {
    currentButton = BTN_NONE;
  }
}

// ==================== Setup ====================

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println(F("\n\n========================================"));
  Serial.println(F("  KidsBar Tamagotchi"));
  Serial.println(F("  Based on ArduinoGotchi"));
  Serial.println(F("========================================\n"));

  // Initialize E-ink display
  Serial.println(F("[Display] Initializing E-ink..."));
  display.init(115200);
  display.setRotation(1);  // Landscape
  display.setTextColor(GxEPD_BLACK);

  // Welcome screen
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setFont();
    display.setCursor(60, 60);
    display.print(F("Tamagotchi"));
    display.setCursor(50, 75);
    display.print(F("Loading..."));
  } while (display.nextPage());

  // Initialize encoder
  Serial.println(F("[Input] Initializing encoder..."));
  encoderPcntBegin(ENC_CLK_PIN, ENC_DT_PIN);
  pinMode(ENC_SW_PIN, INPUT_PULLUP);

  // Initialize LED
  Serial.println(F("[LED] Initializing status LED..."));
  ledStatusBegin(NEOPIXEL_PIN, NEOPIXEL_COUNT, BOARD_RGB_PIN, BOARD_RGB_COUNT);
  ledStatusSetMasterBrightness(0.5);  // 50% brightness
  setLed(0, 255, 0);  // Green = ready

  // Initialize TamaLib
  Serial.println(F("[Tama] Initializing TamaLib..."));
  tamalib_register_hal(&hal);
  tamalib_set_framerate(TAMA_DISPLAY_FRAMERATE);
  tamalib_init(1000000);

  // Initialize NVS/Preferences
  Serial.println(F("[Storage] Initializing NVS..."));
  initEEPROM();

  // TEMPORARY: Force load egg state and clear old saves
  // This ensures fresh start with proper initial state
  Serial.println(F("[Storage] Clearing old state and loading fresh egg..."));
  eraseStateFromEEPROM();
  loadHardcodedState(&cpuState);

  // TODO: Restore normal save/load logic after testing:
  // if (validEEPROM()) {
  //   loadStateFromEEPROM(&cpuState);
  // } else {
  //   loadHardcodedState(&cpuState);
  // }

  Serial.println(F("\n[Main] Tamagotchi initialized!"));
  Serial.println(F("[Main] Use encoder to navigate, press to select\n"));

  // CRITICAL: Run TamaLib to populate LCD matrix after state load
  // cpu_set_state doesn't trigger LCD updates - need to run CPU cycles
  Serial.println(F("[Main] Running TamaLib to populate LCD..."));
  for (int i = 0; i < 100; i++) {
    tamalib_mainloop_step_by_step();
  }

  // Debug: Check matrix_buffer contents
  Serial.println(F("[Debug] Checking matrix_buffer after TamaLib run:"));
  int pixelCount = 0;
  for (uint8_t y = 0; y < LCD_HEIGHT; y++) {
    for (uint8_t x = 0; x < LCD_WIDTH; x++) {
      uint8_t mask = 0b10000000 >> (x % 8);
      if ((matrix_buffer[y][x / 8] & mask) != 0) {
        pixelCount++;
        if (pixelCount <= 10) {
          Serial.printf("[Debug] Pixel ON at (%d,%d)\n", x, y);
        }
      }
    }
  }
  Serial.printf("[Debug] Total pixels ON: %d (should be > 0 for egg!)\n", pixelCount);

  // Force initial screen update to show the egg
  Serial.println(F("[Main] Forcing initial screen update..."));
  hal_update_screen();

  setLedOff();  // LED off
}

// ==================== Main Loop ====================

void loop() {
  // Handle encoder input
  handleEncoderInput();

  // Poll encoder hardware
  encoderPcntPoll(true, &g_encStepAccum, &g_encMux);

  // Run Tamagotchi emulation
  tamalib_mainloop_step_by_step();

  // Auto-save every N minutes
  if ((millis() - lastSaveTimestamp) > (AUTO_SAVE_MINUTES * 60 * 1000UL)) {
    lastSaveTimestamp = millis();
    Serial.println(F("[Storage] Auto-saving state..."));
    saveStateToEEPROM(&cpuState);
  }

  // Long press middle button (5 seconds) to reset
  static uint32_t resetPressStart = 0;
  if (digitalRead(ENC_SW_PIN) == LOW) {
    if (resetPressStart == 0) {
      resetPressStart = millis();
    } else if (millis() - resetPressStart > 5000) {
      Serial.println(F("[Main] RESET - Erasing saved state..."));
      eraseStateFromEEPROM();
      setLed(255, 0, 0);  // Red
      delay(1000);
      ESP.restart();
    }
  } else {
    resetPressStart = 0;
  }
}
