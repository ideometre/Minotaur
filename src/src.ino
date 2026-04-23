#include <Arduboy2.h>
#include "sprites.h"

Arduboy2 arduboy;

// Game mode enumeration for state machine
enum class GameMode
{
  Menu,
  Help,
  Splash,
  Game
};

enum class MenuOption
{
  Play,
  Help,
  Credits
};

// Player state tracking
struct PlayerState
{
  bool isHuman;
  bool is_armed;
};

enum class FacingDirection
{
  Left,
  Right,
  Up,
  Down
};

struct LevelConfig
{
  int left;
  int right;
  int top;
  int bottom;
  int spawnX;
  int spawnY;
};

enum class ItemType
{
  Sword,
  String,
  Key
};

struct WorldItem
{
  ItemType type;
  uint8_t screen; // which of the SCREEN_COUNT screens this item lives on
  int x;
  int y;
  bool collected;
};

struct InventoryState
{
  bool hasSword;
  bool hasString;
  bool hasKey;
};

struct GameState
{
  GameMode mode;
  PlayerState player;
  InventoryState inventory;
  int playerX;
  int playerY;
  int menuCursorY;
  int splashIndex;
  FacingDirection facing;
  uint8_t currentScreen;
};

// Runtime state lives in one container for easier maintenance.
GameState game = {
    GameMode::Menu,
    {true, false},
    {false, false, false},
    16,
    16,
    28,
    6,
    FacingDirection::Down,
    0 // currentScreen
};

// Aliases keep existing logic readable while still centralizing state.
GameMode &current_mode = game.mode;
PlayerState &player = game.player;
InventoryState &inventory = game.inventory;
int &playerx = game.playerX;
int &playery = game.playerY;
int &select_pos = game.menuCursorY;
int &splash = game.splashIndex;
FacingDirection &playerFacing = game.facing;
uint8_t &current_screen = game.currentScreen;

// Single playfield config (one bounded area, shared by all screens)
const LevelConfig level = {16, 112, 16, 48, 32, 24};

WorldItem levelItems[3] = {
    {ItemType::Sword, 0, 32, 16, false},
    {ItemType::String, 1, 64, 16, false},
    {ItemType::Key, 2, 80, 32, false}};

// Collision boundaries (playable area)
constexpr int PLAYER_WIDTH = 16;
constexpr int PLAYER_HEIGHT = 16;
constexpr int WALL_THICKNESS = 16;
// Screen grid: 2×2 rooms navigable by walking to the edge
constexpr uint8_t SCREEN_COLS = 2;
constexpr uint8_t SCREEN_ROWS = 2;
constexpr uint8_t SCREEN_COUNT = SCREEN_COLS * SCREEN_ROWS;

// Menu layout constants
constexpr int MENU_CURSOR_X = 48;
constexpr int MENU_CURSOR_TOP = 28;
constexpr int MENU_CURSOR_BOTTOM = 44;
constexpr int MENU_CURSOR_STEP = 8;

// Bottom HUD constants
constexpr int HUD_SWORD_X = 16;
constexpr int HUD_STRING_X = 32;
constexpr int HUD_KEY_X = 48;
constexpr int HUD_ITEMS_Y = 56;

// Mini-map constants (2x2 rooms)
constexpr int MINIMAP_X = 2;
constexpr int MINIMAP_Y = 49;
constexpr int MINIMAP_SIZE = 16;
constexpr int MINIMAP_CELL_SIZE = 8;
constexpr int MINIMAP_MARKER_SIZE = 2;

// Splash screen configuration
constexpr uint8_t SPLASH_COUNT = 7;
const uint8_t *const splashScreens[SPLASH_COUNT] = {
    mx_0,
    mx_1,
    mx_2,
    mx_3,
    mx_4,
    mx_5,
    mx_6};

bool hasAdjacentScreen(FacingDirection dir);

// Collision detection functions
bool canMoveTo(int x, int y)
{
  int leftBound = hasAdjacentScreen(FacingDirection::Left) ? 0 : WALL_THICKNESS;
  int rightBound = hasAdjacentScreen(FacingDirection::Right) ? WIDTH : WIDTH - WALL_THICKNESS;
  int topBound = hasAdjacentScreen(FacingDirection::Up) ? 0 : WALL_THICKNESS;
  int bottomBound = hasAdjacentScreen(FacingDirection::Down) ? HEIGHT : HEIGHT - WALL_THICKNESS;

  return x >= leftBound &&
         x + PLAYER_WIDTH <= rightBound &&
         y >= topBound &&
         y + PLAYER_HEIGHT <= bottomBound;
}

