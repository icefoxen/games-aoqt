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
   loadAtlas(&(g->mobAtlas), "data/mob.bmp", ren, 64);
   loadAtlas(&(g->weaponAtlas), "data/weapons.bmp", ren, 64);
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

void fireArrow(gamestate *g) {
   player *p = &(g->player);
   // If we haven't fired an arrow already
   // And we have ammo
   if(!p->arrowFired && p->arrowTimer == 0 && p->arrows > 0) {
      printf("Arrow fired\n");
      p->arrowFired = true;
      p->arrowTimer = ARROWREFIRE;
      p->arrowFacing = p->facing;
      p->arrowX = p->x + (p->size / 4);
      p->arrowY = p->y + (p->size / 4);
      p->arrows -= 1;
   }
}

void swingSword(gamestate *g) {
   player *p = &(g->player);
   if(p->swordTimer == 0) {
      p->swordTimer = SWORDREFIRE;
      p->swordFacing = p->facing;
      switch(p->swordFacing) {
	 case F_UP:
	    p->swordXOffset = 0;
	    p->swordYOffset = -p->swordSize;
	    break;
	 case F_DOWN:
	    p->swordXOffset = 0;
	    p->swordYOffset = p->size;
	    break;
	 case F_LEFT:
	    p->swordXOffset = -p->swordSize;
	    p->swordYOffset = 0;
	    break;
	 case F_RIGHT:
	    p->swordXOffset = p->size;
	    p->swordYOffset = 0;
	    break;
      }
   }
}

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
      swingSword(g);
   }

   if(i->arrow) {
      fireArrow(g);
   }
}

void damagePlayer(player *p, int damage) {
   if(p->flashyTime <= 0) {
      p->hits -= damage;
      p->flashyTime = FLASHYTIME;
      printf("Ow!  Took %d damage, HP = %d\n", damage, p->hits);
   }
}

void damageMob(mob *m, int damage) {
   if(m->flashyTime <= 0) {
      m->hits -= damage;
      m->flashyTime = FLASHYTIME;
   }
}

void calcPlayer(gamestate *g, int dt) {
   player *p = &(g->player);
   double fdt = (double) dt;
   //double movementAmount = g->player.movementSpeed * fdt / 1000.0;

   // Handle flashy time
   p->flashyTime -= dt;
   if(p->flashyTime <= 0) {
      p->flashyTime = 0;
      p->show = true;
   } else {
      p->flashTimer -= dt;
      if(p->flashTimer <= 0) {
	 p->show = !p->show;
	 p->flashTimer = FLASHINTERVAL;
      }
   }

   // Handle arrows
   double arrowMovement = ARROWSPEED * fdt / 1000.0;
   if(p->arrowFired) {
      //printf("Arrow at %f %f\n", p->arrowX, p->arrowY);
      switch(p->arrowFacing) {
	 case F_UP:
	    p->arrowY -= arrowMovement;
	    break;
	 case F_DOWN:
	    p->arrowY += arrowMovement;
	    break;
	 case F_LEFT:
	    p->arrowX -= arrowMovement;
	    break;
	 case F_RIGHT:
	    p->arrowX += arrowMovement;
	    break;
      }

      if(p->arrowY < 0 || p->arrowY > SCREEN_HEIGHT ||
	 p->arrowX < 0 || p->arrowX > SCREEN_WIDTH) {
	 p->arrowFired = false;
      }
   }
   p->arrowTimer = fmax(0, p->arrowTimer - dt);

   // Handle sword
   if(p->swordTimer > 0) {
      p->swordTimer -= dt;
   } else {
      p->swordTimer = 0;
   }


   // Handle movement & input
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
      // This idiom gets remainder rather than modulo,
      // so we don't have (negative % positive = negative)
      g->zx = (g->zx + WORLDWIDTH - 1) % WORLDWIDTH;
      p->x = SCREEN_WIDTH - p->size;
      printf("ZX is now %d\n", g->zx);
   } else if(p->x + p->size >= SCREEN_WIDTH) {
      // move one zone to the right
      g->zx = (g->zx + WORLDWIDTH + 1) % WORLDWIDTH;
      p->x = 0;
      printf("ZX is now %d\n", g->zx);
   }

   if(p->y < 0) {
      // Move one zone up
      g->zy = (g->zy + WORLDHEIGHT - 1) % WORLDHEIGHT;
      p->y = SCREEN_HEIGHT - p->size;
      printf("ZY is now %d\n", g->zy);
   } else if(p->y + p->size >= SCREEN_HEIGHT) {
      // Move one zone down
      g->zy = (g->zy + WORLDHEIGHT + 1) % WORLDHEIGHT;
      p->y = 0;
      printf("ZY is now %d\n", g->zy);
   }
}

double clamp(double from, double to, double val) {
   return fmax(from, fmin(to, val));
}

