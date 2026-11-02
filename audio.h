
#pragma once
#include <stdint.h>

// Soundeffekte des Spiels.
// Die Reihenfolge entspricht der SFX-Tabelle in audio.cpp.
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

// Initialisiert I2S und den PCM5101-DAC.
void audioBegin();

// Spielt einen Soundeffekt nicht-blockierend ab.
void sfxPlay(uint8_t id);

// Ton ein-/ausschalten; Einstellung wird gespeichert.
void audioSetEnabled(bool on);
bool audioEnabled();

// Audiozustand der Spielfigur.
void audioSetSleeping(bool sleeping);

// True, solange ein Soundeffekt im Ruhemodus noch gehalten wird.
bool audioBusy();

// Energiesparzustand bei längerer Inaktivität.
void audioSetIdle(bool idle);
