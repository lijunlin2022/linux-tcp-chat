#include "customfile.h"

FILE *Fopen(const char *filename, const char *mode) {
  FILE *fp = fopen(filename, mode);
  return fp;
}

int Fclose(FILE *fp) {
  int ret = fclose(fp);
  if (ret == EOF) {
    perr_exit("fclose error\n");
  }
  return ret;
}
