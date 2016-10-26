/*
 * ===============
 * == FireStalk ==
 * ===============
 */
#include <FastLED.h>
#include "RingCoder.h"
#include "Audio.h"


#define SERIAL_DEBUG        true

/* Status LED */
const int STATUS_LED = 13;

/* Pixels */
const int MAX_PIXEL_BRIGHTNESS = 204; // 4/5th total
int brightness = MAX_PIXEL_BRIGHTNESS / 3;

const int PIXEL_COUNT = 40;
const int PIXEL_STRAND_0 = 11;
const int PIXEL_STRAND_1 = 12;
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
float fftGain = 8.0;
const float FFT_GAIN_MIN = 1;             // min audio gain slider
const float FFT_GAIN_MAX = 12;            // max audio gain slider
const float FFT_LERP = 0.4;              // smooth out FFT value changes
const double RMS_GAIN_MULT = 0.333;         // RMS audio power adjustment ties to FFT
const float RMS_LERP = 0.02;              // smooth out RMS value changes
AudioInputAnalog        adc(A0);
AudioAnalyzeRMS         rms;
// AudioAnalyzeFFT1024     fft;
// AudioConnection         patchCord1(adc, fft);
AudioConnection         patchCord2(adc, rms);
const int FREQUENCY_FILTER_RANGE = 64;
double audioRMS = 0.0;
float freqMagnitudes[FREQUENCY_FILTER_RANGE];

/* Mode */
int controlMode = 0;    // 0 (BLU) == set ANIMATION mode
                        // 1 (GRN) == set LED brightness
                        // 2 (RED) == set FFT gain

/* Animations Timing */
const int ANIMATION_MAX_FPS = 14;  // about 72 FPS (remember FFT has limit of about 86 samples per second)
elapsedMillis fps;
elapsedMillis elapsed;


// === LIFECYCLE ===
void setup() {
  if (SERIAL_DEBUG) {
    Serial.begin(115200);
  }

  // status LED
  pinMode(STATUS_LED, OUTPUT);
  digitalWrite(STATUS_LED, LOW);

  // setup pixels
  delay(500); // give LED cap time to charge?
  FastLED.addLeds<NEOPIXEL, PIXEL_STRAND_0>(pixels, PIXEL_COUNT);
  FastLED.addLeds<NEOPIXEL, PIXEL_STRAND_1>(pixels, PIXEL_COUNT);
  FastLED.setCorrection(0xFFA0C0);
  FastLED.setTemperature(0xFFC0A0);
  FastLED.setBrightness(brightness);
  fill_solid(&(pixels[0]), PIXEL_COUNT, CRGB(0, 0, 0));
  FastLED.clear();

  // boot animation on ring coder
  ringcoder.setKnobRgb(255, 0, 255);
  ringcoder.spin();

  // audio processing
  AudioMemory(8);
  // fft.windowFunction(AudioWindowHanning1024);

  // let first animation run setup
  switchAnimation(0);

  // set control mode
  setMode(0);
}

int serialReadIndex = 0;
int serialColorIndex = 0;

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
  if (fps < ANIMATION_MAX_FPS) {
    FastLED.delay(8);
    return;
  }
  fps = 0; // reset - fps variale automatically counts up

  // process new Peak value
  if (rms.available()) {
     audioRMS = rms.read() * (fftGain * RMS_GAIN_MULT); // dail in fft and RMS at same time
  }

  /*
  // process new FFT values
  if (fft.available()) {
    // each bin is 43Hz - sample from 0Hz to 2752Hz (0 - 64)
    for (int i = 0; i < FREQUENCY_FILTER_RANGE; i++) {
      // normalize freq values across spectrum
      float val = fft.read(i);
      if (val < 0.005) val = 0;
      freqMagnitudes[i] = lerp(freqMagnitudes[i], val * fftGain * (log(i+1) + 1.0), FFT_LERP);
    }
  }
  */

  runCurrentAnimation();

  // output values for Processing prototype visualization
  if (SERIAL_DEBUG) {
    Serial.println(audioRMS);


    // spit all pixel values to serial
    // int i;
    // for (i = 0; i < PIXEL_COUNT; i++) {
    //   Serial.print(pixels[i].r);
    //   Serial.print(" ");
    //   Serial.print(pixels[i].g);
    //   Serial.print(" ");
    //   Serial.print(pixels[i].b);
    //   Serial.print(" ");
    // }

    // spit all fft values to serial
    // for (i = 0; i < FREQUENCY_FILTER_RANGE; i++) {
      // Serial.print(freqMagnitudes[i]);
      // Serial.print(" ");
    // }
    // Serial.println();
  }
}


