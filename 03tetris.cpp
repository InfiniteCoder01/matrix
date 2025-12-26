#include "matrix.h"

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
  shape = random(7);
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

  float score = cleared * 5.0;
  for (uint32_t x = START_X + 1; x < END_X; x++) {
    uint32_t holes = 0;
    for (int32_t y = HEIGHT - 1; y >= 0; y--) {
      if (clear[y]) continue;
      if (leds[idx(x, y)] == CRGB(0, 0, 0)) holes++;
      else score -= holes, holes = 0;
    }
  }

  vec2i l = vec2i(12, 0), r = vec2i(4, 0);
  for (const auto d : shapes[shape]) {
    const auto v = pos + rotate(d);
    if (v.x - 1 < l.x) l = v - vec2i(1, 0);
    if (v.x + 1 > r.x) r = v + vec2i(1, 0);
    score += v.y * 0.1;
  }

  int sidel = 0, sider = 0;
  while (l.y + sidel < HEIGHT && leds[idx(l + vec2i(0, sidel))] == CRGB(0, 0, 0)) sidel++;
  while (r.y + sider < HEIGHT && leds[idx(l + vec2i(0, sider))] == CRGB(0, 0, 0)) sider++;
  score -= (sidel + sider) * 0.1;
  score -= (max(sidel - 4, 0) + max(sider - 4, 0)) * 0.5;

  return score;
}

void tetris(bool init) {
  if (init) reset();
  drawShape(CRGB(0, 0, 0));

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

        const auto score_ = score();

        if (score_ > bestScore) {
          bestScore = score_;
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

  bool place = false;
  if (millis() - lastDrop >= 100) {
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