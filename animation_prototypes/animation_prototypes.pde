import processing.serial.*;

Serial serial;

// pixels
int PIXEL_COUNT = 40;
color[] pixels = new color[PIXEL_COUNT];
color[] pixBuff = new color[PIXEL_COUNT];
float pixelFadeLerp = 0.03;

// animations
boolean firstFrame = true; // switch on animation mode switch
PImage img;


void setup() {
  size(160, 960);
  frameRate(72); // match Teensy

  println(Serial.list()[1]);
  serial = new Serial (this, Serial.list()[1], 115200);
  serial.bufferUntil('\n');

  // img = loadImage("test.bmp");
  img = loadImage("test4.bmp");
}


float angle = 0;

void draw() {
  // TODO: sample audio


// TEST
  //colorMode(HSB, 360, 255, 255);
  //for (int i = 0; i < PIXEL_COUNT; i++) {
  //  pixels[i] = color(angle, 255, 255);
  //}
  //angle += 0.2;
  //if (angle > 360) angle = 0;
  //colorMode(RGB, 255, 255, 255);

  //println(nf((char)red(pixels[0])) + " " + nf((char)green(pixels[0])) + " " + nf((char)blue(pixels[0])));
//updateDisplay();

  // photoscan();
  // plasma();
  // fire();
  fireworks();



  firstFrame = false;
}

void stop() {
  serial.stop();
}

// TODO:
// + Plasma
// + 1 D Lissouj Figures
// + Drip - plasma drop
// + Ball bounces

// + Audio fire
// + Roman Candle - Audio




/* == PHOTOSCAN == */
static int PHOTOSCAN_SAMPLE_X_MILLIS = 200;
int photoscanLastScanMillis = 0;
int photoscanColumn = 0;

void photoscan() {
  pixelFadeLerp = 0.03;

  // update every fourth frame
  if (millis() - photoscanLastScanMillis > PHOTOSCAN_SAMPLE_X_MILLIS || firstFrame) {
    photoscanLastScanMillis = millis();

    // scan image left to right
    for (int i = 0; i < PIXEL_COUNT && i < img.height; i++) {
      color col = img.get(photoscanColumn, i);
      pixBuff[i] = col;
    }
    println(photoscanColumn);

    // move onto next scan column
    photoscanColumn++;
    if (photoscanColumn >= img.width) photoscanColumn = 0;
  }

  fadeLEDs();
}


/* == PLASMA == */
// https://github.com/johncarl81/neopixelplasma
float plasmaPhase = 0.0;
// float phaseIncrement = 0.03;  // Controls the speed of the moving points. Higher == faster. I like 0.08 .
// float colorStretch = 0.3;    // Higher numbers will produce tighter color bands. I like 0.11 .

