#include <math.h>
#include <time.h>
#include <SDL2/SDL.h>

#include "sdl2-game-window.h"
#include "primitives.h"

#define SCREEN_WIDTH		1000
#define SCREEN_HEIGHT	1000

#define MAX_OBJECTS     20

#define SHIP_SIZE       20
#define SHIP_SPEED      500
#define SHIP_TURN_SPEED 8

#define BULLET_SIZE     3
#define BULLET_SPEED    600
#define BULLET_INTERVAL 500

#define ASTROIDS_SIZE   50
#define ASTROIDS_SCALE  4
#define ASTROIDS_SPEED  100

#define PI              3.1415926535897932384626433832795

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

   // draw bullets and astroids
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
      // acceleration
      player.velocity.x += cos(player.ship->angle) * (float)SHIP_SPEED * game.delta_t;
      player.velocity.y += sin(player.ship->angle) * (float)SHIP_SPEED * game.delta_t;

      /*
       * note:
       * I found this by accident but setting the thruster
       * position before setting the player position gives a
       * cool lag effect for the polygon giving a better
       * feel for speed. This only works at lower fps.
       */

      // set thruster polygon stuff
      player.thruster->angle = player.ship->angle + PI;
      player.thruster->x = cos(player.thruster->angle) * (float)SHIP_SIZE + player.ship->x;
      player.thruster->y = sin(player.thruster->angle) * (float)SHIP_SIZE + player.ship->y;
      polygon_rebuild(player.thruster);
   }

   if (game.keypress[SDLK_a])
   {
      player.ship->angle -= (float)SHIP_TURN_SPEED * game.delta_t;
   }

   if (game.keypress[SDLK_d])
   {
      player.ship->angle += (float)SHIP_TURN_SPEED * game.delta_t;
   }

   // give player drag to simulate speed limit
   player.velocity.x -= player.velocity.x * game.delta_t;
   player.velocity.y -= player.velocity.y * game.delta_t;

   // move ship
   player.ship->x += player.velocity.x * game.delta_t;
   player.ship->y += player.velocity.y * game.delta_t;

   // wrap ship around screen
   if (player.ship->x < 0) player.ship->x = SCREEN_WIDTH;
   if (player.ship->y < 0) player.ship->y = SCREEN_HEIGHT;
   if (player.ship->x > SCREEN_WIDTH) player.ship->x = 0;
   if (player.ship->y > SCREEN_HEIGHT) player.ship->y = 0;

   polygon_rebuild(player.ship);

   /*
    * bullet stuff
    */

   // shoot bullet
   if (game.keypress[SDLK_SPACE])
   {
      // set first avalible space in array for bullet
      if (bullet_timer <= 0)
      {
         for (int i = 0; i < MAX_OBJECTS; i++)
         {
            if (bullets[i].shape == NULL)
            {
               bullets[i].shape = create_reg_polygon(6, player.ship->vertices[0], player.ship->vertices[1], player.ship->angle, BULLET_SIZE);
               bullets[i].velocity.x = cos(player.ship->angle) * (float)BULLET_SPEED;
               bullets[i].velocity.y = sin(player.ship->angle) * (float)BULLET_SPEED;
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

   // reduce timer by milliseconds per frame
   if (bullet_timer > 0) bullet_timer -= game.delta_t * 1000.0f;

   /*
    * astroids stuff
    */

   // move all bullets
   for (int i = 0; i < MAX_OBJECTS; i++)
   {
      if (astroids[i].shape != NULL)
      {
         astroids[i].shape->x += astroids[i].velocity.x * game.delta_t;
         astroids[i].shape->y += astroids[i].velocity.y * game.delta_t;

         // wrap ship around screen
         if (astroids[i].shape->x < 0) astroids[i].shape->x = SCREEN_WIDTH;
         if (astroids[i].shape->y < 0) astroids[i].shape->y = SCREEN_HEIGHT;
         if (astroids[i].shape->x > SCREEN_WIDTH)  astroids[i].shape->x = 0;
         if (astroids[i].shape->y > SCREEN_HEIGHT) astroids[i].shape->y = 0;

         polygon_rebuild(astroids[i].shape);
      }
   }
}

// function needed for game window code
int on_game_update()
{
   update_objects();
   render_objects();

   return 0;
}

int on_game_creation()
{
   SDL_ShowCursor(SDL_DISABLE);
   srand(SDL_GetTicks());

   // init player
   player.ship = create_reg_polygon(3, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 0, SHIP_SIZE);
   player.thruster = create_reg_polygon(3, 0, 0, 0, (float)SHIP_SIZE / 2.0f);
   player.velocity.x = 0;
   player.velocity.y = 0;

   // init bullets and astroids
   for (int i = 0; i < MAX_OBJECTS; i++)
   {
      bullets[i].shape = NULL;
      astroids[i].shape = NULL;
   }

   // init random astroids
   for (int i = 0; i < 2; i++)
   {
      astroids[i].shape = create_rand_polygon(24, rand() % SCREEN_WIDTH, rand() % SCREEN_HEIGHT, (float)((double)rand() * (double)((2 * PI) / RAND_MAX)), ASTROIDS_SIZE, ASTROIDS_SIZE * 0.7f, 1);
      astroids[i].shape->scale.x = ASTROIDS_SCALE;
      astroids[i].shape->scale.y = ASTROIDS_SCALE;
      astroids[i].velocity.x = cos(astroids[i].shape->angle) * (rand() % ASTROIDS_SPEED);
      astroids[i].velocity.y = sin(astroids[i].shape->angle) * (rand() % ASTROIDS_SPEED);
      polygon_rebuild(astroids[i].shape);
   }

   return 0;
}

int main()
{
   init_game_window(SCREEN_WIDTH, SCREEN_HEIGHT, "astroids");
   start_game();

   // return memory stuff
   close_game_window();
   free_polygon(player.ship);

   return 0;
}
