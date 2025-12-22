#include "matrix.h"
#include <vector>

static std::vector<vec2i> body;
static vec2i apple;
static void reset() {
  for (uint32_t i = 0; i < AREA; i++) leds[i] = CRGB(0, 0, 0);
  body = { vec2i(-3, 0), vec2i(-2, 0), vec2i(-1, 0) };
  apple = vec2i(random(WIDTH), random(HEIGHT));
  leds[idx(apple)] = CRGB(255, 0, 0);
}

void snake(bool init) {
  if (init) reset();

  uint32_t dst[AREA];
  for (uint32_t i = 0; i < AREA; i++) dst[i] = AREA;
  {  // Flood fill.
    std::vector<vec2i> q;
    q.push_back(apple);
    dst[apple.x + apple.y * WIDTH] = 0;
    while (!q.empty()) {
      vec2i v = q.front();
      q.erase(q.begin());
      for (const auto d : { vec2i(-1, 0), vec2i(1, 0), vec2i(0, -1), vec2i(0, 1) }) {
        const auto n = v + d;
        if (n.x < 0 || n.y < 0 || n.x >= WIDTH || n.y >= HEIGHT) continue;
        if (leds[idx(n)] != CRGB(0, 0, 0)) continue;
        if (dst[n.x + n.y * WIDTH] <= dst[v.x + v.y * WIDTH] + 1) continue;
        dst[n.x + n.y * WIDTH] = dst[v.x + v.y * WIDTH] + 1;
        q.push_back(n);
      }
    }
  }

  vec2i next = body.back() + vec2i(1, 0);
  for (const auto d : { vec2i(-1, 0), vec2i(0, -1), vec2i(0, 1) }) {
    const auto n = body.back() + d;
    if (n.x < 0 || n.y < 0 || n.x >= WIDTH || n.y >= HEIGHT) continue;
    if (next.x < 0 || next.y < 0 || next.x >= WIDTH || next.y >= HEIGHT || dst[n.x + n.y * WIDTH] < dst[next.x + next.y * WIDTH]) next = n;
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
  } else if (next.x < 0 || next.x >= WIDTH || next.y < 0 || next.y >= HEIGHT || leds[idx(next)].g >= 127) {  // Loosing
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