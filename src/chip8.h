#ifndef _CHIP8
#define _CHIP8
#include "adr_stack.h"
#include "types.h"

#define RAM_SIZE 0x1000 // 4Kb (12 bits) addressable
#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32
#define PIXEL_DIM 32 // (PIXEL_DIM x PIXEL_DIM) pixels represent native 1x1

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
   u8 VF;

} Chip8;

bool chip8_init(Chip8 **state);
void chip8_terminate(Chip8 **state);

void chip8_load_app(Chip8 *state, void *data, u32 size);

void chip8_tick(Chip8 *state);

#endif
