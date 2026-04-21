#include <Arduboy2.h>
#include "sprites.h"

Arduboy2 arduboy;

// Game mode enumeration for state machine
enum class GameMode {
  Menu,
  Splash,
  Game
};

// Player state tracking
struct PlayerState {
  bool is_minos;
  bool is_armed;
};

// Global game state
GameMode current_mode = GameMode::Menu;
PlayerState player = {true, false};

// Game variables
int playerx = 16;
int playery = 16;
int select_pos = 28;
int splash = 6;

// Collision boundaries (playable area)
constexpr int COLLISION_LEFT = 0;
constexpr int COLLISION_RIGHT = 128;
constexpr int COLLISION_TOP = 0;
constexpr int COLLISION_BOTTOM = 64;
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
  return x >= COLLISION_LEFT && 
         x + PLAYER_WIDTH <= COLLISION_RIGHT && 
         y >= COLLISION_TOP && 
         y + PLAYER_HEIGHT <= COLLISION_BOTTOM;
}

void restrictPlayerPosition() {
  if (playerx < COLLISION_LEFT) playerx = COLLISION_LEFT;
  if (playerx + PLAYER_WIDTH > COLLISION_RIGHT) playerx = COLLISION_RIGHT - PLAYER_WIDTH;
  if (playery < COLLISION_TOP) playery = COLLISION_TOP;
  if (playery + PLAYER_HEIGHT > COLLISION_BOTTOM) playery = COLLISION_BOTTOM - PLAYER_HEIGHT;
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

void handleSplashScreen() {
  if (arduboy.justPressed(B_BUTTON)) {
    splash = (splash + 1) % SPLASH_COUNT;
    Sprites::drawOverwrite(0, 0, splashScreens[splash], 0);
    arduboy.display();
  }
}

void handleGameplay() {
  // Draw background
  for (int backgroundx = 16; backgroundx < 112; backgroundx = backgroundx + 16) {
    for (int backgroundy = 16; backgroundy < 48; backgroundy = backgroundy + 16) {
      Sprites::drawOverwrite(backgroundx, backgroundy, empty, 0);
    }
  }

  // Handle button inputs for player state
  if (arduboy.justPressed(A_BUTTON)) {
    player.is_armed = !player.is_armed;
  }
  else if (arduboy.justPressed(B_BUTTON)) {
    player.is_minos = !player.is_minos;
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

  // Draw bottom UI row
  Sprites::drawOverwrite(16, 48, sword, 0);
  Sprites::drawOverwrite(32, 48, string, 0);
  Sprites::drawOverwrite(48, 48, key, 0);
  Sprites::drawOverwrite(64, 48, dot, 0);
  Sprites::drawOverwrite(80, 48, dot, 0);
  Sprites::drawOverwrite(96, 48, input, 0);

  // Handle player movement with collision detection
  if (arduboy.pressed(LEFT_BUTTON)) {
    int newX = playerx - 1;
    if (canMoveTo(newX, playery)) {
      playerx = newX;
      drawPlayerSprite();
      arduboy.display();
    }
  }
  if (arduboy.pressed(RIGHT_BUTTON)) {
    int newX = playerx + 1;
    if (canMoveTo(newX, playery)) {
      playerx = newX;
      drawPlayerSprite();
      arduboy.display();
    }
  }
  if (arduboy.pressed(UP_BUTTON)) {
    int newY = playery - 1;
    if (canMoveTo(playerx, newY)) {
      playery = newY;
      drawPlayerSprite();
      arduboy.display();
    }
  }
  if (arduboy.pressed(DOWN_BUTTON)) {
    int newY = playery + 1;
    if (canMoveTo(playerx, newY)) {
      playery = newY;
      drawPlayerSprite();
      arduboy.display();
    }
  }
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
    current_mode = GameMode::Game;
  }
  else if (arduboy.justPressed(B_BUTTON)) {
    current_mode = GameMode::Splash;
  }

  Sprites::drawOverwrite(0, 0, qrcode, 0);
  Sprites::drawOverwrite(48, select_pos, select, 0);
  arduboy.display();
}

void drawPlayerSprite() {
  if (!player.is_minos) {
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
