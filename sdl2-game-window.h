#ifndef GAME_WINDOW
#define GAME_WINDOW

typedef struct game_window game_window;

struct game_window
{
   // window stuff
   SDL_Window *window;
   SDL_Renderer *renderer;
   SDL_Event event;

   // settings
   int width;
   int height;

   // frame info
   double delta_t;
   double fps;
   int fps_max;

   // bools
   char running;
   char pause;

   // key handle
   char keypress[128];
};

extern struct game_window game;

int init_game_window       (int width, int height, const char *title);
void close_game_window     ();
int start_game             ();
int on_game_update         ();

#endif
