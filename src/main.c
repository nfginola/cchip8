#include "chip8.h"
#include "sdl_helper.h"
#include "types.h"
#include "utils.h"

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

   bool keep_window_open = true;
   while (keep_window_open) {
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
      chip8_tick(state);

      // color the screen
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

   free(app);
   sdl2_terminate(&sdl);
   chip8_terminate(&state);
   return 0;
}
