#ifndef _CUSTOMFILE_H_
#define _CUSTOMFILE_H_

#include "customsocket.h"

FILE *Fopen(const char *filename, const char *mode);
int Fclose(FILE *fp);
int Fputs(const char *s, FILE *fp);
char *Fgets(char *buf, int n, FILE *fp);

#endif
