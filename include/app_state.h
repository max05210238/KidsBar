// KidsBar - Application State
// Global state management for virtual pet system
#pragma once

#include <Arduino.h>
#include "config.h"

// ==================== Version ====================
extern const char* KIDSBAR_VERSION;

// ==================== Pet State ====================
struct PetState {
  uint8_t hunger;      // 0-100 (0 = starving, 100 = full)
  uint8_t happiness;   // 0-100 (0 = sad, 100 = very happy)
  uint8_t health;      // 0-100 (0 = sick, 100 = healthy)
  uint8_t level;       // Pet level (1-10)
  uint32_t experience; // Experience points

  // Activity timestamps (Unix time or millis)
  uint32_t lastFeedTime;
  uint32_t lastPlayTime;
  uint32_t lastCheckTime;
};

extern PetState g_petState;

// ==================== UI State ====================
enum UiMode {
  UI_MODE_HOME     = 0,  // Home screen (show pet)
  UI_MODE_MENU     = 1,  // Main menu
  UI_MODE_FEED     = 2,  // Feeding screen
  UI_MODE_PLAY     = 3,  // Play screen
  UI_MODE_STATUS   = 4,  // Status/stats screen
  UI_MODE_SETTINGS = 5   // Settings screen
};

extern UiMode g_uiMode;

// Menu state
extern int g_menuIndex;
extern int g_menuTopIndex;

// ==================== Encoder State ====================
extern bool g_lastEncSw;
extern unsigned long g_encPressStart;
extern volatile int g_encStepAccum;
extern portMUX_TYPE g_encMux;

// ==================== LED Settings ====================
extern float g_ledBrightness;  // 0.0 - 1.0

// ==================== Timing ====================
extern unsigned long g_lastPetUpdate;
extern unsigned long g_lastUiDrawMs;

// ==================== Functions ====================
void initAppState();
void updatePetState();
void resetPetState();
void savePetState();
void loadPetState();
