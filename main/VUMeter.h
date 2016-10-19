#ifndef VUMeter_h
#define VUMeter_h

#include "BasicAnimation.h"

class VUMeter: public BasicAnimation {
public:
    using BasicAnimation::BasicAnimation;

    void step(elapsedMillis elapsed, float audioPeak, float freqMagnitudes[]);
};

#endif
