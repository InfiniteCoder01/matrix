#include "matrix.h"

CRGB leds[AREA];

void setupMatrix() {
  FastLED.addLeds<WS2811, 4, GRB>(leds, AREA).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(30);
}

void redrawMatrix() {
  FastLED.show();
}