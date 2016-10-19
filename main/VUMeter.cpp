#include "VUMeter.h"

void VUMeter::step(elapsedMillis elapsed, float audioPeak, float freqMagnitudes[]) {
  // TODO: LERP values for smoothness (up & down)

  // cycle hues
  // int hue = (int)(elapsed / 10.0) % 360;
  // map peak output to where it will be in the output
  int peak = audioPeak * _pixelCount;
  for (int i = 0; i < _pixelCount; i++) {
    int hue = ((float)i / (float)_pixelCount) * 360.0;
    _pixels[i] = CHSV(hue, 255, i < peak ? 255 : 0); // turn on pixels below mapped peak
  }
}
