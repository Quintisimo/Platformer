#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curses.h>
#include <cab202_graphics.h>
#include <cab202_sprites.h>
#include <cab202_timers.h>

#define DELAY (10)
#define HERO_HEIGHT (3)
#define HERO_WIDTH (3)
#define DOOR_HEIGHT (4)
#define DOOR_WIDTH (4)

bool game_over = false;
bool update_screen = true;
bool spin = true;
bool can_unlock = false;
int key;
int lives = 10;
int level = 1;
int score = 0;

timer_id my_timer;
int timer;
int timer_2;

sprite_id hero;
char * hero_image =
      " 0 "
      "/|\\"
      "/'\\ ";

char * hero_jump_image =
      " 0 "
      "\\|/"
      "/'\\ ";

sprite_id door;
char * door_image =
      "EXIT"
      "|  |"
      "| _|"
      "|  |";

sprite_id platform;
sprite_id bottom_platform;
sprite_id bottom_platform_2;
sprite_id top_platform;
char * platform_image = "=============================================================================================================================";

sprite_id vertical_platform;
char * vertical_platform_image =
      "|"
      "|"
      "|"
      "|"
      "|"
      "|"
      "|";

sprite_id unlock_door;
char * unlock_door_image =
        "[]"
        "[]"
        "[]"
        "[]"
        "[]"
        "[]"
        "[]"
        "[]"
        "[]"
        "[]"
        "[]"
        "[]"
        "[]"
        "[]"
        "[]"
        "[]"
        "[]";

sprite_id zombie;
char * zombie_image =
      "ZZZZ"
      "  Z "
      " Z  "
      "ZZZZ";

sprite_id bat;
char * bat_image =
      " | "
      "/ \\";

char * bat_image_inverted =
      "\\ /"
      " | ";

sprite_id rock;
char * rock_image =
      " ---- "
      "/    \\"
      "------";

sprite_id treasure;
char * treasure_image = "$";

sprite_id door_key;
char * key_image = "0-*";

sprite_id medal;
char * medal_image =
      " - "
      "/ \\"
      "\\ /"
      " e ";

sprite_id lost;
char * lost_image =
  " ----------------------- "
  "|       Game Over       |"
  "| Press any key to exit |"
  " ----------------------- ";

void draw_border(void) {
  int left = 0;
  int right = (screen_width() - 1);
  int top = 1;
  int bottom = (screen_height() - 1);

  draw_line(left, top, right, top, '-');
  draw_line(right, top + 1, right, bottom - 1, '|');
  draw_line(left, top + 1, left, bottom - 1, '|');
}

void draw_sprites(void) {
  draw_border();
  if (timer < 10) {
    draw_formatted(5, 0, "Time: 0%d:0%d", timer_2, timer);
  } else {
    draw_formatted(5, 0, "Time: 0%d:%d", timer_2, timer);
  }
  draw_formatted(25, 0, "Lives: %d", lives);
  draw_formatted(45, 0, "Level: %d", level);
  draw_formatted(65, 0, "Score: %d", score);
  sprite_draw(hero);
  if (level == 1) sprite_draw(zombie);
  sprite_draw(platform);
  if (level < 5)sprite_draw(door);
  sprite_draw(bottom_platform);

  if (level == 2 || level == 3) {
    sprite_draw(bottom_platform_2);
    sprite_draw(bat);
    sprite_draw(treasure);
  }

  if (level == 3) {
    sprite_draw(vertical_platform);
    sprite_draw(top_platform);
  }

  if (level == 4) {
    sprite_draw(top_platform);
    sprite_draw(bottom_platform_2);
    sprite_draw(door_key);
    sprite_draw(unlock_door);
  }

  if (level == 5) {
    sprite_draw(bottom_platform);
    sprite_draw(top_platform);
    sprite_draw(bottom_platform_2);
    sprite_draw(medal);
    sprite_draw(rock);
    // draw_formatted(10, 11, "%f", sprite_x(rock));
    // draw_formatted(10, 10, "%f", sprite_y(rock));
    // draw_formatted(10, 12, "%d", screen_height());
  }
}

