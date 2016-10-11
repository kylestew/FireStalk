/*
 * PROJECT InGaN
 * =============
 * LED based #IXArt platform
 */
#include <Adafruit_NeoPixel.h>
#include <Bounce2.h>
#define ARM_MATH_CM4
#include <arm_math.h>

// == CONSTANTS ==
// pixels
const int NEO_PIXEL_PIN = 6;
const int NEO_PIXEL_COUNT = 5;

// fft
const int AUDIO_INPUT_PIN = 14;
const int ANALOG_READ_RESOLUTION = 10;   // Bits of resolution for the ADC.
const int ANALOG_READ_AVERAGING = 16;    // Number of samples to average with each ADC reading.
const int FFT_SIZE = 256;                // Size of the FFT.  Realistically can only be at most 256

// spectrum display
const int SAMPLE_RATE_HZ = 9000;         // Sample rate of the audio in hertz
const int BAND_START = 40.0;             // start of freq sample band
const int BAND_END = 920.0;              // end of freq sample band
float GAIN = 3.0;                        // increase the color volume
float LERP_DOWN = 0.09;                       // smooth out responsiveness
float LERP_UP = 0.7;                       // smooth out responsiveness
float HUE_SHIFT = 40.0;                  // shift hue based on intensity
float SPECTRUM_MIN_DB = 45.0;            // Audio intensity (in decibels) that maps to low LED brightness.
float SPECTRUM_MAX_DB = 95.0;            // Audio intensity (in decibels) that maps to high LED brightness.

// == GLOBAL STATE ==
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NEO_PIXEL_COUNT, NEO_PIXEL_PIN, NEO_GRB + NEO_KHZ400);
IntervalTimer samplingTimer;
int sampleCounter = 0;
float samples[FFT_SIZE*2];
float magnitudes[FFT_SIZE];

float hue = 0.0;
float value = 0.0;


void setup() {
  // initialze NeoPixels
  pixels.begin();
  pixels.show();

  // ADC -> audio input
  pinMode(AUDIO_INPUT_PIN, INPUT);
  analogReadResolution(ANALOG_READ_RESOLUTION);
  analogReadAveraging(ANALOG_READ_AVERAGING);
  // begin sampling audio
  samplingBegin();

  Serial.begin(9600);
}

void loop() {
  // Calculate FFT if a full sample is available.
  if (samplingIsDone()) {
    // Run FFT on sample data.
    arm_cfft_radix4_instance_f32 fft_inst;
    arm_cfft_radix4_init_f32(&fft_inst, FFT_SIZE, 0, 1);
    arm_cfft_radix4_f32(&fft_inst, samples);
    // Calculate magnitude of complex numbers output by the FFT.
    arm_cmplx_mag_f32(samples, magnitudes, FFT_SIZE);

    // display
    sampleIntensity();

    // Restart audio sampling.
    samplingBegin();
  }

  // rotate hue
  hue += 0.008;
  if (hue > 360) hue -= 360;

  updatePixleStates();
}

void updatePixleStates() {
  for(int i=0; i<pixels.numPixels(); i++) {
    pixels.setPixelColor(i, pixelHSVtoRGBColor(hue, 1.0, 0.1+value));
  }
  pixels.show();
}

// === Audio Sampling ===
void samplingCallback() {
  // Interrupt everything to grab a sample - fill the buffer - and disable interrupts
  // Read from the ADC and store the sample data
  samples[sampleCounter] = (float32_t)analogRead(AUDIO_INPUT_PIN);
  // Complex FFT functions require a coefficient for the imaginary part of the input.
  // Since we only have real data, set this coefficient to zero.
  samples[sampleCounter+1] = 0.0;
  // Update sample buffer position and stop after the buffer is filled
  sampleCounter += 2;
  if (sampleCounter >= FFT_SIZE*2) {
    samplingTimer.end();
  }
}

void samplingBegin() {
  // Reset sample buffer position and start callback at necessary rate
  sampleCounter = 0;
  samplingTimer.begin(samplingCallback, 1000000/SAMPLE_RATE_HZ);
}

