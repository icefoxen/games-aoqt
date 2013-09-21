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

zone* getCurrentZone(gamestate *g) {
   return &(g->zones[g->zx][g->zy]);
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
   printf("Loading assets... ");
   loadAtlas(&(g->terrainAtlas), "data/terrain.bmp", ren, 64);
   loadAtlas(&(g->playerAtlas), "data/player.bmp", ren, 64);
   printf("Done.\n");
   
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

void handlePlayerInput(gamestate *g) {
   inputState *i = &(g->input);
   player *p = &(g->player);

   if(i->up) {
      p->velX = 0;
      p->velY = -p->movementSpeed;
      p->state = PS_WALKING;
      p->facing = F_UP;
   } else if(i->down) {
      p->velX = 0;
      p->velY = p->movementSpeed;
      p->state = PS_WALKING;
      p->facing = F_DOWN;
      //printf("DOWN: %f %f\n", movementAmount, p->y);
   } else if(i->left) {
      p->velX = -p->movementSpeed;
      p->velY = 0;
      p->state = PS_WALKING;
      p->facing = F_LEFT;
      //printf("LEFT: %f %f\n", movementAmount, p->x);
   } else if(i->right) {
      p->velX = p->movementSpeed;
      p->velY = 0;
      p->state = PS_WALKING;
      p->facing = F_RIGHT;
      //printf("RIGHT: %f %f\n", movementAmount, p->x);
   } else {
      p->state = PS_STANDING;
      p->velX = 0;
      p->velY = 0;
   }
   
   if(i->sword) {
   }

   if(i->arrow) {
   }
}

void calcPlayer(gamestate *g, int dt) {
   player *p = &(g->player);
   double fdt = (double) dt;
   //double movementAmount = g->player.movementSpeed * fdt / 1000.0;

   handlePlayerInput(g);
   double xOffset = p->velX * fdt / 1000.0;
   double yOffset = p->velY * fdt / 1000.0;

   p->x = p->x + xOffset;
   p->y = p->y + yOffset;
   // If x or y are out of bounds of the screen, we flip over to the
   // adjacent zone and move the player to the opposite side of the screen,
   // making it so they have gone off one edge in one zone and entered the
   // opposite edge of the other zone.
   // This means all we have to do is make sure the appropraite walls of each
   // zone are unblocked, and that the world as a whole doesn't let anyone wander
   // off of it.
   // ...oooh, or we could just wrap the zones...  I like that idea.
   if(p->x < 0) {
      // Move one zone to the left
      g->zx = (g->zx - 1) % WORLDWIDTH;
      p->x = SCREEN_WIDTH - p->size;
      printf("ZX is now %d\n", g->zx);
   } else if(p->x + p->size > SCREEN_WIDTH) {
      // move one zone to the right
      g->zx = (g->zx + 1) % WORLDWIDTH;
      p->x = 0;
      printf("ZX is now %d\n", g->zx);
   }

   if(p->y < 0) {
      // Move one zone up
      g->zy = (g->zy - 1) % WORLDHEIGHT;
      p->y = SCREEN_HEIGHT - p->size;
      printf("ZY is now %d\n", g->zy);
   } else if(p->y + p->size > SCREEN_HEIGHT) {
      // Move one zone down
      g->zy = (g->zy + 1) % WORLDHEIGHT;
      p->y = 0;
      printf("ZY is now %d\n", g->zy);
   }
}

void calcMob(gamestate *g, mob *m, int dt) {
   double fdt = (double) dt;
   double xOffset = m->velX * fdt / 1000.0;
   double yOffset = m->velY * fdt / 1000.0;

   m->x = fmax(0.0, fmin(SCREEN_WIDTH  - m->size, m->x + xOffset));
   m->y = fmax(0.0, fmin(SCREEN_HEIGHT - m->size, m->y + yOffset));
}

void calcMobs(gamestate *g, int dt) {
   zone *z = getCurrentZone(g);
   for(int i = 0; i < NUMMOBS; i++) {
      mob *m = &(z->mobs[i]);
      if(m->hits > 0) {
	 calcMob(g, m, dt);
      }
   }
}

// Get bounding boxes for collision
void getPlayerBB(player *p, SDL_Rect *rect) {
   rect->x = (int) p->x;
   rect->y = (int) p->y;
   rect->w = p->size;
   rect->h = p->size;
}

void getMobBB(mob *m, SDL_Rect *rect) {
   rect->x = (int) m->x;
   rect->y = (int) m->y;
   rect->w = m->size;
   rect->h = m->size;
}

void handlePlayerTerrainCollision(gamestate *g, SDL_Rect *collision) {
   player *p = &(g->player);

   // This assumes that nothing except deliberate player movement
   // can move the player.
   // No knockback, etc.  
   if(p->velX > 0 && collision->x > p->x) {
      // We are hitting a tile on the right
      p->velX = 0;
      p->x -= collision->w;
   } else if(p->velX < 0 && collision->x <= p->x) {
      // We are hitting a tile on the left
      p->velX = 0;
      p->x += collision->w;
   } else if(p->velY > 0 && collision->y > p->y) {
      // We are hitting a tile below us
      p->velY = 0;
      p->y -= collision->h;
   } else if(p->velY < 0 && collision->y <= p->y) {
      // We are hitting a tile above us
      p->velY = 0;
      p->y += collision->h;
   }   
}

// By definition the bottom half of an atlas image is made of
// things that collide with you, and the top half isn't.
void collideTerrain(gamestate *g) {
   atlas *terrain = &(g->terrainAtlas);
   int tileCollideThreshold = terrain->width * (terrain->height / 2);
   SDL_Rect playerBB;
   getPlayerBB(&(g->player), &playerBB);
   zone *z = getCurrentZone(g);
   
   // KISS
   SDL_Rect tileRect;
   tileRect.w = terrain->spriteSize;
   tileRect.h = terrain->spriteSize;
   for(int x = 0; x < ZONEWIDTH; x++) {
      for(int y = 0; y < ZONEHEIGHT; y++) {
	 int tile = z->tiles[x][y];
	 if(tile < tileCollideThreshold) {
	    continue;
	 } else {
	    tileRect.x = x * terrain->spriteSize;
	    tileRect.y = y * terrain->spriteSize;
	    SDL_Rect result;
	    if(SDL_IntersectRect(&playerBB, &tileRect, &result)) {
	       handlePlayerTerrainCollision(g, &result);
	       //printf("Colliding: %d %d %d %d\n", result.x, result.y, result.w, result.h);
	    }
	 }
      }
   }
}

void collideMobs(gamestate *g) {
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

void drawMob(SDL_Renderer *ren, gamestate *g, mob *m) {
}

void drawMobs(SDL_Renderer *ren, gamestate *g) {
   zone *z = getCurrentZone(g);
   for(int i = 0; i < NUMMOBS; i++) {
      mob *m = &(z->mobs[i]);
      if(m->hits > 0) {
	 drawMob(ren, g, m);
      }
   }
}

void drawWorld(SDL_Renderer* ren, gamestate *g) {
   zone *currentZone = getCurrentZone(g);
   drawZone(ren, g, currentZone);
   drawPlayer(ren, g);
   drawMobs(ren, g);
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
		  //printf("UP\n");
		  i->up = true;
		  break;
	       case SDLK_DOWN:
		  //printf("DOWN\n");
		  i->down = true;
		  break;
	       case SDLK_LEFT:
		  //printf("LEFT\n");
		  i->left = true;
		  break;
	       case SDLK_RIGHT:
		  //printf("RIGHT\n");
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
// WORLD GEN

void generateZone(zone *z, atlas* terrain) {
   //int tileCollideThreshold = terrain->width * (terrain->height / 2);
   int tileMax = terrain->width * terrain->height;
   for(int x = 0; x < ZONEWIDTH; x++) {
      for(int y = 0; y < ZONEHEIGHT; y++) {
	 int tile = random() % tileMax;
	 z->tiles[x][y] = tile;
      }
   }
}



void generateWorld(gamestate *g) {
   printf("Generating world... ");
   for(int x = 0; x < WORLDWIDTH; x++) {
      for(int y = 0; y < WORLDHEIGHT; y++) {
	 generateZone(&(g->zones[x][y]), &(g->terrainAtlas));
      }
   }
   printf("Done.\n");
}



//////////////////////////////////////////////////////////////////////
// MAIN STUFF

void initPlayer(gamestate *g, player *p) {
   p->x = 150;
   p->y = 400;
   p->hits = MAXHITS;
   p->arrows = STARTINGARROWS;
   p->facing = F_DOWN;
   p->movementSpeed = 100;
   p->size = g->playerAtlas.spriteSize;
}

void initGamestate(gamestate *g) {
   g->zx = 0;
   g->zy = 0;
   initPlayer(g, &(g->player));
   clearInput(&(g->input));
   generateWorld(g);
}

void destroyGamestate(gamestate *g) {
   printf("Freeing assets... ");
   SDL_DestroyTexture(g->terrainAtlas.tex);
   SDL_DestroyTexture(g->playerAtlas.tex);
   printf("Done\n.");
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

   loadAssets(ren, &g);
   initGamestate(&g);

   while(keepgoing) {
      then = now;
      now = SDL_GetTicks();
      dt = now - then;
      framecount += 1;

      // Get input
      handleEvents(&(g.input));
      keepgoing = g.input.keepgoing;
      // Update physics
      calcPlayer(&g, dt);
      calcMobs(&g, dt);
      collideTerrain(&g);
      collideMobs(&g);


      // Draw stuff
      SDL_RenderClear(ren);
      // Draw things here
      drawWorld(ren, &g);
      SDL_RenderPresent(ren);

      //printf("Then: %d  Now: %d  Frame time: %d\n", then, now, dt);
      // Yield to scheduler, I guess.
      SDL_Delay(1);
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
