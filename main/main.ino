/*
 * ===============
 * == FireStalk ==
 * ===============
 */
#define FASTLED_ALLOW_INTERRUPTS 0    // FastLED and

#include <FastLED.h>
#include "RingCoder.h"
#include "Audio.h"


#define SERIAL_DEBUG        false

/* Status LED */
const int STATUS_LED = 13;

/* Pixels */
const int MAX_PIXEL_BRIGHTNESS = 230;
int brightness = MAX_PIXEL_BRIGHTNESS / 4;
const int PIXEL_COUNT = 60;
const int NUM_LEDS = 60;
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
const float AUDIO_GAIN_MIN = 1.0;
const float AUDIO_GAIN_MAX = 32.0;
float audioGain = 24.0;
AudioInputAnalog        adc; // default: A2 - pin 16
AudioAnalyzeRMS         rms;
AudioConnection         patchCord2(adc, rms);
double audioRMS = 0.0;

/* Animation Globals */
const int BASE_ANIMATION_FRAME_HOLD = 20; // 50 fps
float animationSpeedMultiplier = 1.0;
elapsedMillis elapsed;

/* Mode */
int controlMode = 0;    // 0 (BLU) == set ANIMATION mode
                        // 1 (GRN) == set LED brightness
                        // 2 (RED) == set FFT gain


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

  // audio processing
  AudioMemory(2);

  // set control mode
  setMode(0);
}

void loop() {
  // main loop blocked by animations - each animation needs to call a special
  // update function to make sure important polling code is handled
  runCurrentAnimation();
}

void holdFrame() {
  holdFrame(1);
}

void holdFrame(float fpsMultiplier) {
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

  // process new audio Peak value
  if (rms.available()) {
    // audio hits hard upwards and tails down
    double val = rms.read() * (0.333 * audioGain);
    if (val > audioRMS)
      audioRMS = val;
    else
      audioRMS = lerp(audioRMS, (float)val, 0.05);
  }

  // output values for Processing prototype visualization
  if (SERIAL_DEBUG) {
    Serial.println(audioRMS);

    // Serial.print(animationSpeedMultiplier);
    // Serial.print(" ");
    // Serial.print(audioGain);
    // Serial.println();
  }

  // apply animation timing multiplier
  float mult = (1.0 / animationSpeedMultiplier) / fpsMultiplier;
  FastLED.delay((int)(BASE_ANIMATION_FRAME_HOLD * mult));
}

// =============================================================================
// === ANIMATIONS ==============================================================
// =============================================================================
const int ANIMATION_COUNT = 16;
typedef enum {
  CONVERGE,
  PANBACKFORTH,
  PAPARAZZI,
  TAKEOFF,
  STEPPER,
  THREEBARS,
  RANDOMFADINGBARS,
  VEGAS,
  KITT,
  // RAIN,
  BLACKOUT,
  // ===========
  FIREWORK,
  INTERFERENCE,
  FIRE,
  BOUNCEBALL,
  PLASMA,
  VUMOVER,

  RAINBOW,
} ANIMATIONS;
int currentAnimationIdx = 0;

void runCurrentAnimation() {
  // AUTO MODE - cycle animations randomly
  if (ANIMATION_COUNT == 1) {
    currentAnimationIdx = 0;
  } else {
    int tryIndex;
    do {
      tryIndex = random(0, ANIMATION_COUNT);
    } while (tryIndex == currentAnimationIdx);
    currentAnimationIdx = tryIndex;
  }

  int loop = 0;
  switch(currentAnimationIdx) {
    case CONVERGE:
      converge();
      break;
    case PANBACKFORTH:
      panBarBackForth();
      break;
    case PAPARAZZI:
      for (; loop < 200; ++loop)
        paparazzi();
      break;
    case TAKEOFF:
      takeOff();
      break;
    case STEPPER:
      for (; loop < 2; ++loop)
        stepper();
      break;
    case THREEBARS:
      for (; loop < 300; ++loop)
        threeBars();
      break;
    case RANDOMFADINGBARS:
      for (; loop < 14; ++loop)
        randomFadingBars();
      break;
    case VEGAS:
      for (; loop < 2; ++loop)
        vegas();
      break;
    case KITT:
      for (; loop < 4; ++loop)
        kitt();
      break;
    // case RAIN:
    //   for (; loop < 10; ++loop)
    //     rain();
    //   break;
    case BLACKOUT:
      for (; loop < 8; ++loop)
        blackout();
      break;
    //=========================
    case FIREWORK:
      for(; loop < 800; ++loop)
        fireworks();
      break;
    case INTERFERENCE:
      interferenceSetup();
      for(; loop < 600; ++loop)
        interference();
      break;
    case FIRE:
      for(; loop < 400; ++loop)
        firewalkwithme();
      break;
    case BOUNCEBALL:
      bounceballSetup();
      for(; loop < 600; ++loop)
        bounceball();
      break;
    case PLASMA:
      for(; loop < 300; ++loop)
        plasma();
      break;
    case VUMOVER:
      vumoverSetup();
      for(; loop < 400; ++loop)
        vuMover();
      break;

    case RAINBOW:
    default:
      for (; loop < 100; ++loop)
        rainbowCycle();
  }
}