// === ANIMATIONS ===
const int ANIMATION_COUNT = 1;
typedef enum {

  TAKEOFF,

  FIREWORKS, // (audio)
  BOUNCEBALL,
  PLASMA,
  FIRE,

  // BEN's animations
  PAPARAZZI,


  VUMETER,
  RAINBOW,
} ANIMATIONS;
int currentAnimationIdx = 0;

long frameCount = 0;
unsigned int prevElapsed;
double timeDelta; // fractions of a second

void switchAnimation(int newValue) {
  currentAnimationIdx = newValue;

  switch(currentAnimationIdx) {
    case FIREWORKS:
      fireworksSetup();
      break;

    case BOUNCEBALL:
      bounceballSetup();
      break;

    case PLASMA:
      plasmaSetup();
      break;

    default:
      break;
  }
}

void runCurrentAnimation() {
  if (frameCount == 0) {
    // skip first frame to setup time delta
    prevElapsed = elapsed;
    frameCount++;
    return;
  }

  // time delta (in seconds) since last frame
  timeDelta = (elapsed - prevElapsed) / 1000.0; // convert to seconds

  switch(currentAnimationIdx) {
    case FIREWORKS:
      fireworks();
      break;

    case BOUNCEBALL:
      bounceball();
      break;

    case PLASMA:
      plasma();
      break;

    case FIRE:
      break;

    // == BEN ==========
    case PAPARAZZI:
      paparazzi();
      break;

    case TAKEOFF:
      takeOff();
      break;


    // =================

    case VUMETER:
      vuMeter();
      break;
    case RAINBOW:
    default:
      rainbowCycle();
  }

  prevElapsed = elapsed;
  frameCount++;
}


/* == FIREWORKS == */
struct Firework {
  boolean alive;
  int hue;
  float pos;
  float vel;
  float decel;
};

const int FIREWORK_MAX_VISIBLE = 12;
const int FIREWORK_MAX_EJECT_TIME = 1400;
const int FIREWORK_MIN_EJECT_TIME = 200;
const float FIREWORK_AUDIO_THRESHOLD = 0.575;
long lastFireballEjectMillis = 0;
Firework fireballs[FIREWORK_MAX_VISIBLE];
int fireworkIndex = 0;

void ejectFireball() {
  Firework f;
  f.alive = true;
  f.hue = (byte)random(256);
  f.pos = 0;
  // f.vel = 0.9 + random(1000)/500.0;
  f.vel = 0.5 + (random(100) / 100.0) + (audioRMS * 1.4);
  f.decel = 0.972;
  fireballs[fireworkIndex++] = f;
  if (fireworkIndex > FIREWORK_MAX_VISIBLE - 1)
    fireworkIndex = 0;

  lastFireballEjectMillis = elapsed;
}

void fireworksSetup() {
}

