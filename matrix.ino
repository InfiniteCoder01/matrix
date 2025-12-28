#include "matrix.h"
#include "data/secrets.h"

const uint8_t BUTTON = 14;
uint8_t mode = 0;

void setupUI();
void updateUI();

void setup() {
  Serial.begin(115200);
  setupMatrix();
  pinMode(BUTTON, INPUT_PULLUP);

  setupAP();
  setupUI();
}

void rainbow();
void fire(bool init, int hueHot, int hueEdge);
void snake(bool init);
void tetris(bool init);

void loop() {
  if (!digitalRead(BUTTON)) {
    mode = (mode + 1) % 5;
    updateUI();
    delay(300);
  }

  static uint8_t lastMode = 255;
  volatile uint8_t mode_ = mode;
  if (mode_ == 0) rainbow();
  else if (mode_ == 1) fire(lastMode != mode_, 40, 0);
  else if (mode_ == 2) fire(lastMode != mode_, 256, 210);
  else if (mode_ == 3) snake(lastMode != mode_);
  else if (mode_ == 4) tetris(lastMode != mode_);
  lastMode = mode_;
  redrawMatrix();
}
