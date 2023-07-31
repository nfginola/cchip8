#ifndef _SDL_HELPER
#define _SDL_HELPER
#include "SDL2/SDL.h"
#include "types.h"

typedef struct SDLCtx {
   SDL_Window *window;
   SDL_Surface *surface;

   s32 num_keys_;
   const u8 *kb_is_down_;
   u8 kb_is_released_[SDL_NUM_SCANCODES];

} SDLCtx;

typedef struct SDLConfig {
   char title[70];
   u32 width;
   u32 height;
} SDLConfig;

SDLCtx *sdl2_init(const SDLConfig *conf);
void sdl2_terminate(SDLCtx **ctx);

void sdl2_pump_events(SDLCtx *ctx);

bool sdl2_is_key_released(SDLCtx *ctx, SDL_Scancode sc);
bool sdl2_is_key_down(SDLCtx *ctx, SDL_Scancode sc);

#endif