void fireworks() {
  // don't always wait for audio hits
  if (elapsed - lastFireballEjectMillis > FIREWORK_MAX_EJECT_TIME) {
    ejectFireball();
  } else if (audioRMS > FIREWORK_AUDIO_THRESHOLD && FIREWORK_MIN_EJECT_TIME) {
    // audio hit - eject
    ejectFireball();
  }

  for (int i = 0; i < FIREWORK_MAX_VISIBLE; i++) {
    Firework firework = fireballs[i];

    if (firework.alive == true && firework.pos >= 0 && firework.pos < PIXEL_COUNT) {
      // body
      int idx = (int)firework.pos;
      pixels[idx] += CHSV(firework.hue, 255, 255);

      // tail - random hue wobble and decreasing brightness
      if (idx - 1 >= 0)
        pixels[idx - 1] += CHSV(random(firework.hue - 18, firework.hue + 18), 255, 192);
      if (idx - 2 >= 0)
        pixels[idx - 2] += CHSV(random(firework.hue - 18, firework.hue + 18), 255, 129);
      if (idx - 3 >= 0)
        pixels[idx - 3] = CHSV(random(firework.hue - 18, firework.hue + 18), 255, 66);

      // apply physics
      firework.pos += firework.vel;
      firework.vel *= firework.decel;

      // don't let the firework stall
      if (firework.vel < 0.1) firework.alive = false;

      fireballs[i] = firework;
    } else if (firework.pos >= PIXEL_COUNT) {
      // kill fireworks off top of stalks
      firework.alive = false;
    }
  }

  FastLED.show();

  // fade out tails and trails
  fadeToBlackBy(&(pixels[0]), PIXEL_COUNT, 32);
}


/* == BOUNCEBALL == */
const float BOUNCEBALL_STALK_HEIGHT = 20000; // pretend they are 12 foot high

struct Ball {
  int hue;
  float pos;
  float vel;
  float accel;
};
struct Ball ball;

void bounceballSetup() {
  ball.hue = (byte)random(256);
  ball.pos = BOUNCEBALL_STALK_HEIGHT - 0.01;
  ball.vel = 0;
  ball.accel = -9.8;
}

void bounceball() {
  // clear each frame
  // fill_solid(&(pixels[0]), PIXEL_COUNT, CRGB(0, 0, 0));

  // map ball position in physical space to pixel array
  int idx = floor(ball.pos * (PIXEL_COUNT / BOUNCEBALL_STALK_HEIGHT));
  if (idx >= 0 && idx < PIXEL_COUNT) {
    // balls are 3px tall - bottom of ball is position
    pixels[idx] = CHSV(ball.hue, 255, 255);

    // apply physics
    ball.pos += ball.vel;
    ball.vel += ball.accel;

    if (ball.pos <= 0) {
      // bounce!
      if (ball.vel > -130) {
        // kill it from bouncing all day
        ball.vel = 0;
        ball.pos = 0;
        ball.accel = 0;
      } else {
        ball.vel *= -0.8;
        ball.pos = -ball.pos;
      }
    }
  }

  FastLED.show();

  fadeToBlackBy(&(pixels[0]), PIXEL_COUNT, 32);
}


/* == PLASMA == */
const float PLASMA_SPEED = 0.01;
float plasmaAngle = 0;

struct Point {
  float x;
  float y;
};

void plasmaSetup() {
}

void plasma() {
  plasmaAngle += PLASMA_SPEED;

  Point p1 = { PIXEL_COUNT * 0.5 * ( 1.0 + sin(plasmaAngle) ), 0.0 };
  Point p2 = { PIXEL_COUNT * 0.5 * ( 1.0 + sin(plasmaAngle + 1.77) ), 0.0 };

  for (int i = 0; i < PIXEL_COUNT; i++) {
    float pX = float(i);

    float d1 = sqrt( (pX - p1.x) * (pX - p1.x) + (p1.y * p1.y) );
    float d2 = sqrt( (pX - p2.x) * (pX - p2.x) + (p2.y * p2.y) );

    float effector = sin(d1 * d1) + 2.0 * 0.5;

    float color1 = d1 * effector;
    float color2 = d2 * effector;
    float color3 = 0;

    pixels[i] = CRGB((int)color1, (int)color2, (int)color3);
  }

  FastLED.show();
}









