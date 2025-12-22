#include "matrix.h"

const uint8_t BUTTON = 14;
static uint8_t mode = 0;

void setup() {
  setupMatrix();
  pinMode(BUTTON, INPUT_PULLUP);
}

void rainbow();
void fire(bool init, int hueHot, int hueEdge);
void snake(bool init);
void tetris(bool init);

void loop() {
  if (!digitalRead(BUTTON)) {
    mode++;
    delay(300);
  }

  static uint8_t lastMode = 255;
  if (mode == 0) rainbow();
  else if (mode == 1) fire(lastMode != mode, 40, 0);
  else if (mode == 2) fire(lastMode != mode, 256, 210);
  else if (mode == 3) snake(lastMode != mode);
  else if (mode == 4) tetris(lastMode != mode);
  else mode = 0;
  lastMode = mode;
  redrawMatrix();
}
