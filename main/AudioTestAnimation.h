#ifndef AudioTestAnimation_h
#define AudioTestAnimation_h

#include "BasicAnimation.h"

class AudioTestAnimation: public BasicAnimation {
public:
    using BasicAnimation::BasicAnimation;

    void step(elapsedMillis elapsed, float audioPeak, float freqMagnitudes[]);
};

#endif
