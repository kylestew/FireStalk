
#include "RingCoder.h"
#define UNCONNECTED_PIN 14
RingCoder::RingCoder(int bPin, int aPin, 
                     int redPin, int bluPin, int grnPin, 
                     int swhPin, 
                     int datPin, int clrPin, int clkPin, 
                     int latchPin, int enPin) : 
  _latchPin(latchPin), _datPin(datPin), _clkPin(clkPin), _currentShift(0x0000), _range(LED_COUNT), _encoderPosition(0), _moved(false), _myEnc(bPin, aPin), _pushButton()

{

  _ledPins[RED] = redPin;
  _ledPins[BLUE] = bluPin;
  _ledPins[GREEN] = grnPin;

  setPushButtonPins(swhPin);
  setLedPins();
  setShiftRegisterPins(enPin, latchPin, clkPin, clrPin, datPin);

  randomSeed(analogRead(UNCONNECTED_PIN));
  setShift(0x0000);
}

inline int RingCoder::positive_modulo(int i, int n) {
  return (i % n + n) % n;
}

void RingCoder::setPushButtonPins(int swhPin) {
  // setup switch pins, set as an input, no pulled up
  pinMode(swhPin, INPUT);
  digitalWrite(swhPin, LOW);  // Disable internal pull-up
  _pushButton.attach(swhPin);
  _pushButton.interval(5);
}

void RingCoder::setEncoderRange(int range) {
  if (range > 0) {
    _range = range;
  }
}

void RingCoder::setLedPins() {
  // Setup led pins as outputs, and write their intial value.
  // initial value is defined by the ledValue global variable

  pinMode(_ledPins[RED], OUTPUT);
  analogWrite(_ledPins[RED], _ledValue[RED]);  // Red off
  pinMode(_ledPins[BLUE], OUTPUT);
  analogWrite(_ledPins[BLUE], _ledValue[BLUE]);  // Blue off
  pinMode(_ledPins[GREEN], OUTPUT);
  analogWrite(_ledPins[GREEN], _ledValue[GREEN]);  // Blue off
}

void RingCoder::setShiftRegisterPins(int enPin, int latchPin, int clkPin, int clrPin, int datPin) {
  // Setup shift register pins
  pinMode(enPin, OUTPUT);  // Enable, active low, this'll always be LOW
  digitalWrite(enPin, LOW);  // Turn all outputs on
  pinMode(latchPin, OUTPUT);  // this must be set before calling shiftOut16()
  digitalWrite(latchPin, LOW);  // start latch low
  pinMode(clkPin, OUTPUT);  // we'll control this in shiftOut16()
  digitalWrite(clkPin, LOW);  // start sck low
  pinMode(clrPin, OUTPUT);  // master clear, this'll always be HIGH
  digitalWrite(clrPin, HIGH);  // disable master clear
  pinMode(datPin, OUTPUT);  // we'll control this in shiftOut16()
  digitalWrite(datPin, LOW);  // start ser low
}

void RingCoder::blink(int singleLED) {
  unsigned int shift = _currentShift | calculateShift(false, singleLED);
  unsigned int offShift = _currentShift;
  setShift(shift);
  delay(250);
  setShift(offShift);
}

void RingCoder::blink() {
  unsigned int prev = _currentShift;
  unsigned int onShift = 0xFFFF;
  unsigned int offShift = 0x0000;

  setShift(onShift);
  delay(250);
  setShift(offShift);
  delay(250);
  setShift(prev);

}

void RingCoder::spin(bool reverse) {
  for(unsigned int i = 0; i < LED_COUNT; i++) {
    if (reverse) {
      setShift(32768 >> i); //32768 has highest bit (msb) 1 and all others 0
    } else {
      setShift(1 << i);
    }
    delay(1000/LED_COUNT);
  }
  setShift(0x0000);
}

void RingCoder::spin() {
  spin(false);
}

void RingCoder::reverse_spin() {
  spin(true);
}