bool rectanglesOverlap(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2)
{
  return x1 < x2 + w2 &&
         x1 + w1 > x2 &&
         y1 < y2 + h2 &&
         y1 + h1 > y2;
}

const uint8_t *spriteForItem(ItemType type)
{
  if (type == ItemType::Sword)
    return sword;
  if (type == ItemType::String)
    return string;
  return key;
}

void setInventoryItem(ItemType type, bool value)
{
  if (type == ItemType::Sword)
    inventory.hasSword = value;
  else if (type == ItemType::String)
    inventory.hasString = value;
  else
    inventory.hasKey = value;
}

void resetItems()
{
  levelItems[0] = {ItemType::Sword, 0, 32, 24, false};
  levelItems[1] = {ItemType::String, 2, 64, 24, false};
  levelItems[2] = {ItemType::Key, 3, 80, 24, false};
}

void collectItemsAtPlayerPosition()
{
  // In minotaur form, items are ignored and can be crossed without pickup.
  if (!player.isHuman)
  {
    return;
  }

  for (uint8_t i = 0; i < 3; ++i)
  {
    if (levelItems[i].collected || levelItems[i].screen != current_screen)
    {
      continue;
    }
    if (rectanglesOverlap(playerx, playery, PLAYER_WIDTH, PLAYER_HEIGHT,
                          levelItems[i].x, levelItems[i].y, PLAYER_WIDTH, PLAYER_HEIGHT))
    {
      levelItems[i].collected = true;
      setInventoryItem(levelItems[i].type, true);
      if (levelItems[i].type == ItemType::Sword)
      {
        player.is_armed = true;
      }
    }
  }
}

void drawLevelItems()
{
  for (uint8_t i = 0; i < 3; ++i)
  {
    if (!levelItems[i].collected && levelItems[i].screen == current_screen)
    {
      Sprites::drawOverwrite(levelItems[i].x, levelItems[i].y, spriteForItem(levelItems[i].type), 0);
    }
  }
}

void restrictPlayerPosition()
{
  int leftBound = hasAdjacentScreen(FacingDirection::Left) ? 0 : WALL_THICKNESS;
  int rightBound = hasAdjacentScreen(FacingDirection::Right) ? WIDTH : WIDTH - WALL_THICKNESS;
  int topBound = hasAdjacentScreen(FacingDirection::Up) ? 0 : WALL_THICKNESS;
  int bottomBound = hasAdjacentScreen(FacingDirection::Down) ? HEIGHT : HEIGHT - WALL_THICKNESS;

  if (playerx < leftBound)
    playerx = leftBound;
  if (playerx + PLAYER_WIDTH > rightBound)
    playerx = rightBound - PLAYER_WIDTH;
  if (playery < topBound)
    playery = topBound;
  if (playery + PLAYER_HEIGHT > bottomBound)
    playery = bottomBound - PLAYER_HEIGHT;
}

void startGame()
{
  current_screen = 0;
  playerx = level.spawnX;
  playery = level.spawnY;
  playerFacing = FacingDirection::Down;
  player.isHuman = true;
  player.is_armed = false;
  inventory = {false, false, false};
  resetItems();
  restrictPlayerPosition();
}

bool tryReturnToMenu()
{
  if (arduboy.pressed(A_BUTTON) && arduboy.pressed(B_BUTTON))
  {
    current_mode = GameMode::Menu;
    return true;
  }
  return false;
}

bool hasAdjacentScreen(FacingDirection dir)
{
  uint8_t col = current_screen % SCREEN_COLS;
  uint8_t row = current_screen / SCREEN_COLS;

  switch (dir)
  {
  case FacingDirection::Right:
    return col < SCREEN_COLS - 1;
  case FacingDirection::Left:
    return col > 0;
  case FacingDirection::Down:
    return row < SCREEN_ROWS - 1;
  case FacingDirection::Up:
    return row > 0;
  }

  return false;
}

