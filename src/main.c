#include "chip8.h"
#include "sdl_helper.h"
#include "types.h"

void *read_bin_file(char *fname, u32 *out_size);

int main(int argc, char **argv) {
   Chip8 *state;
   chip8_init(&state);

   // get cwd
   char cwd[100];
   getcwd(cwd, 100);
   printf("cwd: %s\n", cwd);

   // load app
   u32 app_size = 0;
   assert(argc > 1);
   void *app = read_bin_file(argv[1], &app_size);
   assert(app);
   chip8_load_app(state, app, app_size);

   // start SDL
   SDLCtx *sdl;
   SDLConfig sdl_conf = {
       .title = "Chip 8 Emulator", .width = DISPLAY_WIDTH * PIXEL_DIM, .height = DISPLAY_HEIGHT * PIXEL_DIM};
   assert(sdl2_init(&sdl, &sdl_conf));

   bool keep_window_open = true;
   while (keep_window_open) {
      SDL_Event e;
      while (SDL_PollEvent(&e) > 0) {
         switch (e.type) {
         case SDL_QUIT:
            keep_window_open = false;
            break;
         }
      }

      chip8_tick(state);

      for (int y = 0; y < DISPLAY_HEIGHT; ++y) {
         for (int x = 0; x < DISPLAY_WIDTH; ++x) {
            const u8 color = state->DISPLAY[y][x] ? 255 : 0;
            const SDL_Rect rect = {.x = x * PIXEL_DIM, .y = y * PIXEL_DIM, .w = PIXEL_DIM, .h = PIXEL_DIM};
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

void *read_bin_file(char *fname, u32 *out_size) {
   void *bin = NULL;
   FILE *file;
   if ((file = fopen(fname, "rb"))) {
      // get size
      fseek(file, 0L, SEEK_END);
      *out_size = ftell(file);
      rewind(file);

      // dump app to memory
      bin = malloc(*out_size);
      fread(bin, *out_size, 1, file);

      fclose(file);
   }
   return bin;
}
