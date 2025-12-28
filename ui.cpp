#include "ui.h"
#include <ESPUI.h>
#include <umm_malloc/umm_heap_select.h>

uint8_t mode = 0;
vec2i joy = 0;

void updateUI();
static uint16_t selectorId, joystickId;

void setupUI() {
  HeapSelectIram doAllocationsInIRAM;

  selectorId = ESPUI.addControl(Select, "Mode", "Mode", ControlColor::Peterriver, Control::noParent, [](Control *sender, int type) {
    for (uint8_t i = 0; i < MODE_COUNT; i++) {
      if (sender->value == modes[i]) mode = i;
    }
    updateUI();
  });
  for (const auto mode : modes) ESPUI.addControl(Option, mode, mode, None, selectorId);

  joystickId = ESPUI.pad(
    "Joystick", [](Control *sender, int type) {
      if (type == -2) joy = vec2i(-1, 0);
      else if (type == -3) joy = vec2i(1, 0);
      else if (type == -4) joy = vec2i(0, -1);
      else if (type == -5) joy = vec2i(0, 1);
      else joy = 0;
    },
    ControlColor::Peterriver);

  ESPUI.begin("InfiniteMatrix");
  delay(100);
  updateUI();
}

void updateUI() {
  ESPUI.updateSelect(selectorId, modes[mode]);
  ESPUI.updateVisibility(joystickId, mode == 3 || mode == 4);
}