bool sprite_collided(sprite_id sprite_1, sprite_id sprite_2) {
  bool collided = true;

  int sprite_1x = round(sprite_x(sprite_1));
  int sprite_1y = round(sprite_y(sprite_1));
  int sprite_1r = sprite_1x + sprite_width(sprite_1) - 1;
  int sprite_1b = sprite_1y + sprite_height(sprite_1) - 1;

  int sprite_2x = round(sprite_x(sprite_2));
  int sprite_2y = round(sprite_y(sprite_2));
  int sprite_2r = sprite_2x + sprite_width(sprite_2) - 1;
  int sprite_2b = sprite_2y + sprite_height(sprite_2) - 1;

  if (sprite_1r < sprite_2x) collided = false;
  if (sprite_1b < sprite_2y) collided = false;
  if (sprite_2r < sprite_1x) collided = false;
  if (sprite_2b < sprite_1y) collided = false;

  return collided;
}

void platform_collision(sprite_id hero, sprite_id platform, bool person) {
  int hx = sprite_x(hero);
  int hy = sprite_y(hero);
  int px = sprite_x(platform);
  int py = sprite_y(platform);
  int hh = sprite_height(hero);

  double hdx = sprite_dx(hero);
  double hdy = sprite_dy(hero);

  if (sprite_collided(hero, platform)) {
    if (person) sprite_set_image(hero, hero_image);

    if (hy == py + sprite_height(platform) - 1 && hdy < 0) {
      hdy = -hdy;
    } else if (hx + sprite_width(hero) == px && hdx > 0) {
      hdx = 0;
    } else if (hx == px + sprite_width(platform) && hdx < 0) {
      hdx = 0;
    } else {
      hdy = 0;
    }
    sprite_back(hero);
  } else if ((hx + sprite_width(hero) < px || hx > px + sprite_width(platform) - 1) && hy + hh == py && hdy == 0) {
    hdy = 0.5;
    if (person) sprite_set_image(hero, hero_jump_image);
  }
  sprite_turn_to(hero, hdx, hdy);
}

void levels(void) {
  int hw = HERO_WIDTH;
  int hh = HERO_HEIGHT;
  hero = sprite_create(2, screen_height() - 4, hw, hh, hero_image);
  sprite_back(hero);
  sprite_turn_to(hero, 0, 0);

  if (level < 5) {
    int dw = DOOR_WIDTH;
    int dh = DOOR_HEIGHT;
    door = sprite_create(screen_width() - 6, screen_height() - 5, dw, dh, door_image);
  }

  if (level == 1) {
    int pw = 30;
    int ph = 1;
    platform = sprite_create((screen_width()/2) - (pw/2), screen_height() - 10, pw, ph, platform_image);

    int bpw = screen_width();
    int bph = 1;
    bottom_platform = sprite_create(0, screen_height() - 1, bpw, bph, platform_image);

    int zw = 4;
    int zh = 4;
    zombie = sprite_create(screen_width() - 5, screen_height() - 5, zw, zh, zombie_image);

    sprite_turn_to(zombie, 0.1, 0);
    sprite_turn(zombie, 180);

  } else if (level == 2) {
    sprite_destroy(zombie);

    int bpw = screen_width() / 3;
    int bph = 1;
    bottom_platform = sprite_create(0, screen_height() - 1.1, bpw, bph, platform_image);
    bottom_platform_2 = sprite_create(screen_width() - bpw, screen_height() - 1, bpw, bph, platform_image);

    int tw = 1;
    int th = 1;
    treasure = sprite_create(screen_width()/2, (screen_height()/2) - 5, tw, th, treasure_image);

    int bw = 3;
    int bh = 2;
    bat = sprite_create(screen_width() - 9, screen_height() - 4, bw, bh, bat_image);
    sprite_turn_to(bat, 0.2, 0);
    sprite_turn(bat, 180);

  } else if (level == 3) {
    int pw = 40;
    int ph = 1;
    platform = sprite_create((screen_width()/2) - (pw/2), screen_height() - 8, pw, ph, platform_image);

    int vw = 1;
    int vh = 7;
    vertical_platform = sprite_create(screen_width()/2, screen_height() - 15, vw, vh, vertical_platform_image);

    int tpw = screen_width() / 3;
    int tph = 1;
    top_platform = sprite_create((screen_width()/2 - (tpw/2)), screen_height() - 16, tpw, tph, platform_image);

    int tw = 1;
    int th = 1;
    treasure = sprite_create(screen_width()/2, (screen_height()/2) - 7, tw, th, treasure_image);

    int bw = 3;
    int bh = 2;
    bat = sprite_create(screen_width() - 9, screen_height() - 20, bw, bh, bat_image);
    sprite_turn_to(bat, 0.2, 0);
    sprite_turn(bat, 180);

  } else if (level == 4) {
    sprite_destroy(bat);

    int bpw = screen_width();
    int bph = 1;
    bottom_platform_2 = sprite_create(0, screen_height() - 1, bpw, bph, platform_image);

    int kw = 3;
    int kh = 1;
    door_key = sprite_create(screen_width() - kw - 2, screen_height() / 4 - screen_height() / 8, kw, kh, key_image);

    int pw = (screen_width() / 3) * 2;
    int ph = 1;
    top_platform = sprite_create(screen_width() - pw - 1, screen_height() / 4, pw, ph, platform_image);
    platform = sprite_create(1, screen_height() / 2, pw, ph, platform_image);
    bottom_platform = sprite_create(screen_width() - pw - 1, (screen_height() / 4) + (screen_height() / 2), pw, ph, platform_image);

    int udw = 2;
    int udh = (screen_height() / 4) - 1;
    unlock_door = sprite_create((screen_width() / 4) + (screen_width() / 2), ((screen_height() / 4) + (screen_height() / 2)) + 1, udw, udh, unlock_door_image);

  } else if (level == 5) {
    sprite_destroy(door_key);
    sprite_destroy(platform);
    sprite_destroy(door);

    int bpw = screen_width() / 4;
    int bph = 1;
    bottom_platform = sprite_create(0, screen_height() - 1, bpw, bph, platform_image);

    int pw = screen_width() / 2;
    int ph = 1;
    top_platform = sprite_create(1, screen_height() / 2, pw, ph, platform_image);
    bottom_platform_2 = sprite_create(screen_width() - pw - 1, (screen_height() / 2) + (screen_height() / 4), pw, ph, platform_image);

    int rw = 6;
    int rh = 3;
    rock = sprite_create(5, -10, rw, rh, rock_image);

    int mw = 3;
    int mh = 4;
    medal = sprite_create((screen_width() / 2) - (mw / 2), 5, mw, mh, medal_image);

    sprite_turn_to(top_platform, 0.2, 0);
    sprite_turn_to(bottom_platform_2, -0.2, 0);
    sprite_turn_to(rock, 0.4, 0.4);
  }
}