void plasma() {
  // phase += phaseIncrement;

  // The two points move along Lissajious curves, see: http://en.wikipedia.org/wiki/Lissajous_curve
  // We want values that fit the LED grid: x values between 0..13, y values between 0..8 .
  // The sin() function returns values in the range of -1.0..1.0, so scale these to our desired ranges.
  // The phase value is multiplied by various constants; I chose these semi-randomly, to produce a nice motion.
  // Point p1 = { (sin(phase*1.000)+1.0) * 4.5, (sin(phase*1.310)+1.0) * 4.0 };
  // Point p2 = { (sin(phase*1.770)+1.0) * 4.5, (sin(phase*2.865)+1.0) * 4.0 };
  // Point p3 = { (sin(phase*0.250)+1.0) * 4.5, (sin(phase*0.750)+1.0) * 4.0 };

  // TODO: tune for 1D
  // PVector p1 = new PVector( (sin(phase*1.000)+1.0) * 4.5, (sin(phase*1.310)+1.0) * 4.0 );
  // PVector p2 = new PVector( (sin(phase*1.770)+1.0) * 4.5, (sin(phase*2.865)+1.0) * 4.0 );
  // PVector p3 = new PVector( (sin(phase*0.250)+1.0) * 4.5, (sin(phase*0.750)+1.0) * 4.0 );



  PVector p1 = new PVector( 0.5 * (sin(plasmaPhase) + 1.0) * PIXEL_COUNT, 0.0 );

  for (float i = 0; i < PIXEL_COUNT; i++) {
    PVector point = new PVector(i, 0.0);

    // distance between this LED and the points
    float d1 = PVector.dist(point, p1);


    // float d2 = PVector.dist(point, p2);
    // float d3 = PVector.dist(point, p3);

    // colors are inverse square of distances
    float color1 = 255 * (2 / (d1 * d1));
    color1 = max(0, min(255, color1));
    float color2 = 0;
    float color3 = 0;

    // warp the distance with a sin() function
    // as the distance value increases, the LEDs will get light, dark, light, dark, etc...
    // TODO: experiment with other functions here
    // float color2 = d2;
    // float color3 = d3;
    // float color4 = (sin( d1 * d2 * colorStretch )) + 2.0 * 0.5;

    // // Square the color_f value to weight it towards 0. The image will be darker and have higher contrast.
    // color1 *= color1 * color4;
    // color2 *= color2 * color4;
    // color3 *= color3 * color4;
    // color4 *= color4;


    // pixels[(int)i] = CRGB((int)color1, (int)color2, (int)color3);
    pixels[(int)i] = CRGB((int)color1, (int)color2, (int)color3);
  }

  FastLED_show();
}


/* == FIRE == */
int FIRE_COOLING = 55;
int FIRE_SPARK_CHANCE = 120;
int FIRE_FRAME_HOLD = 20; // ms

char[] heat = new char[PIXEL_COUNT];

void fire() {
  // 1: cool down every cell a little
  // int cooldown;
  // for (int i = 0; i < PIXEL_COUNT; i++) {
  //   // ??? what is happening here
  //   cooldown = (int)random(0, ((FIRE_COOLING * 10) / PIXEL_COUNT) + 2);
  //
  //   if (cooldown > heat[i]) {
  //     heat[i] = 0;
  //   } else {
  //     heat[i] -= cooldown;
  //   }
  // }
  //
  // // 2: heat from each cell drifts 'up' and diffuses a little
  // for (int i = PIXEL_COUNT - 1; i >= 2; i--) {
  //   // shift heat upwards
  //   heat[i] = (char)((heat[i - 1] + heat[i - 2] + heat[i -2]) / 3);
  // }
  //
  // // 3: randomly ignite new 'sparks' near the bottom
  // if (random(255) < FIRE_SPARK_CHANCE) {
  //   // only the bottom 1/8th of the pixels can ignite
  //   int i = (int)random(PIXEL_COUNT * 0.125);
  //   int spark = (char)random(160, 255);
  //   if (heat[i] + spark > 255) {
  //     heat[i] = 255;
  //   } else {
  //     heat[i] += spark;
  //   }
  // }
  //
  // // 4: display current fire status
  // for (int i = 0; i < PIXEL_COUNT; i++) {
  //   setPixelHeatColor(i, heat[i]);
  // }
  // println();
  //
  // updateDisplay(); // FastLED.show();
  // delay(FIRE_FRAME_HOLD);
}

void setPixelHeatColor(int pixel, char temp) {
  // // scale 'heat' down from 0-255 to 0-191
  // char t192 = (char)round((temp/255.0)*191);
  // if (t192 > 191) t192 = 191;
  //
  //
  //   print(nf(t192, 4));
  //   print(" ");
  //
  //
  // //// calculate ramp up from
  // int heatramp = t192 & 0x3F; // 0..63
  // //heatramp <<= 2; // scale up to 0..252
  //
  // // figure out which third of the spectrum we're in
  // if(t192 > 0x80) {                     // hottest
  //   pixels[pixel] = CRGB(255, 192, heatramp);
  // } else if( t192 > 0x40 ) {            // middle
  //   pixels[pixel] = CRGB(255, heatramp, 0);
  // } else {                              // coolest
  //   pixels[pixel] = CRGB(heatramp, 0, 0);
  // }
}

