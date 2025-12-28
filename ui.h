#pragma once
#include "matrix.h"

void setupUI();
void updateUI();

extern uint8_t mode;
extern vec2i joy;

static const char *modes[] = {
  "Rainbow",
  "Fire",
  "Purple Fire",
  "Snake",
  "Tetris",
};

const uint8_t MODE_COUNT = sizeof(modes) / sizeof(modes[0]);