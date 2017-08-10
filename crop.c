
#include "crop.h"

// global variables
BITMAPFILEHEADER bf;
BITMAPINFOHEADER bi;
RGBTRIPLE **imgBuffer;

int main(int argc, char *argv[]) {

	if (argc != 3) {
		printf("./crop [SOURCE] [DEST]\n");
		exit(0);
	}

	char *infile = argv[1];
	char *outfile = argv[2];

	// read image
	readBMP(infile);

	// find bounds for cropped image
	BOUNDS bounds = findBounds();

	// write new image
	writeBMP(outfile, &bounds);

	// free memory
	free(imgBuffer);
}

// returns RGB sum of two triples
TRIPLESUM addTriple(TRIPLESUM *a, RGBTRIPLE *b) {
	TRIPLESUM sum;
	sum.blue = a->blue + b->rgbtBlue;
	sum.green = a->green + b->rgbtGreen;
	sum.red = a->red + b->rgbtRed;
	return sum;
}

// returns average RGB of a triple
TRIPLESUM findAvg(TRIPLESUM *t, int n) {
	TRIPLESUM avg;
	avg.blue = (t->blue + n - 1) / n;
	avg.green = (t->green + n - 1) / n;
	avg.red = (t->red + n - 1) / n;
	return avg;
}

// returns boundaries of cropped image
BOUNDS findBounds() {

	// helper variables
	int width = bi.biWidth;
	int height = bi.biHeight;
	bool foundImg = false;

	// variables to store total RGB values of rows and columns
	TRIPLESUM rowSum;
	TRIPLESUM *colSum = malloc(width * sizeof(TRIPLESUM));

	// allocate memory for colSum array
	for (int i = 0; i < width; i++) {
		initTriple(&colSum[i]);
	}

	// boundaries object
	BOUNDS bounds;

	// read buffer
	for (int i = 0; i < height; i++) {

		// reset row sum
		initTriple(&rowSum);

		// add each triple to row and column sums
		for (int j = 0; j < width; j++) {
			rowSum = addTriple(&rowSum, &imgBuffer[i][j]);
			colSum[j] = addTriple(&colSum[j], &imgBuffer[i][j]);
		}

		// find average RGB of current row
		TRIPLESUM rowAvg = findAvg(&rowSum, width);

		// set top/bottom bounds at first/last occurences of colored pixels
		if (!isWhite(&rowAvg)) {
			if (!foundImg) {
				bounds.top = i;
				foundImg = true;
			} else {
				bounds.bottom = i + 1;
			}
		}
	}

	// reset to find left/right bounds
	foundImg = false;

	// iterate through column sums
	for (int i = 0; i < width; i++) {

		// find average RGB for each column
		TRIPLESUM colAvg = findAvg(&colSum[i], height);

		// set left/right bounds at first/last occurences of colored pixels
		if (!isWhite(&colAvg)) {
			if (!foundImg) {
				bounds.left = i;
				foundImg = true;
			} else {
				bounds.right = i + 1;
			}
		}
	}

	free(colSum);
	return bounds;
}

// initializes RBG values to zero
void initTriple(TRIPLESUM *t) {
	t->blue = 0;
	t->green = 0;
	t->red = 0;
}

// returns true if white, false otherwise
bool isWhite(TRIPLESUM *t) {
	if (t->blue == 255 && t->green == 255 && t->red == 255) {
		return true;
	}
	return false;
}

// reads BMP to global buffers
void readBMP(char *path) {

	// open file
	FILE *ptr= fopen(path, "r");

	// store headers
	fread(&bf, BFSIZE, 1, ptr);
	fread(&bi, BISIZE, 1, ptr);

	// find dimensions
	int width = bi.biWidth;
	int height = bi.biHeight;
	int padding = (4 - width * TRIPLESIZE % 4) % 4;

	// allocate buffer memory for rows
	imgBuffer = (RGBTRIPLE **)malloc(height * sizeof(RGBTRIPLE *));

	// store image in buffer
	for (int i = 0; i < height; i++) {

		// allocate buffer memory for triple
		imgBuffer[i] = (RGBTRIPLE *)malloc(width * TRIPLESIZE);

		// store triple in buffer
		for (int j = 0; j < width; j++) {
			fread(&imgBuffer[i][j], TRIPLESIZE, 1, ptr);
		}

		// skip padding
		fseek(ptr, padding, SEEK_CUR);
	}

	// close file
	fclose(ptr);
}

// writes cropped image to BMP
void writeBMP(char *path, BOUNDS *b) {

	// open file
	FILE *ptr = fopen(path, "w");

	// calculate new dimensions
	int width = b->right - b->left;
	int height = b->bottom - b->top;
	int padding = (4 - width * TRIPLESIZE % 4) % 4;

	// update headers
	bi.biWidth = width;
	bi.biHeight = height;
	bi.biSizeImage = (width * TRIPLESIZE % 4) % 4;
	bf.bfSize = BFSIZE + BISIZE + bi.biSizeImage;

	// write headers
	fwrite(&bf, BFSIZE, 1, ptr);
	fwrite(&bi, BISIZE, 1, ptr);

	// write cropped image from buffer
	for (int i = b->top; i < b->bottom; i++) {

		// write row
		for (int j = b->left; j < b->right; j++ ) {
			fwrite(&imgBuffer[i][j], TRIPLESIZE, 1, ptr);
		}

		// write padding
		for (int p = 0; p < padding; p++) {
			fputc(0x00, ptr);
		}
	}

	// close file
	fclose(ptr);
}