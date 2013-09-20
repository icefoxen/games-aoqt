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
void LogWarning(char* message) {
   fprintf(errorStream, "WARNING: %s\n", message);
}

void LogError(char* message) {
   fprintf(errorStream, "ERROR: %s %s\n", message, SDL_GetError());
   fprintf(errorStream, "Bailing.\n");
   Cleanup();
   exit(1);
}

void CheckError(bool errorTest, char* message) {
   if(errorTest) {
      LogError(message);
   }
}


SDL_Texture* loadTexture(char* file, SDL_Renderer* ren) {
   SDL_Surface* imgSurf = SDL_LoadBMP(file);
   if(!imgSurf) {
      LogWarning(file);
      LogError("Could not load image");
   }
   SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, imgSurf);
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
      SDL_RenderPresent(ren);
      SDL_Delay(30);
   }

   SDL_DestroyTexture(tex);
   SDL_DestroyRenderer(ren);
   SDL_DestroyWindow(win);
   SDL_Quit();

   return 0;
}
