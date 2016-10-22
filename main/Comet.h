#ifndef Comet_h
#define Comet_h

#include "BasicAnimation.h"

class Comet: public BasicAnimation {
public:
    using BasicAnimation::BasicAnimation;

    void step(elapsedMillis elapsed, float audioPeak, float freqMagnitudes[]);

  private:
    unsigned int prevElapsed = 0;

    float position = 0.0;
    float velocity = 3.0;
};

#endif
