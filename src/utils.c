#include "utils.h"

void *read_bin_file(char *fname, u32 *out_size) {
   void *bin = NULL;
   FILE *file;
   if ((file = fopen(fname, "rb"))) {
      // get size
      fseek(file, 0L, SEEK_END);
      *out_size = ftell(file);
      rewind(file);

      // dump app to memory
      bin = malloc(*out_size);
      fread(bin, *out_size, 1, file);

      fclose(file);
   }
   return bin;
}

u64 time_in_ms() {
   struct timeval tv;
   gettimeofday(&tv, NULL);
   return (((long long)tv.tv_sec) * 1000) + (tv.tv_usec / 1000);
}
