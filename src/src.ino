#include <Arduboy2.h>
#include "sprites.h"

Arduboy2 arduboy;

// Game mode enumeration for state machine
enum class GameMode {
  Menu,
  Help,
  Splash,
  Game
};

enum class MenuOption {
  Play,
  Help,
  Credits
};

// Player state tracking
struct PlayerState {
  bool isHuman;
  bool is_armed;
};

struct LevelConfig {
  int left;
  int right;
  int top;
  int bottom;
  int spawnX;
  int spawnY;
};

enum class ItemType {
  Sword,
  String,
  Key
};

struct WorldItem {
  ItemType type;
  int x;
  int y;
  bool collected;
};

struct InventoryState {
  bool hasSword;
  bool hasString;
  bool hasKey;
};

// Global game state
GameMode current_mode = GameMode::Menu;
PlayerState player = {true, false};
InventoryState inventory = {false, false, false};

// Game variables
int playerx = 16;
int playery = 16;
int select_pos = 28;
int splash = 6;

constexpr uint8_t LEVEL_COUNT = 3;
const LevelConfig levels[LEVEL_COUNT] = {
  {0, 128, 0, 64, 16, 16},
  {16, 112, 16, 48, 24, 24},
  {8, 120, 8, 56, 56, 24}
};
uint8_t current_level = 0;
WorldItem levelItems[3] = {
  {ItemType::Sword, 32, 16, false},
  {ItemType::String, 64, 16, false},
  {ItemType::Key, 80, 32, false}
};

// Collision boundaries (playable area)
constexpr int PLAYER_WIDTH = 16;
constexpr int PLAYER_HEIGHT = 16;

// Splash screen configuration
constexpr uint8_t SPLASH_COUNT = 7;
const uint8_t* const splashScreens[SPLASH_COUNT] = {
  mx_0,
  mx_1,
  mx_2,
  mx_3,
  mx_4,
  mx_5,
  mx_6
};

// Collision detection functions
bool canMoveTo(int x, int y) {
  const LevelConfig& level = levels[current_level];
  return x >= level.left &&
         x + PLAYER_WIDTH <= level.right &&
         y >= level.top &&
         y + PLAYER_HEIGHT <= level.bottom;
}

