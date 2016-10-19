#include "AudioTestAnimation.h"

void AudioTestAnimation::step(elapsedMillis elapsed, float audioPeak, float freqMagnitudes[]) {
  // cycle hues
  int hue = (int)(elapsed / 10.0) % 360;
  for (int i = 0; i < _pixelCount; i++) {
    _pixels[i] = CHSV(hue, 255, map(freqMagnitudes[i], 0.0, 0.8, 20.0, 255.0));
  }
}
