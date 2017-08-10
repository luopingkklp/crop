#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "bmp.h"

#define TRIPLESIZE sizeof(RGBTRIPLE)
#define BFSIZE sizeof(BITMAPFILEHEADER)
#define BISIZE sizeof(BITMAPINFOHEADER)

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
TRIPLESUM addTriple(TRIPLESUM *a, RGBTRIPLE *b);
TRIPLESUM findAvg(TRIPLESUM *t, int n);
BOUNDS findBounds();
void initTriple(TRIPLESUM *t);
bool isWhite(TRIPLESUM *t);
void readBMP(char *path);
void writeBMP(char *path, BOUNDS *b);