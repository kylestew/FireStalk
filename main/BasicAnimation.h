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

    float lerp(float start, float end, float percent);
    float map(float value, float istart, float istop, float ostart, float ostop);
    int map(int value, int istart, int istop, int ostart, int ostop);
};

#endif
