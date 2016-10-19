#ifndef BasicAnimation_h
#define BasicAnimation_h

class BasicAnimation {
  public:
    BasicAnimation(struct CRGB *pixels);

    // void step(elapsedMillis elapsed, float audioPeak, float[] fftMagnitides);

  private:
    CRGB* _pixels;

};

#endif
