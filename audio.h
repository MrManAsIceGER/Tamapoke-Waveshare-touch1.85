
#pragma once
#include <stdint.h>

// Game sound effects.
// Order matches the SFX table in audio.cpp.
enum Sfx : uint8_t {
  SFX_TAP = 0,
  SFX_EAT,
  SFX_PLAY,
  SFX_HEART,
  SFX_HATCH,
  SFX_EVOLVE,
  SFX_MEDAL,
  SFX_DENY,
  SFX_BYE,
  SFX_LEVEL,
  SFX_COUNT
};

// Initialize I2S and the PCM5101 DAC.
void audioBegin();

// Play a sound effect without blocking.
void sfxPlay(uint8_t id);

// Enable/disable sound; the setting is saved.
void audioSetEnabled(bool on);
bool audioEnabled();

// Audio state of the pet.
void audioSetSleeping(bool sleeping);

// True while a sound effect is still held during idle mode.
bool audioBusy();

// Power-saving state after an extended period of inactivity.
void audioSetIdle(bool idle);
