#include "aoqt.h"

//////////////////////////////////////////////////////////////////////
// MISC FUNCTIONS
void Cleanup() {
   SDL_Quit();
}

FILE* errorStream = NULL;
//const size_t BUFLENGTH = 256;
void logWarning(char *message) {
   fprintf(errorStream, "WARNING: %s\n", message);
}

void logError(char *message) {
   fprintf(errorStream, "ERROR: %s: %s\n", message, SDL_GetError());
   fprintf(errorStream, "Bailing.\n");
   Cleanup();
   exit(1);
}

void checkError(bool errorTest, char *message) {
   if(errorTest) {
      logError(message);
   }
}

//////////////////////////////////////////////////////////////////////
// RESOURCE HANDLING

SDL_Texture* loadTexture(char *file, SDL_Renderer *ren) {
   SDL_Surface *imgSurf = SDL_LoadBMP(file);
   if(!imgSurf) {
      logWarning(file);
      logError("Could not load image");
   }
   SDL_Texture *tex = SDL_CreateTextureFromSurface(ren, imgSurf);
   SDL_FreeSurface(imgSurf);
   if(!tex) {
      logError("Could not create texture from image.");
   }
   return tex;
}

void loadAtlas(atlas* atl, char *file, SDL_Renderer *ren, int spriteSize) {
   atl->tex = loadTexture(file, ren);
   atl->spriteSize = spriteSize;
   int w, h;
   SDL_QueryTexture(atl->tex, NULL, NULL, &w, &h);
   atl->width = w / atl->spriteSize;
   atl->height = h / atl->spriteSize;
   if((w % atl->spriteSize) != 0 ||
      (h % atl->spriteSize) != 0) {
      logWarning("Atlas does not divide evenly into integer tiles!");
      printf("%d %% %d = %d\n", w, atl->spriteSize, (w % atl->spriteSize));
   }
}

void loadAssets(SDL_Renderer* ren, gamestate* g) {
   loadAtlas(&(g->terrainAtlas), "data/terrain.bmp", ren, 64);
   loadAtlas(&(g->playerAtlas), "data/player.bmp", ren, 64);
   
}


// Takes an index and turns it into a rect that tells you
// where in the atlas the indexed sprite is.
void atlasCoords(atlas* atl, int index, SDL_Rect* rect) {
   SDL_assert(index >= 0);
   int xoffset = index % atl->width;
   int yoffset = index / atl->width;
   SDL_assert(
      // Always true
      // xoffset < atl->width &&
      yoffset < atl->height);
   
   rect->x = xoffset * atl->spriteSize;
   rect->y = yoffset * atl->spriteSize;
   rect->w = atl->spriteSize;
   rect->h = atl->spriteSize;
}



//////////////////////////////////////////////////////////////////////
// GAMEPLAY


//////////////////////////////////////////////////////////////////////
// DRAWING
// Draw a texture at x,y without changing its size.
void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y) {
   SDL_Rect dest = {.x = x, .y = y};
   SDL_QueryTexture(tex, NULL, NULL, &dest.w, &dest.h);
   SDL_RenderCopy(ren, tex, NULL, &dest);
}

// Same as above, but takes a sprite number too
/*
void renderSprite(SDL_Texture *tex, SDL_Renderer *ren, int x, int y) {
   SDL_Rect dest = {.x = x, .y = y};
   SDL_QueryTexture(tex, NULL, NULL, &dest.w, &dest.h);
   SDL_RenderCopy(ren, tex, NULL, &dest);
}
*/


void drawZone(SDL_Renderer *ren, gamestate *g, zone *z) {
   SDL_Rect sourceRect, destRect;
   const int spriteSize = g->terrainAtlas.spriteSize;
   //printf("Sprite size: %d\n", spriteSize);
   destRect.w = spriteSize;
   destRect.h = spriteSize;
   for(int x = 0; x < ZONEWIDTH; x++) {
      for(int y = 0; y < ZONEHEIGHT; y++) {
	 atlasCoords(&(g->terrainAtlas), z->tiles[x][y], &sourceRect);
	 destRect.x = x * spriteSize;
	 destRect.y = y * spriteSize;
	 SDL_RenderCopy(ren, g->terrainAtlas.tex, &sourceRect, &destRect);
      }
   }
}

