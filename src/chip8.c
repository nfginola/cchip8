#include "chip8.h"
#include "utils.h"

#define FONT_ADR 0x50
#define FONT_STRIDE 5

// Timers decremented at a rate of 60 Hz (60 times per second)
#define TIMER_FREQ_HZ 60
#define TIMER_THRESHOLD_MS ((1.0 / TIMER_FREQ_HZ) * 1000.0)

/*
 * Select target hardware in CMakeLists
 * CHIP8 / SCHIP / XOCHIP
 *
 * Note: Extensions for SCHIP and XOCHIP are unimplemented.
 * Only quirks were fixed for the sake of the tests.
 *
 * Quirks:
 *
 * Q_VF_RESET
 * Q_MEMORY
 * Q_DISP_WAIT
 * Q_CLIPPING
 * Q_SHIFTING
 * Q_JUMPING
 */
#ifdef CHIP8
#define Q_VF_RESET
#define Q_MEMORY
#define Q_DISP_WAIT
#define Q_CLIPPING

#elif defined(SCHIP)
#define Q_CLIPPING
#define Q_SHIFTING
#define Q_JUMPING

#elif defined(XOCHIP)
#define Q_MEMORY

#endif

/*
 * Map accordingly to your desired key layout
 * 1 2 3 C
 * 4 5 6 D
 * 7 8 9 E
 * A 0 B F
 */
static const u8 KEY_MAPPING[16] = {1, 2, 3, 0xC, 4, 5, 6, 0xD, 7, 8, 9, 0xE, 0xA, 0, 0xB, 0xF};
static const u8 FONT[] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0 : each element represents a row, 5 byte sprite font
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

static void timer_tick(Chip8 *state);

Chip8 *chip8_init() {
   Chip8 *state = calloc(1, sizeof(*state));

   // init chip8 font (anywhere in the interpreter space, but commonly at FONT_ADR)
   memcpy(&state->RAM[FONT_ADR], &FONT, sizeof(FONT));

   state->PREV_TIME = time_in_ms();

   return state;
}

void chip8_terminate(Chip8 **state) {
   if (*state)
      free(*state);
   state = NULL;
}

void chip8_load_app(Chip8 *state, void *data, u32 size) {
   memcpy(&state->RAM[state->PC = PROGRAM_START_ADR], data, size);
}