/* == FIREWORKS == */
float p = 0;
float v = 1;
float d = 0;

void fireworks() {
  // random chance to eject firework updwards with speed and velocity
  // cumulative light from each
  // add tail - trailing somehow

  pixels[(int)p] = CHSV(0, 255, 255);

  p += v;


  // TODO: if p > PIXEL_COUNT - die

  FastLED_show();

  // fadeToBlackBy(&(pixels[0]), PIXEL_COUNT, 8);
  fadeToBlackBy(8);

  // TEMP: see whats going on
  delay(1000);


/*
  constSpeed = false;                                  // The speed will be controlled by the slope of the accelerometer (y-Axis)
 showLED(LEDPosition, potVal, 255, intensity);        // Hue will change with potentiometer.

 //The following lines create the comet effect
 bright = random(50, 100);                            // Randomly select a brightness between 50 and 100
 leds[LEDPosition] = CHSV((potVal+40),255, bright);   // The trailing LEDs will have a different hue to the leading LED, and will have a random brightness
 fadeLEDs(8);                                         // This will affect the length of the Trailing LEDs
 setDelay(LEDSpeed);
 */

}










/* == PIXEL UTILITIES == */
void lerpLEDs(int amount) {
  // fade towards pixBuff
  for (int i = 0; i < PIXEL_COUNT; i++) {
    pixels[i] = lerpColor(pixels[i], pixBuff[i], pixelFadeLerp);
  }

  FastLED_show();
}

void setPixel(int i, int r, int g, int b) {
  pixels[i] = color(r, g, b);
}


/* == FAST LED == */
void fadeToBlackBy(int amount) {
  // fade towards pixBuff
  fadeToBlackBy()
  for (int i = 0; i < PIXEL_COUNT; i++) {
    // pixels[i].fadeToBlackBy(amount);
    pixels[i].
  }

  FastLED_show();
}

color CRGB(int r, int g, int b) {
  return color(r, g, b);
}

color CHSV(int h, int s, int v) {
  int i;
  	float f, p, q, t;

    float r, g, b;
  	if( s == 0 ) {
  		// achromatic (grey)
  		r = g = b = v;
  	} else {
  	h /= 60;			// sector 0 to 5
  	i = floor( h );
  	f = h - i;			// factorial part of h
  	p = v * ( 1 - s );
  	q = v * ( 1 - s * f );
  	t = v * ( 1 - s * ( 1 - f ) );
  	switch( i ) {
  		case 0:
  			r = v;
  			g = t;
  			b = p;
  			break;
  		case 1:
  			r = q;
  			g = v;
  			b = p;
  			break;
  		case 2:
  			r = p;
  			g = v;
  			b = t;
  			break;
  		case 3:
  			r = p;
  			g = q;
  			b = v;
  			break;
  		case 4:
  			r = t;
  			g = p;
  			b = v;
  			break;
  		default:		// case 5:
  			r = v;
  			g = p;
  			b = q;
  			break;
    }
  	}

  return color(r, g, b);
}

void FastLED_show() {
  updateDisplay();
}









/* == MISC == */
void updateDisplay() {
  background(0);
  translate(64,32);
  float width = 32;
  float height = 12;
  for (int i = PIXEL_COUNT-1; i >= 0; i--) {
    stroke(128);
    fill(pixels[i]);
    rect(0, 0, width, height);
    translate(0, height+2);
  }

  // dump to serial also
  for (int i = 0; i < PIXEL_COUNT; i++) {
    color col = pixels[i];
    serial.write((char)red(col));
    serial.write((char)green(col));
    serial.write((char)blue(col));
// println(nf((char)red(col)) + " :: " + (char)blue(col) + " :: " + (char)green(col));
  }
  serial.write('\n');
}
