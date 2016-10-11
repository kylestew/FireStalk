/* PINOUTS */

// encoder with RGB LED
const int ENCB = 2; // INT 0
const int ENCA = 3; // INT 1
const int LEDR = 5; // PWM enabled LED driver
const int LEDB = 6; // PWM enabled LED driver
const int LEDG = 9; // PWM enabled LED driver
const int SW = 7;

// ring led shift register
const int DAT = 8;
const int CLR = 10;
const int CLK = 11;
const int LATCH = 12;
const int EN = 13;

// globals
//int switchValue = 0;

enum ledCounter {RED = 0, BLUE = 1, GREEN = 2, NONE = 3};
byte ledValue[3] = {255, 255, 255};
byte ledPins[3] = {LEDR, LEDB, LEDG};




volatile int lastEncoded = 0;
volatile long encoderValue = 0;

long lastencoderValue = 0;

int lastMSB = 0;
int lastLSB = 0;



void setup() {
  Serial.begin(9600); // debugger
  
  // encoder 
  pinMode(ENCA, INPUT);  
  digitalWrite(ENCA, HIGH); // pull up input
  pinMode(ENCB, INPUT);
  digitalWrite(ENCB, HIGH);
  
  noInterrupts(); // let setup finish before firing interrupts
  attachInterrupt(0, readEncoder, CHANGE);
  attachInterrupt(1, readEncoder, CHANGE);
  
  // encoder RGB
  pinMode(LEDR, OUTPUT);
  analogWrite(LEDR, ledValue[RED]);
  pinMode(LEDG, OUTPUT);
  analogWrite(LEDR, ledValue[GREEN]);
  pinMode(LEDB, OUTPUT);
  analogWrite(LEDR, ledValue[BLUE]);
  
  // encoder SW
  pinMode(SW, INPUT);
  digitalWrite(SW, LOW); // disable internal pull-up
  
  // setup ring led shift register
  pinMode(EN, OUTPUT); // stays low to enable outputs
  digitalWrite(EN, LOW);
  pinMode(LATCH, OUTPUT);
  digitalWrite(LATCH, LOW);
  pinMode(CLK, OUTPUT); 
  digitalWrite(CLK, LOW);
  pinMode(CLR, OUTPUT); 
  digitalWrite(CLR, HIGH); // disable master clear
  pinMode(DAT, OUTPUT); 
  digitalWrite(DAT, LOW); // set ser low
  
  // turn off ring led
  writeToShiftRegister(0x0000);
  
  // listen for changes on encoder
  interrupts();
}

void loop() {
  /*
  int newSwitchValue = digitalRead(SW);
  if (switchValue != newSwitchValue) {
    switchValue = newSwitchValue;
    
    if (switchValue)
      shiftOut16(0xffff);
    else
      shiftOut16(0x0000);
  }
  */
  
  
  
   Serial.println(-1 * (encoderValue / 4));
  delay(20); //just here to slow down the output, and show it will work  even during a delay
  
  
  uint16_t ledRingValue = -1 * (encoderValue / 4);
  writeToShiftRegister(ledRingValue);
  
  
  // update LED states
  analogWrite(LEDR, ledValue[0]); // r
  analogWrite(LEDB, ledValue[1]); // b
  analogWrite(LEDG, ledValue[2]); // g
  ledValue[0] = ledValue[0] - 1;
  ledValue[1] = ledValue[1] - 3;
  ledValue[2] = ledValue[2] - 6;
  
  
  /*
  // animate the ring
  for(uint16_t i = 0; i < 65,535; ) {
    writeToShiftRegister(i);
    delay(500);
    i = (i << 1) + 1; // moved LED over by 1
  }
  */
}

/* Write output to shift registers driving LED ring */
void writeToShiftRegister(uint16_t data) {
  digitalWrite(LATCH, LOW);
  
  // Sending to two shift registers, need to break into parts
  byte datamsb;
  byte datalsb;
  
  // Isolate the MSB and LSB
  datamsb = (data&0xFF00)>>8;  // mask out the MSB and shift it right 8 bits
  datalsb = data & 0xFF;  // Mask out the LSB
  
  // First shift out the MSB, MSB first.
  shiftOut(DAT, CLK, MSBFIRST, datamsb);
  // Then shift out the LSB
  shiftOut(DAT, CLK, MSBFIRST, datalsb);
  
  digitalWrite(LATCH, HIGH);
}

// called whenever A or B pins (INT0 or INT1) are toggled
void readEncoder() {
  int MSB = digitalRead(ENCB); //MSB = most significant bit
  int LSB = digitalRead(ENCA); //LSB = least significant bit

  int encoded = (MSB << 1) |LSB; //converting the 2 pin value to single number
  int sum  = (lastEncoded << 2) | encoded; //adding it to the previous encoded value

  if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderValue ++;
  if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderValue --;

  lastEncoded = encoded; //store this value for next time
}
