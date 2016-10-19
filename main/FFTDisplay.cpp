#include "FFTDisplay.h"

void FFTDisplay::step(elapsedMillis elapsed, float audioPeak, float freqMagnitudes[]) {
  for (int i = 0; i < _pixelCount; i++) {
    int hue = ((float)i / (float)_pixelCount) * 360.0;
    _pixels[i] = CHSV(hue, 255, map(freqMagnitudes[i], 0.0, 1.0, 64.0, 255.0));
  }
}
