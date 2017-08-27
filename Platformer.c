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
bool jumped = false;
bool peak = false;
int key;
int lives = 10;
int level = 1;
int score = 0;

timer_id my_timer;
double timer;

sprite_id hero;
char * hero_image =
      " 0 "
      "/|\\"
      "/'\\ ";

sprite_id door;
char * door_image =
      "EXIT"
      "|  |"
      "| _|"
      "|  |";

sprite_id platform;
char * platform_image = "=========================================================================";

sprite_id zombie;
char * zombie_image =
      "ZZZZ"
      "  Z "
      " Z  "
      "ZZZZ";

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
  draw_line(left, bottom, right, bottom, '=');
  draw_line(right, top + 1, right, bottom - 1, '|');
  draw_line(left, top + 1, left, bottom - 1, '|');
}

void draw_sprites(void) {
  draw_border();
  draw_formatted(5, 0, "Time: %f", timer);
  draw_formatted(25, 0, "Lives: %d", lives);
  draw_formatted(45, 0, "Level: %d", level);
  draw_formatted(65, 0, "Score: %d", score);
  sprite_draw(hero);
  sprite_draw(zombie);
  sprite_draw(platform);
  sprite_draw(door);
}

void jump(void) {
  if (jumped) {
    key = 0;
    double dx = sprite_dx(hero);
    double dy = sprite_dy(hero);
    double height = screen_height() - 15;
    char * jump_image =
          " 0 "
          "\\|/"
          "/'\\ ";

    if (sprite_y(hero) > height && !peak) {
      sprite_step(hero);
      dy = -0.1;
      sprite_turn_to(hero, dx, dy);
      sprite_set_image(hero, jump_image);
    } else {
      peak = true;
      dy = 0.1;
      sprite_step(hero);
      sprite_turn_to(hero, dx, dy);
      sprite_set_image(hero, hero_image);
    }

    if (peak && sprite_y(hero) == screen_height() - 4) {
      jumped = false;
      peak = false;
      dy = 0;
      sprite_turn_to(hero, dx, dy);
    }
    key = get_char();
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

//Setup Game
void setup(void) {
  my_timer = create_timer(1000);

  int lw = 25;
  int lh = 4;
  lost = sprite_create((screen_width()/2) - (lw/2), (screen_height()/2) - (lh/2), lw, lh, lost_image);

  int hw = HERO_WIDTH;
  int hh = HERO_HEIGHT;
  hero = sprite_create(2, screen_height() - 4, hw, hh, hero_image);

  int dw = DOOR_WIDTH;
  int dh = DOOR_HEIGHT;
  door = sprite_create(screen_width() - 6, screen_height() - 5, dw, dh, door_image);

  int pw = PLATFORM_WIDTH;
  int ph = PLATFORM_HEIGHT;
  platform = sprite_create((screen_width()/2) - (pw/2), screen_height() - 10, pw, ph, platform_image);

  int zw = ZOMBIE_WIDTH;
  int zh = ZOMBIE_HEIGHT;
  zombie = sprite_create(screen_width() - 5, screen_height() - 5, zw, zh, zombie_image);

  draw_sprites();
  sprite_turn_to(zombie, 0.1, 0);
  sprite_turn(zombie, 180);
}

//Play one turn of game
void process(void) {
  key = get_char();
  int zx = round(sprite_x(zombie));
  double zdx = sprite_dx(zombie);
  int hx = round(sprite_x(hero));
  int hdx = sprite_dx(hero);
  int hy = round(sprite_y(hero));
  jump();

  if (zx <= 0) {
    zdx = fabs(zdx);
  } else if (zx >= screen_width() - ZOMBIE_WIDTH) {
    zdx = -fabs(zdx);
  }

  if (zdx != sprite_dx(zombie)) {
    sprite_back(zombie);
    sprite_turn_to(zombie, zdx, sprite_dy(zombie));
  }

  if (key == KEY_UP && hy > 2) {
    jumped = true;
  }

  if (key == KEY_LEFT && hx > 1) {
    if (sprite_dx(hero) == 0.1) {
      sprite_turn_to(hero, 0, 0);
    } else if (hdx < sprite_dx(hero)) {
      sprite_turn_to(hero, 0.1, 0);
    } else if (hdx == sprite_dx(hero)) {
      sprite_turn_to(hero, -0.1, 0);
    } else {
      sprite_turn_to(hero, -0.4, 0);
    }
  }

  if (key == KEY_RIGHT && hx < screen_width() - sprite_width(hero) - 1) {
    if (sprite_dx(hero) == -0.1) {
      sprite_turn_to(hero, 0, 0);
    } else if (hdx > sprite_dx(hero)) {
      sprite_turn_to(hero, -0.1, 0);
    } else if (hdx == sprite_dx(hero)) {
      sprite_turn_to(hero, 0.1, 0);
    } else {
      sprite_turn_to(hero, 0.4, 0);
    }
  }

  if ((hx > 1 && hx < screen_width() - sprite_width(hero) - 1)) {
    sprite_step(hero);
  } else {
    sprite_back(hero);
    sprite_turn_to(hero, 0, 0);
  }

  if (sprite_collided(hero, zombie)) {
    lives = lives - 1;
    setup();
  }

  if (timer_expired(my_timer)) {
    timer++;
  }

  if (lives == 0) {
    while ( get_char() >= 0 ) {}
    clear_screen();
    sprite_draw(lost);
    game_over = true;
    wait_char();
    return;
  }

  sprite_step(zombie);
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
