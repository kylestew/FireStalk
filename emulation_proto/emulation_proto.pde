import processing.serial.*;
import ddf.minim.*;
import ddf.minim.analysis.*;

Serial serial;
//Minim minim;
FFT fft;

int bands = 512;
float samples[] = new float[bands];

void setup () {
  size(512, 360);
  println(Serial.list()[1]);
  serial = new Serial (this, Serial.list()[1], 115200);
  serial.bufferUntil('\n');
  
  //fft = new FFT(this, bands);
}

void draw() {
  background(0);
  
  for (int i = 0; i < bands; i++) {
    line(i, height, i, height - samples[i]*height*5); 
  }
}

void serialEvent(Serial serial) {
  String rawSampleString = serial.readStringUntil('\n');
  float samples[] = float(split(rawSampleString, ' '));
}