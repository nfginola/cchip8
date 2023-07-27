#include "types.h"

u32 viz_bits(u32 n) {
   printf("==== Bit Viz ====\n");

   int len = sizeof(u32) * 8;
   char viz[MAX_BIT_VIZ];
   memset(viz, 0, sizeof(char) * MAX_BIT_VIZ);

   int bits[MAX_BIT_VIZ];
   memset(bits, BIT_OFF, sizeof(int) * MAX_BIT_VIZ);

   int slot = 1;
   for (int i = 0; i < len; ++i) {
      // mask
      int on = (n & slot) == slot ? BIT_ON : BIT_OFF;
      bits[i] = on;

      slot *= 2;
   }

   printf("(%6d):\t", n);
   for (int i = MAX_BIT_VIZ - 1; i >= 0; --i) {
      printf("%d", bits[i]);

      if (i % 4 == 0)
         printf(" ");
   }

   printf("\n");
   return n;
}
