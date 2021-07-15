#include "customfile.h"

FILE *Fopen(const char *filename, const char *mode) {
  FILE *fp = fopen(filename, mode);
  if (fp == NULL) {
    perr_exit("fopen error\n");
  }
  return fp;
}

int Fclose(FILE *fp) {
  int ret = fclose(fp);
  if (ret == EOF) {
    perr_exit("fclose error\n");
  }
  return ret;
}

int Fputs(const char *s, FILE *fp) {
  int ret = fputs(s, fp);
  if (ret == EOF) {
    perr_exit("fputs error\n");
  }
  return ret;
}

char *Fgets(char *buf, int n, FILE *fp) {
  return fgets(buf, n, fp);
}
