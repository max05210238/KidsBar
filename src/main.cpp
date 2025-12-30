// KidsBar v0.1.0-alpha
// Main program - Virtual Pet Hardware Driver Layer
// Ported from CryptoBar hardware foundation

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <nvs_flash.h>

#include "config.h"
#include "app_state.h"
#include "encoder_pcnt.h"
#include "led_status.h"
#include "settings_store.h"

// ==================== Hardware Objects ====================

// External WS2812B LED
Adafruit_NeoPixel ledExternal(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// Onboard LED (forced off to save power)
Adafruit_NeoPixel ledOnboard(BOARD_RGB_COUNT, BOARD_RGB_PIN, NEO_GRB + NEO_KHZ800);

// ==================== Setup ====================

void setup() {
  Serial.begin(115200);
  delay(500);  // Wait for serial

  Serial.println("\n\n");
  Serial.println("========================================");
  Serial.println("  KidsBar - Virtual Pet Device");
  Serial.printf("  Version: %s\n", KIDSBAR_VERSION);
  Serial.println("  Hardware: ESP32-S3 + E-ink + Encoder");
  Serial.println("========================================\n");

  // Initialize NVS (Non-Volatile Storage)
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    Serial.println("[NVS] Erasing and reinitializing...");
    ESP.restart();
  }
  Serial.println("[NVS] Initialized");

  // Initialize settings storage
  settingsBegin();

  // Initialize application state
  initAppState();

  // Initialize rotary encoder (PCNT hardware)
  Serial.println("[Hardware] Initializing rotary encoder...");
  encoderPcntBegin(ENC_CLK_PIN, ENC_DT_PIN);
  pinMode(ENC_SW_PIN, INPUT_PULLUP);

  // Initialize LEDs
  Serial.println("[Hardware] Initializing LEDs...");
  ledExternal.begin();
  ledExternal.setBrightness(uint8_t(g_ledBrightness * 255));
  ledExternal.show();  // Turn off initially

  ledOnboard.begin();
  ledOnboard.setBrightness(0);  // Force onboard LED off
  ledOnboard.show();

  // Welcome animation
  Serial.println("[Hardware] Playing welcome animation...");
  for (int i = 0; i < 3; i++) {
    ledExternal.setPixelColor(0, ledExternal.Color(0, 50, 0));  // Green
    ledExternal.show();
    delay(200);
    ledExternal.setPixelColor(0, ledExternal.Color(0, 0, 0));  // Off
    ledExternal.show();
    delay(200);
  }

  // TODO: Initialize E-ink display
  Serial.println("[Hardware] E-ink display initialization (TODO)");

  // TODO: Display welcome screen
  Serial.println("[UI] Welcome screen (TODO)");

  Serial.println("\n[Main] Initialization complete!");
  Serial.println("[Main] Entering main loop...\n");

  // Initial LED status: Blue (ready)
  ledSetStatusColor(0, 0, 255, g_ledBrightness);
}

// ==================== Main Loop ====================

void loop() {
  static unsigned long lastEncoderPoll = 0;
  static unsigned long lastSerialDebug = 0;
  unsigned long now = millis();

  // Poll rotary encoder (every 10ms for responsive feel)
  if (now - lastEncoderPoll >= 10) {
    bool appRunning = true;  // TODO: set to false during long operations
    encoderPcntPoll(appRunning, &g_encStepAccum, &g_encMux);
    lastEncoderPoll = now;
  }

  // Read encoder button
  bool encSw = digitalRead(ENC_SW_PIN);

  // Detect button press (falling edge)
  if (!encSw && g_lastEncSw) {
    g_encPressStart = now;
    Serial.println("[Input] Encoder button pressed");

    // Button feedback: flash LED white
    ledSetStatusColor(255, 255, 255, g_ledBrightness);
  }

  // Detect button release
  if (encSw && !g_lastEncSw) {
    unsigned long pressDuration = now - g_encPressStart;
    Serial.printf("[Input] Encoder button released (held %lu ms)\n", pressDuration);

    // Restore status LED
    ledSetStatusColor(0, 0, 255, g_ledBrightness);

    // TODO: Handle button actions based on UI mode
  }

  g_lastEncSw = encSw;

  // Handle encoder rotation
  int steps = 0;
  portENTER_CRITICAL(&g_encMux);
  steps = g_encStepAccum;
  g_encStepAccum = 0;
  portEXIT_CRITICAL(&g_encMux);

  if (steps != 0) {
    Serial.printf("[Input] Encoder rotated: %d steps\n", steps);

    // Visual feedback: pulse LED
    if (steps > 0) {
      ledSetStatusColor(0, 255, 0, g_ledBrightness);  // Green for CW
    } else {
      ledSetStatusColor(255, 0, 0, g_ledBrightness);  // Red for CCW
    }

    // TODO: Update UI based on rotation
    delay(50);
    ledSetStatusColor(0, 0, 255, g_ledBrightness);  // Back to blue
  }

  // Update pet state (hunger, happiness, etc.)
  updatePetState();

  // Debug output (every 5 seconds)
  if (now - lastSerialDebug >= 5000) {
    Serial.printf("[Pet] Hunger: %d, Happiness: %d, Health: %d, Level: %d\n",
                  g_petState.hunger, g_petState.happiness,
                  g_petState.health, g_petState.level);
    lastSerialDebug = now;
  }

  // Small delay to prevent busy-waiting
  delay(1);
}
