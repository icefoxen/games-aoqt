
#ifndef _AOQT_H
#define _AOQT_H

#include <stdbool.h>
#include <stdio.h>
#include <SDL2/SDL.h>


// So const's in C99 aren't actually constant, because they
// have memory locations which can be messed with in various
// ways.
#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768



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

typedef struct {
   int x;
   int y;
   int hits;
   facing facing;
} mob;

// So apparently consts in C99 aren't actually const.
// Because they're still stored in memory and so can be modified
// by other means.
#define ZONEWIDTH 16
#define ZONEHEIGHT 12
#define NUMMOBS 16
typedef struct {
   char tiles[ZONEWIDTH][ZONEHEIGHT];
   mob mobs[NUMMOBS];
   // Exits...
} zone;


typedef struct {
   SDL_Texture* tex;
   uint32_t spriteSize;
   // Width and height are in tiles
   uint32_t width;
   uint32_t height;
} atlas;

// The gamestate object that contains all the main game data.
#define WORLDWIDTH 10
#define WORLDHEIGHT 10
typedef struct {
   zone zones[WORLDWIDTH][WORLDHEIGHT];
   // Current zone coordinates
   uint32_t zx;
   uint32_t zy;

   player player;
   atlas terrainAtlas;
} gamestate;

#endif  // _AOQT_H
