#include "SDL_scancode.h"
#include "chip8.h"
#include "sdl_helper.h"
#include "types.h"
#include "utils.h"
#include "viz_internals.h"

// Default sound asset path
#ifndef AUDIO_PATH
#define AUDIO_PATH "../assets/beep.wav"
#endif

#define INSTRUCTIONS_PER_SECOND 700
#define BUDGET_IN_MICROSECONDS (1000000 / INSTRUCTIONS_PER_SECOND)

#define PIXEL_OFF_COLOR 14
#define PIXEL_ON_COLOR 255

static const SDL_Scancode CONTROLS[] = {
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
static u8 get_ch8_keydown(SDLCtx *sdl);
static u8 get_ch8_keyup(SDLCtx *sdl);

typedef struct AudioData {
   u8 *buf;
   u32 pos;
   u32 len;
} AudioData;

static void audio_cb(void *userdata, u8 *stream, int len) {
   AudioData *dat = userdata;

   if (dat->len <= 0)
      return;

   len = len > dat->len ? dat->len : len;
   SDL_MixAudio(stream, dat->buf, len, SDL_MIX_MAXVOLUME / 6);

   dat->buf += len;
   dat->len -= len;
}

int main(int argc, char **argv) {
   Chip8 *ch8 = chip8_init();
   SDLConfig sdl_conf = {
       .title = "Chip 8 Emulator", .width = DISPLAY_WIDTH * PIXEL_DIM, .height = DISPLAY_HEIGHT * PIXEL_DIM};
   SDLCtx *sdl = sdl2_init(&sdl_conf);

   // load ROM
   void *app = NULL;
   {
      assert(argc > 1);
      u32 app_size = 0;
      app = read_bin_file(argv[1], &app_size);
      assert(app);
      chip8_load_app(ch8, app, app_size);
   }

   // load beep sound
   AudioData dat = {0};
   AudioData dat_base = {0};
   {
      SDL_AudioSpec spec = {0};

      if (!SDL_LoadWAV(AUDIO_PATH, &spec, &dat.buf, &dat.len))
         assert(false);

      spec.userdata = &dat;
      spec.callback = audio_cb;

      // save original
      dat_base = dat;

      if (SDL_OpenAudio(&spec, NULL) < 0) {
         printf("Error: %s\n", SDL_GetError());
         assert(false);
      }
   }

   // [0, 15] CHIP8 keys
   u8 key_pressed = 0;
   u8 key_released = 0;

   bool beep = false;

   bool keep_window_open = true;
   while (keep_window_open) {
      u64 time_beg = time_in_ms();

      sdl2_pump_events(sdl);

      // use ESC for QUIT as well
      keep_window_open = !sdl2_is_key_down(sdl, SDL_SCANCODE_ESCAPE);

      // poll for any events this frame
      SDL_Event e;
      while (SDL_PollEvent(&e) > 0) {
         switch (e.type) {
         case SDL_QUIT:
            keep_window_open = false;
            break;
         }
      }

      // fetch, decode, execute
      chip8_tick(ch8, get_ch8_keydown(sdl), get_ch8_keyup(sdl));

      // toggle noise
      if (!beep && chip8_should_beep(ch8)) {
         SDL_PauseAudio(0); // resume
         beep = chip8_should_beep(ch8);
      } else if (beep && !chip8_should_beep(ch8)) {
         SDL_PauseAudio(1); // pause
         beep = chip8_should_beep(ch8);
      }
      if (dat.len == 0)
         dat = dat_base; // keep resetting wav

      // color the screen
      if (chip8_should_draw(ch8)) {
         for (int y = 0; y < DISPLAY_HEIGHT; ++y) {
            for (int x = 0; x < DISPLAY_WIDTH; ++x) {
               const u8 color = ch8->DISPLAY[y][x] ? PIXEL_ON_COLOR : PIXEL_OFF_COLOR;
               const SDL_Rect rect = {.x = x * PIXEL_DIM + PIXEL_EDGE_OFFSET,
                                      .y = y * PIXEL_DIM + PIXEL_EDGE_OFFSET,
                                      .w = PIXEL_DIM - PIXEL_EDGE_OFFSET,
                                      .h = PIXEL_DIM - PIXEL_EDGE_OFFSET};
               SDL_FillRect(sdl->surface, &rect, SDL_MapRGB(sdl->surface->format, color, color, color));
            }
         }
         SDL_UpdateWindowSurface(sdl->window);
      }

      // chip8_viz(ch8);

      u64 time_diff_us = (time_in_ms() - time_beg) * 1000;
      if (time_diff_us > BUDGET_IN_MICROSECONDS)
         continue;

      u64 sleep = BUDGET_IN_MICROSECONDS - time_diff_us;
      if (chip8_sync_display())
         usleep(sleep); // usleep takes microsecs
   }

   SDL_CloseAudio();
   SDL_FreeWAV(dat_base.buf);

   free(app);
   sdl2_terminate(&sdl);
   chip8_terminate(&ch8);
   return 0;
}

u8 get_ch8_keydown(SDLCtx *sdl) {
   for (s32 i = 0; i < 16; ++i) {
      if (sdl2_is_key_down(sdl, CONTROLS[i]))
         return i;
   }
   return UINT8_MAX;
}

u8 get_ch8_keyup(SDLCtx *sdl) {
   for (s32 i = 0; i < 16; ++i) {
      if (sdl2_is_key_released(sdl, CONTROLS[i]))
         return i;
   }
   return UINT8_MAX;
}
