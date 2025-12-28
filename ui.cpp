#include <ESPUI.h>

extern uint8_t mode;
static const char *modes[] = {
  "Rainbow",
  "Fire",
  "Purple Fire",
  "Snake",
  "Tetris",
};

void updateUI();
static uint16_t selectorId;

void modeCallback(Control *sender, int type) {
  for (uint8_t i = 0; i < sizeof(modes) / sizeof(modes[0]); i++) {
    if (sender->value == modes[i]) mode = i;
  }
  updateUI();
}

void setupUI() {
  selectorId = ESPUI.addControl(Select, "Mode", "Mode", Wetasphalt, Control::noParent, modeCallback);
  for (const auto mode : modes) ESPUI.addControl(Option, mode, mode, None, selectorId);

  ESPUI.begin("InfiniteMatrix");
  updateUI();
}

void updateUI() {
  ESPUI.updateSelect(selectorId, modes[mode]);
}