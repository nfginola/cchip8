#ifndef _CHIP8
#define _CHIP8
#include "adr_stack.h"
#include "types.h"

#define RAM_SIZE 0x1000 // 4Kb (12 bits) addressable
#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32

#define PIXEL_DIM 24        // (PIXEL_DIM x PIXEL_DIM) pixels represent native 1x1
#define PIXEL_EDGE_OFFSET 1 // Offset colors from pixel edge for some better looking large blocks!

#define INTERPRETER_START_ADR 0x0 // [0, 0x1FF]
#define PROGRAM_START_ADR 0x200

#define NUM_GPRS 16

typedef struct Chip8 {
   u16 PC;
   u8 RAM[RAM_SIZE];
   bool DISPLAY[DISPLAY_HEIGHT][DISPLAY_WIDTH];
   u16 I;
   AdrStack STACK;
   u8 DELAY_TIMER;
   u8 SOUND_TIMER;
   u8 GPR[NUM_GPRS];

   bool KEYS[16];
   bool SHOULD_DRAW;
   u64 PREV_TIME;

} Chip8;

bool chip8_init(Chip8 **state);
void chip8_terminate(Chip8 **state);

void chip8_load_app(Chip8 *state, void *data, u32 size);

bool chip8_should_draw(Chip8 *state);

void chip8_tick(Chip8 *state, u8 key_pressed, u8 key_released);
void chip8_timer_tick(Chip8 *state);

#endif