bool rectanglesOverlap(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {
  return x1 < x2 + w2 &&
         x1 + w1 > x2 &&
         y1 < y2 + h2 &&
         y1 + h1 > y2;
}

const uint8_t* spriteForItem(ItemType type) {
  if (type == ItemType::Sword) return sword;
  if (type == ItemType::String) return string;
  return key;
}

void setInventoryItem(ItemType type, bool value) {
  if (type == ItemType::Sword) inventory.hasSword = value;
  else if (type == ItemType::String) inventory.hasString = value;
  else inventory.hasKey = value;
}

void resetLevelItems() {
  if (current_level == 0) {
    levelItems[0] = {ItemType::Sword, 32, 16, false};
    levelItems[1] = {ItemType::String, 64, 16, false};
    levelItems[2] = {ItemType::Key, 80, 32, false};
  }
  else if (current_level == 1) {
    levelItems[0] = {ItemType::Sword, 80, 16, false};
    levelItems[1] = {ItemType::String, 32, 32, false};
    levelItems[2] = {ItemType::Key, 48, 16, false};
  }
  else {
    levelItems[0] = {ItemType::Sword, 16, 32, false};
    levelItems[1] = {ItemType::String, 64, 32, false};
    levelItems[2] = {ItemType::Key, 96, 16, false};
  }
}

void collectItemsAtPlayerPosition() {
  // In minotaur form, items are ignored and can be crossed without pickup.
  if (!player.isHuman) {
    return;
  }

  for (uint8_t i = 0; i < 3; ++i) {
    if (levelItems[i].collected) {
      continue;
    }
    if (rectanglesOverlap(playerx, playery, PLAYER_WIDTH, PLAYER_HEIGHT,
                          levelItems[i].x, levelItems[i].y, PLAYER_WIDTH, PLAYER_HEIGHT)) {
      levelItems[i].collected = true;
      setInventoryItem(levelItems[i].type, true);
      if (levelItems[i].type == ItemType::Sword) {
        player.is_armed = true;
      }
    }
  }
}

void drawLevelItems() {
  for (uint8_t i = 0; i < 3; ++i) {
    if (!levelItems[i].collected) {
      Sprites::drawOverwrite(levelItems[i].x, levelItems[i].y, spriteForItem(levelItems[i].type), 0);
    }
  }
}

void restrictPlayerPosition() {
  const LevelConfig& level = levels[current_level];
  if (playerx < level.left) playerx = level.left;
  if (playerx + PLAYER_WIDTH > level.right) playerx = level.right - PLAYER_WIDTH;
  if (playery < level.top) playery = level.top;
  if (playery + PLAYER_HEIGHT > level.bottom) playery = level.bottom - PLAYER_HEIGHT;
}

void startLevel(uint8_t levelIndex) {
  current_level = levelIndex % LEVEL_COUNT;
  playerx = levels[current_level].spawnX;
  playery = levels[current_level].spawnY;
  player.isHuman = true;
  player.is_armed = false;
  inventory = {false, false, false};
  resetLevelItems();
  restrictPlayerPosition();
}

MenuOption selectedMenuOption() {
  uint8_t optionIndex = (select_pos - 28) / 8;
  if (optionIndex == 0) {
    return MenuOption::Play;
  }
  if (optionIndex == 1) {
    return MenuOption::Help;
  }
  return MenuOption::Credits;
}

void setup() {
  arduboy.begin();
  arduboy.setFrameRate(60);
  arduboy.clear();
}

void loop() {
  if (!arduboy.nextFrame()) {
    return;
  }
  arduboy.clear();
  arduboy.pollButtons();

  switch (current_mode) {
    case GameMode::Help:
      handleHelpScreen();
      break;
    case GameMode::Splash:
      handleSplashScreen();
      break;
    case GameMode::Game:
      handleGameplay();
      break;
    case GameMode::Menu:
      handleMenu();
      break;
  }
}

void handleHelpScreen() {
  if (arduboy.pressed(A_BUTTON) && arduboy.pressed(B_BUTTON)) {
    current_mode = GameMode::Menu;
    return;
  }

  Sprites::drawOverwrite(0, 0, background, 0);

  // Overlay controls legend to distinguish Help from Menu
  arduboy.setCursor(2, 2);
  arduboy.print(F("-- CONTROLS --"));
  arduboy.setCursor(2, 14);
  arduboy.print(F("Arrows : move"));
  arduboy.setCursor(2, 22);
  arduboy.print(F("B      : transform"));
  arduboy.setCursor(2, 30);
  arduboy.print(F("A+B    : menu"));

  arduboy.display();
}

void handleSplashScreen() {
  if (arduboy.pressed(A_BUTTON) && arduboy.pressed(B_BUTTON)) {
    current_mode = GameMode::Menu;
    return;
  }

  Sprites::drawOverwrite(0, 0, splashScreens[splash], 0);

  if (arduboy.justPressed(B_BUTTON)) {
    splash = (splash + 1) % SPLASH_COUNT;
  }

  arduboy.display();
}

void handleGameplay() {
  // Return to menu if A + B pressed simultaneously
  if (arduboy.pressed(A_BUTTON) && arduboy.pressed(B_BUTTON)) {
    current_mode = GameMode::Menu;
    return;
  }

  const uint8_t* backgroundTile = empty;
  if (current_level == 1) {
    backgroundTile = straight;
  }
  else if (current_level == 2) {
    backgroundTile = dot;
  }

  // Draw background
  for (int backgroundx = 16; backgroundx < 112; backgroundx = backgroundx + 16) {
    for (int backgroundy = 16; backgroundy < 48; backgroundy = backgroundy + 16) {
      Sprites::drawOverwrite(backgroundx, backgroundy, backgroundTile, 0);
    }
  }

  // Handle button inputs for player state
  if (arduboy.justPressed(A_BUTTON) && inventory.hasSword) {
    player.is_armed = !player.is_armed;
  }
  else if (arduboy.justPressed(B_BUTTON)) {
    player.isHuman = !player.isHuman;
  }

  // Draw UI borders
  Sprites::drawOverwrite(0, 0, cross, 0);
  Sprites::drawOverwrite(0, 16, straight, 0);
  Sprites::drawOverwrite(0, 32, straight, 0);
  Sprites::drawOverwrite(0, 48, cross, 0);
  Sprites::drawOverwrite(112, 0, cross, 0);
  Sprites::drawOverwrite(112, 16, straight, 0);
  Sprites::drawOverwrite(112, 32, straight, 0);
  Sprites::drawOverwrite(112, 48, cross, 0);

  // Draw top UI row
  Sprites::drawOverwrite(16, 0, output, 0);
  Sprites::drawOverwrite(32, 0, dot, 0);
  Sprites::drawOverwrite(48, 0, dot, 0);
  Sprites::drawOverwrite(64, 0, dot, 0);
  Sprites::drawOverwrite(80, 0, dot, 0);
  Sprites::drawOverwrite(96, 0, dot, 0);

  Sprites::drawOverwrite(16, 48, dot, 0);
  Sprites::drawOverwrite(32, 48, dot, 0);
  Sprites::drawOverwrite(48, 48, dot, 0);
  // Draw bottom UI row with text labels
  if (inventory.hasSword) {
    arduboy.setCursor(16, 56);
    arduboy.print("S");
  }
  if (inventory.hasString) {
    arduboy.setCursor(32, 56);
    arduboy.print("Y");
  }
  if (inventory.hasKey) {
    arduboy.setCursor(48, 56);
    arduboy.print("K");
  }
  Sprites::drawOverwrite(64, 48, dot, 0);
  Sprites::drawOverwrite(80, 48, dot, 0);
  Sprites::drawOverwrite(96, 48, input, 0);

  // Collect items first, then draw so pickup takes effect in the same frame
  collectItemsAtPlayerPosition();
  drawLevelItems();

  arduboy.setCursor(2, 56);
  arduboy.print("L");
  arduboy.print(current_level + 1);

  // Handle player movement with collision detection
  if (arduboy.pressed(LEFT_BUTTON)) {
    int newX = playerx - 1;
    if (canMoveTo(newX, playery)) playerx = newX;
  }
  if (arduboy.pressed(RIGHT_BUTTON)) {
    int newX = playerx + 1;
    if (canMoveTo(newX, playery)) playerx = newX;
  }
  if (arduboy.pressed(UP_BUTTON)) {
    int newY = playery - 1;
    if (canMoveTo(playerx, newY)) playery = newY;
  }
  if (arduboy.pressed(DOWN_BUTTON)) {
    int newY = playery + 1;
    if (canMoveTo(playerx, newY)) playery = newY;
  }

  // Single display call per frame covers all state changes (movement, HUD, items)
  drawPlayerSprite();
  arduboy.display();
}

void handleMenu() {
  if (arduboy.justPressed(UP_BUTTON)) {
    if (select_pos > 28) {
      select_pos = select_pos - 8;
    }
  }
  else if (arduboy.justPressed(DOWN_BUTTON)) {
    if (select_pos < 44) {
      select_pos = select_pos + 8;
    }
  }
  else if (arduboy.justPressed(A_BUTTON)) {
    MenuOption option = selectedMenuOption();
    if (option == MenuOption::Play) {
      startLevel(0);
      current_mode = GameMode::Game;
    }
    else if (option == MenuOption::Help) {
      current_mode = GameMode::Help;
    }
    else if (option == MenuOption::Credits) {
      splash = 0;
      current_mode = GameMode::Splash;
    }
  }

  Sprites::drawOverwrite(0, 0, background, 0);
  Sprites::drawOverwrite(48, select_pos, select, 0);
  arduboy.display();
}

void drawPlayerSprite() {
  if (!player.isHuman) {
    if (arduboy.pressed(LEFT_BUTTON)) Sprites::drawOverwrite(playerx, playery, minleft, 0);
    else if (arduboy.pressed(RIGHT_BUTTON)) Sprites::drawOverwrite(playerx, playery, minright, 0);
    else if (arduboy.pressed(UP_BUTTON)) Sprites::drawOverwrite(playerx, playery, minback, 0);
    else if (arduboy.pressed(DOWN_BUTTON)) Sprites::drawOverwrite(playerx, playery, minfront, 0);
  }
  else if (player.is_armed) {
    if (arduboy.pressed(LEFT_BUTTON)) Sprites::drawOverwrite(playerx, playery, armed_left, 0);
    else if (arduboy.pressed(RIGHT_BUTTON)) Sprites::drawOverwrite(playerx, playery, armed_right, 0);
    else if (arduboy.pressed(UP_BUTTON)) Sprites::drawOverwrite(playerx, playery, armed_back, 0);
    else if (arduboy.pressed(DOWN_BUTTON)) Sprites::drawOverwrite(playerx, playery, armed_front, 0);
  }
  else {
    if (arduboy.pressed(LEFT_BUTTON)) Sprites::drawOverwrite(playerx, playery, left, 0);
    else if (arduboy.pressed(RIGHT_BUTTON)) Sprites::drawOverwrite(playerx, playery, right, 0);
    else if (arduboy.pressed(UP_BUTTON)) Sprites::drawOverwrite(playerx, playery, back, 0);
    else if (arduboy.pressed(DOWN_BUTTON)) Sprites::drawOverwrite(playerx, playery, front, 0);
  }
}