// *** FIRE ***
// http://www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/#fire
// const int FIRE_COOLING = 55;
// const int FIRE_SPARK_CHANCE = 120;
// const int FIRE_FRAME_HOLD = 20; // ms
//
// void fire() {
//   static byte heat[PIXEL_COUNT];
//   int cooldown;
//
//   // 1: cool down every cell a little
//   for (int i = 0; i < PIXEL_COUNT; i++) {
//     // ??? what is happening here
//     cooldown = random(0, ((FIRE_COOLING * 10) / PIXEL_COUNT) + 2);
//
//     if (cooldown > heat[i]) {
//       heat[i] = 0;
//     } else {
//       heat[i] -= cooldown;
//     }
//   }
//
//   // 2: heat from each cell drifts 'up' and diffuses a little
//   for (int i = PIXEL_COUNT - 1; i >= 2; i--) {
//     // shift heat upwards
//     heat[i] = (heat[i - 1] + heat[i - 2] + heat[i -2]) / 3;
//   }
//
//   // 3: randomly ignite new 'sparks' near the bottom
//   if (random(255) < FIRE_SPARK_CHANCE) {
//     // only the bottom 1/8th of pixels can ignite
//     int i = random(PIXEL_COUNT * 0.125);
//     heat[i] = heat[i] + random(160, 255);
//   }
//
//   // 4: display current fire status
//   for (int i = 0; i < PIXEL_COUNT; i++) {
//     setPixelHeatColor(i, heat[i]);
//   }
//
//   FastLED.show();
//   delay(FIRE_FRAME_HOLD);
// }
//
// void setPixelHeatColor(int pixel, byte temp) {
//   // Scale 'heat' down from 0-255 to 0-191
//   byte t192 = round((temp/255.0)*191);
//
//   // calculate ramp up from
//   byte heatramp = t192 & 0x3F; // 0..63
//   // heatramp <<= 2; // scale up to 0..252
//
//   // figure out which third of the spectrum we're in:
//   if( t192 > 0x80) {                     // hottest
//     pixels[pixel] = CRGB(255, 128, heatramp);
//   } else if( t192 > 0x40 ) {             // middle
//     pixels[pixel] = CRGB(255, heatramp, 0);
//   } else {                               // coolest
//     pixels[pixel] = CRGB(heatramp, 0, 0);
//   }
// }


/* == PAPARAZZI == */
const int PAPARAZZI_FRAME_HOLD = 20; // ms

void paparazzi() {
  // clear leds
  fill_solid(&(pixels[0]), PIXEL_COUNT, CRGB(0, 0, 0));

  // pick random leds
  for (int i = 0; i < 8; i++){
    int randomNum = random(0, PIXEL_COUNT);
    setPixel(randomNum, 0xff, 0xff, 0xff);
  }

  FastLED.show();
  delay(PAPARAZZI_FRAME_HOLD);
}

/* == TAKEOFF == */
void takeOff() {
  panBar(1, 0, NUM_LEDS);
  panBar(1, 0, NUM_LEDS, 2);
  panBar(2, 0, NUM_LEDS, 3);
  panBar(2, 0, NUM_LEDS, 4);
  panBar(4, 0, NUM_LEDS, 5);
  panBar(4, 0, NUM_LEDS, 6);
  panBar(8, 0, NUM_LEDS, 7);
  panBar(8, 0, NUM_LEDS, 8);
  panBar(16, 0, NUM_LEDS, 9);
  panBar(16, 0, NUM_LEDS, 10);
  panBar(16, 0, NUM_LEDS, 10);
  panBar(16, 0, NUM_LEDS, 15);
  panBar(16, 0, NUM_LEDS, 15);
  panBar(16, 0, NUM_LEDS, 15);
  panBar(1, NUM_LEDS, 0);
}