boolean samplingIsDone() {
  return sampleCounter >= FFT_SIZE*2;
}

// === Spectrum Display ===
void sampleIntensity() {
  float intensity, otherMean;
  // grab average of frequency band
  windowMean(magnitudes,
             frequencyToBin(BAND_START),
             frequencyToBin(BAND_END),
             &intensity,
             &otherMean);
  // convert intensity to decibels.
  intensity = 20.0*log10(intensity);
  // boost signals that stand out
  // otherMean = 20.0*log10(otherMean);
  // if (intensity > otherMean) {
  //   intensity += (intensity - otherMean) * BAND_ISOLATION;
  // }

  // map to intensity values (clamp 0-1)
  intensity = constrain(map(intensity, SPECTRUM_MIN_DB, SPECTRUM_MAX_DB, -0.2, 1.0*GAIN), 0.0, 1.0);
  // smooth out transitions (only in desc direction)
  value = intensity;
  // if (intensity < value)
  //   value = lerp(value, intensity, LERP_DOWN);
  // else
  //   value = lerp(value, intensity, LERP_UP);

  Serial.print(intensity);
  Serial.print(" :: ");
  Serial.println(value);

  //   // shift hue based on intensity (center on pure)
  //   float hue = hues[i];
  //   hue += (intensity-0.5)*HUE_SHIFT*2.0; // deg swing around center
  //   if (hue>360.0) hue -= 360.0;
  //   if (hue<0.0) hue += 360.0;
  //
  //   pixels.setPixelColor(i, pixelHSVtoRGBColor(hue, 1.0, intensities[i]));
  //   if (i == 0) {
  //     Serial.println(intensity);
  //   }
  //
  // }
  // pixels.show();
}

// === Utility Functions ===
float lerp(float start, float end, float percent) {
  return start + percent * (end - start);
}

// float capable MAP
float map(float value, float istart, float istop, float ostart, float ostop) {
  return ostart + (ostop - ostart) * ((value - istart) / (istop - istart));
}

// Compute the average magnitude of a target frequency window vs. all other frequencies.
void windowMean(float* magnitudes, int lowBin, int highBin, float* windowMean, float* otherMean) {
  *windowMean = 0;
  *otherMean = 0;
  // notice the first magnitude bin is skipped because it represents the
  // average power of the signal.
  for (int i = 1; i < FFT_SIZE/2; ++i) {
    if (i >= lowBin && i <= highBin) {
      *windowMean += magnitudes[i];
    } else {
      *otherMean += magnitudes[i];
    }
  }
  *windowMean /= (highBin - lowBin) + 1;
  *otherMean /= (FFT_SIZE / 2 - (highBin - lowBin));
}

// Convert a frequency to the appropriate FFT bin it will fall within.
int frequencyToBin(float frequency) {
  float binFrequency = float(SAMPLE_RATE_HZ) / float(FFT_SIZE);
  return int(frequency / binFrequency);
}

// Convert from HSV to RBG
// hue [0-360], saturation [0-1.0], value [0-1.0]
uint32_t pixelHSVtoRGBColor(float hue, float saturation, float value) {
 // Implemented from algorithm at http://en.wikipedia.org/wiki/HSL_and_HSV#From_HSV
 float chroma = value * saturation;
 float h1 = float(hue)/60.0;
 float x = chroma*(1.0-fabs(fmod(h1, 2.0)-1.0));
 float r = 0;
 float g = 0;
 float b = 0;
 if (h1 < 1.0) {
   r = chroma;
   g = x;
 }
 else if (h1 < 2.0) {
   r = x;
   g = chroma;
 }
 else if (h1 < 3.0) {
   g = chroma;
   b = x;
 }
 else if (h1 < 4.0) {
   g = x;
   b = chroma;
 }
 else if (h1 < 5.0) {
   r = x;
   b = chroma;
 }
 else // h1 <= 6.0
 {
   r = chroma;
   b = x;
 }
 float m = value - chroma;
 r += m;
 g += m;
 b += m;
 return pixels.Color(int(255*r), int(255*g), int(255*b));
}
