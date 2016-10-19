# BurnStick

TODO:

+ Tune Peak audio to be 0-1
+ Tune FFT

+ Generalize animation code

Links:
http://www.keil.com/pack/doc/cmsis/dsp/html/group__group_filters.html

Notes:
+ Use 96MHZ clock speed on programming - Audio library needs this
+ Aim for a 1m distance between power connections - you can wire power supply anywhere in the strand
+ Need a logic level shifter for 3.3 to 5V levels if you want high output LEDs
+ To bridge pixels to controller, match grounds but not VCC levels












// === Utility Functions ===
float lerp(float start, float end, float percent) {
  return start + percent * (end - start);
}

// float capable MAP
float map(float value, float istart, float istop, float ostart, float ostop) {
  return ostart + (ostop - ostart) * ((value - istart) / (istop - istart));
}

int map(int value, int istart, int istop, int ostart, int ostop) {
  return ostart + (ostop - ostart) * ((value - istart) / (istop - istart));
}




// === FFT Processing ===
void sampleIntensity() {
  // float otherMean;
  // grab average of frequency band
  // windowMean(magnitudes,
  //            frequencyToBin(BAND_START),
  //            frequencyToBin(BAND_END),
  //            &audioIntensity,
  //            &otherMean);
  // convert intensity to decibels.
  // audioIntensity = 20.0*log10(audioIntensity);
  // boost signals that stand out
  // otherMean = 20.0*log10(otherMean);
  // if (intensity > otherMean) {
  //   intensity += (intensity - otherMean) * BAND_ISOLATION;
  // }
/*
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
  */
}

// Compute the average magnitude of a target frequency window vs. all other frequencies.
void windowMean(float* magnitudes, int lowBin, int highBin, float* windowMean, float* otherMean) {
  // *windowMean = 0;
  // *otherMean = 0;
  // // notice the first magnitude bin is skipped because it represents the
  // // average power of the signal.
  // for (int i = 1; i < FFT_SIZE/2; ++i) {
  //   if (i >= lowBin && i <= highBin) {
  //     *windowMean += magnitudes[i];
  //   } else {
  //     *otherMean += magnitudes[i];
  //   }
  // }
  // *windowMean /= (highBin - lowBin) + 1;
  // *otherMean /= (FFT_SIZE / 2 - (highBin - lowBin));
}

// Convert a frequency to the appropriate FFT bin it will fall within.
int frequencyToBin(float frequency) {
  // float binFrequency = float(SAMPLE_RATE_HZ) / float(FFT_SIZE);
  // return int(frequency / binFrequency);
  return 0;
}
