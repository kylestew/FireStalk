#ifndef BasicAnimation_h
#define BasicAnimation_h

#include <FastLED.h>

class BasicAnimation {
  public:
    BasicAnimation();
    BasicAnimation(struct CRGB *pixels, int pixelCount);

    virtual void step(elapsedMillis elapsed, float audioPeak, float freqMagnitudes[]);

  protected:
    CRGB *_pixels;
    int _pixelCount;

    float map(float value, float istart, float istop, float ostart, float ostop);
};

#endif
