#include "viz_internals.h"
#include "chip8.h"

/*
 * PC:   [...x..............................]: 0x... --> Divide RAM into hardcoded sizes and move X!
 * I:    [..............x...................]: 0x...
 *
 * STACK [top, top-1, top-2, top-3..]
 *
 * DELAY_TIMER: num
 *
 * SOUND_TIMER: num
 *
 * GPR[n] = a
 *
 */

#define VIZ_MEM_CHUNK_SIZE 32
#define VIZ_MEM_MAX_UNITS (RAM_SIZE / VIZ_MEM_CHUNK_SIZE)

#define VIZ_SPACES "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"

static void show_memory_position(u16 adr, char *caption);

void chip8_viz(Chip8 *state) {
   // Mild clear-screen
   printf(VIZ_SPACES VIZ_SPACES VIZ_SPACES VIZ_SPACES);

   // timers
   printf("Delay Timer: %d\n", state->DELAY_TIMER);
   printf("Sound Timer: %d\n", state->SOUND_TIMER);

   // call stack
   printf("\n");
   printf("Call Stack: ");
   printf("[");
   for (s32 i = state->STACK.head; i > 0; --i) {
      if (i == 1)
         printf("0x%04hX", state->STACK.addresses[i - 1]);
      else
         printf("0x%04hX, ", state->STACK.addresses[i - 1]);
   }
   printf("]");

   // registers
   printf("\n");
   for (s32 i = 0; i < NUM_GPRS; ++i) {
      printf("V[%d] = %d\n", i, state->GPR[i]);
   }

   // memory positions
   printf("\n");
   show_memory_position(state->PC, "PC");
   show_memory_position(state->I, "I");
}

static void show_memory_position(u16 adr, char *caption) {
   s32 chunk = adr / VIZ_MEM_CHUNK_SIZE;
   printf("%3s === [%-4d / %4d] ==== [0x%04hX / 0x%04hX] ", caption, adr, RAM_SIZE, adr, RAM_SIZE);
   printf("[");
   for (s32 i = 0; i < VIZ_MEM_MAX_UNITS; ++i) {
      if (i == chunk)
         printf("x");
      else
         printf(".");
   }
   printf("]");
   printf(" [%-3d / %3d]", chunk, VIZ_MEM_MAX_UNITS);
   printf("\n");
}
