#pragma once
#include <stdint.h>

// Merge adjacent pixels of the same color in a sprite row: draw one fillRect
// per run instead of one per pixel (the displayed result is identical).
// key(c) returns the color key for pixel c, or -1 if it is transparent;
// emit(c0, len, key) receives each opaque run.
template <class Key, class Emit>
inline void forRuns(int w, Key key, Emit emit) {
  int c = 0, k = w > 0 ? key(0) : -1;
  while (c < w) {
    int c0 = c, kn = k;
    while (++c < w && (kn = key(c)) == k) {}
    if (k >= 0) emit(c0, c - c0, k);
    k = kn;  // The key that ended one run starts the next; it does not need to be read again
  }
}

// Key for an indexed pixel: -1 = transparent (0xFF); in silhouette mode, all
// opaque pixels share one key (they are drawn in the same ink color)
inline int indexedKey(uint8_t idx, bool sil) { return idx == 0xFF ? -1 : (sil ? 0 : idx); }

// The same applies to character maps: '.' is transparent; every other character is its key
// (unsigned: spriteColor() receives it back as (char)key)
inline int charKey(char ch, bool sil) { return ch == '.' ? -1 : (sil ? 0 : (unsigned char)ch); }
