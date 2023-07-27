#include "chip8.h"

#define FONT_ADR 0x50

static u8 FONT[] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0 : each element represents a row chunk --> 5 rows x 8 width (bits)
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

bool chip8_init(Chip8 **state) {
   *state = calloc(1, sizeof(**state));

   // init chip8 font (anywhere in the interpreter space, but commonly at FONT_ADR)
   memcpy(&(*state)->RAM[FONT_ADR], &FONT, sizeof(FONT));

   return true;
}

void chip8_terminate(Chip8 **state) {
   if (*state)
      free(*state);
   state = NULL;
}

void chip8_load_app(Chip8 *state, void *data, u32 size) {
   memcpy(&state->RAM[PROGRAM_START_ADR], data, size);
   state->PC = PROGRAM_START_ADR;
}

void chip8_tick(Chip8 *state) {
   // fetch
   const u16 instr = ((u16)state->RAM[state->PC] << 8) | ((u16)state->RAM[state->PC + 1]);
   state->PC += 2;

   const u16 op = instr & 0xF000;
   const u16 VX = (instr & 0x0F00) >> 8; // x-gpr index
   const u16 VY = (instr & 0x00F0) >> 4; // y-gpr index
   const u16 N = instr & 0x000F;
   const u16 NN = instr & 0x00FF;  // reused 3rd and 4th nibbles
   const u16 NNN = instr & 0x0FFF; // reused [2nd, 4th] nibbles

   // decode
   switch (op) {
   case 0x0000: {
      switch (instr) {
      case 0x00E0:
         memset(state->DISPLAY, 0, sizeof(state->DISPLAY));
         printf("Instruction (0x%04hX): Clear screen\n", instr);
         break;
      case 0x00EE:
         break;
      }
      break;
   }
   case 0x1000:
      state->PC = NNN;
      // printf("Instruction (%x): Set PC to (%x)\n", instr, NNN);
      break;
   case 0x6000:
      state->GPR[VX] = NN;
      printf("Instruction (0x%04hX): GPR[%d] = %d\n", instr, VX, NN);
      break;
   case 0x7000:
      state->GPR[VX] += NN;
      printf("Instruction (0x%04hX): GPR[%d] += %d\n", instr, VX, NN);
      break;
   case 0xA000:
      state->I = NNN;
      printf("Instruction (0x%04hX): I = 0x%04hX\n", instr, NNN);
      break;
   case 0xD000: {
      u16 x = state->GPR[VX] % DISPLAY_WIDTH;
      u16 y = state->GPR[VY] % DISPLAY_HEIGHT;
      state->VF = 0;
      printf("Instruction (0x%04hX): Drawing sprite with height %d (N) at (%d, %d) from GPR[%d] and GPR [%d]\n", instr,
             N, x, y, VX, VY);

      // For each row
      for (u16 n = 0; n < N; ++n) {
         const u8 row = state->RAM[state->I + n];

         // For each column
         for (s32 i = 0; i < 8; ++i) { // msb to lsb
            const bool disp_pix_on = state->DISPLAY[y][x];
            const bool sprite_pix_on = row & (1 << (7 - i));

            if (sprite_pix_on && disp_pix_on) {
               state->DISPLAY[y][x] = false;
               state->VF = 1;
            } else if (sprite_pix_on && !disp_pix_on) {
               state->DISPLAY[y][x] = true;
            }

            // stop drawing at border
            if (x >= DISPLAY_WIDTH)
               break;
            ++x;
         }
         // reset (this was the bug..)
         x -= 8;

         //  stop drawing at border
         if (y >= DISPLAY_HEIGHT)
            break;
         ++y;
      }
      break;
   }
   default:
      break;
   }
}