void monster_movement(sprite_id sprite) {
  int x = round(sprite_x(sprite));
  double dx = sprite_dx(sprite);
  double dy = sprite_dy(sprite);

  if (x <= 0) {
    dx = fabs(dx);
  } else if (x >= screen_width() - sprite_width(sprite)) {
    dx = -fabs(dx);
  }

  if (dx != sprite_dx(sprite)) {
    sprite_back(sprite);
    sprite_turn_to(sprite, dx, dy);
  }
}

void rock_movement() {
  double rx = sprite_x(rock);
  double ry = sprite_y(rock);
  double rh = sprite_height(rock);

  monster_movement(rock);

  if (ry + rh > screen_height()) {
    ry = -ry;
    sprite_back(rock);
  }
  sprite_move_to(rock, rx, ry);
}

void hero_movement(void) {
  key = get_char();
  int hx = round(sprite_x(hero));
  double hdx = sprite_dx(hero);
  double hdy = sprite_dy(hero);

  if (key == 'l') {
    level += 1;
    levels();
  }

  if (key == KEY_LEFT && hx > 2) {
    if (hdx == 0) {
      hdx = -0.1;
    } else if (hdx == 0.1) {
      hdx = 0;
    } else if (hdx == 0.3) {
      hdx = 0.1;
    } else {
      hdx = -0.3;
    }
  }

  if (key == KEY_RIGHT && hx < screen_width() - HERO_WIDTH - 2) {
    if (hdx == 0) {
      hdx = 0.1;
    } else if (hdx == -0.1) {
      hdx = 0;
    } else if (hdx == -0.3) {
      hdx = -0.1;
    } else {
      hdx = 0.3;
    }
  }

  if (key == KEY_UP) {
    sprite_set_image(hero, hero_jump_image);
    if (hdy == 0) {
      hdy = -0.5;
    } else {
      hdy -= 0.01;
    }
  }
  else if (hdy != 0) {
    hdy += 0.01;
  }

  if ((hx > 1 && hx < screen_width() - HERO_WIDTH - 2)) {
    sprite_step(hero);
  } else {
    hdx = 0;
    sprite_back(hero);
  }
  sprite_turn_to(hero, hdx, hdy);
}