void panBar(int barWidth, int start, int finish, int fpsMultiplier = 1){
  //
  // if( start < finish ){
  //
  //   // up
  //   for(int i=start; i<finish; i++){
  //     drawBar(barWidth, i);
  //     runFPS(fpsMultiplier);
  //   }
  // }else{
  //
  //   // down
  //   for(int i=start; i>finish; i--){
  //     drawBar(barWidth, i);
  //     runFPS(fpsMultiplier);
  //   }
  // }
}

void drawBar(int barWidth, int start) {
  //
  // // turn off all leds
  // for (int k=0; k < NUM_LEDS; k++){
  //   setPixel(k, 0x00, 0x00, 0x00);
  // }
  //
  // // draw bar
  // for (int i=start; i < start + barWidth; i++){
  //   setPixel(i, 0x00, 0xff, 0xff);
  // }
  //
  // showStrip();
}






/* == VU METER == */
void vuMeter() {
  int peak = audioRMS * PIXEL_COUNT; // RMS = 0 - 1
  for (int i = 0; i < PIXEL_COUNT; i++) {
    int hue = ((float)i / (float)PIXEL_COUNT) * 255.0;
    pixels[i] = CHSV(hue, 255, i < peak ? 255 : 0); // turn on pixels below mapped peak
  }
}


/* == RAINBOW CYCLE == */
void rainbowCycle() {
  float angle = (elapsed / 20) % 255; // offset starting angle over time
  float angleStep = 255 / PIXEL_COUNT; // show whole color wheel across strip
  for (int i = 0; i < PIXEL_COUNT; i++){
    pixels[i] = CHSV(angle, 255, 255);
    angle += angleStep;
  }

  FastLED.show();
}




// === MODES ===
void setMode(int newMode) {
  controlMode = newMode;
  if (controlMode == 0) {
    // BLUE mode - select animation
    ringcoder.setEncoderRange(ANIMATION_COUNT);
    ringcoder.writeEncoder(currentAnimationIdx);
    ringcoder.ledRingFollower();
    currentAnimationIdx == 0 ? ringcoder.setKnobRgb(255, 255, 255) : ringcoder.setKnobRgb(0, 0, 255);
  }
  else if (controlMode == 1) {
    // GREEN mode - select brightness
    // don't require user to click ever value - skip every 4
    // add a few ticks so we don't flip from brightess to darkest abruptly
    ringcoder.setEncoderRange(MAX_PIXEL_BRIGHTNESS / 4);
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
    if (currentAnimationIdx != (int)newValue) {
      switchAnimation(newValue);
    }
    ringcoder.ledRingFollower();
    currentAnimationIdx == 0 ? ringcoder.setKnobRgb(255, 255, 255) : ringcoder.setKnobRgb(0, 0, 255);
  }
  else if (controlMode == 1) {
    brightness = newValue * 4; // skipping every 4th value
    if (brightness > MAX_PIXEL_BRIGHTNESS) brightness = MAX_PIXEL_BRIGHTNESS;
    FastLED.setBrightness(brightness);
    ringcoder.ledRingFiller();
  }
  else if (controlMode == 2) {
    fftGain = newValue + FFT_GAIN_MIN;
    ringcoder.ledRingFiller();
  }
}


// === Utility Functions ===
void setPixel(int idx, byte r, byte g, byte b) {
  pixels[idx] = CRGB(r, g, b);
}

float lerp(float start, float end, float percent) {
  return start + percent * (end - start);
}

/*
float BasicAnimation::lerp(float start, float end, float percent) {
  return start + percent * (end - start);
}

float BasicAnimation::map(float value, float istart, float istop, float ostart, float ostop) {
  return ostart + (ostop - ostart) * ((value - istart) / (istop - istart));
}

int BasicAnimation::map(int value, int istart, int istop, int ostart, int ostop) {
  return ostart + (ostop - ostart) * ((value - istart) / (istop - istart));
}
*/
