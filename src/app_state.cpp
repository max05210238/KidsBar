// KidsBar - Application State Implementation
#include "app_state.h"

// ==================== Version ====================
const char* KIDSBAR_VERSION = "v0.1.0-alpha";

// ==================== Global State Variables ====================

// Pet state
PetState g_petState = {
  .hunger = 50,
  .happiness = 50,
  .health = 100,
  .level = 1,
  .experience = 0,
  .lastFeedTime = 0,
  .lastPlayTime = 0,
  .lastCheckTime = 0
};

// UI state
UiMode g_uiMode = UI_MODE_HOME;
int g_menuIndex = 0;
int g_menuTopIndex = 0;

// Encoder state
bool g_lastEncSw = HIGH;
unsigned long g_encPressStart = 0;
volatile int g_encStepAccum = 0;
portMUX_TYPE g_encMux = portMUX_INITIALIZER_UNLOCKED;

// LED settings
float g_ledBrightness = 0.5f;  // 50% brightness default

// Timing
unsigned long g_lastPetUpdate = 0;
unsigned long g_lastUiDrawMs = 0;

// ==================== Functions ====================

void initAppState() {
  // Initialize default state
  g_petState.hunger = 50;
  g_petState.happiness = 50;
  g_petState.health = 100;
  g_petState.level = 1;
  g_petState.experience = 0;

  g_uiMode = UI_MODE_HOME;
  g_menuIndex = 0;

  Serial.println("[AppState] Initialized");
}

void updatePetState() {
  // Decrease hunger over time (1 point per minute)
  unsigned long now = millis();
  unsigned long elapsed = now - g_lastPetUpdate;

  if (elapsed >= PET_UPDATE_INTERVAL_MS) {
    if (g_petState.hunger > 0) {
      g_petState.hunger--;
    }

    // Decrease happiness if hungry
    if (g_petState.hunger < 20 && g_petState.happiness > 0) {
      g_petState.happiness--;
    }

    // Decrease health if very hungry
    if (g_petState.hunger < 10 && g_petState.health > 0) {
      g_petState.health--;
    }

    // Increase happiness slowly when well-fed
    if (g_petState.hunger > 70 && g_petState.happiness < 100) {
      g_petState.happiness++;
    }

    g_lastPetUpdate = now;
  }
}

void resetPetState() {
  g_petState.hunger = 50;
  g_petState.happiness = 50;
  g_petState.health = 100;
  g_petState.level = 1;
  g_petState.experience = 0;

  Serial.println("[AppState] Pet state reset");
}

void savePetState() {
  // TODO: Implement NVS save
  Serial.println("[AppState] Pet state saved (stub)");
}

void loadPetState() {
  // TODO: Implement NVS load
  Serial.println("[AppState] Pet state loaded (stub)");
}
