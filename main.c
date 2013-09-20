#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

const int SCREEN_WIDTH = 1024;
const int SCREEN_HEIGHT = 768;


void Cleanup() {
   SDL_Quit();
}

FILE* errorStream = NULL;
//const size_t BUFLENGTH = 256;
void LogWarning(char *message) {
   fprintf(errorStream, "WARNING: %s\n", message);
}

void LogError(char *message) {
   fprintf(errorStream, "ERROR: %s: %s\n", message, SDL_GetError());
   fprintf(errorStream, "Bailing.\n");
   Cleanup();
   exit(1);
}

void CheckError(bool errorTest, char *message) {
   if(errorTest) {
      LogError(message);
   }
}


SDL_Texture* loadTexture(char *file, SDL_Renderer *ren) {
   SDL_Surface *imgSurf = SDL_LoadBMP(file);
   if(!imgSurf) {
      LogWarning(file);
      LogError("Could not load image");
   }
   SDL_Texture *tex = SDL_CreateTextureFromSurface(ren, imgSurf);
   SDL_FreeSurface(imgSurf);
   if(!tex) {
      LogError("Could not create texture from image.");
   }
   return tex;
}


// Draw a texture at x,y without changing its size.
void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y) {
   SDL_Rect dest = {.x = x, .y = y};
   SDL_QueryTexture(tex, NULL, NULL, &dest.w, &dest.h);
   SDL_RenderCopy(ren, tex, NULL, &dest);
}

typedef enum {
   F_UP,
   F_DOWN,
   F_LEFT,
   F_RIGHT
} facing;


#define MAXHITS 10
#define STARTINGARROWS 10
#define SWORDDAMAGE 3
#define ARROWDAMAGE 2
typedef struct {
   int x;
   int y;
   int hits;
   int arrows;
   facing facing;
} player;

void initPlayer(player *p) {
   p->x = 0;
   p->y = 0;
   p->hits = MAXHITS;
   p->arrows = STARTINGARROWS;
   p->facing = F_DOWN;
}

typedef struct {
   int x;
   int y;
   int hits;
   facing facing;
} mob;

// So apparently consts in C99 aren't actually const.
// Because they're still stored in memory and so can be modified
// by other means.
#define ZONEWIDTH 40
#define ZONEHEIGHT 30
#define NUMMOBS 16
typedef struct {
   char tiles[ZONEWIDTH][ZONEHEIGHT];
   mob mobs[NUMMOBS];
} zone;

void drawZone(zone *z) {
}

#define WORLDWIDTH 10
#define WORLDHEIGHT 10
typedef struct {
   zone zones[WORLDWIDTH][WORLDHEIGHT];
   // Current zone coordinates
   uint32_t zx;
   uint32_t zy;
} world;

void drawWorld(world *w) {
   zone *currentZone = &w->zones[w->zx][w->zy];
   drawZone(currentZone);
}

static player thePlayer;
static world theWorld;

void mainloop(SDL_Renderer *ren) {
   bool keepgoing = true;
   uint32_t then = SDL_GetTicks();
   uint32_t now = then;

   while(keepgoing) {
      then = now;
      uint32_t now = SDL_GetTicks();

      // Get input
      // Update physics


      // Draw stuff
      SDL_RenderClear(ren);
      // Draw things here
      SDL_RenderPresent(ren);

      uint32_t dt = SDL_GetTicks() - now;
      printf("Frame time: %d\n", dt);
   }
}


int main(int argc, char** argv) {
   errorStream = stdout;

   CheckError(SDL_Init(SDL_INIT_EVERYTHING) != 0, "SDL init error");

   SDL_Window *win = SDL_CreateWindow(
      "Adventure Odyssey Quest Trek",
      SDL_WINDOWPOS_UNDEFINED,
      SDL_WINDOWPOS_UNDEFINED,
      SCREEN_WIDTH,
      SCREEN_HEIGHT,
      SDL_WINDOW_SHOWN);

   CheckError(!win, "SDL window create error");
   
   SDL_Renderer *ren = SDL_CreateRenderer(win, -1,
					  SDL_RENDERER_ACCELERATED);
   CheckError(!ren, "SDL renderer create error");

   SDL_Texture *tex = loadTexture("data/hello.bmp", ren);

   for(int i = 0; i < 100; i++) {
      SDL_RenderClear(ren);
      renderTexture(tex, ren, i, i);
      SDL_Delay(30);
   }

   SDL_DestroyTexture(tex);
   SDL_DestroyRenderer(ren);
   SDL_DestroyWindow(win);
   SDL_Quit();

   return 0;
}