void drawPlayer(SDL_Renderer *ren, gamestate *g) {
   SDL_Rect sourceRect, destRect;
   atlasCoords(&(g->playerAtlas), 0, &sourceRect);
   destRect.x = g->player.x;
   destRect.y = g->player.y;
   SDL_RenderCopy(ren, g->playerAtlas.tex, &sourceRect, &destRect);
}

void drawWorld(SDL_Renderer* ren, gamestate *g) {
   zone *currentZone = &(g->zones[g->zx][g->zy]);
   drawZone(ren, g, currentZone);
   drawPlayer(ren, g);
}

//////////////////////////////////////////////////////////////////////
// INPUT HANDLING


bool handleEvents() {
   SDL_Event e;
   SDL_KeyboardEvent kev;
   while(SDL_PollEvent(&e)) {
      switch(e.type) {
	 case SDL_KEYDOWN:
	    kev = e.key;
	    if(kev.keysym.sym == SDLK_q || 
	       kev.keysym.sym == SDLK_ESCAPE) {
	       return false;
	    }
	    break;

	 case SDL_KEYUP:
	    break;

	 case SDL_QUIT:
	    return false;

	 default:
	    break;
      }
   }
   return true;
}



//////////////////////////////////////////////////////////////////////
// MAIN STUFF

void initPlayer(player *p) {
   p->x = 0;
   p->y = 0;
   p->hits = MAXHITS;
   p->arrows = STARTINGARROWS;
   p->facing = F_DOWN;
}



void initGamestate(gamestate *g) {
   g->zx = 0;
   g->zy = 0;
   initPlayer(&(g->player));
}

void destroyGamestate(gamestate *g) {
   SDL_DestroyTexture(g->terrainAtlas.tex);
   SDL_DestroyTexture(g->playerAtlas.tex);
}

void mainloop(SDL_Renderer *ren) {
   bool keepgoing = true;
   uint32_t then = SDL_GetTicks();
   uint32_t now = then;

   // This is static not to persist the variable across
   // multiple calls of the function, but rather to put
   // it in the data segment so we don't have this big
   // structure on the stack.
   static gamestate g;

   initGamestate(&g);
   loadAssets(ren, &g);

   while(keepgoing) {
      then = now;
      uint32_t now = SDL_GetTicks();

      // Get input
      keepgoing = handleEvents();
      // Update physics


      // Draw stuff
      SDL_RenderClear(ren);
      // Draw things here
      drawWorld(ren, &g);
      SDL_RenderPresent(ren);

      uint32_t dt = SDL_GetTicks() - now;
      printf("Frame time: %d\n", dt);
   }

   destroyGamestate(&g);
}


int main(int argc, char** argv) {
   errorStream = stdout;

   checkError(SDL_Init(SDL_INIT_EVERYTHING) != 0, "SDL init error");

   SDL_Window *win = SDL_CreateWindow(
      "Adventure Odyssey Quest Trek",
      SDL_WINDOWPOS_UNDEFINED,
      SDL_WINDOWPOS_UNDEFINED,
      SCREEN_WIDTH,
      SCREEN_HEIGHT,
      SDL_WINDOW_SHOWN);

   checkError(!win, "SDL window create error");
   
   SDL_Renderer *ren = SDL_CreateRenderer(win, -1,
					  SDL_RENDERER_ACCELERATED);
   checkError(!ren, "SDL renderer create error");

   mainloop(ren);

   SDL_DestroyRenderer(ren);
   SDL_DestroyWindow(win);
   SDL_Quit();

   return 0;
}
