
#ifndef _AOQT_H
#define _AOQT_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
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

typedef enum {
   PS_STANDING,
   PS_WALKING,
   PS_ATTACKING,
   PS_FIRING
} playerState;

typedef struct {
   bool up;
   bool down;
   bool left;
   bool right;
   bool sword;
   bool arrow;

   bool keepgoing;
} inputState;


// The player and mobs share many characteristics;
// hit points, flashy time, facing, etc.
// It _would_ be nice to refactor that out, but,
// game jam.

#define PLAYERSPEED 150
#define ARROWSPEED 500
#define MAXHITS 10
#define STARTINGARROWS 10
#define SWORDDAMAGE 3
#define ARROWDAMAGE 2
#define FLASHYTIME 750
#define FLASHINTERVAL 50
#define ARROWREFIRE 300
#define SWORDREFIRE 250
typedef struct {
   double x;
   double y;
   double velX;
   double velY;
   int size;
   int hits;
   int arrows;
   facing facing;
   double movementSpeed;
   playerState state;
   int flashyTime;
   bool show;
   int flashTimer;

   // We can have one arrow on the screen at a time!
   // But that means if you mash up against an enemy you
   // can fire one arrow per frame, so we still have a
   // refire rate.
   bool arrowFired;
   int arrowTimer;
   double arrowX;
   double arrowY;
   facing arrowFacing;

   // And of course one sword.
   // The sword has a refire rate.
   // However what it REALLY needs is a cooldown...
   int swordTimer;
   facing swordFacing;
   double swordXOffset;
   double swordYOffset;
   double swordSize;
   
} player;

typedef struct {
   double x;
   double y;
   double velX;
   double velY;
   int size;
   int hits;
   facing facing;
   int damage;

   int flashyTime;
   bool show;
   int flashTimer;
} mob;

// So apparently consts in C99 aren't actually const.
// Because they're still stored in memory and so can be modified
// by other means.
#define ZONEWIDTH 16
#define ZONEHEIGHT 12
#define NUMMOBS 16
typedef struct {
   int tiles[ZONEWIDTH][ZONEHEIGHT];
   mob mobs[NUMMOBS];
   // Exits...
} zone;


typedef struct {
   SDL_Texture* tex;
   uint32_t spriteSize;
   // Width and height are in tiles
   uint32_t width;
   uint32_t height;
   //int scalingFactor;
} atlas;

// The gamestate object that contains all the main game data.
#define WORLDWIDTH 10
#define WORLDHEIGHT 10
typedef struct {
   zone zones[WORLDWIDTH][WORLDHEIGHT];
   // Current zone coordinates
   uint32_t zx;
   uint32_t zy;

   inputState input;

   player player;
   atlas terrainAtlas;
   atlas playerAtlas;
   atlas mobAtlas;
   atlas weaponAtlas;
} gamestate;

#endif  // _AOQT_H
