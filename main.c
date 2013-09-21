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

void movePlayer(gamestate *g, int dt) {
   inputState *i = &(g->input);
   player *p = &(g->player);
   double fdt = (double) dt;
   double movementAmount = g->player.movementSpeed * fdt / 1000.0;
   if(i->up) {
      p->y = fmax(0.0, p->y - movementAmount);
      printf("UP: %f %f\n", movementAmount, p->y);
   } else if(i->down) {
      p->y = fmin(SCREEN_HEIGHT - g->playerAtlas.spriteSize, p->y + movementAmount);
      printf("DOWN: %f %f\n", movementAmount, p->y);
   }

   if(i->left) {
      p->x = fmax(0.0, p->x - movementAmount);
      printf("LEFT: %f %f\n", movementAmount, p->x);
   } else if(i->right) {
      p->x = fmin(SCREEN_WIDTH - g->playerAtlas.spriteSize, p->x + movementAmount);
      printf("RIGHT: %f %f\n", movementAmount, p->x);
   }
}


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
   destRect.x = (int) g->player.x;
   destRect.y = (int) g->player.y;
   SDL_RenderCopy(ren, g->playerAtlas.tex, &sourceRect, &destRect);
}

void drawWorld(SDL_Renderer* ren, gamestate *g) {
   zone *currentZone = &(g->zones[g->zx][g->zy]);
   drawZone(ren, g, currentZone);
   drawPlayer(ren, g);
}

//////////////////////////////////////////////////////////////////////
// INPUT HANDLING


void clearInput(inputState *i) {
   i->up = false;
   i->down = false;
   i->left = false;
   i->right = false;
   i->sword = false;
   i->arrow = false;

   i->keepgoing = true;
}

void handleEvents(inputState *i) {
   SDL_Event e;
   SDL_KeyboardEvent kev;
   while(SDL_PollEvent(&e)) {
      switch(e.type) {
	 case SDL_KEYDOWN:
	    kev = e.key;
	    switch(kev.keysym.sym) {
	       case SDLK_q:
		  // fallthrough
	       case SDLK_ESCAPE:
		  i->keepgoing = false;
		  break;
	       case SDLK_UP:
		  printf("UP\n");
		  i->up = true;
		  break;
	       case SDLK_DOWN:
		  printf("DOWN\n");
		  i->down = true;
		  break;
	       case SDLK_LEFT:
		  printf("LEFT\n");
		  i->left = true;
		  break;
	       case SDLK_RIGHT:
		  printf("RIGHT\n");
		  i->right = true;
		  break;
	       case SDLK_c:
		  printf("SWORD\n");
		  i->sword = true;
		  break;
	       case SDLK_x:
		  printf("ARROW\n");
		  i->arrow = true;
		  break;
	    }
	    break;

	 case SDL_KEYUP:
	    kev = e.key;
	    switch(kev.keysym.sym) {
	       case SDLK_UP:
		  i->up = false;
		  break;
	       case SDLK_DOWN:
		  i->down = false;
		  break;
	       case SDLK_LEFT:
		  i->left = false;
		  break;
	       case SDLK_RIGHT:
		  i->right = false;
		  break;
	       case SDLK_c:
		  i->sword = false;
		  break;
	       case SDLK_x:
		  i->arrow = false;
		  break;
	    }
	    break;

	 case SDL_QUIT:
	    i->keepgoing = false;
	    break;

	 default:
	    break;
      }
   }
}



//////////////////////////////////////////////////////////////////////
// MAIN STUFF

void initPlayer(player *p) {
   p->x = 0;
   p->y = 0;
   p->hits = MAXHITS;
   p->arrows = STARTINGARROWS;
   p->facing = F_DOWN;
   p->movementSpeed = 100;
}



void initGamestate(gamestate *g) {
   g->zx = 0;
   g->zy = 0;
   initPlayer(&(g->player));
   clearInput(&(g->input));
}

void destroyGamestate(gamestate *g) {
   SDL_DestroyTexture(g->terrainAtlas.tex);
   SDL_DestroyTexture(g->playerAtlas.tex);
}

void mainloop(SDL_Renderer *ren) {
   bool keepgoing = true;
   uint32_t then = SDL_GetTicks();
   uint32_t now = then;
   uint32_t dt = 0;
   uint64_t framecount = 0;

   // This is static not to persist the variable across
   // multiple calls of the function, but rather to put
   // it in the data segment so we don't have this big
   // structure on the stack.
   static gamestate g;

   initGamestate(&g);
   loadAssets(ren, &g);

   while(keepgoing) {
      then = now;
      now = SDL_GetTicks();
      dt = now - then;
      framecount += 1;

      // Get input
      handleEvents(&(g.input));
      keepgoing = g.input.keepgoing;
      // Update physics
      movePlayer(&g, dt);


      // Draw stuff
      SDL_RenderClear(ren);
      // Draw things here
      drawWorld(ren, &g);
      SDL_RenderPresent(ren);

      //printf("Then: %d  Now: %d  Frame time: %d\n", then, now, dt);
      //SDL_Delay(1);
   }

   double timef = ((double) SDL_GetTicks()) / 1000.0;
   printf("Game time: %f  Frames: %ld  Avg. FPS: %f\n", 
	  timef, framecount, framecount / timef);

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