void calcMob(gamestate *g, mob *m, int dt) {
   double fdt = (double) dt;
   double xOffset = m->velX * fdt / 1000.0;
   double yOffset = m->velY * fdt / 1000.0;

   m->x = clamp(0.0, SCREEN_WIDTH  - m->size, m->x + xOffset);
   m->y = clamp(0.0, SCREEN_HEIGHT - m->size, m->y + yOffset);

   // Handle flashy time
   m->flashyTime -= dt;
   if(m->flashyTime <= 0) {
      m->flashyTime = 0;
      m->show = true;
   } else {
      m->flashTimer -= dt;
      if(m->flashTimer <= 0) {
	 m->show = !m->show;
	 m->flashTimer = FLASHINTERVAL;
      }
   }

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

void getArrowBB(player *p, SDL_Rect *rect) {
   rect->x = (int) p->arrowX;
   rect->y = (int) p->arrowY;
   // XXX: Arrows are smaller than a full tile, really...
   // And the bounding box depends on the orientation, too.
   rect->w = 64;
   rect->h = 64;
}

void getSwordBB(player *p, SDL_Rect *rect) {
   // XXX: Sword size should probably be a rectangle, not a square.
   rect->w = p->swordSize;
   rect->h = p->swordSize;
   rect->x = (int) (p->x + p->swordXOffset);
   rect->y = (int) (p->y + p->swordYOffset);
}



void handlePlayerTerrainCollision(gamestate *g, SDL_Rect *collision) {
   player *p = &(g->player);

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

// XXX: More code duplication
// This is exactly the same as above, but
// for mobs instead of players.
void handleMobTerrainCollision(mob *m, SDL_Rect *collision) {
   if(m->velX > 0 && collision->x > m->x) {
      // We are hitting a tile on the right
      m->velX = 0;
      m->x -= collision->w;
   } else if(m->velX < 0 && collision->x <= m->x) {
      // We are hitting a tile on the left
      m->velX = 0;
      m->x += collision->w;
   } else if(m->velY > 0 && collision->y > m->y) {
      // We are hitting a tile below us
      m->velY = 0;
      m->y -= collision->h;
   } else if(m->velY < 0 && collision->y <= m->y) {
      // We are hitting a tile above us
      m->velY = 0;
      m->y += collision->h;
   }   
}

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

   // Then we do the exact same thing for mobs.
   // XXX: Yay code duplication!
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
	    for(int i = 0; i < NUMMOBS; i++) {
	       mob *m = &(z->mobs[i]);
	       SDL_Rect mobBB;
	       getMobBB(m, &mobBB);
	       if(m->hits > 0 && SDL_IntersectRect(&mobBB, &tileRect, &result)) {
		  handleMobTerrainCollision(m, &result);
	       }
	    }
	 }
      }
   }
}


// Handles weapon collisions too.
void collidePlayerWithMobs(gamestate *g) {
   zone *z = getCurrentZone(g);
   player *p = &(g->player);
   for(int i = 0; i < NUMMOBS; i++) {
      mob *m = &(z->mobs[i]);
      if(m->hits <= 0) {
	 continue;
      }
      SDL_Rect playerBB, mobBB, result;
      getPlayerBB(p, &playerBB);
      getMobBB(m, &mobBB);
      if(SDL_IntersectRect(&playerBB, &mobBB, &result)) {
	 //printf("Mob colliding with player\n");
	 damagePlayer(p, m->damage);
      }

      // Arrows
      if(p->arrowFired) {
	 SDL_Rect arrowBB;
	 getArrowBB(p, &arrowBB);
	 //printf("Arrow BB: %d %d %d %d\n", arrowBB.x, arrowBB.y, arrowBB.w, arrowBB.h);
	 if(SDL_IntersectRect(&arrowBB, &mobBB, &result)) {
	    //printf("Hit mob %d\n", i);
	    damageMob(m, ARROWDAMAGE);
	    p->arrowFired = false;
	 }
      }

      // Sword
      if(p->swordTimer > 0) {
	 SDL_Rect swordBB;
	 getSwordBB(p, &swordBB);
	 //printf("Sword BB: %d %d %d %d\n", swordBB.x, swordBB.y, swordBB.w, swordBB.h);
	 if(SDL_IntersectRect(&swordBB, &mobBB, &result)) {
	    // Crap, do mobs get flashy time too?  They have to...
	    damageMob(m, SWORDDAMAGE);
	 }
      }
   }
}

