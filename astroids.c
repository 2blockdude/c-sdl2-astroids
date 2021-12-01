#include <math.h>
#include <time.h>
#include <SDL2/SDL.h>

#include "sdl2-game-window.h"
#include "primitives.h"

#define SCREEN_WIDTH		700
#define SCREEN_HEIGHT	700

#define PI 3.1415926535897932384626433832795

struct ship
{
   polygon *model;
   struct { float x, y; } velocity;
};

struct ship s;

void render_objects()
{
   SDL_RenderClear(game.renderer);
   SDL_SetRenderDrawColor(game.renderer, 255, 255, 255, 255);

   draw_polygon(game.renderer, s.model);
   SDL_RenderDrawLineF(game.renderer, s.model->x, s.model->y, s.model->vertices[0], s.model->vertices[1]);

   SDL_SetRenderDrawColor(game.renderer, 0, 0, 0, 255);
   SDL_RenderPresent(game.renderer);
}

void update_objects()
{
   if (game.keypress[SDLK_w])
   {
      s.velocity.x += cos(s.model->angle) * 500.0f * game.delta_t;
      s.velocity.y += sin(s.model->angle) * 500.0f * game.delta_t;
   }

   if (game.keypress[SDLK_s])
   {
   }

   if (game.keypress[SDLK_a])
   {
      s.model->angle -= 8.0f * game.delta_t;
   }

   if (game.keypress[SDLK_d])
   {
      s.model->angle += 8.0f * game.delta_t;
   }

   // reduce velocity proportional to current velocity over time
   s.velocity.x -= s.velocity.x * game.delta_t;
   s.velocity.y -= s.velocity.y * game.delta_t;

   // move ship
   s.model->x += s.velocity.x * game.delta_t;
   s.model->y += s.velocity.y * game.delta_t;

   // wrap ship around screen
   if (s.model->x < 0) s.model->x = SCREEN_WIDTH;
   if (s.model->x > SCREEN_WIDTH) s.model->x = 0;
   if (s.model->y < 0) s.model->y = SCREEN_HEIGHT;
   if (s.model->y > SCREEN_HEIGHT) s.model->y = 0;

   polygon_rebuild(s.model);
}

// function needed for game window code
int on_game_update()
{
   update_objects();
   render_objects();

   return 0;
}

int main()
{
   s.model = create_reg_polygon(3, 350, 350, 0, 20);
   s.velocity.x = 0;
   s.velocity.y = 0;

   init_game_window(SCREEN_WIDTH, SCREEN_HEIGHT, "astroids");
   start_game();
   destroy_game_window();

   return 0;
}
