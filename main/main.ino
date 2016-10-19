/*
 * ===============
 * == FireStalk ==
 * ===============
 */
#include <FastLED.h>
#include "RingCoder.h"
#include "Audio.h"

#include "BasicAnimation.h"
#include "FFTDisplay.h"
#include "VUMeter.h"


#define SERIAL_DEBUG        true

/* Status LED */
const int STATUS_LED = 13;

/* Pixels */
const int PIXEL_COUNT = 64;
const int PIXEL_STRAND_0 = 11;
const int PIXEL_STRAND_1 = 12;
int brightness = 32;
CRGB pixels[PIXEL_COUNT]; // mirrored

/* Ring Encoder */
const int ENCB = 0; // All pins interrupt on Teensy
const int ENCA = 1;
const int ENC_SW = 2;
const int LEDR = 3; // (not enabled)
const int LEDB = 4; // PWM enabled LED driver
const int LEDG = 5; // PWM enabled LED driver
// ring led shift register
const int DAT = 6;
const int CLR = 7;
const int CLK = 8;
const int LATCH = 9;
const int EN = 10;
RingCoder ringcoder = RingCoder(ENCB, ENCA, LEDR, LEDB, LEDG, ENC_SW, DAT, CLR, CLK, LATCH, EN);

/* Audio FFT Sampling */
bool runFFTSampling = false;
float fftGain = 32.0;
const int FFT_GAIN_MIN = 8;
const int FFT_GAIN_MAX = 92;
AudioInputAnalog        adc(A0);
AudioAnalyzeRMS         rms;
AudioAnalyzeFFT1024     fft;
AudioConnection         patchCord1(adc, fft);
AudioConnection         patchCord2(adc, rms);
const int FREQUENCY_FILTER_RANGE = 64;
double audioRMS = 0;
float freqMagnitudes[FREQUENCY_FILTER_RANGE];

/* Mode */
int controlMode = 0;    // 0 (BLU) == set animation program
                        // 1 (GRN) == set LED brightness
                        // 2 (RED) == set FFT gain

/* Animations Timing */
const int ANIMATION_DELAY_MS = 12;  // about 82 FPS (remember FFT has limit of about 86 samples per second)
elapsedMillis fps;
elapsedMillis ellapsed;

/* Animations */
const int ANIMATION_COUNT = 2;
BasicAnimation* animations[ANIMATION_COUNT];
int currentAnimationIdx = 0;


// === LIFECYCLE ===
void setup() {
  if (SERIAL_DEBUG) {
    Serial.begin(115200);
  }

  // status LED
  pinMode(STATUS_LED, OUTPUT);
  digitalWrite(STATUS_LED, LOW);

  // setup pixels
  FastLED.addLeds<NEOPIXEL, PIXEL_STRAND_0>(pixels, PIXEL_COUNT);
  FastLED.addLeds<NEOPIXEL, PIXEL_STRAND_1>(pixels, PIXEL_COUNT);
  // TODO: set a color profile
  fill_solid(&(pixels[0]), PIXEL_COUNT, CRGB(0, 0, 0));
  FastLED.clear();
  FastLED.setBrightness(brightness);

  // boot animation on ring coder
  ringcoder.setKnobRgb(255, 255, 255);
  ringcoder.spin();
  ringcoder.reverse_spin();

  // audio processing
  AudioMemory(12);
  fft.windowFunction(AudioWindowHanning1024);

  // create animations
  animations[0] = new VUMeter(pixels, PIXEL_COUNT);
  animations[1] = new FFTDisplay(pixels, PIXEL_COUNT);

  // set control mode
  setMode(0);
}

void loop() {
  // DON'T BLOCK MAIN LOOP
  // ring coder is a polling input
  // watch for button changes
  if (ringcoder.update() && ringcoder.button() == HIGH) {
    // toggle mode
    int newMode = controlMode + 1;
    if (newMode > 2) newMode = 0;
    setMode(newMode);
  }

  int encValue = ringcoder.readEncoder();
  if (ringcoder.moved()) {
    // update ring coder display
    updateModeDisplay(encValue);
  }

  // throttle visual updates to a set FPS
  if (fps < ANIMATION_DELAY_MS) {
    FastLED.delay(4);
    return;
  }
  fps = 0; // reset - fps variale automatically counts up

  // process new Peak value
  if (rms.available()) {
    // map values more evenly
    audioRMS = constrain((log(rms.read()) / 6.0) + 1, 0.0, 1.0);
  }

  // process new FFT values
  if (fft.available()) {
    // each bin is 43Hz - sample from 0Hz to 2752Hz (0 - 64)
    for (int i = 0; i < FREQUENCY_FILTER_RANGE; i++) {
      // normalize freq values across spectrum
      freqMagnitudes[i] = fft.read(i) * fftGain * (log(i+1) + 1.0);
    }
  }

  // animations set pixels
  animations[currentAnimationIdx]->step(ellapsed, audioRMS, freqMagnitudes);
  FastLED.show();

  // output values for Processing prototype visualization
  if (SERIAL_DEBUG) {
    // spit all pixel values to serial
    int i;
    for (i = 0; i < PIXEL_COUNT; i++) {
      Serial.print(pixels[i].r);
      Serial.print(" ");
      Serial.print(pixels[i].g);
      Serial.print(" ");
      Serial.print(pixels[i].b);
      Serial.print(" ");
    }

    // spit all fft values to serial
    for (i = 0; i < FREQUENCY_FILTER_RANGE; i++) {
      Serial.print(freqMagnitudes[i]);
      Serial.print(" ");
    }
    Serial.println();
  }
}


// === MODES ===
void setMode(int newMode) {
  controlMode = newMode;
  if (controlMode == 0) {
    // BLUE mode - select animation
    ringcoder.setEncoderRange(ANIMATION_COUNT);
    ringcoder.writeEncoder(currentAnimationIdx);
    ringcoder.ledRingFollower();
    ringcoder.setKnobRgb(0, 0, 255);
  }
  else if (controlMode == 1) {
    // GREEN mode - select brightness
    // don't require user to click ever value - skip every 4
    // add a few ticks so we don't flip from brightess to darkest abruptly
    ringcoder.setEncoderRange(68);
    ringcoder.writeEncoder(brightness/4);
    ringcoder.ledRingFiller();
    ringcoder.setKnobRgb(0, 255, 0);
  }
  else if (controlMode == 2) {
    // RED mode - select FFT gain
    ringcoder.setEncoderRange(FFT_GAIN_MAX - FFT_GAIN_MIN);
    ringcoder.writeEncoder(fftGain);
    ringcoder.ledRingFiller();
    ringcoder.setKnobRgb(255, 0, 0);
  }
}

void updateModeDisplay(u_int newValue) {
  if (controlMode == 0) {
    currentAnimationIdx = newValue;
    ringcoder.ledRingFollower();
  }
  else if (controlMode == 1) {
    brightness = newValue * 4; // skipping every 4th value
    if (brightness > 255) brightness = 255;
    FastLED.setBrightness(brightness);
    ringcoder.ledRingFiller();
  }
  else if (controlMode == 2) {
    fftGain = newValue + FFT_GAIN_MIN;
    ringcoder.ledRingFiller();
  }
}
