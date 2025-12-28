#include "matrix.h"
#include "ui.h"

// #define BENCHMARK

static const uint32_t START_X = 3, END_X = 12;
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
  static uint8_t bag[] = { 0, 1, 2, 3, 4, 5, 6 };
  static uint8_t idx = 7;
  if (idx >= 7) {
    for (uint8_t i = 0; i < 7; i++) std::swap(bag[i], bag[random(7)]);
    idx = 0;
  }

  shape = bag[idx++];
  pos = vec2i(7, 0);
  rotation = 0;
  lastDrop = millis();

  if (!canPlace()) reset();
}

static void reset() {
  for (uint32_t i = 0; i < AREA; i++) leds[i] = CRGB(0, 0, 0);
  for (uint32_t y = 0; y < HEIGHT; y++) {
    leds[idx(START_X, y)] = leds[idx(END_X, y)] = CRGB(196, 196, 0);
  }

#ifdef BENCHMARK
  for (uint32_t y = HEIGHT / 2; y < HEIGHT; y++) {
    for (uint32_t x = START_X + 1; x < END_X; x++) {
      if (random(2)) leds[idx(x, y)] = CRGB(96, 96, 96);
    }
  }
#endif

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

static float score() {
  bool clear[HEIGHT];
  uint32_t cleared = 0;
  for (uint32_t y = 0; y < HEIGHT; y++) {
    clear[y] = true;
    for (uint32_t x = START_X + 1; x < END_X; x++) {
      if (leds[idx(x, y)] == CRGB(0, 0, 0)) {
        clear[y] = false;
        break;
      }
    }
    if (clear[y]) cleared++;
  }

  float score = cleared * cleared * 5.0;
  for (uint32_t x = START_X + 1; x < END_X; x++) {
    uint32_t gaps = 0;
    for (int32_t y = HEIGHT - 1; y >= 0; y--) {
      if (clear[y]) continue;
      if (leds[idx(x, y)] == CRGB(0, 0, 0)) gaps = 1;
      else score -= gaps, gaps = 0;
    }
  }

  uint32_t top = HEIGHT;
  vec2i left = vec2i(WIDTH, 0), right = vec2i(0, 0);
  for (const auto d : shapes[shape]) {
    const auto v = pos + rotate(d);
    if (v.y < 0) continue;
    if (v.y < top) top = v.y;
    if (v.x <= left.x) left = v;
    if (v.x >= right.x) right = v;

    if (leds[idx(v - vec2i(1, 0))] != CRGB(0, 0, 0)) score += 1.0;
    if (leds[idx(v + vec2i(1, 0))] != CRGB(0, 0, 0)) score += 1.0;
    if (v.y + 1 >= HEIGHT || leds[idx(v + vec2i(0, 1))] != CRGB(0, 0, 0)) score += 1.0;
    if (v.y - 1 >= 0 && leds[idx(v - vec2i(0, 1))] != CRGB(0, 0, 0)) score += 1.0;

    if (v.y + 1 < HEIGHT && leds[idx(v + vec2i(0, 1))] == CRGB(0, 0, 0)) score -= 20.0;
  }

  uint32_t dleft = 0, dright = 0;
  while (left.y + dleft < HEIGHT && leds[idx(left + vec2i(0, dleft))] == CRGB(0, 0, 0)) dleft++;
  while (right.y + dright < HEIGHT && leds[idx(right + vec2i(0, dright))] == CRGB(0, 0, 0)) dright++;
  score -= (dleft * dleft + dright * dright) * 0.3;

  score += top * 10.0;
  return score;
}

static bool manualMode = false;
void tetris(bool init) {
  if (init) {
    manualMode = false;
    reset();
  }
  drawShape(CRGB(0, 0, 0));

  vec2i joy_ = sign(joy);
  if (joy_ != 0) manualMode = true;
  if (manualMode) {
    if (joy.y < 0) {
      rotation = (rotation + 1) % 4;
      if (!canPlace()) rotation = (rotation + 3) % 4;
      joy = 0;
    } else if (joy.x != 0) {
      pos += joy;
      if (!canPlace()) pos -= joy;
      joy = 0;
    }
  } else {  // AI
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
        const auto score_ = score();
        drawShape(CRGB(0, 0, 0));

        if (score_ > bestScore) {
          bestScore = score_;
          bestPosition = pos;
          bestRotation = rotation;
        }

        pos.y = lastPosition.y;
      }
      pos = lastPosition;
    }
    rotation = bestRotation;

#ifdef BENCHMARK
    pos = bestPosition;
#else
    if (pos.x > bestPosition.x) pos.x--;
    else if (pos.x < bestPosition.x) pos.x++;
#endif
  }

  bool place = false;
  if (millis() - lastDrop >= (manualMode ? (joy.y > 0 ? 50 : 500) : 100)) {
    pos.y++;
    if (!canPlace()) {
      pos.y--;
      place = true;
    }
    lastDrop = millis();
  }

  drawShape(colors[shape]);
  if (place) {
    nextShape();

    bool clear[HEIGHT];
    uint32_t cleared = 0;
    for (uint32_t y = 0; y < HEIGHT; y++) {
      clear[y] = true;
      for (uint32_t x = START_X + 1; x < END_X; x++) {
        if (leds[idx(x, y)] == CRGB(0, 0, 0)) {
          clear[y] = false;
          break;
        }
      }
      if (clear[y]) cleared++;
    }

    if (cleared == 0) return;
    for (uint32_t x = START_X + 1; x < END_X; x++) {
      for (uint32_t y = 0; y < HEIGHT; y++) {
        if (!clear[y]) continue;
        leds[idx(x, y)] = CRGB(255, 255, 255);
      }
      redrawMatrix();
    }

    for (int i = 255; i >= 0; i -= 10) {
      for (uint32_t y = 0; y < HEIGHT; y++) {
        if (!clear[y]) continue;
        for (uint32_t x = START_X + 1; x < END_X; x++) leds[idx(x, y)] = CRGB(i, i, i);
      }
      redrawMatrix();
    }

    uint32_t shift = 0;
    for (int32_t y = HEIGHT - 1; y >= 0; y--) {
      if (clear[y]) {
        shift++;
        continue;
      }

      for (uint32_t x = START_X + 1; x < END_X; x++) leds[idx(x, y + shift)] = leds[idx(x, y)];
    }

    for (uint32_t y = 0; y < shift; y++) {
      for (uint32_t x = START_X + 1; x < END_X; x++) leds[idx(x, y)] = CRGB(0, 0, 0);
    }
    lastDrop = millis();
  }
}