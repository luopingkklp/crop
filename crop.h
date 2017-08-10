#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "bmp.h"

// structs
typedef struct {
	int top;
	int bottom;
	int left;
	int right;
} BOUNDS;

typedef struct {
	int blue;
	int green;
	int red;
} TRIPLESUM;

// functions
void readBMP(char *path);
void writeBMP(char *path, BOUNDS *b);
BOUNDS findBounds();
TRIPLESUM addTriple(TRIPLESUM *a, RGBTRIPLE *b);
TRIPLESUM findAvg(TRIPLESUM *t, int n);
void initTriple(TRIPLESUM *t);
bool isWhite(TRIPLESUM *t);