//////////////////////////////////////////////////////////////////////
// DRAWING
/*
// Draw a texture at x,y without changing its size.
void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y) {
   SDL_Rect dest = {.x = x, .y = y};
   SDL_QueryTexture(tex, NULL, NULL, &dest.w, &dest.h);
   SDL_RenderCopy(ren, tex, NULL, &dest);
}

// Same as above, but takes a sprite number too
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
   if(g->player.show) {
      SDL_Rect sourceRect, destRect;
      atlasCoords(&(g->playerAtlas), 0, &sourceRect);
      destRect.x = (int) g->player.x;
      destRect.y = (int) g->player.y;
      destRect.w = sourceRect.w;
      destRect.h = sourceRect.h;
      SDL_RenderCopy(ren, g->playerAtlas.tex, &sourceRect, &destRect);
   }

   // Draw arrows too!
   if(g->player.arrowFired) {
      SDL_Rect sourceRect, destRect;
      atlasCoords(&(g->weaponAtlas), 0, &sourceRect);
      destRect.x = (int) g->player.arrowX;
      destRect.y = (int) g->player.arrowY;
      destRect.w = sourceRect.w;
      destRect.h = sourceRect.h;
      SDL_RenderCopy(ren, g->weaponAtlas.tex, &sourceRect, &destRect);
   }

   // And draw the sword!
   if(g->player.swordTimer > 0) {
      SDL_Rect sourceRect, destRect;
      atlasCoords(&(g->weaponAtlas), 1, &sourceRect);
      destRect.x = (int) (g->player.x + g->player.swordXOffset);
      destRect.y = (int) (g->player.y + g->player.swordYOffset);
      destRect.w = sourceRect.w;
      destRect.h = sourceRect.h;
      SDL_RenderCopy(ren, g->weaponAtlas.tex, &sourceRect, &destRect);
   }
}

void drawMob(SDL_Renderer *ren, gamestate *g, mob *m) {
   if(m->show) {
      SDL_Rect sourceRect, destRect;
      atlasCoords(&(g->mobAtlas), 0, &sourceRect);
      destRect.x = (int) m->x;
      destRect.y = (int) m->y;
      destRect.w = sourceRect.w;
      destRect.h = sourceRect.h;
      SDL_RenderCopy(ren, g->mobAtlas.tex, &sourceRect, &destRect);
   }
}

void drawMobs(SDL_Renderer *ren, gamestate *g) {
   zone *z = getCurrentZone(g);
   for(int i = 0; i < NUMMOBS; i++) {
      mob *m = &(z->mobs[i]);
      //printf("Drawing mob %d at %f %f...\n", i, m->x, m->y);
      if(m->hits > 0) {
	 drawMob(ren, g, m);
      }
   }
}

void drawWorld(SDL_Renderer* ren, gamestate *g) {
   zone *currentZone = getCurrentZone(g);
   drawZone(ren, g, currentZone);
   drawMobs(ren, g);
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

// Fills a zone with floor tiles, and adds walls around the edges.
void generateEmptyZone(zone *z) {
   for(int x = 0; x < ZONEWIDTH; x++) {
      for(int y = 0; y < ZONEHEIGHT; y++) {
	 if(x == 0 || y == 0 ||
	    x == ZONEWIDTH - 1 || y == ZONEHEIGHT - 1) {
	    // Put in wall
	    z->tiles[x][y] = 254;
	 } else {
	    // Put in floor.
	    z->tiles[x][y] = 0;
	 }
      }
   }
}

void makeZoneExits(zone *z) {
   z->tiles[5][0] = 0;
   z->tiles[5][ZONEHEIGHT-1] = 0;
   z->tiles[6][0] = 0;
   z->tiles[6][ZONEHEIGHT-1] = 0;

   z->tiles[0][5] = 0;
   z->tiles[ZONEWIDTH-1][5] = 0;
   z->tiles[0][6] = 0;
   z->tiles[ZONEWIDTH-1][6] = 0;
}

void initMob1(mob *m) {
   // XXX: Find a better way to place mobs
   m->x = (random() % 10 + 2) * 64;
   m->y = (random() % 8 + 1) * 64;
   m->velX = 0;
   m->velY = 0;
   m->size = 64;
   m->hits = 8;
   m->facing = F_DOWN;
   m->damage = 1;
}

void generateMobs(zone *z) {
   int numMobs = random() % NUMMOBS;
   for(int i = 0; i < NUMMOBS; i++) {
      mob *m = &(z->mobs[i]);
      initMob1(m);
      // We have to init all the mobs to _something_
      // But if they're not doing anything we kill
      // them instantly and they get ignored.
      if(i > numMobs) {
	 m->hits = 0;
      }
   }

   //printf("Number of mobs: %d\n", numMobs);
}

void generateZone(zone *z, atlas* terrain) {
   //int tileCollideThreshold = terrain->width * (terrain->height / 2);
   //int tileMax = terrain->width * terrain->height;
   for(int x = 0; x < ZONEWIDTH; x++) {
      for(int y = 0; y < ZONEHEIGHT; y++) {
	 generateEmptyZone(z);
	 makeZoneExits(z);
	 //int tile = random() % tileMax;
	 //z->tiles[x][y] = tile;
      }
   }

   generateMobs(z);
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
   p->x = SCREEN_WIDTH / 2;
   p->y = SCREEN_HEIGHT / 2;
   p->hits = MAXHITS;
   p->arrows = STARTINGARROWS;
   p->facing = F_DOWN;
   p->movementSpeed = PLAYERSPEED;
   p->size = g->playerAtlas.spriteSize;
   p->show = true;
   p->arrowFired = false;

   p->swordSize = 64;
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
      collidePlayerWithMobs(&g);


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
