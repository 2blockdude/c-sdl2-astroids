#include <math.h>
#include <time.h>
#include <SDL2/SDL.h>

#include "sdl2-game-window.h"
#include "primitives.h"
#include "collision.h"

#define SCREEN_WIDTH		1000
#define SCREEN_HEIGHT	1000

#define MAX_OBJECTS     20

#define SHIP_SIZE       20
#define SHIP_SPEED      500
#define SHIP_TURN_SPEED 8

#define BULLET_SIZE     2
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
int current_round;
struct ship player;
struct space_object astroids[MAX_OBJECTS];
struct space_object bullets[MAX_OBJECTS];

int wrap_position(float x, float y, float *ox, float *oy)
{
   if (x < 0)
   {
      if (ox != NULL)
         *ox = SCREEN_WIDTH;
      return 1;
   }

   if (y < 0)
   {
      if (oy != NULL)
         *oy = SCREEN_HEIGHT;
      return 1;
   }

   if (x > SCREEN_WIDTH)
   {
      if (ox != NULL)
         *ox = 0;
      return 1;
   }

   if (y > SCREEN_HEIGHT)
   {
      if (oy != NULL)
         *oy = 0;
      return 1;
   }

   return 0;
}

void restart_game()
{
   // reset player status
   player.ship->x = SCREEN_WIDTH / 2.0f;
   player.ship->y = SCREEN_HEIGHT / 2.0f;
   player.velocity.x = 0;
   player.velocity.y = 0;
   polygon_rebuild(player.ship);

   // free bullets and astroids
   for (int i = 0; i < MAX_OBJECTS; i++)
   {
      if (bullets[i].shape != NULL)
         free_polygon(bullets[i].shape);

      if (astroids[i].shape != NULL)
         free_polygon(astroids[i].shape);

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
}

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
   wrap_position(player.ship->x, player.ship->y, &player.ship->x, &player.ship->y);

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
         if (wrap_position(bullets[i].shape->x, bullets[i].shape->y, NULL, NULL))
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

         // wrap astroids around screen
         wrap_position(astroids[i].shape->x, astroids[i].shape->y, &astroids[i].shape->x, &astroids[i].shape->y);

         polygon_rebuild(astroids[i].shape);
      }
   }

   /*
    * collision detection
    */

   // check player astroid collision
   for (int i = 0; i < MAX_OBJECTS; i++)
   {
      if (astroids[i].shape != NULL && polygon_polygon_collision(player.ship->vertices, player.ship->nsides, astroids[i].shape->vertices, astroids[i].shape->nsides))
      {
         restart_game();
      }
   }

   // check bullet astroid collision
   for (int i = 0; i < MAX_OBJECTS; i++)
   {
      if (bullets[i].shape == NULL)
         continue;

      // check bullet (i) with astroid (j)
      for (int j = 0; j < MAX_OBJECTS; j++)
      {
         if (astroids[j].shape == NULL)
            continue;

         if (point_polygon_collision(bullets[i].shape->x, bullets[i].shape->y, astroids[j].shape->vertices, astroids[j].shape->nsides))
         {
            float x = astroids[j].shape->x;
            float y = astroids[j].shape->y;
            float scale = (astroids[j].shape->scale.x - (astroids[j].shape->scale.x / 2.0f));

            // remove bullet and astroid stuff
            free_polygon(bullets[i].shape);
            free_polygon(astroids[j].shape);
            bullets[i].shape = NULL;
            astroids[j].shape = NULL;

            if (scale >= 1)
            {
               // create two smaller astroids
               for (int k = 0, num = 0; k < MAX_OBJECTS; k++)
               {
                  if (astroids[k].shape == NULL)
                  {
                     astroids[k].shape = create_rand_polygon(24, x, y, (float)((double)rand() * (double)((2 * PI) / RAND_MAX)), ASTROIDS_SIZE, ASTROIDS_SIZE * 0.7f, 1);
                     astroids[k].shape->scale.x = scale;
                     astroids[k].shape->scale.y = scale;
                     astroids[k].velocity.x = cos(astroids[k].shape->angle) * (rand() % ASTROIDS_SPEED);
                     astroids[k].velocity.y = sin(astroids[k].shape->angle) * (rand() % ASTROIDS_SPEED);
                     polygon_rebuild(astroids[k].shape);
                     num++;
                  }

                  if (num == 2)
                     break;
               }
            }
            
            // break after collision because the bullet no longer exists
            break;
         }
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