// Draw a wall sprite variant selected by rotation.
// rotation: 0=0°, 1=90° CW, 2=180°, 3=270° CW.
void drawRotated16Overwrite(int x, int y, const uint8_t *sprite, uint8_t rotation)
{
  const uint8_t *variant = sprite;
  uint8_t rot = rotation & 0x03;

  if (sprite == straight)
  {
    variant = (rot % 2 == 0) ? straight : straight_h;
  }
  else if (sprite == angle)
  {
    if (rot == 1)
      variant = angle_r1;
    else if (rot == 2)
      variant = angle_r2;
    else if (rot == 3)
      variant = angle_r3;
  }

  Sprites::drawOverwrite(x, y, variant, 0);
}

void drawOuterLabyrinthWalls()
{
  // Draw only the external walls of the global 2x2 labyrinth using wall sprites.
  if (!hasAdjacentScreen(FacingDirection::Left))
  {
    for (int y = 0; y < HEIGHT; y += 16)
    {
      drawRotated16Overwrite(0, y, straight, 0);
    }
  }
  else
  {
    // Transition edge: keep markers only where this side touches outer borders.
    if (!hasAdjacentScreen(FacingDirection::Up))
    {
      drawRotated16Overwrite(0, 0, straight, 1);
    }
    if (!hasAdjacentScreen(FacingDirection::Down))
    {
      drawRotated16Overwrite(0, HEIGHT - 16, straight, 1);
    }
  }

  if (!hasAdjacentScreen(FacingDirection::Right))
  {
    for (int y = 0; y < HEIGHT; y += 16)
    {
      drawRotated16Overwrite(WIDTH - 16, y, straight, 0);
    }
  }
  else
  {
    // Transition edge: keep markers only where this side touches outer borders.
    if (!hasAdjacentScreen(FacingDirection::Up))
    {
      drawRotated16Overwrite(WIDTH - 16, 0, straight, 1);
    }
    if (!hasAdjacentScreen(FacingDirection::Down))
    {
      drawRotated16Overwrite(WIDTH - 16, HEIGHT - 16, straight, 1);
    }
  }

  if (!hasAdjacentScreen(FacingDirection::Up))
  {
    for (int x = level.left; x <= level.right - 16; x += 16)
    {
      drawRotated16Overwrite(x, 0, straight, 1);
    }
  }
  else
  {
    // Transition edge: keep markers only where this side touches outer borders.
    if (!hasAdjacentScreen(FacingDirection::Left))
    {
      drawRotated16Overwrite(0, 0, straight, 0);
    }
    if (!hasAdjacentScreen(FacingDirection::Right))
    {
      drawRotated16Overwrite(WIDTH - 16, 0, straight, 0);
    }
  }

  if (!hasAdjacentScreen(FacingDirection::Down))
  {
    for (int x = level.left; x <= level.right - 16; x += 16)
    {
      drawRotated16Overwrite(x, HEIGHT - 16, straight, 3);
    }
  }
  else
  {
    // Transition edge: keep markers only where this side touches outer borders.
    if (!hasAdjacentScreen(FacingDirection::Left))
    {
      drawRotated16Overwrite(0, HEIGHT - 16, straight, 0);
    }
    if (!hasAdjacentScreen(FacingDirection::Right))
    {
      drawRotated16Overwrite(WIDTH - 16, HEIGHT - 16, straight, 0);
    }
  }

  // Corner caps
  if (!hasAdjacentScreen(FacingDirection::Left) && !hasAdjacentScreen(FacingDirection::Up))
  {
    drawRotated16Overwrite(0, 0, angle, 0);
  }
  if (!hasAdjacentScreen(FacingDirection::Right) && !hasAdjacentScreen(FacingDirection::Up))
  {
    drawRotated16Overwrite(WIDTH - 16, 0, angle, 1);
  }
  if (!hasAdjacentScreen(FacingDirection::Left) && !hasAdjacentScreen(FacingDirection::Down))
  {
    drawRotated16Overwrite(0, HEIGHT - 16, angle, 3);
  }
  if (!hasAdjacentScreen(FacingDirection::Right) && !hasAdjacentScreen(FacingDirection::Down))
  {
    drawRotated16Overwrite(WIDTH - 16, HEIGHT - 16, angle, 2);
  }
}

