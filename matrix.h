#include <FastLED.h>
#include "OreonMath.hpp"

using namespace VectorMath;

const uint32_t WIDTH = 16;
const uint32_t HEIGHT = 16;
const uint32_t AREA = WIDTH * HEIGHT;

extern CRGB leds[AREA];

inline uint32_t idx(uint32_t x, uint32_t y) { return (y % 2 == 0 ? WIDTH - x - 1 : x) + y * WIDTH; }
inline uint32_t idx(vec2i p) { return idx(p.x, p.y); }

void setupMatrix();
void redrawMatrix();