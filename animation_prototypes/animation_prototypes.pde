import processing.serial.*;


int PIXEL_COUNT = 64;
color[] pixels = new color[PIXEL_COUNT];

void setup() {
  size(160, 960);
  
  //serial = new Serial (this, Serial.list()[1], 115200);
  //serial.bufferUntil('\n');
  
  
}

void draw() {
  background(0);
  
  fireWalk();
}


/* == FIRE == */
int FIRE_COOLING = 55;
int FIRE_SPARK_CHANCE = 120;
int FIRE_FRAME_HOLD = 20; // ms
char[] heat = new char[PIXEL_COUNT];
void fireWalk() {
  // 1: cool down every cell a little
  int cooldown;
  for (int i = 0; i < PIXEL_COUNT; i++) {
    // ??? what is happening here
    cooldown = (int)random(0, ((FIRE_COOLING * 10) / PIXEL_COUNT) + 2);
    
    cooldown = 1;

    if (cooldown > heat[i]) {
      heat[i] = 0;
    } else {
      heat[i] -= cooldown;
    }
  }
  
  // 2: heat from each cell drifts 'up' and diffuses a little
  //for (int i = PIXEL_COUNT - 1; i >= 2; i--) {
  //  // shift heat upwards
  //  heat[i] = (char)((heat[i - 1] + heat[i - 2] + heat[i -2]) / 3);
  //}
  
  // 3: randomly ignite new 'sparks' near the bottom 
  if (random(255) < FIRE_SPARK_CHANCE) {
    // only the bottom 1/8th of the pixels can ignite
    int i = (int)random(PIXEL_COUNT * 0.125);
    int spark = (char)random(160, 255); 
    if (heat[i] + spark > 255) {
      heat[i] = 255;
    } else {
      heat[i] += spark
    }
  }
  
  // for java only - flip to negative value
  if (heat > 255) heat = 255;

  // 4: display current fire status
  for (int i = 0; i < PIXEL_COUNT; i++) {
    setPixelHeatColor(i, heat[i]);
  }
  println();
    
  updateDisplay(); // FastLED.show(); 
  delay(20 * FIRE_FRAME_HOLD);
}

void setPixelHeatColor(int pixel, char temp) {
  // scale 'heat' down from 0-255 to 0-191
  char t192 = (char)round((temp/255.0)*191);
  if (t192 > 191) t192 = 191;


    print(nf(t192, 4));
    print(" ");


  //// calculate ramp up from
  //byte heatramp = t192 & 0x3F; // 0..63
  //// heatramp <<= 2; // scale up to 0..252

  // figure out which third of the spectrum we're in
  if(t192 > 0x80) {                     // hottest
  //  pixels[pixel] = CRGB(255, 128, heatramp);
  } else if( t192 > 0x40 ) {            // middle
  //  pixels[pixel] = CRGB(255, heatramp, 0);
  } else {                              // coolest
  //  pixels[pixel] = CRGB(heatramp, 0, 0);
  }
}





void updateDisplay() {
  translate(64,32);
  float width = 32;
  float height = 12;
  for (int i = 0; i < PIXEL_COUNT; i++) {
    stroke(128);
    //fill(pixels[i]);
    fill(255, 0, 0);
    
    rect(0, 0, width, height);
    translate(0, height+2);
  }
}