#include "Comet.h"

void Comet::step(elapsedMillis elapsed, float audioPeak, float freqMagnitudes[]) {
  // cycle hues
  // int hue = 360.0 * ((sin(elapsed / 6000.0) + 1.0) * 0.5);
  int hue = 180.0;

  // set LED at current position
  int idx = (position/100.0) * _pixelCount;
  _pixels[idx + 2] = CHSV(hue, 255, 255);

  // apply velocity to position
  double timeDelta = (elapsed - prevElapsed) / 1000.0; // convert to seconds
  position += (velocity * timeDelta); // velocity in M/s


// TODO: move an LED


  // showLED(LEDPosition, potVal, 255, intensity);       // Illuminate the LED
  // fadeLEDs(8);                                        // Fade LEDs by a value of 8. Higher numbers will create a shorter tail.
  // setDelay(LEDSpeed);                                 // The LEDSpeed is constant, so the delay is constant


  prevElapsed = elapsed;
}
