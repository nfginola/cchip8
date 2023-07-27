#include "sdl_helper.h"

bool sdl2_init(SDLCtx **ctx, const SDLConfig *conf) {
   *ctx = calloc(1, sizeof(**ctx));
   if (!(*ctx))
      return false;

   if (SDL_Init(SDL_INIT_VIDEO) < 0)
      return false;

   (*ctx)->window =
       SDL_CreateWindow(conf->title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, conf->width, conf->height, 0);
   if (!(*ctx)->window)
      return false;

   (*ctx)->surface = SDL_GetWindowSurface((*ctx)->window);
   if (!(*ctx)->surface)
      return false;

   SDL_UpdateWindowSurface((*ctx)->window);
   // SDL_Delay(5000);
   return true;
}

void sdl2_terminate(SDLCtx **ctx) {
   SDL_DestroyWindow((*ctx)->window);

   free(*ctx);
   ctx = NULL;
}
