#ifndef FILTERS_H
#define FILTERS_H

#include "pgm.h"

void smooth(int** matr,pgm*, int n, int m);
void blur(int** matr, pgm*,  int n, int m);
void sharpen(int** matr, pgm*, int n, int m);
void meanRemoval(int** matr, pgm*, int n, int m);

#endif