void chip8_tick(Chip8 *state, u8 key_pressed, u8 key_released) {
   // fetch
   const u16 instr = ((u16)state->RAM[state->PC] << 8) | ((u16)state->RAM[state->PC + 1]);
   state->PC += 2;

   const u16 op = instr & 0xF000;
   const u16 VX = (instr & 0x0F00) >> 8; // x-gpr index
   const u16 VY = (instr & 0x00F0) >> 4; // y-gpr index
   const u16 N = instr & 0x000F;
   const u16 NN = instr & 0x00FF;  // reused 3rd and 4th nibbles
   const u16 NNN = instr & 0x0FFF; // reused [2nd, 4th] nibbles

   state->SHOULD_DRAW = false;

   // decode
   switch (op) {
   case 0x0000: {
      switch (instr) {
      case 0x00E0:
         memset(state->DISPLAY, 0, sizeof(state->DISPLAY));
         d_printf(("Instruction (0x%04hX): Clear screen\n", instr));
         break;
      case 0x00EE: {
         u16 prev_adr = state->PC;
         state->PC = adr_pop(&state->STACK);
         d_printf(
             ("Instruction (0x%04hX): Returning from subroutine at 0x%04hX to 0x%04hX\n", instr, prev_adr, state->PC));
         break;
      }
      }

      break;
   }
   case 0x1000:
      state->PC = NNN;
      // d_printf(("Instruction (%x): Set PC to (%x)\n", instr, NNN);
      break;
   case 0x2000:
      adr_push(&state->STACK, state->PC); // save jump back adr
      state->PC = NNN;
      d_printf(("Instruction (0x%04hX): Calling subroutine at 0x%04hX\n", instr, state->PC));
      break;
   case 0x3000:
      if (state->GPR[VX] == NN)
         state->PC += 2;
      break;
   case 0x4000:
      if (state->GPR[VX] != NN)
         state->PC += 2;
      break;
   case 0x5000:
      if (state->GPR[VX] == state->GPR[VY])
         state->PC += 2;
      break;
   case 0x6000:
      state->GPR[VX] = NN;
      d_printf(("Instruction (0x%04hX): GPR[%d] = %d\n", instr, VX, NN));
      break;
   case 0x7000:
      state->GPR[VX] += NN;
      d_printf(("Instruction (0x%04hX): GPR[%d] += %d\n", instr, VX, NN));
      break;
   case 0x8000: {
      switch (N) {
      case 0:
         state->GPR[VX] = state->GPR[VY];
         d_printf(("Instruction (0x%04hX): GPR[%d] = GPR[%d]\n", instr, VX, VY));
         break;
      case 1:
         state->GPR[VX] |= state->GPR[VY];
#ifdef Q_VF_RESET
         state->GPR[0xF] = 0;
#endif
         d_printf(("Instruction (0x%04hX): GPR[%d] |= GPR[%d]\n", instr, VX, VY));
         break;
      case 2:
         state->GPR[VX] &= state->GPR[VY];
#ifdef Q_VF_RESET
         state->GPR[0xF] = 0;
#endif
         d_printf(("Instruction (0x%04hX): GPR[%d] &= GPR[%d]\n", instr, VX, VY));
         break;
      case 3:
         state->GPR[VX] ^= state->GPR[VY];
#ifdef Q_VF_RESET
         state->GPR[0xF] = 0;
#endif
         d_printf(("Instruction (0x%04hX): GPR[%d] ^= GPR[%d]\n", instr, VX, VY));
         break;
      case 4: {
         const u8 op_l = state->GPR[VX];
         const u8 op_r = state->GPR[VY];

         // must be performed first, before carry flag is set!
         // math will be off if vF is used as input
         state->GPR[VX] += state->GPR[VY];

         if (op_l > (UINT8_MAX - op_r))
            state->GPR[0xF] = 1;
         else
            state->GPR[0xF] = 0;

         break;
      }
      case 5: {
         const u8 op_l = state->GPR[VX];
         const u8 op_r = state->GPR[VY];

         state->GPR[VX] -= state->GPR[VY];

         // will underflow --> set to 0 if borrow occurs
         if (op_l < op_r)
            state->GPR[0xF] = 0;
         else
            state->GPR[0xF] = 1;
         /* Another way to think of above is:
          * VF = 1
          *
          * if (VX < VY) state->VF = 0     --> The subtraction borrows from VF and therefore sets it to 0!
          *
          */
         break;
      }
      case 6: {
#ifdef Q_SHIFTING
         const u8 old = state->GPR[VX];
         state->GPR[VX] = state->GPR[VX] >> 1;
#else
         const u8 old = state->GPR[VY];
         state->GPR[VX] = state->GPR[VY] >> 1;

#endif

         // VF = LSB of old value
         state->GPR[0xF] = old & 0x1;
         break;
      }
      case 7: {
         const u8 op_l = state->GPR[VY];
         const u8 op_r = state->GPR[VX];

         state->GPR[VX] = state->GPR[VY] - state->GPR[VX];

         if (op_l < op_r)
            state->GPR[0xF] = 0;
         else
            state->GPR[0xF] = 1;

         break;
      }
      case 0xE: {
#ifdef Q_SHIFTING
         const u8 old = state->GPR[VX];
         state->GPR[VX] = state->GPR[VX] << 1;
#else
         const u8 old = state->GPR[VY];
         state->GPR[VX] = state->GPR[VY] << 1;
#endif

         // VF = MSB of old value
         state->GPR[0xF] = (old & (0x1 << 7)) >> 7; // put back in place
         break;
      }
      }
      break;
   }
   case 0x9000:
      if (state->GPR[VX] != state->GPR[VY])
         state->PC += 2;
      break;
   case 0xA000:
      state->I = NNN;
      d_printf(("Instruction (0x%04hX): I = 0x%04hX\n", instr, NNN));
      break;
   case 0xB000:
#ifdef Q_JUMPING
      state->PC = NNN + state->GPR[(NNN & (0xF << 8)) >> 8];
#else
      state->PC = NNN + state->GPR[0];
#endif
      d_printf(("Instruction (0x%04hX): Jump to address 0x%04hX + 0x%04hX\n", instr, NNN, state->GPR[0]));
      break;
   case 0xC000:
      state->GPR[VX] = (rand() % 255) & NN;
      break;
   case 0xD000: {
      state->SHOULD_DRAW = true;
      const u16 base_x = state->GPR[VX] % DISPLAY_WIDTH;
      const u16 base_y = state->GPR[VY] % DISPLAY_HEIGHT;
      state->GPR[0xF] = 0;
      d_printf(("Instruction (0x%04hX): Drawing sprite with height %d (N) at (%d, %d) from GPR[%d] and GPR [%d]\n",
                instr, N, base_x, base_y, VX, VY));

      // For each row
      for (u32 offset_y = 0; offset_y < N; ++offset_y) {
         const u8 sprite_row = state->RAM[state->I + offset_y];
         u16 y = (base_y + offset_y);

// clip at borderj
#ifdef Q_CLIPPING
         if (y >= DISPLAY_HEIGHT)
            break;
#else
         y %= DISPLAY_HEIGHT;
#endif

         // For each column
         for (u8 offset_x = 0; offset_x < 8; ++offset_x) { // msb to lsb
            u16 x = (base_x + offset_x);

#ifdef Q_CLIPPING
            if (x >= DISPLAY_WIDTH)
               break;
#else
            x %= DISPLAY_WIDTH;
#endif

            const bool disp_pix_on = state->DISPLAY[y][x];
            const bool sprite_pix_on = sprite_row & (1 << (7 - offset_x));

            if (sprite_pix_on && disp_pix_on) {
               state->DISPLAY[y][x] = false;
               state->GPR[0xF] = 1;
            } else if (sprite_pix_on && !disp_pix_on) {
               state->DISPLAY[y][x] = true;
            }
         }
      }
      break;
   }
   case 0xE000: {
      switch (NN) {
      case 0x009E: {
         const bool extra_cond = key_pressed < 16 && KEY_MAPPING[key_pressed] == state->GPR[VX];
         state->KEYS[state->GPR[VX]] = extra_cond; // sync with direct input

         if (state->KEYS[state->GPR[VX]] || extra_cond) {
            state->PC += 2;
            state->KEYS[state->GPR[VX]] = false; // reset
         }
         break;
      }
      case 0x00A1: {
         const bool extra_cond = key_pressed < 16 && KEY_MAPPING[key_pressed] == state->GPR[VX];
         state->KEYS[state->GPR[VX]] = extra_cond; // sync with direct input

         if (!state->KEYS[state->GPR[VX]] && !extra_cond) {
            state->PC += 2;
            state->KEYS[state->GPR[VX]] = false; // reset
         }
         break;
      }
      }
      break;
   }
   case 0xF000: {
      switch (NN) {
      case 0x0007:
         state->GPR[VX] = state->DELAY_TIMER;
         d_printf(("Instruction (0x%04hX): GPR[%d] = %d (Delay Timer)\n", instr, VX, state->DELAY_TIMER));
         break;
      case 0x000A:
         if (key_released < 16) {
            state->GPR[VX] = KEY_MAPPING[key_released];
            state->KEYS[state->GPR[VX]] = true; // set key to true
            d_printf(("key pressed: 0x%04hX, set to %d\n", state->GPR[VX], KEYS[state->GPR[VX]]));
         } else {
            state->PC -= 2; // wait if no keypress
            d_printf(("waiting for keypress..\n"));
         }
         break;
      case 0x0015:
         state->DELAY_TIMER = state->GPR[VX];
         d_printf(("Instruction (0x%04hX): Delay Timer = GPR[%d] = %d\n", instr, VX, state->DELAY_TIMER));
         break;
      case 0x0018:
         state->SOUND_TIMER = state->GPR[VX];
         d_printf(("Instruction (0x%04hX): Sound Timer = GPR[%d] = %d\n", instr, VX, state->SOUND_TIMER));
         break;
      case 0x001E:
         state->I += state->GPR[VX];
         d_printf(("Instruction (0x%04hX): I += GPR[%d]\n", instr, VX));
         break;
      case 0x0029:
         state->I = FONT_ADR + state->GPR[VX] * FONT_STRIDE;
         break;
      case 0x0033: {
         u8 div = 100;
         u8 val = state->GPR[VX];
         for (s32 i = 0; i < 3; ++i) {
            state->RAM[state->I + i] = val / div;
            val -= (state->RAM[state->I + i]) * div;
            div /= 10;
         }
         assert(val == 0);
         break;
      }
      case 0x0055:
         for (size_t i = 0; i <= VX; ++i) // last included (through i+x)
            state->RAM[state->I + i] = state->GPR[i];
#ifdef Q_MEMORY
         state->I += VX + 1;
#endif

         d_printf(("Instruction (0x%04hX): Saving [V0, V%d] --> [RAM[0x%04hX], RAM[0x%04hX + %d]]\n", instr, VX,
                   state->I, state->I, VX));
         break;
      case 0x0065:
         for (size_t i = 0; i <= VX; ++i)
            state->GPR[i] = state->RAM[state->I + i];
#ifdef Q_MEMORY
         state->I += VX + 1;
#endif

         d_printf(("Instruction (0x%04hX): Saving [V0, V%d] <-- [RAM[0x%04hX], RAM[0x%04hX + %d]]\n", instr, VX,
                   state->I, state->I, VX));
         break;
      }
      break;
   }
   default:
      d_printf(("Instruction (0x%04hX): Unhandled!\n", instr));
      assert(false);
      break;
   }

   timer_tick(state);
}

bool chip8_should_draw(Chip8 *state) {
   return state->SHOULD_DRAW;
}

bool chip8_should_beep(Chip8 *state) {
   if (state->SOUND_TIMER != 0)
      return true;
   return false;
}

void timer_tick(Chip8 *state) {
   const u64 curr = time_in_ms();
   const u64 diff = curr - state->PREV_TIME;
   if (diff > ceilf(TIMER_THRESHOLD_MS)) {
      if (state->DELAY_TIMER >= 1)
         state->DELAY_TIMER -= 1;
      if (state->SOUND_TIMER >= 1)
         state->SOUND_TIMER -= 1;
      state->PREV_TIME = curr;
   }
}

bool chip8_sync_display() {
#ifdef Q_DISP_WAIT
   return true;
#else
   return false;
#endif
}