// some fun globals I don't quite understand
int bluey = 0;
int redy = 54;
int greeny = 20;
int red = 0;
int green = 255;
int blue = 0;

// color palette
byte colors[8][3] = { {0, 0, 255},   // blue
                    {255, 255, 255}, // white
                    {0, 255, 100},   // teal
                    {200, 50, 250},  // violet
                    {50, 30, 150},   // purple
                    {250, 50, 0},    // orange
                    {0, 255, 50},    // green
                    {0, 255, 255} }; // light blue


/* == CONVERGE == */
void converge() {
  chooseRandomColor();

  panBar(1, 0, NUM_LEDS / 2);
  delay(300);
  panBar(1, NUM_LEDS-1, NUM_LEDS / 2 - 1);
  delay(300);
  panBar(1, NUM_LEDS / 4, (NUM_LEDS / 4) * 3);
  delay(300);
  panBar(1, (NUM_LEDS / 4) * 3, NUM_LEDS / 4);
  delay(300);
  panBar(1, 0, NUM_LEDS, 2);
  delay(150);
  panBar(1, 0, NUM_LEDS, 2);
  delay(150);
  panBar(1, NUM_LEDS, 0, 4);
  panBar(1, NUM_LEDS, 0, 4);
  panBar(1, NUM_LEDS, 0, 4);
  // panBar(NUM_LEDS, NUM_LEDS, 0, 4);
  // delay(300);
}


/* == PAPARAZZI == */
void paparazzi() {
  // pick random leds
  for (int i = 0; i < 3; i++) {
    int randomNum = random(0, PIXEL_COUNT);
    setPixel(randomNum, 0xff, 0xff, 0xff);
  }

  FastLED.show();

  // fade out
  holdFrame(0.5);
  fadeToBlackBy(&(pixels[0]), PIXEL_COUNT, 120);
}

/* == PANBAR == */
const int PANBAR_BAR_SIZE = 6;

void panBarBackForth() {
  // pan up
  for (int k = 0; k < NUM_LEDS - PANBAR_BAR_SIZE; k++) {
    fadeToBlackBy(&(pixels[0]), PIXEL_COUNT, 80);
    fill_solid(&(pixels[k]), PANBAR_BAR_SIZE, CRGB(255, 255, 255));
    FastLED.show();
    holdFrame(1);
  }

  // pan down
  for (int k = NUM_LEDS - PANBAR_BAR_SIZE; k > 0; k--) {
    fadeToBlackBy(&(pixels[0]), PIXEL_COUNT, 80);
    fill_solid(&(pixels[k]), PANBAR_BAR_SIZE, CRGB(255, 255, 255));
    FastLED.show();
    holdFrame(1);
  }
}