//Setup Game
void setup(void) {
  my_timer = create_timer(1000);
  levels();

  int lw = 25;
  int lh = 4;
  lost = sprite_create((screen_width()/2) - (lw/2), (screen_height()/2) - (lh/2), lw, lh, lost_image);
}

//Play one turn of game
void process(void) {
  int hy = sprite_y(hero);
  int hh = sprite_height(hero);

  if (level == 1) monster_movement(zombie);
  if (level == 5) {
    monster_movement(top_platform);
    monster_movement(bottom_platform_2);
    rock_movement();
  }

  if (level == 2 || level == 3) {
    monster_movement(bat);
    if (spin) {
      sprite_set_image(bat, bat_image_inverted);
      spin = false;
    } else {
      sprite_set_image(bat, bat_image);
      spin = true;
    }
  }
  hero_movement();

  if ((hy + hh > screen_height()) ||(level == 1 && sprite_collided(hero, zombie)) || ((level == 2 || level == 3) && sprite_collided(hero, bat)) || (level == 5 && sprite_collided(hero, rock))) {
    lives -= 1;
    levels();
  }

  if (sprite_collided(hero, door) && level < 5) {
    sprite_step(hero);
    clear_screen();
    level += 1;
    score += 100;
    levels();
  }

  if ((level == 2 || level == 3) && sprite_collided(hero, treasure) && sprite_visible(treasure)) {
    sprite_hide(treasure);
    score += 50;
  }

  if ((level == 5 && sprite_collided(hero, medal)) || level == 6) {
    while ( get_char() >= 0 ) {}
    clear_screen();
    draw_formatted((screen_width() / 2) - 4, (screen_height() / 2) - 4, "You Won!!");
    draw_formatted((screen_width() / 2) - 11, (screen_height() / 2) - 2, "Your Final Score was %d", score);
    draw_formatted((screen_width() / 2) - 10, (screen_height() / 2), "Press any key to exit");
    game_over = true;
    show_screen();
    wait_char();
    return;
  }

  if (level == 4 && sprite_collided(hero, door_key) && sprite_visible(door_key)) {
    sprite_hide(door_key);
    can_unlock = true;
  }

  platform_collision(hero, bottom_platform, true);
  platform_collision(hero, platform, true);

  if (level == 2 || level == 3 || level == 4 || level == 5) platform_collision(hero, bottom_platform_2, true);

  if (level == 3) {
    if (sprite_collided(hero, vertical_platform)) {
      double hdx = sprite_dx(hero);
      double hdy = sprite_dy(hero);
      hdx = 0;
      sprite_back(hero);
      sprite_turn_to(hero, hdx, hdy);
    }
    platform_collision(hero, top_platform, true);
  }

  if (level == 4) {
    if (sprite_collided(hero, unlock_door) && !can_unlock) {
      double hdx = sprite_dx(hero);
      double hdy = sprite_dy(hero);
      hdx = 0;
      sprite_back(hero);
      sprite_turn_to(hero, hdx, hdy);
    } else if (sprite_collided(hero, unlock_door) && can_unlock) {
      sprite_hide(unlock_door);
    }
    platform_collision(hero, top_platform, true);
  }

  if (level == 5) {
    platform_collision(hero, top_platform, true);
    platform_collision(rock, top_platform, false);
    platform_collision(rock, bottom_platform_2, false);
    platform_collision(rock, bottom_platform, false);
  }

  if (timer_expired(my_timer)) {
    if (timer < 60) {
      timer++;
    } else if (timer_2 < 60) {
      timer_2++;
      timer = 0;
    }
  }

  if (lives == 0) {
    while ( get_char() >= 0 ) {}
    clear_screen();
    sprite_draw(lost);
    game_over = true;
    show_screen();
    wait_char();
    return;
  }

  if (level == 1) sprite_step(zombie);
  if (level == 2 || level == 3) sprite_step(bat);
  if (level == 5) {
    sprite_step(top_platform);
    sprite_step(bottom_platform_2);
    sprite_step(rock);
  }
  clear_screen();
  draw_sprites();
}

int main(void) {
	setup_screen();
  setup();
  show_screen();

	while (!game_over) {
    process();

    if (update_screen) {
      show_screen();
    }

    timer_pause(DELAY);
  }
	cleanup_screen();
	return 0;
}
