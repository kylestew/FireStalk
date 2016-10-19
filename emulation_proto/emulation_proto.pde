import processing.serial.*;

Serial serial;

int bins = 64;
int spectroHeight = bins*2;
float magnitudes[] = new float[bins];
PImage spectrogram;

int colors[] = new int[64];

void setup () {
  size(1024, 1024);
  println(Serial.list()[1]);
  serial = new Serial (this, Serial.list()[1], 115200);
  serial.bufferUntil('\n');
  
  spectrogram = createImage(bins, spectroHeight, RGB);
  
  noLoop();
}

int highIdx = 0;
void draw() {
  background(0);
  
  // move the pixels down a row each frame
  spectrogram.copy(0, 0, bins, spectroHeight-1, 0, 1, bins, spectroHeight-1); 
  
  // update image
  spectrogram.loadPixels();
  colorMode(HSB, 360, 100, 100);
  for (int i = 1; i < magnitudes.length && i < bins; i++) {
    // draw into spectrogram - first row of pixels
    if (magnitudes[0] > 1.0)
    spectrogram.pixels[i-1] = color(0, 0, 100);
    else
    spectrogram.pixels[i-1] = color(map(magnitudes[i], 0, 1, 240, 0), 100, 100);
  }
  spectrogram.updatePixels();
  
  image(spectrogram, 24, 24, bins*2, bins*4);
}


void serialEvent(Serial serial) {
  String rawSampleString = serial.readStringUntil('\n');
  String[] values = split(rawSampleString, ' ');
  
  // first 64 values are LED colors, rest is FFT magnitudes
  for (int i = 0; i < values.length && i < 64 + bins; i++) {
    if (i < 64) {
      colors[i] = int(values[i]);
    } else {
      magnitudes[i-64] = float(values[i]);
    }
  }
  redraw();
}