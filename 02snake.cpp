#include "matrix.h"
#include "ui.h"
#include <vector>

static std::vector<vec2i> body;
static vec2i apple;
static vec2i manualDir = 0;

static void reset() {
  for (uint32_t i = 0; i < AREA; i++) leds[i] = CRGB(0, 0, 0);
  body = { vec2i(-3, 0), vec2i(-2, 0), vec2i(-1, 0) };
  apple = vec2i(random(WIDTH), random(HEIGHT));
  leds[idx(apple)] = CRGB(255, 0, 0);
  if (manualDir != 0) manualDir = vec2i(1, 0);
}

static bool inside(vec2i v) {
  return v.x >= 0 && v.x < WIDTH && v.y >= 0 && v.y < HEIGHT;
}

void snake(bool init) {
  if (init) {
    manualDir = 0;
    reset();
  }

  if (manualDir != 0) delay(100);
  if (joy != 0) manualDir = sign(joy);

  vec2i next = body.back() + manualDir;
  if (manualDir == 0) {  // AI
    next += vec2i(1, 0);

    uint32_t dst[AREA];
    for (uint32_t i = 0; i < AREA; i++) dst[i] = AREA;

    std::vector<vec2i> q;
    q.push_back(apple);
    dst[idx(apple)] = 0;
    while (!q.empty()) {
      const auto v = q.front();
      q.erase(q.begin());
      for (const auto d : { vec2i(-1, 0), vec2i(1, 0), vec2i(0, -1), vec2i(0, 1) }) {
        const auto n = v + d;
        if (!inside(n)) continue;
        if (leds[idx(n)] != CRGB(0, 0, 0)) continue;
        if (dst[idx(n)] <= dst[idx(v)] + 1) continue;
        dst[idx(n)] = dst[idx(v)] + 1;
        q.push_back(n);
      }
    }

    for (const auto d : { vec2i(-1, 0), vec2i(0, -1), vec2i(0, 1) }) {
      const auto n = body.back() + d;
      if (!inside(n)) continue;
      if (!inside(next) || dst[idx(n)] < dst[idx(next)]) next = n;
    }

    if (!inside(next) || dst[idx(next)] == AREA) {  // Panic mode
      for (uint32_t i = 0; i < AREA; i++) dst[i] = AREA;

      bool pushed[body.size()];
      for (size_t i = 0; i < body.size(); i++) {
        dst[idx(body[i])] = i;
        pushed[i] = false;
      }

      q.push_back(body.front());
      pushed[0] = true;
      while (!q.empty()) {
        const auto v = q.front();
        q.erase(q.begin());

        // Hack to not have to use priority_queue
        const uint32_t pushIdx = dst[idx(v)] + 1;
        if (pushIdx < body.size() && !pushed[pushIdx]) {
          pushed[pushIdx] = true;
          q.push_back(body[pushIdx]);
        }

        for (const auto d : { vec2i(-1, 0), vec2i(1, 0), vec2i(0, -1), vec2i(0, 1) }) {
          const auto n = v + d;
          if (!inside(n)) continue;
          if (leds[idx(n)] != CRGB(0, 0, 0)) continue;
          if (dst[idx(n)] <= dst[idx(v)] + 1) continue;
          dst[idx(n)] = dst[idx(v)] + 1;
          q.push_back(n);
        }
      }

      // Find shortest path
      vec2i dshortest;
      for (const auto d : { vec2i(-1, 0), vec2i(1, 0), vec2i(0, -1), vec2i(0, 1) }) {
        const auto n = body.back() + d;
        if (!inside(n) || leds[idx(n)] != CRGB(0, 0, 0)) continue;
        if (!inside(next) || leds[idx(next)] != CRGB(0, 0, 0) || dst[idx(n)] < dst[idx(next)]) {
          next = n;
          dshortest = d;
        }
      }

      if (inside(next) && leds[idx(next)] == CRGB(0, 0, 0)) {  // Try to make it longer
        for (const auto d : { vec2i(dshortest.y, dshortest.x), -vec2i(dshortest.y, dshortest.x) }) {
          const auto n = body.back() + d, n2 = n + dshortest;
          if (!inside(n) || leds[idx(n)] != CRGB(0, 0, 0)) continue;
          if (!inside(n2) || leds[idx(n2)] != CRGB(0, 0, 0)) continue;
          if (dst[idx(n)] > dst[idx(next)]) next = n;
        }
      }
    }
  }

  if (next == apple) {  // Apple
    for (uint32_t i = 0; i < 1024; i++) {
      apple = vec2i(random(WIDTH), random(HEIGHT));
      if (leds[idx(apple)] == CRGB(0, 0, 0)) break;
    }
    if (leds[idx(apple)] != CRGB(0, 0, 0)) {
      reset();
      return;
    }
    leds[idx(apple)] = CRGB(255, 0, 0);
  } else if (!inside(next) || leds[idx(next)].g >= 127) {  // Loosing
    reset();
    return;
  } else {
    if (body.front().x >= 0) leds[idx(body.front())] = CRGB(0, 0, 0);
    body.erase(body.begin());
  }
  body.push_back(next);
  leds[idx(next)] = CRGB(0, 255, 0);

  delay(50);
}