// Attempt to transition to an adjacent screen in the given direction.
// If no adjacent screen exists, the wall simply blocks movement.
void checkScreenTransition(FacingDirection dir)
{
  uint8_t col = current_screen % SCREEN_COLS;

  switch (dir)
  {
  case FacingDirection::Right:
    if (hasAdjacentScreen(FacingDirection::Right))
    {
      current_screen++;
      playerx = 0;
    }
    break;
  case FacingDirection::Left:
    if (hasAdjacentScreen(FacingDirection::Left))
    {
      current_screen--;
      playerx = WIDTH - PLAYER_WIDTH;
    }
    break;
  case FacingDirection::Down:
    if (hasAdjacentScreen(FacingDirection::Down))
    {
      current_screen += SCREEN_COLS;
      playery = 0;
    }
    break;
  case FacingDirection::Up:
    if (hasAdjacentScreen(FacingDirection::Up))
    {
      current_screen -= SCREEN_COLS;
      playery = HEIGHT - PLAYER_HEIGHT;
    }
    break;
  }

  restrictPlayerPosition();
}

void drawGameplayBackground()
{
  // Fill the full room with the default empty tile.
  for (int x = 0; x < WIDTH; x += 16)
  {
    for (int y = 0; y < HEIGHT; y += 16)
    {
      Sprites::drawOverwrite(x, y, empty, 0);
    }
  }

  // Keep only the external labyrinth walls over the default background.
  drawOuterLabyrinthWalls();
}

void handlePlayerStateInput()
{
  if (arduboy.justPressed(A_BUTTON) && inventory.hasSword)
  {
    player.is_armed = !player.is_armed;
  }
  else if (arduboy.justPressed(B_BUTTON))
  {
    player.isHuman = !player.isHuman;
  }
}

void drawGameplayHud()
{
  // Inventory labels
  if (inventory.hasSword)
  {
    arduboy.setCursor(HUD_SWORD_X, HUD_ITEMS_Y);
    arduboy.print("S");
  }
  if (inventory.hasString)
  {
    arduboy.setCursor(HUD_STRING_X, HUD_ITEMS_Y);
    arduboy.print("Y");
  }
  if (inventory.hasKey)
  {
    arduboy.setCursor(HUD_KEY_X, HUD_ITEMS_Y);
    arduboy.print("K");
  }

  // Mini-map: solid black 16x16, split into 4 cells
  arduboy.fillRect(MINIMAP_X, MINIMAP_Y, MINIMAP_SIZE, MINIMAP_SIZE, BLACK);
  arduboy.drawRect(MINIMAP_X, MINIMAP_Y, MINIMAP_SIZE, MINIMAP_SIZE, WHITE);
  arduboy.drawFastVLine(MINIMAP_X + MINIMAP_CELL_SIZE, MINIMAP_Y, MINIMAP_SIZE, WHITE);
  arduboy.drawFastHLine(MINIMAP_X, MINIMAP_Y + MINIMAP_CELL_SIZE, MINIMAP_SIZE, WHITE);

  uint8_t screenCol = current_screen % SCREEN_COLS;
  uint8_t screenRow = current_screen / SCREEN_COLS;
  int markerX = MINIMAP_X + screenCol * MINIMAP_CELL_SIZE + (MINIMAP_CELL_SIZE - MINIMAP_MARKER_SIZE) / 2;
  int markerY = MINIMAP_Y + screenRow * MINIMAP_CELL_SIZE + (MINIMAP_CELL_SIZE - MINIMAP_MARKER_SIZE) / 2;
  arduboy.fillRect(markerX, markerY, MINIMAP_MARKER_SIZE, MINIMAP_MARKER_SIZE, WHITE);
}

