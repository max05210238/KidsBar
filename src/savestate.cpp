/*
 * Savestate implementation for ESP32 using Preferences (NVS)
 * Adapted from ArduinoGotchi EEPROM version
 */

#include <Arduino.h>
#include <Preferences.h>
#include "savestate.h"

extern "C" {
#include "cpu.h"
}

// NVS namespace
#define NVS_NAMESPACE "tamagotchi"
#define NVS_KEY_STATE "state"
#define NVS_KEY_MEMORY "memory"
#define NVS_KEY_MAGIC "magic"

// Magic number to verify valid save
#define SAVE_MAGIC 0x54414D41  // "TAMA"

static Preferences prefs;

void initEEPROM() {
  Serial.println(F("[Storage] Initializing NVS..."));
  prefs.begin(NVS_NAMESPACE, false);
}

bool validEEPROM() {
  uint32_t magic = prefs.getUInt(NVS_KEY_MAGIC, 0);
  return (magic == SAVE_MAGIC);
}

void saveStateToEEPROM(cpu_state_t *state) {
  Serial.println(F("[Storage] Saving state to NVS..."));

  cpu_get_state(state);

  // Save CPU state
  prefs.putBytes(NVS_KEY_STATE, (const uint8_t *)state, sizeof(cpu_state_t));

  // Save memory separately
  prefs.putBytes(NVS_KEY_MEMORY, (const uint8_t *)state->memory, MEMORY_SIZE * sizeof(u4_t));

  // Save magic number
  prefs.putUInt(NVS_KEY_MAGIC, SAVE_MAGIC);

  Serial.println(F("[Storage] State saved successfully"));
}

void loadStateFromEEPROM(cpu_state_t *state) {
  Serial.println(F("[Storage] Loading state from NVS..."));

  if (!validEEPROM()) {
    Serial.println(F("[Storage] No valid save found"));
    return;
  }

  // Load CPU state
  size_t stateSize = prefs.getBytesLength(NVS_KEY_STATE);
  if (stateSize != sizeof(cpu_state_t)) {
    Serial.println(F("[Storage] State size mismatch"));
    return;
  }

  prefs.getBytes(NVS_KEY_STATE, (uint8_t *)state, sizeof(cpu_state_t));

  // Load memory
  size_t memSize = prefs.getBytesLength(NVS_KEY_MEMORY);
  if (memSize != MEMORY_SIZE * sizeof(u4_t)) {
    Serial.println(F("[Storage] Memory size mismatch"));
    return;
  }

  prefs.getBytes(NVS_KEY_MEMORY, (uint8_t *)state->memory, MEMORY_SIZE * sizeof(u4_t));

  cpu_set_state(state);

  Serial.println(F("[Storage] State loaded successfully"));
}

void eraseStateFromEEPROM() {
  Serial.println(F("[Storage] Erasing saved state..."));
  prefs.clear();
  Serial.println(F("[Storage] State erased"));
}
