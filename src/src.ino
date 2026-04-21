#include <Arduboy2.h>
#include "sprites.h"

Arduboy2 arduboy;

int playerx = 16;
int playery = 16;
bool is_minos = true;
bool is_armed = false;
bool in_game = false;
bool in_splash = false;
int select_pos = 28;
int splash = 6;

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

  if (in_splash) {
    if (arduboy.justPressed(B_BUTTON)) {
      if (splash >= 6) splash = 0;
      else splash = splash + 1;

      if (splash == 0) Sprites::drawOverwrite(0, 0, mx_0, 0);
      else if (splash == 1) Sprites::drawOverwrite(0, 0, mx_1, 0);
      else if (splash == 2) Sprites::drawOverwrite(0, 0, mx_2, 0);
      else if (splash == 3) Sprites::drawOverwrite(0, 0, mx_3, 0);
      else if (splash == 4) Sprites::drawOverwrite(0, 0, mx_4, 0);
      else if (splash == 5) Sprites::drawOverwrite(0, 0, mx_5, 0);
      else Sprites::drawOverwrite(0, 0, mx_6, 0);

      arduboy.display();
    }
  }
  else if (in_game) {
    //For each column on the screen
    for (int backgroundx = 16; backgroundx < 112; backgroundx = backgroundx + 16) {
      //For each row in the column
      for ( int backgroundy = 16; backgroundy < 48; backgroundy = backgroundy + 16) {
        //Draw a background tile
        Sprites::drawOverwrite(backgroundx, backgroundy, empty, 0);
      }
    }

    if (arduboy.justPressed(A_BUTTON)) {
      is_armed = !is_armed;
    }
    else if (arduboy.justPressed(B_BUTTON)) {
      is_minos = !is_minos;
    }

    Sprites::drawOverwrite(0, 0, cross, 0);
    Sprites::drawOverwrite(0, 16, straight, 0);
    Sprites::drawOverwrite(0, 32, straight, 0);
    Sprites::drawOverwrite(0, 48, cross, 0);
    Sprites::drawOverwrite(112, 0, cross, 0);
    Sprites::drawOverwrite(112, 16, straight, 0);
    Sprites::drawOverwrite(112, 32, straight, 0);
    Sprites::drawOverwrite(112, 48, cross, 0);
    Sprites::drawOverwrite(16, 0, output, 0);
    Sprites::drawOverwrite(32, 0, dot, 0);
    Sprites::drawOverwrite(48, 0, dot, 0);
    Sprites::drawOverwrite(64, 0, dot, 0);
    Sprites::drawOverwrite(80, 0, dot, 0);
    Sprites::drawOverwrite(96, 0, dot, 0);
    Sprites::drawOverwrite(16, 48, sword, 0);
    Sprites::drawOverwrite(32, 48, string, 0);
    Sprites::drawOverwrite(48, 48, key, 0);
    Sprites::drawOverwrite(64, 48, dot, 0);
    Sprites::drawOverwrite(80, 48, dot, 0);
    Sprites::drawOverwrite(96, 48, input, 0);

    if (arduboy.pressed(LEFT_BUTTON)) {
      if (playerx <= -16) playerx = 128 + 16;
      else playerx = playerx - 1;
      //Draw player sprite
      if (!is_minos) Sprites::drawOverwrite(playerx, playery, minleft, 0);
      else if (is_armed) Sprites::drawOverwrite(playerx, playery, armed_left, 0);
      else Sprites::drawOverwrite(playerx, playery, left, 0);
      arduboy.display();
    }
    if (arduboy.pressed(RIGHT_BUTTON)) {
      if (playerx >= 128) playerx = -16;
      else playerx = playerx + 1;
      //Draw player sprite
      if (!is_minos) Sprites::drawOverwrite(playerx, playery, minright, 0);
      else if (is_armed) Sprites::drawOverwrite(playerx, playery, armed_right, 0);
      else Sprites::drawOverwrite(playerx, playery, right, 0);
      arduboy.display();
    }
    if (arduboy.pressed(UP_BUTTON)) {
      if (playery <= -16) playery = 64 + 16;
      else playery = playery - 1;
      //Draw player sprite
      if (!is_minos) Sprites::drawOverwrite(playerx, playery, minback, 0);
      else if (is_armed) Sprites::drawOverwrite(playerx, playery, armed_back, 0);
      else Sprites::drawOverwrite(playerx, playery, back, 0);
      arduboy.display();
    }
    if (arduboy.pressed(DOWN_BUTTON)) {
      if (playery >= 64) playery = -16;
      else playery = playery + 1;
      //Draw player sprite
      if (!is_minos) Sprites::drawOverwrite(playerx, playery, minfront, 0);
      else if (is_armed) Sprites::drawOverwrite(playerx, playery, armed_front, 0);
      else Sprites::drawOverwrite(playerx, playery, front, 0);
      arduboy.display();
    }
  }
  else {
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
      in_game = true;
    }
    else if (arduboy.justPressed(B_BUTTON)) {
      in_splash = true;
    }
    Sprites::drawOverwrite(0, 0, qrcode, 0);
    Sprites::drawOverwrite(48, select_pos, select, 0);
    arduboy.display();
  }
}