/* == TAKEOFF == */
void takeOff() {
  chooseRandomColor();

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


/* == STEPPER == */
void stepper() {
  chooseRandomColor();

  panBar(1, 0, NUM_LEDS / 4);
  delay(300);
  panBar(1, NUM_LEDS / 4, NUM_LEDS / 2);
  delay(300);
  panBar(1, NUM_LEDS / 2, NUM_LEDS / 1.33);
  delay(300);
  panBar(1, NUM_LEDS / 1.33, NUM_LEDS);
  panBar(1, NUM_LEDS, 1, 40);
}


/* == THREEBARS == */
void threeBars() {
  // blue
  for (int k=bluey; k < bluey + 3; k++){
    setPixel(k, float(0), float(0), float(256));
  }

  // red
  for (int i=redy; i < redy + 6; i++){
    setPixel(i, 0xFF, 0x00, 0x00);
  }

  // green
  setPixel(greeny, 0x00, 0xFF, 0x00);

  showStrip();
  holdFrame();

  //clear
  for(int i=0; i<NUM_LEDS; i++){
    setPixel(i, 0x00, 0x00, 0x00);
  }

  bluey = bluey + 1.5;
  redy --;
  greeny = greeny + 2;

  if(bluey >= NUM_LEDS-1){
    bluey = -6;
  }
  if(redy <= -6){
    redy= NUM_LEDS-1;
  }
  if(greeny > NUM_LEDS-1){
    greeny = 0;
  }
}


/* == RANDOMFADINGBARS == */
void randomFadingBars() {
  int p = random(0,NUM_LEDS);
  int s = random(6, 20);

  for(int i=0; i<255; i=i+15){
    for(int k=p; k<p+s; k++){
      setPixel(k, float(0), float(0), float(i));
    }
    showStrip();
    holdFrame();
  }

  for(int i=255; i>0; i=i-15){
    for(int k=p; k<p+s; k++){
      setPixel(k, float(0), float(0), float(i));
    }
    showStrip();
    holdFrame();
  }

  //reset all
  fill_solid(&(pixels[0]), NUM_LEDS, CRGB(0, 0, 0));
}


/* == VEGAS == */
void vegas() {
  // teal
  red = 0; green = 255; blue = 100;

  panBar(4, NUM_LEDS, 0, 1, false);
  delay(100);

  for (int i = 0; i < NUM_LEDS; i = i+8) {
    drawBar(4, i, false);
    delay(100);
  }

  //magenta
  red = 255; green = 0; blue = 37;
  panBar(1, NUM_LEDS, -1, 1, false);

  red = 0; green = 0; blue = 0;
  for(int i=0; i<NUM_LEDS; i=i+8){
    drawBar(4, i, false);
    delay(100);
  }
}


/* == KITT == */
void kitt() {
  for(int i=0; i<NUM_LEDS/2; i++){

    //purple
    red = 50; green = 30; blue = 150;
    drawBar(1, i, false);
    drawBar(1, NUM_LEDS - 1 - i, false);
    delay(30);
  }

  for(int i=NUM_LEDS/2; i>=0; i--){

    //black
    red = 0; green = 0; blue = 0;
    drawBar(1, i, false);
    drawBar(1, NUM_LEDS - 1 - i, false);
    delay(30);
  }

  for(int i=NUM_LEDS/2; i>=0; i--){

    //orange
    red = 250; green = 50; blue = 0;
    drawBar(1, i, false);
    drawBar(1, NUM_LEDS - 1 - i, false);
    delay(30);
  }

  for(int i=0; i<NUM_LEDS/2; i++){

    //black
    red = 0; green = 0; blue = 0;
    drawBar(1, i, false);
    drawBar(1, NUM_LEDS - 1 - i, false);
    delay(30);
  }
}


/* == RAIN == */
void rain() {
int randomSize = random(6, 16);
int randomStart = random(0, NUM_LEDS);

//teal
red = 0; green = 255; blue = 100;

for(int i=randomStart; i>randomStart - randomSize; i--){
drawBar(1, i, false);
delay(20);
}

randomSize = random(6, 16);
randomStart = randomStart - randomSize;

//blue
red = 0; green = 50; blue = 200;

for(int i=randomStart; i>randomStart - randomSize; i--){
drawBar(1, i, false);
delay(20);
}

randomSize = random(6, 16);
randomStart = random(0, NUM_LEDS);

//black
red = 0; green = 0; blue = 0;

for(int i=randomStart; i>randomStart - randomSize; i--){
drawBar(1, i, false);
delay(20);
}

// random clearing
int seed = random(0,100);

if(seed > 85){

//black
red = 0; green = 0; blue = 0;

for(int i=0; i<NUM_LEDS; i++){
  drawBar(1, i, false);
  delay(15);
}
}

//violet
red = 200; green = 50; blue = 250;

for(int i=randomStart; i>randomStart - randomSize; i--){
drawBar(1, i, false);
delay(20);
}

randomSize = random(6, 16);
randomStart = random(0, NUM_LEDS);

//black
red = 0; green = 0; blue = 0;

for(int i=randomStart; i>randomStart - randomSize; i--){
drawBar(1, i, false);
delay(20);
}
}


/* == BLACKOUT == */
void blackout() {
  for(int j=0; j<255; j++){
    setAll(j,j/5,j);
    delay(2);
  }

  for(int k=0; k<NUM_LEDS * 2; k++){
    red = 0; green = 0; blue = 0;
    drawBar(1, random(0,NUM_LEDS), false);
    delay(2);
  }
}


/* == BEN'S ANIMATION HELPERS == */
void panBar(int barWidth, int start, int finish) {
  panBar(barWidth, start, finish, 1, true);
}

void panBar(int barWidth, int start, int finish, int fpsMultiplier) {
  panBar(barWidth, start, finish, fpsMultiplier, true);
}

void panBar(int barWidth, int start, int finish, int fpsMultiplier, boolean clearAll) {
  if (start < finish) {
    // up
    for (int i = start; i < finish; i++) {
      drawBar(barWidth, i, clearAll);
      holdFrame(fpsMultiplier);
    }
  } else {
    // down
    for (int i = start; i > finish; i--) {
      drawBar(barWidth, i, clearAll);
      holdFrame(fpsMultiplier);
    }
  }
}

void drawBar(int barWidth, int start, boolean clearAll) {
  if (clearAll){
    // turn off all leds
    // fill_solid(&(pixels[0]), PIXEL_COUNT, CRGB(0, 0, 0));
    fadeToBlackBy(&(pixels[0]), PIXEL_COUNT, 200);
  }

  // draw bar
  for (int i=start; i < start + barWidth; i++){
    setPixel(i, red, green, blue);
  }

  FastLED.show();
}

void chooseRandomColor() {
  // pick random color
  int randomColor = random(0, 8); // 0-7
  red = colors[randomColor][0];
  green = colors[randomColor][1];
  blue = colors[randomColor][2];
}

void showStrip() {
  FastLED.show();
}

void setAll(byte red, byte green, byte blue) {
  fill_solid(&(pixels[0]), NUM_LEDS, CRGB(red, green, blue));
  FastLED.show();
}


// =============================================================================


/* == FIREWORKS == */
struct Firework {
  boolean alive;
  int hue;
  float pos;
  float vel;
  float decel;
};

const int FIREWORK_MAX_VISIBLE = 12;
const int FIREWORK_MAX_EJECT_TIME = 800;
const int FIREWORK_MIN_EJECT_TIME = 200;
const float FIREWORK_AUDIO_THRESHOLD = 0.8;
long lastFireballEjectMillis = 0;
Firework fireballs[FIREWORK_MAX_VISIBLE];
int fireworkIndex = 0;

void ejectFireball() {
  Firework f;
  f.alive = true;
  f.hue = (byte)random(256);
  f.pos = 0;
  // f.vel = 0.9 + random(1000)/500.0;
  f.vel = 0.6 + (random(100) / 100.0) + (audioRMS * 1.5);
  f.decel = 0.972;
  fireballs[fireworkIndex++] = f;
  if (fireworkIndex > FIREWORK_MAX_VISIBLE - 1)
    fireworkIndex = 0;

  lastFireballEjectMillis = elapsed;
}

void fireworks() {
  // don't always wait for audio hits
  if (elapsed - lastFireballEjectMillis > FIREWORK_MAX_EJECT_TIME) {
    ejectFireball();
  } else if (audioRMS > FIREWORK_AUDIO_THRESHOLD && elapsed - lastFireballEjectMillis > FIREWORK_MIN_EJECT_TIME) {
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

  holdFrame();
}


/* == INTERFERENCE == */
// color bands caused by interference patterns of overlapping waves
// https://i.ytimg.com/vi/sXlYmLQdJU4/maxresdefault.jpg
const float INTERFERENCE_ANGLE_SWEEP = 0.75;
const float INTERFERENCE_SPEED = 0.3;
int interferenceHue1;
int interferenceHue2;
float interferenceWave1 = 0;
float interferenceWave2 = 0;

void interferenceSetup() {
  interferenceHue1 = random(0, 255);
  interferenceHue2 = random(0, 255);
}

void interference() {
  // clear frame
  fill_solid(&(pixels[0]), PIXEL_COUNT, CRGB(0, 0, 0));

  // wave 1 - sine
  float angle = interferenceWave1;
  float bri;
  for (int i = 0; i < PIXEL_COUNT; i++) {
    bri = map(sin(angle), -1.0, 1.0, 0.0, 200.0);
    pixels[i] += CHSV(interferenceHue1, 128+bri*0.5, bri); // additive
    angle += INTERFERENCE_ANGLE_SWEEP;
  }

  // wave 2 - sine with PI/2 phase angle
  angle = interferenceWave2;
  for (int i = 0; i < PIXEL_COUNT; i++) {
    bri = map(sin(angle), -1.0, 1.0, 0.0, 200.0);
    pixels[i] += CHSV(interferenceHue2, 128+bri*0.5, bri); // additive
    angle += INTERFERENCE_ANGLE_SWEEP; // sweep opposite direction
  }

  interferenceWave1 += INTERFERENCE_SPEED;
  interferenceWave2 += INTERFERENCE_SPEED * 1.05;

  FastLED.show();
  holdFrame();
}


// *** FIRE ***
// http://www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/#fire
const int FIRE_COOLING = 45;
const int FIRE_SPARK_CHANCE_MAX = 190;
const int FIRE_SPARK_CHANCE_MIN = 120;

void firewalkwithme() {
  static byte heat[PIXEL_COUNT];
  int cooldown;

  // 1: cool down every cell a little
  for (int i = 0; i < PIXEL_COUNT; i++) {
    cooldown = random(0, ((FIRE_COOLING * 10) / PIXEL_COUNT) + 2);

    if (cooldown > heat[i]) {
      heat[i] = 0;
    } else {
      heat[i] -= cooldown;
    }
  }

  // 2: heat from each cell drifts 'up' and diffuses a little
  for (int i = PIXEL_COUNT - 1; i >= 2; i--) {
    // shift heat upwards
    heat[i] = (heat[i - 1] + heat[i - 2] + heat[i -2]) / 3;
  }

  // 3: randomly ignite new 'sparks' near the bottom
  // audio RMS changes randomness
  int sparkChance = (int)map(audioRMS, 0.0, 1.0, (float)FIRE_SPARK_CHANCE_MIN, (float)FIRE_SPARK_CHANCE_MAX);
  if (random(255) < sparkChance) {
    // only the bottom 1/8th of pixels can ignite
    int i = random(PIXEL_COUNT * 0.125);
    heat[i] = heat[i] + random(160, 255);
  }

  // 4: display current fire status
  for (int i = 0; i < PIXEL_COUNT; i++) {
    pixels[i] = HeatColor(heat[i]);
  }

  FastLED.show();
  holdFrame(0.6);
}


/* == BOUNCEBALL == */
const float BOUNCEBALL_STALK_HEIGHT = 30000; // pretend they are 12 foot high

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
      if (ball.vel > -10) {
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
  holdFrame(3.0);

  fadeToBlackBy(&(pixels[0]), PIXEL_COUNT, 32);
}


/* == PLASMA == */
// https://github.com/johncarl81/neopixelplasma
const float PLASMA_SPEED = 0.03;
float plasmaAngle = 0;

struct Point {
  float x;
  float y;
};

void plasma() {
  plasmaAngle += PLASMA_SPEED;

  float halfPixels = PIXEL_COUNT * 0.5;
  Point p1 = { halfPixels * (float)( 1.0 + sin(plasmaAngle * 1.000) ), (float)(sin(plasmaAngle*1.310)+1.0) * 0.3 * halfPixels };
  Point p2 = { halfPixels * (float)( 1.0 + sin(plasmaAngle * 1.770) ), (float)(sin(plasmaAngle*2.865)+1.0) * 0.3 * halfPixels };
  Point p3 = { halfPixels * (float)( 1.0 + sin(plasmaAngle * 0.250) ), (float)(sin(plasmaAngle*0.750)+1.0) * 0.3 * halfPixels };

  for (int i = 0; i < PIXEL_COUNT; i++) {
    float pX = float(i);

    float d1 = sqrt( (pX - p1.x) * (pX - p1.x) + (p1.y * p1.y) );
    float d2 = sqrt( (pX - p2.x) * (pX - p2.x) + (p2.y * p2.y) );
    float d3 = sqrt( (pX - p3.x) * (pX - p3.x) + (p3.y * p3.y) );

    float color1 = 0.3 * d1 * d1;
    float color2 = 0.3 * d2 * d2;
    float color3 = 0.3 * d3 * d3;

    pixels[i] = CRGB((int)color1, (int)color2, (int)color3);
  }

  FastLED.show();
  holdFrame(0.5);
}


/* == VUMOVER == */
float vumoverAngle = 0;
float VUMOVER_ANGLE_SWEEP = 0.3;
int vumoverHue1;
int vumoverHue2;

void vumoverSetup() {
  vumoverHue1 = random(0, 255);
  vumoverHue2 = random(0, 255);
}

void vuMover() {
  // clear frame
  fill_solid(&(pixels[0]), PIXEL_COUNT, CRGB(0, 0, 0));

  // wave 1 - sine
  float angle = vumoverAngle;
  float bri;
  for (int i = 0; i < PIXEL_COUNT; i++) {
    bri = map(sin(angle), -1.0, 1.0, 0.0, 200.0);
    pixels[i] += CHSV(vumoverHue1, 128+bri*0.5, bri); // additive
    angle -= VUMOVER_ANGLE_SWEEP;
  }

  // wave 2 - sine with PI/2 phase angle
  angle = vumoverAngle + PI/2.0;
  for (int i = 0; i < PIXEL_COUNT; i++) {
    bri = map(sin(angle), -1.0, 1.0, 0.0, 200.0);
    pixels[i] += CHSV(vumoverHue2, 128+bri*0.5, bri); // additive
    angle += VUMOVER_ANGLE_SWEEP; // sweep opposite direction
  }

  FastLED.show();
  holdFrame();

  vumoverAngle += 0.1 + 0.35 * audioRMS;
}


/* == RAINBOW CYCLE (test) == */
void rainbowCycle() {
  float angle = (elapsed / 2) % 255; // offset starting angle over time
  float angleStep = 255 / PIXEL_COUNT; // show whole color wheel across strip
  for (int i = 0; i < PIXEL_COUNT; i++){
    pixels[i] = CHSV(angle, 255, 255);
    angle += angleStep;
  }

  FastLED.show();
  holdFrame();
}


// === MODES ===
void setMode(int newMode) {
  controlMode = newMode;
  if (controlMode == 0) {
    // BLUE mode - animation speed multiplier
    ringcoder.setEncoderRange(32);
    ringcoder.writeEncoder(map(animationSpeedMultiplier, 0.25, 2.0, 0, 31));
    ringcoder.ledRingFiller();
    ringcoder.setKnobRgb(0, 0, 255);
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
    // RED mode - select audio gain
    ringcoder.setEncoderRange(AUDIO_GAIN_MAX - AUDIO_GAIN_MIN);
    ringcoder.writeEncoder(audioGain);
    ringcoder.ledRingFiller();
    ringcoder.setKnobRgb(255, 0, 0);
  }
}

void updateModeDisplay(u_int newValue) {
  if (controlMode == 0) {
    animationSpeedMultiplier = map(newValue, 0, 32, 0.25, 2.0);
    ringcoder.ledRingFiller();
  }
  else if (controlMode == 1) {
    brightness = newValue * 4; // skipping every 4th value
    if (brightness > MAX_PIXEL_BRIGHTNESS) brightness = MAX_PIXEL_BRIGHTNESS;
    FastLED.setBrightness(brightness);
    ringcoder.ledRingFiller();
  }
  else if (controlMode == 2) {
    audioGain = newValue + AUDIO_GAIN_MIN;
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

float map(float value, float istart, float istop, float ostart, float ostop) {
  return ostart + (ostop - ostart) * ((value - istart) / (istop - istart));
}
