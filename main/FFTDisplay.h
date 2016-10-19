#ifndef FFTDisplay_h
#define FFTDisplay_h

#include "BasicAnimation.h"

class FFTDisplay: public BasicAnimation {
public:
    using BasicAnimation::BasicAnimation;

    void step(elapsedMillis elapsed, float audioPeak, float freqMagnitudes[]);
};

#endif
