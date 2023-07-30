#include "SDL_scancode.h"
#include "chip8.h"
#include "sdl_helper.h"
#include "types.h"
#include "utils.h"

static SDL_Scancode CONTROLS[] = {
    SDL_SCANCODE_1, SDL_SCANCODE_2, SDL_SCANCODE_3, SDL_SCANCODE_4, SDL_SCANCODE_Q, SDL_SCANCODE_W,
    SDL_SCANCODE_E, SDL_SCANCODE_R, SDL_SCANCODE_A, SDL_SCANCODE_S, SDL_SCANCODE_D, SDL_SCANCODE_F,
    SDL_SCANCODE_Z, SDL_SCANCODE_X, SDL_SCANCODE_C, SDL_SCANCODE_V,
};

/* Usually mapped to:
 *
 * 1 2 3 4
 * Q W E R
 * A S D F
 * Z X C V
 */

#define INSTRUCTIONS_PER_SECOND 200
#define BUDGET_IN_MICROSECONDS (1000000 / INSTRUCTIONS_PER_SECOND)

static u64 time_in_ms() {
   struct timeval tv;
   gettimeofday(&tv, NULL);
   return (((long long)tv.tv_sec) * 1000) + (tv.tv_usec / 1000);
}

int main(int argc, char **argv) {
   Chip8 *state;
   chip8_init(&state);

   void *app = NULL;
   {
      assert(argc > 1);
      u32 app_size = 0;
      app = read_bin_file(argv[1], &app_size);
      assert(app);
      chip8_load_app(state, app, app_size);
   }

   SDLCtx *sdl = NULL;
   {
      SDLConfig sdl_conf = {
          .title = "Chip 8 Emulator", .width = DISPLAY_WIDTH * PIXEL_DIM, .height = DISPLAY_HEIGHT * PIXEL_DIM};
      assert(sdl2_init(&sdl, &sdl_conf));
   }

   u8 key_pressed = 0;
   const u8 *kb_state = NULL;
   s32 num_keys = 0;

   bool keep_window_open = true;
   while (keep_window_open) {
      u64 time_beg = time_in_ms();

      SDL_PumpEvents();
      kb_state = SDL_GetKeyboardState(&num_keys);

      for (s32 i = 0; i < 16; ++i) {
         if (kb_state[CONTROLS[i]]) {
            key_pressed = i;
            break;
         }
      }

      // use ESC for QUIT as well
      keep_window_open = !kb_state[SDL_SCANCODE_ESCAPE];

      // poll for any events this frame
      SDL_Event e;
      while (SDL_PollEvent(&e) > 0) {
         switch (e.type) {
         case SDL_QUIT:
            keep_window_open = false;
            break;
         }
      }

      // fetch, decode, execute an instruction
      chip8_tick(state, key_pressed);
      key_pressed = UINT8_MAX; // set to invalid

      // color the screen
      if (chip8_should_draw()) {
         for (int y = 0; y < DISPLAY_HEIGHT; ++y) {
            for (int x = 0; x < DISPLAY_WIDTH; ++x) {
               const u8 color = state->DISPLAY[y][x] ? 255 : 14; // Modify ON/OFF color :)
               const SDL_Rect rect = {.x = x * PIXEL_DIM + PIXEL_EDGE_OFFSET,
                                      .y = y * PIXEL_DIM + PIXEL_EDGE_OFFSET,
                                      .w = PIXEL_DIM - PIXEL_EDGE_OFFSET,
                                      .h = PIXEL_DIM - PIXEL_EDGE_OFFSET};
               SDL_FillRect(sdl->surface, &rect, SDL_MapRGB(sdl->surface->format, color, color, color));
            }
         }
         SDL_UpdateWindowSurface(sdl->window);
      }

      u64 time_diff_us = (time_in_ms() - time_beg) * 1000;

      if (time_diff_us > BUDGET_IN_MICROSECONDS)
         continue;

      u64 sleep = BUDGET_IN_MICROSECONDS - time_diff_us;
      usleep(sleep); // usleep takes microsecs
   }

   free(app);
   sdl2_terminate(&sdl);
   chip8_terminate(&state);
   return 0;
}
