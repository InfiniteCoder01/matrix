#include "matrix.h"

void rainbow() {
  uint32_t t = millis();
  for (uint32_t y = 0; y < HEIGHT; y++) {
    for (uint32_t x = 0; x < WIDTH; x++) {
      leds[idx(x, y)] = CHSV((x + y) * 5 - t / 100.0, 255, 255);
    }
  }
}

void fire(bool init, int hueHot, int hueEdge) {
  // Inspiration: https://github.com/giladaya/arduino-led-matrix/blob/master/fire/fire.ino

  // Shape of the flame, defined as a heightmap
  const uint8_t heights[WIDTH] = {
    0, 2, 4, 6, 7, 8, 9, 9, 9, 9, 8, 7, 6, 4, 2, 0
  };

  static uint8_t matrix[AREA + WIDTH];
  if (init) {
    for (uint32_t i = 0; i < AREA; i++) matrix[i] = 0;
  }

  static float lastT = 0.0;
  float t = fmod(millis() / 100.0, 1.0);
  if (t < lastT) {
    for (uint32_t i = 0; i < AREA; i++) matrix[i] = matrix[i + WIDTH];
    for (uint32_t x = 0; x < WIDTH; x++) matrix[AREA + x] = random(64, 255);
  }
  lastT = t;

  for (uint32_t y = 0; y < HEIGHT; y++) {
    for (uint32_t x = 0; x < WIDTH; x++) {
      const uint8_t iy = HEIGHT - y - 1;
      const uint8_t hue = map(max((int)heights[x] - (int)iy, 0), 0, 9, hueEdge, hueHot);
      const uint8_t value = max((int)Math::lerp(matrix[x + y * WIDTH], matrix[x + (y + 1) * WIDTH], t) - max((int)iy - (int)heights[x] + 1, 0) * 256 / (int)HEIGHT, 0);
      leds[idx(x, y)] = CHSV(hue, 255, value);
    }
  }
}