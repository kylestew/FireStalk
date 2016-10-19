#include "BasicAnimation.h"

BasicAnimation::BasicAnimation() {
}

BasicAnimation::BasicAnimation(struct CRGB *pixels, int pixelCount) {
  _pixels = pixels;
  _pixelCount = pixelCount;
}

void BasicAnimation::step(elapsedMillis elapsed, float audioPeak, float freqMagnitudes[]) {
  // cycle hues
  int hue = (int)(elapsed / 100.0) % 360;
  fill_solid(&(_pixels[0]), _pixelCount, CHSV(hue, 255, 255));
}

// === Utility Functions ===
// float lerp(float start, float end, float percent) {
//   return start + percent * (end - start);
// }

float BasicAnimation::map(float value, float istart, float istop, float ostart, float ostop) {
  return ostart + (ostop - ostart) * ((value - istart) / (istop - istart));
}

int BasicAnimation::map(int value, int istart, int istop, int ostart, int ostop) {
  return ostart + (ostop - ostart) * ((value - istart) / (istop - istart));
}