void handlePlayerMovement()
{
  if (arduboy.pressed(LEFT_BUTTON))
  {
    playerFacing = FacingDirection::Left;
    int newX = playerx - 1;
    if (canMoveTo(newX, playery))
      playerx = newX;
    else
      checkScreenTransition(FacingDirection::Left);
  }
  if (arduboy.pressed(RIGHT_BUTTON))
  {
    playerFacing = FacingDirection::Right;
    int newX = playerx + 1;
    if (canMoveTo(newX, playery))
      playerx = newX;
    else
      checkScreenTransition(FacingDirection::Right);
  }
  if (arduboy.pressed(UP_BUTTON))
  {
    playerFacing = FacingDirection::Up;
    int newY = playery - 1;
    if (canMoveTo(playerx, newY))
      playery = newY;
    else
      checkScreenTransition(FacingDirection::Up);
  }
  if (arduboy.pressed(DOWN_BUTTON))
  {
    playerFacing = FacingDirection::Down;
    int newY = playery + 1;
    if (canMoveTo(playerx, newY))
      playery = newY;
    else
      checkScreenTransition(FacingDirection::Down);
  }
}

MenuOption selectedMenuOption()
{
  uint8_t optionIndex = (select_pos - MENU_CURSOR_TOP) / MENU_CURSOR_STEP;
  if (optionIndex == 0)
  {
    return MenuOption::Play;
  }
  if (optionIndex == 1)
  {
    return MenuOption::Help;
  }
  return MenuOption::Credits;
}

void setup()
{
  arduboy.begin();
  arduboy.setFrameRate(60);
  arduboy.clear();
}

void loop()
{
  if (!arduboy.nextFrame())
  {
    return;
  }
  arduboy.clear();
  arduboy.pollButtons();

  switch (current_mode)
  {
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

void handleHelpScreen()
{
  if (tryReturnToMenu())
  {
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

void handleSplashScreen()
{
  if (tryReturnToMenu())
  {
    return;
  }

  Sprites::drawOverwrite(0, 0, splashScreens[splash], 0);

  if (arduboy.justPressed(B_BUTTON))
  {
    splash = (splash + 1) % SPLASH_COUNT;
  }

  arduboy.display();
}

void handleGameplay()
{
  if (tryReturnToMenu())
  {
    return;
  }

  drawGameplayBackground();
  handlePlayerStateInput();
  drawGameplayHud();

  // Collect items first, then draw so pickup takes effect in the same frame
  collectItemsAtPlayerPosition();
  drawLevelItems();

  handlePlayerMovement();

  // Single display call per frame covers all state changes (movement, HUD, items)
  drawPlayerSprite();
  arduboy.display();
}

void handleMenu()
{
  if (arduboy.justPressed(UP_BUTTON))
  {
    if (select_pos > MENU_CURSOR_TOP)
    {
      select_pos = select_pos - MENU_CURSOR_STEP;
    }
  }
  else if (arduboy.justPressed(DOWN_BUTTON))
  {
    if (select_pos < MENU_CURSOR_BOTTOM)
    {
      select_pos = select_pos + MENU_CURSOR_STEP;
    }
  }
  else if (arduboy.justPressed(A_BUTTON))
  {
    MenuOption option = selectedMenuOption();
    if (option == MenuOption::Play)
    {
      startGame();
      current_mode = GameMode::Game;
    }
    else if (option == MenuOption::Help)
    {
      current_mode = GameMode::Help;
    }
    else if (option == MenuOption::Credits)
    {
      splash = 0;
      current_mode = GameMode::Splash;
    }
  }

  Sprites::drawOverwrite(0, 0, background, 0);
  Sprites::drawOverwrite(MENU_CURSOR_X, select_pos, select, 0);
  arduboy.display();
}

void drawPlayerSprite()
{
  const uint8_t *sprite = nullptr;

  if (!player.isHuman)
  {
    if (playerFacing == FacingDirection::Left)
      sprite = minleft;
    else if (playerFacing == FacingDirection::Right)
      sprite = minright;
    else if (playerFacing == FacingDirection::Up)
      sprite = minback;
    else
      sprite = minfront;
  }
  else if (player.is_armed)
  {
    if (playerFacing == FacingDirection::Left)
      sprite = armed_left;
    else if (playerFacing == FacingDirection::Right)
      sprite = armed_right;
    else if (playerFacing == FacingDirection::Up)
      sprite = armed_back;
    else
      sprite = armed_front;
  }
  else
  {
    if (playerFacing == FacingDirection::Left)
      sprite = left;
    else if (playerFacing == FacingDirection::Right)
      sprite = right;
    else if (playerFacing == FacingDirection::Up)
      sprite = back;
    else
      sprite = front;
  }

  Sprites::drawOverwrite(playerx, playery, sprite, 0);
}
