#include <stdio.h>
#include <SDL2/SDL.h>


int main(int argc, char** argv) {
   if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
      printf("SDL init error: %s\n", SDL_GetError());
      return 1;
   }

   SDL_Window *win = SDL_CreateWindow("Hello world!", 100, 100, 640, 480, SDL_WINDOW_SHOWN);
   if(!win) {
      printf("SDL window create error: %s\n", SDL_GetError());
      return 1;
   }

   SDL_Renderer *ren = SDL_CreateRenderer(win, -1,
					  SDL_RENDERER_ACCELERATED);
   if(!ren) {
      printf("SDL renderer create error: %s\n", SDL_GetError());
      return 1;
   }

   SDL_Surface *bmp = SDL_LoadBMP("data/hello.bmp");
   if(!bmp){
      printf("SDL BMP load error: %s\n", SDL_GetError());
      return 1;
   }

   SDL_Texture *tex = SDL_CreateTextureFromSurface(ren, bmp);
   SDL_FreeSurface(bmp);
   if(!tex) {
      printf("SDL create texture from surface error: %s\n", SDL_GetError());
      return 1;
   }

   SDL_RenderClear(ren);
   SDL_RenderCopy(ren, tex, NULL, NULL);
   SDL_RenderPresent(ren);
   
   SDL_Delay(3000);
   
   SDL_DestroyTexture(tex);
   SDL_DestroyRenderer(ren);
   SDL_DestroyWindow(win);
   SDL_Quit();

   return 0;
}
