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
#define PLATFORM_HEIGHT (1)
#define PLATFORM_WIDTH (30)
#define ZOMBIE_HEIGHT (4)
#define ZOMBIE_WIDTH (4)

bool game_over = false;
bool update_screen = true;
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
char * platform_image = "=============================================================================================================================";

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

sprite_id treasure;
char * treasure_image = "$";

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
  sprite_draw(zombie);
  sprite_draw(platform);
  sprite_draw(door);
  sprite_draw(bottom_platform);

  if (level == 2) {
    sprite_draw(bottom_platform_2);
    sprite_draw(bat);
    sprite_draw(treasure);
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

void platform_collision(sprite_id hero, sprite_id platform) {
  if (sprite_collided(hero, platform)) {
    int hx = sprite_x(hero);
    int hy = sprite_y(hero);
    int px = sprite_x(platform);
    int py = sprite_y(platform);

    double hdx = sprite_dx(hero);
    double hdy = sprite_dy(hero);
    sprite_set_image(hero, hero_image);

    if (hy == py + sprite_height(platform) - 1 && hdy < 0) {
      hdy = -hdy;
    } else if (hx + sprite_width(hero) - 1 == px && hdx > 0) {
      hdx = 0;
    } else if (hx == px + sprite_width(platform) - 1 && hdx < 0) {
      hdx = 0;
    } else {
      hdy = 0;
    }
    sprite_back(hero);
    sprite_turn_to(hero, hdx, hdy);
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

void hero_movement(void) {
  key = get_char();
  int hx = round(sprite_x(hero));
  int hy = round(sprite_y(hero));
  double hdx = sprite_dx(hero);
  double hdy = sprite_dy(hero);

  if (key == KEY_LEFT && hx > 2) {
    if (sprite_dx(hero) == 0.1) {
      hdx = 0;
    } else if (hdx < sprite_dx(hero)) {
      hdx += 0.1;
    } else if (hdx == sprite_dx(hero)) {
      hdx -= 0.1;
    } else {
      hdx -= 0.4;
    }
  }

  if (key == KEY_RIGHT && hx < screen_width() - HERO_WIDTH - 2) {
    if (sprite_dx(hero) == -0.1) {
      hdx = 0;
    } else if (hdx > sprite_dx(hero)) {
      hdx -= 0.1;
    } else if (hdx == sprite_dx(hero)) {
      hdx += 0.1;
    } else {
      hdx += 0.4;
    }
  }

  if (key == KEY_UP && hy > 2) {
    sprite_set_image(hero, hero_jump_image);
    if (hdy == 0) {
      hdy = -0.5;
    } else {
      hdy -= 0.01;
    }
  }
  else if (hdy != 0){
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

void levels(void) {
  int hw = HERO_WIDTH;
  int hh = HERO_HEIGHT;
  hero = sprite_create(2, screen_height() - 4, hw, hh, hero_image);

  int dw = DOOR_WIDTH;
  int dh = DOOR_HEIGHT;
  door = sprite_create(screen_width() - 6, screen_height() - 5, dw, dh, door_image);

  if (level == 1) {
    int pw = PLATFORM_WIDTH;
    int ph = PLATFORM_HEIGHT;
    platform = sprite_create((screen_width()/2) - (pw/2), screen_height() - 10, pw, ph, platform_image);

    int bpw = screen_width();
    int bph = 1;
    bottom_platform = sprite_create(0, screen_height() - 1, bpw, bph, platform_image);

    int zw = ZOMBIE_WIDTH;
    int zh = ZOMBIE_HEIGHT;
    zombie = sprite_create(screen_width() - 5, screen_height() - 5, zw, zh, zombie_image);

    sprite_turn_to(zombie, 0.1, 0);
    sprite_turn(zombie, 180);
  } else if (level == 2) {
    sprite_destroy(zombie);

    int bpw = screen_width() / 3;
    int bph = 1;
    bottom_platform = sprite_create(screen_width() - bpw, screen_height() - 1, bpw, bph, platform_image);
    bottom_platform_2 = sprite_create(0, screen_height() - 1, bpw, bph, platform_image);

    int tw = 1;
    int th = 1;
    treasure = sprite_create(screen_width()/2, (screen_height()/2) - 5, tw, th, treasure_image);

    int bw = 3;
    int bh = 2;
    bat = sprite_create(screen_width() - 9, screen_height() - 4, bw, bh, bat_image);
    sprite_turn_to(bat, 0.2, 0);
    sprite_turn(bat, 180);
  }
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
  if (level == 1) monster_movement(zombie);
  if (level == 2) {
    monster_movement(bat);
    if (timer % 2) {
      sprite_set_image(bat, bat_image_inverted);
    } else {
      sprite_set_image(bat, bat_image);
    }
  }
  hero_movement();

  if ((level == 1 && sprite_collided(hero, zombie)) || (level == 2 && sprite_collided(hero, bat))) {
    lives -= 1;
    levels();
  }

  if (sprite_collided(hero, door)) {
    sprite_step(hero);
    clear_screen();
    level += 1;
    score += 100;
    levels();
  }

  if (level == 2 && sprite_collided(hero, treasure) && sprite_visible(treasure)) {
    sprite_hide(treasure);
    score += 50;
  }

  platform_collision(hero, bottom_platform);
  platform_collision(hero, platform);

  if (level == 2) platform_collision(hero, bottom_platform_2);

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
  if (level == 2) sprite_step(bat);
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
