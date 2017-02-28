#ifndef PGM_H
#define PGM_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_SIZE 10000

struct pgm {
	int w;
	int h;
	int maxval;
	int **image;
	int **filteredImg;
	char *filter;
	char *special;
	char *comments;
	int sizeComments;
};

typedef struct pgm pgm;

pgm* getpgmFile (char*);

#endif