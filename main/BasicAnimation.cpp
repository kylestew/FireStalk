#include "BasicAnimation.h"

BasicAnimation::BasicAnimation(struct CRGB *pixels)
{
  //
  // _ledPins[RED] = redPin;
  // _ledPins[BLUE] = bluPin;
  // _ledPins[GREEN] = grnPin;
  //
  // setPushButtonPins(swhPin);
  // setLedPins();
  // setShiftRegisterPins(enPin, latchPin, clkPin, clrPin, datPin);
  //
  // randomSeed(analogRead(UNCONNECTED_PIN));
  // setShift(0x0000);
}



// void stepAnimation() {
//   switch(animationProgram) {
//     case responsiveFFT:
//
//       // rotate hue
//       hue += 0.4;
//       if (hue > 360) hue -= 360;
//
//       // output with intensity
//       // fill_solid(&(pixels[0]), PIXEL_COUNT, CHSV(hue, 255, map(audioIntensity, 0.0, 60.0, 0.0, 255.0)));
//       // FastLED.show();
//
//       break;
//
//     case simpleHueShift:
//
//       // rotate hue
//       hue += 0.8;
//       if (hue > 360) hue -= 360;
//
//       // output with intensity
//       // fill_solid(&(pixels[0]), PIXEL_COUNT, CHSV(hue, 255, 255));
//       // FastLED.show();
//
//       break;
//   }
// }
