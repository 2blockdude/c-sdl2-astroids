#include <math.h>
#include <time.h>
#include <SDL2/SDL.h>

#include "sdl2-game-window.h"
#include "primitives.h"

#define SCREEN_WIDTH		1000
#define SCREEN_HEIGHT	1000

#define MAX_OBJECTS     20
#define SHIP_SIZE       20

#define BULLET_INTERVAL 1000

#define PI 3.1415926535897932384626433832795

struct ship
{
   polygon *ship;
   polygon *thruster;
   struct { float x, y; } velocity;
};

struct space_object
{
   polygon *shape;
   struct { float x, y; } velocity;
};

int bullet_timer;
struct ship player;
struct space_object astroids[MAX_OBJECTS];
struct space_object bullets[MAX_OBJECTS];

void render_objects()
{
   SDL_RenderClear(game.renderer);
   SDL_SetRenderDrawColor(game.renderer, 255, 255, 255, 255);

   // draw player ship
   draw_polygon(game.renderer, player.ship);
   SDL_RenderDrawLineF(game.renderer, player.ship->x, player.ship->y, player.ship->vertices[0], player.ship->vertices[1]);

   // draw thruster
   if (game.keypress[SDLK_w])
      draw_polygon(game.renderer, player.thruster);

   for (int i = 0; i < MAX_OBJECTS; i++)
   {
      draw_polygon(game.renderer, astroids[i].shape);
      draw_polygon(game.renderer, bullets[i].shape);
   }

   SDL_SetRenderDrawColor(game.renderer, 0, 0, 0, 255);
   SDL_RenderPresent(game.renderer);
}

void update_objects()
{
   /*
    * player ship stuff
    */

   if (game.keypress[SDLK_w])
   {
      player.velocity.x += cos(player.ship->angle) * 500.0f * game.delta_t;
      player.velocity.y += sin(player.ship->angle) * 500.0f * game.delta_t;

      /*
       * note:
       * I found this by accident but setting the thruster
       * position before setting the player position gives a
       * cool lag effect for the polygon giving a better
       * feel for speed.
       */

      // set thruster polygon stuff
      player.thruster->angle = player.ship->angle + PI;
      player.thruster->x = cos(player.thruster->angle) * 20.0f + player.ship->x;
      player.thruster->y = sin(player.thruster->angle) * 20.0f + player.ship->y;
      polygon_rebuild(player.thruster);
   }

   if (game.keypress[SDLK_a])
   {
      player.ship->angle -= 8.0f * game.delta_t;
   }

   if (game.keypress[SDLK_d])
   {
      player.ship->angle += 8.0f * game.delta_t;
   }

   // reduce velocity proportional to current velocity over time
   player.velocity.x -= player.velocity.x * game.delta_t;
   player.velocity.y -= player.velocity.y * game.delta_t;

   // move ship
   player.ship->x += player.velocity.x * game.delta_t;
   player.ship->y += player.velocity.y * game.delta_t;

   // wrap ship around screen
   if (player.ship->x < 0) player.ship->x = SCREEN_WIDTH;
   if (player.ship->x > SCREEN_WIDTH) player.ship->x = 0;
   if (player.ship->y < 0) player.ship->y = SCREEN_HEIGHT;
   if (player.ship->y > SCREEN_HEIGHT) player.ship->y = 0;

   polygon_rebuild(player.ship);

   /*
    * bullet stuff
    */

   if (game.keypress[SDLK_SPACE])
   {
      // set first avalible space for bullet
      if (bullet_timer <= 0)
      {
         for (int i = 0; i < MAX_OBJECTS; i++)
         {
            if (bullets[i].shape == NULL)
            {
               bullets[i].shape = create_reg_polygon(6, player.ship->vertices[0], player.ship->vertices[1], player.ship->angle, 3);
               bullets[i].velocity.x = cos(player.ship->angle) * 600.0f;
               bullets[i].velocity.y = sin(player.ship->angle) * 600.0f;
               bullet_timer = BULLET_INTERVAL;
               break;
            }
         }
      }
   }

   // move all bullets
   for (int i = 0; i < MAX_OBJECTS; i++)
   {
      if (bullets[i].shape != NULL)
      {
         bullets[i].shape->x += bullets[i].velocity.x * game.delta_t;
         bullets[i].shape->y += bullets[i].velocity.y * game.delta_t;

         polygon_rebuild(bullets[i].shape);

         // remove bullet that reached edge of space
         if (bullets[i].shape->x < 0 || bullets[i].shape->y < 0 || bullets[i].shape->x > SCREEN_WIDTH || bullets[i].shape->y > SCREEN_HEIGHT)
         {
            free_polygon(bullets[i].shape);
            bullets[i].shape = NULL;
         }
      }
   }

   // reduce timer by time per frame
   if (bullet_timer > 0) bullet_timer -= game.delta_t * 1000.0f;
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
   // init player, astroids, and bulllets
   player.ship = create_reg_polygon(3, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 0, 20);
   player.thruster = create_reg_polygon(3, 0, 0, 0, 10);
   player.velocity.x = 0;
   player.velocity.y = 0;

   // init bullets and astroids
   for (int i = 0; i < MAX_OBJECTS; i++)
   {
      bullets[i].shape = NULL;
      astroids[i].shape = NULL;
   }

   init_game_window(SCREEN_WIDTH, SCREEN_HEIGHT, "astroids");
   SDL_ShowCursor(SDL_DISABLE);
   start_game();

   // return memory stuff
   close_game_window();
   free_polygon(player.ship);

   return 0;
}
