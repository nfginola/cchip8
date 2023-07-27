#ifndef _SDL_HELPER
#define _SDL_HELPER
#include "SDL2/SDL.h"
#include "types.h"

typedef struct SDLCtx {
   SDL_Window *window;
   SDL_Surface *surface;
} SDLCtx;

typedef struct SDLConfig {
   char title[70];
   u32 width;
   u32 height;
} SDLConfig;

bool sdl2_init(SDLCtx **ctx, const SDLConfig *conf);
void sdl2_terminate(SDLCtx **ctx);
#endif
