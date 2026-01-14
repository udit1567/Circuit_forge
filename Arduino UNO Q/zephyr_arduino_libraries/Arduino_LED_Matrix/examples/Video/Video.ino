#include "Arduino_LED_Matrix.h"
#include "bad_apple.h"

Arduino_LED_Matrix matrix;

void setup() {
  matrix.begin();
  matrix.setGrayscaleBits(8);
}

void loop() {
  matrix.playVideo(bad_apple, bad_apple_len);
}