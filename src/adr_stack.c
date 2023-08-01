#include "adr_stack.h"

void adr_push(AdrStack *self, u16 adr) {
   if (self->head == MAX_ADR_STACK) {
      printf("Failed to push, full!\n");
      exit(0);
   }
   self->addresses[self->head++] = adr;
}

u16 adr_pop(AdrStack *self) {
   if (self->head <= 0) {
      printf("Failed to pop, no elements!\n");
      exit(0);
   }

   return self->addresses[--self->head];
}
