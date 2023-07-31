#include "sdl_helper.h"

SDLCtx *sdl2_init(const SDLConfig *conf) {
   SDLCtx *ctx = calloc(1, sizeof(*ctx));

   if (SDL_Init(SDL_INIT_VIDEO) < 0)
      return false;

   ctx->window =
       SDL_CreateWindow(conf->title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, conf->width, conf->height, 0);
   if (!ctx->window)
      return false;

   ctx->surface = SDL_GetWindowSurface(ctx->window);
   if (!ctx->surface)
      return false;

   SDL_UpdateWindowSurface(ctx->window);

   if (SDL_MUSTLOCK(ctx->surface))
      printf("Must lock surface (SDL_LockSurface()) to access pixels\n");

   memset(&ctx->kb_is_released_, SDL_NUM_SCANCODES, sizeof(ctx->kb_is_released_[0]));

   return ctx;
}

void sdl2_terminate(SDLCtx **ctx) {
   SDL_DestroyWindow((*ctx)->window);

   free(*ctx);
   ctx = NULL;
}

void sdl2_pump_events(SDLCtx *ctx) {
   // track prev
   u8 prev[SDL_NUM_SCANCODES] = {0};
   if (ctx->kb_is_down_) {
      memcpy(&prev, ctx->kb_is_down_, sizeof(prev[0]) * ctx->num_keys_);
   }

   // clear old released
   memset(ctx->kb_is_released_, 0, SDL_NUM_SCANCODES * sizeof(ctx->kb_is_released_[0]));

   // update is-down
   SDL_PumpEvents();
   ctx->kb_is_down_ = SDL_GetKeyboardState(&ctx->num_keys_);

   // apply on-release
   for (s32 i = 0; i < ctx->num_keys_; ++i) {
      // if was down, but not anymore..
      if (prev[i] && !ctx->kb_is_down_[i]) {
         ctx->kb_is_released_[i] = 1;
      }
   }
}

bool sdl2_is_key_released(SDLCtx *ctx, SDL_Scancode sc) {
   return ctx->kb_is_released_[sc];
}

bool sdl2_is_key_down(SDLCtx *ctx, SDL_Scancode sc) {
   return ctx->kb_is_down_[sc];
}
