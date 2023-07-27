#include "adr_stack.h"
#include "sdl_helper.h"
#include "types.h"

#define RAM_SIZE 0x1000 // 4Kb (12 bits) addressable
#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32
#define PIXEL_DIM 32 // (PIXEL_DIM x PIXEL_DIM) pixels represent native 1x1

#define INTERPRETER_START_ADR 0x0 // [0, 0x1FF]
#define PROGRAM_START_ADR 0x200

static u8 FONT[] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

typedef struct Chip8 {
   u16 PC;
   u8 RAM[RAM_SIZE];
   bool DISPLAY[DISPLAY_HEIGHT][DISPLAY_WIDTH];
   u16 I;
   AdrStack STACK;
   u8 DELAY_TIMER;
   u8 SOUND_TIMER;
   u8 GPR[16];

} Chip8;

int main(int argc, char **argv) {
   Chip8 *state = calloc(1, sizeof(*state));

   // init chip8 font (anywhere in the interpreter space)
   memcpy(&state->RAM[0x50], &FONT, sizeof(FONT));

   SDLCtx *sdl;
   SDLConfig sdl_conf = {
       .title = "Chip 8 Emulator", .width = DISPLAY_WIDTH * PIXEL_DIM, .height = DISPLAY_HEIGHT * PIXEL_DIM};
   assert(sdl2_init(&sdl, &sdl_conf));

   if (SDL_MUSTLOCK(sdl->surface))
      printf("Must lock surface (SDL_LockSurface)\n");

   // mapping to rects to minimize API calls
   SDL_Rect disp_on[DISPLAY_WIDTH * DISPLAY_HEIGHT];
   SDL_Rect disp_off[DISPLAY_WIDTH * DISPLAY_HEIGHT];

   s32 thingy = 0;

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

      for (s32 i = 0; i < DISPLAY_HEIGHT; i += 3)
         state->DISPLAY[i][thingy % DISPLAY_WIDTH] = true;

      s32 on_next = 0;
      s32 off_next = 0;
      for (int y = 0; y < DISPLAY_HEIGHT; ++y) {
         for (int x = 0; x < DISPLAY_WIDTH; ++x) {
            const SDL_Rect rect = {.x = x * PIXEL_DIM, .y = y * PIXEL_DIM, .w = PIXEL_DIM, .h = PIXEL_DIM};
            if (state->DISPLAY[y][x])
               disp_on[on_next++] = rect;
            else
               disp_off[off_next++] = rect;
         }
      }
      SDL_FillRects(sdl->surface, disp_on, on_next, SDL_MapRGB(sdl->surface->format, 255, 255, 255));
      SDL_FillRects(sdl->surface, disp_off, off_next, SDL_MapRGB(sdl->surface->format, 0, 0, 0));

      // for (int y = 0; y < DISPLAY_HEIGHT; ++y) {
      //    for (int x = 0; x < DISPLAY_WIDTH; ++x) {
      //       const u8 color = state->DISPLAY[y][x] ? 255 : 0;
      //       const SDL_Rect rect = {.x = x * PIXEL_DIM, .y = y * PIXEL_DIM, .w = PIXEL_DIM, .h = PIXEL_DIM};
      //       SDL_FillRect(sdl->surface, &rect, SDL_MapRGB(sdl->surface->format, color, color, color));
      //    }
      // }

      for (s32 i = 0; i < DISPLAY_HEIGHT; i += 3)
         state->DISPLAY[i][thingy++ % DISPLAY_WIDTH] = false;

      SDL_UpdateWindowSurface(sdl->window);
   }

   sdl2_terminate(&sdl);
   return 0;
}