//Wheel of fortune like spinner.  
//Will spin around 3 times and land on a random 
//number of the 4th, moving progressively slower 
//as it approaches the number.
//The speed of the slowdown is controlled by the rotary encoder's initial state
//This uses delays where it probably should use a combination "start" and "update"
//methods so it can be called in the sketch's loop() and allow for other interactions.
void RingCoder::spin_the_wheel() {
  //fast spin around 1-2 time, slower 3rd, land on 4th
  int pos = readEncoder();
  int scaleDelay = pos * 50 / _range;
  int min = LED_COUNT * 4;
  int max = LED_COUNT * 5;
  unsigned int randNumber = random(min, max);
  int lastLap = randNumber - LED_COUNT;
  for(unsigned int i = 0; i < randNumber; i++) {
    setShift(1 << i % LED_COUNT);
    delay(scaleDelay * 2);
    if (i > LED_COUNT * 2) {
      delay(scaleDelay);
    }
    if (i > LED_COUNT * 3) {
      delay(scaleDelay);
    }
    if (i > lastLap) {
      delay((i - lastLap) * scaleDelay);
    }
  }

}

void RingCoder::random_the_wheel() {
  int pos = readEncoder() + 1; //prevent 0
  random_the_wheel(pos);
}

//Blinks random LEDs as it approaches a final random value
void RingCoder::random_the_wheel(int multiplier) {
  int scaleDelay = multiplier * 50;
  unsigned int randNumber;
  for(unsigned int i = 0; i < LED_COUNT; i++) {
    randNumber = random(LED_COUNT);
    setShift(1 << randNumber);
    delay(scaleDelay);
  }

}

//Read/Write methods to adjust for hardware only settling on multiples of 4
int RingCoder::readEncoder() {
  int newPosition = positive_modulo( (_myEnc.read() / ENCODER_STEP), _range);
  _moved = (newPosition != _encoderPosition);
  _encoderPosition = newPosition;
  return _encoderPosition;
}

bool RingCoder::moved() {
  return _moved;
}

void RingCoder::writeEncoder(signed int newPosition) {
  _encoderPosition = newPosition;
  _myEnc.write(_encoderPosition * ENCODER_STEP);
}

void RingCoder::setKnobRgb(int r, int g, int b) {
  int safe_r = positive_modulo(r, KNOB_LED_RANGE);
  int safe_g = positive_modulo(g, KNOB_LED_RANGE);
  int safe_b = positive_modulo(b, KNOB_LED_RANGE);
  //LEDs use 255-0 range

  analogWrite(_ledPins[RED], KNOB_LED_MAX - safe_r);
  analogWrite(_ledPins[GREEN], KNOB_LED_MAX - safe_g);
  analogWrite(_ledPins[BLUE], KNOB_LED_MAX - safe_b);
}

bool RingCoder::update() {
  return _pushButton.update();
}

int RingCoder::button() {
  return _pushButton.read();
}

void RingCoder::setShift(unsigned int ledOutput) {
  if (ledOutput != _currentShift) {
    _currentShift = ledOutput;
    digitalWrite(_latchPin, LOW);  // first send latch low
    shiftOut16(ledOutput);  // send the ledOutput value to shiftOut16
    digitalWrite(_latchPin, HIGH);  // send latch high to indicate data is done sending
  }
}

unsigned int RingCoder::calculateShift(bool fill) {
  return calculateShift(fill, LED_COUNT * _encoderPosition / _range);
}

unsigned int RingCoder::calculateShift(bool fill, int pos) {
  unsigned int ledOutput = 0;

  if (fill) {
    for (int i = 0; i < pos; i++){
      bitSet(ledOutput, i);
    }
  } else {
    bitSet(ledOutput, pos);
  }

  return ledOutput;
}

void RingCoder::ledRingFiller() {
  setShift(calculateShift(true));
}

void RingCoder::ledRingFollower() {
  setShift(calculateShift(false));
}

void RingCoder::shiftOut16(uint16_t data) {
  // Isolate the MSB and LSB
  byte datamsb = (data & 0xFF00) >> 8;  // mask out the MSB and shift it right 8 bits
  byte datalsb = (data & 0x00FF);  // Mask out the LSB

  // First shift out the MSB, MSB first.
  shiftOut(_datPin, _clkPin, MSBFIRST, datamsb);
  // Then shift out the LSB
  shiftOut(_datPin, _clkPin, MSBFIRST, datalsb);
}

