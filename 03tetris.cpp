#include "matrix.h"

static const vec2i shapes[7][4] = {
  { vec2i(-1, 0), vec2i(0, 0), vec2i(1, 0), vec2i(2, 0) },   // I
  { vec2i(0, 0), vec2i(0, 1), vec2i(1, 0), vec2i(1, 1) },    // O
  { vec2i(-1, 0), vec2i(0, 0), vec2i(1, 0), vec2i(1, 1) },   // J
  { vec2i(-1, 0), vec2i(0, 0), vec2i(1, 0), vec2i(-1, 1) },  // L
  { vec2i(0, 1), vec2i(0, 0), vec2i(1, 0), vec2i(-1, 1) },   // S
  { vec2i(0, 1), vec2i(0, 0), vec2i(1, 0), vec2i(-1, 0) },   // T
  { vec2i(0, 1), vec2i(0, 0), vec2i(1, 1), vec2i(-1, 0) },   // Z
};

static const CRGB colors[7] = {
  CRGB(255, 0, 0),
  CRGB(255, 255, 0),
  CRGB(0, 0, 255),
  CRGB(255, 165, 0),
  CRGB(255, 0, 255),
  CRGB(0, 255, 255),
  CRGB(0, 255, 0),
};

static uint8_t shape, rotation;
static vec2i pos;
static uint32_t lastDrop = 0;

static void reset();
static bool canPlace();
static void nextShape() {
  shape = random(7);
  pos = vec2i(7, 0);
  rotation = 0;
  lastDrop = millis();
  if (!canPlace()) reset();
}

static void reset() {
  for (uint32_t i = 0; i < AREA; i++) leds[i] = CRGB(0, 0, 0);
  for (uint32_t y = 0; y < HEIGHT; y++) {
    leds[idx(3, y)] = leds[idx(12, y)] = CRGB(196, 196, 0);
  }
  nextShape();
}

static vec2i rotate(const vec2i delta) {
  if (shape < 2) {  // I and O
    if (rotation % 2 == 0) return delta;
    else return vec2i(1 - delta.y, delta.x);
  }
  if (rotation == 0) return delta;
  else if (rotation == 1) return vec2i(-delta.y, delta.x);
  else if (rotation == 2) return vec2i(-delta.x, -delta.y);
  else return vec2i(delta.y, -delta.x);
}

static bool canPlace() {
  for (const auto d : shapes[shape]) {
    const auto v = pos + rotate(d);
    if (v.y < 0) continue;
    if (v.y >= HEIGHT) return false;
    if (leds[idx(v)] != CRGB(0, 0, 0)) return false;
  }
  return true;
}

static void drawShape(CRGB color) {
  for (const auto d : shapes[shape]) {
    const auto v = pos + rotate(d);
    if (v.y < 0) continue;
    leds[idx(v)] = color;
  }
}

uint32_t clearLines(bool apply) {
  uint32_t cleared = 0;
  for (uint32_t y = 0; y < HEIGHT; y++) {
    bool clear = true;
    for (uint32_t x = 4; x < 12; x++) {
      if (leds[idx(x, y)] == CRGB(0, 0, 0)) {
        clear = false;
        break;
      }
    }
    if (!clear) continue;
    cleared++;
    if (!apply) continue;
    for (uint32_t x = 4; x < 12; x++) leds[idx(x, y)] = CRGB(255, 255, 255);
  }
  return cleared;
}

void tetris(bool init) {
  if (init) reset();

  drawShape(CRGB(0, 0, 0));
  {  // Clear lines
    uint32_t cleared = 0;
    for (int32_t y = HEIGHT - 1; y >= 0; y--) {
      if (leds[idx(4, y)] == CRGB(255, 255, 255)) {
        cleared++;
        continue;
      }

      for (uint32_t x = 4; x < 12; x++) leds[idx(x, y + cleared)] = leds[idx(x, y)];
    }

    for (uint32_t y = 0; y < cleared; y++) {
      for (uint32_t x = 4; x < 12; x++) leds[idx(x, y)] = CRGB(0, 0, 0);
    }
    if (cleared > 0) delay(200);
  }

  {  // AI
    uint8_t lastRotation = rotation, bestRotation = rotation;
    vec2i lastPosition = pos, bestPosition = pos;
    float bestScore = -10000.0;

    for (rotation = 0; rotation < 4; rotation++) {
      if (!canPlace()) continue;
      while (canPlace()) pos.x--;
      while (true) {
        pos.x++;
        if (!canPlace()) break;
        while (canPlace()) pos.y++;
        pos.y--;
        drawShape(CRGB(128, 128, 128));

        // Score
        float score = clearLines(false) * 10.0;
        for (const auto d : shapes[shape]) {
          const auto v = pos + rotate(d);
          const auto iy = HEIGHT - v.y - 1;
          score -= iy * iy * 0.1;
        }
        for (uint32_t x = 4; x < 12; x++) {
          uint32_t holes = 0;
          for (int32_t y = HEIGHT - 1; y >= 0; y--) {
            if (leds[idx(x, y)] == CRGB(0, 0, 0)) holes++;
            else score -= holes, holes = 0;
          }
        }

        if (score > bestScore) {
          bestScore = score;
          bestPosition = pos;
          bestRotation = rotation;
        }

        drawShape(CRGB(0, 0, 0));
        pos.y = lastPosition.y;
      }
      pos = lastPosition;
    }
    rotation = bestRotation;
    if (pos.x > bestPosition.x) pos.x--;
    else if (pos.x < bestPosition.x) pos.x++;
  }

  if (millis() - lastDrop >= 100) {
    pos.y++;
    if (!canPlace()) {
      pos.y--;
      drawShape(colors[shape]);
      clearLines(true);
      nextShape();
    }
    lastDrop = millis();
  }

  drawShape(colors[shape]);
}