#ifndef _ADR_STACK
#define _ADR_STACK
#include "types.h"

#define MAX_ADR_STACK 32

typedef struct AdrStack {
   u16 addresses[MAX_ADR_STACK];
   s16 head;
} AdrStack;

void adr_push(AdrStack *self, u16 adr);
u16 adr_pop(AdrStack *self);

#endif
