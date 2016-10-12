/*
 * https://www.sparkfun.com/products/11040
 * in conjunction with https://www.pjrc.com/teensy/td_libs_Encoder.html
*/
#ifndef RingCoder_h
#define RingCoder_h

#define LED_COUNT 16
#define ENCODER_STEP 4 //When you feel a single turn, its actually 4 signals

#define KNOB_LED_RANGE 256
#define KNOB_LED_MAX (KNOB_LED_RANGE - 1)

#define ENCODER_OPTIMIZE_INTERRUPTS

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#include <Encoder.h>
#include <Bounce2.h>


class RingCoder
{
  public:
    RingCoder(int bPin, int aPin, 
              int redPin, int bluPin, int grnPin, 
              int swhPin, 
              int datPin, int clrPin, int clkPin, 
              int latchPin, int enPin);
    int readEncoder();
    void writeEncoder(signed int newPosition);
    void setEncoderRange(int range);
    void blink(int led);
    void blink();
    void spin(bool reverse);
    void spin();
    void reverse_spin();
    void spin_the_wheel();
    void random_the_wheel();
    void random_the_wheel(int multiplier);
    void setKnobRgb(int r, int b, int g);

    bool update();
    int button();
    bool moved();

    void ledRingFiller();
    void ledRingFollower();
  private:
    enum ledCounter {RED = 0, BLUE = 1, GREEN = 2, NONE = 3};

    int _latchPin;
    int _datPin;
    int _clkPin;

    long _encoderPosition;
    bool _moved;
    byte _ledCount;
    byte _ledValue[3];
    byte _ledPins[3];

    unsigned int _currentShift;
    Bounce _pushButton;
    Encoder _myEnc;

    int _range;

    void setPushButtonPins(int swhPin);
    void setLedPins();
    void setShiftRegisterPins(int enPin, int latchPin, int clkPin, int clrPin, int datPin);

    void setShift(unsigned int ledOutput);

    inline int positive_modulo(int i, int n);
    unsigned int calculateShift(bool fill);
    unsigned int calculateShift(bool fill, int pos);

    void shiftOut16(uint16_t data);
};

#endif
