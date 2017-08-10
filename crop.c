
#include "crop.h"

// global variables
BITMAPFILEHEADER bf;
BITMAPINFOHEADER bi;
RGBTRIPLE **imgBuffer;

int main(int argc, char *argv[]) {

	// get file paths
	char *infile = "/home/phoebe/Documents/crop/01.bmp";
	char *outfile = "/home/phoebe/Documents/crop/new01.bmp";

	// read image
	readBMP(infile);

	// find new bounds
	BOUNDS bounds = findBounds();

	// test
	printf("top: %i, bot: %i, left: %i, right: %i\n", bounds.top, bounds.bottom, bounds.left, bounds.right);

	// write new image
	writeBMP(outfile, &bounds);

	// free memory
	free(imgBuffer);
}

// reads BMP to global buffers
void readBMP(char *path) {

	// open file
	FILE *ptr= fopen(path, "r");

	// store headers
	fread(&bf, sizeof(BITMAPFILEHEADER), 1, ptr);
	fread(&bi, sizeof(BITMAPINFOHEADER), 1, ptr);

	// find dimensions
	int width = bi.biWidth;
	int height = bi.biHeight;
	int padding = (4 - width * sizeof(RGBTRIPLE) % 4) % 4;

	// allocate buffer memory
	imgBuffer = (RGBTRIPLE **)malloc(height * sizeof(RGBTRIPLE *));

	// store image in buffer
	for (int i = 0; i < height; i++) {

		imgBuffer[i] = (RGBTRIPLE *)malloc(width * sizeof(RGBTRIPLE));

		for (int j = 0; j < width; j++) {
			// store triple in buffer
			fread(&imgBuffer[i][j], sizeof(RGBTRIPLE), 1, ptr);
		}
		// skip padding
		fseek(ptr, padding, SEEK_CUR);
	}

	// close file
	fclose(ptr);
}

void writeBMP(char *path, BOUNDS *b) {

	// open file
	FILE *ptr = fopen(path, "w");

	// header dimensions blah whatever figure it out
	bi.biWidth = b->right - b->left;
	bi.biHeight = b->bottom - b->top;
	int padding = (4 - bi.biWidth * sizeof(RGBTRIPLE) % 4) % 4;

	bi.biSizeImage = (bi.biWidth * sizeof(RGBTRIPLE) + padding) * bi.biHeight;
	bf.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + bi.biSizeImage;

	// write headers
	fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, ptr);
	fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, ptr);

	// write cropped image from buffer
	for (int i = b->top; i < b->bottom; i++) {
		for (int j = b->left; j < b->right; j++ ) {
			fwrite(&imgBuffer[i][j], sizeof(RGBTRIPLE), 1, ptr);
		}
		// write padding
		for (int p = 0; p < padding; p++) {
			fputc(0x00, ptr);
		}
	}

	// close file
	fclose(ptr);
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

	for (int i = 0; i < width; i++) {
		initTriple(&colSum[i]);
	}

	// boundaries object
	BOUNDS bounds;

	// read buffer
	for (int i = 0; i < height; i++) {

		// reset row sum
		initTriple(&rowSum);

		for (int j = 0; j < width; j++) {
			// add triple to respective row and column sums
			rowSum = addTriple(&rowSum, &imgBuffer[i][j]);
			colSum[j] = addTriple(&colSum[j], &imgBuffer[i][j]);
		}

		// find average RGB of current row
		TRIPLESUM rowAvg = findAvg(&rowSum, width);

		//printf("%i, blue: %i, red: %i, green: %i\n", i, rowAvg.blue, rowAvg.red, rowAvg.green);

		// set top bound at first occurrence of colored pixels
		if (!isWhite(&rowAvg) && !foundImg) {
			bounds.top = i;
			foundImg = true;
		}
		// set bottom bound at last occurrence of colored pixels
		if (!isWhite(&rowAvg) && foundImg) {
			bounds.bottom = i;
		}
	}

	// reset to find left and right bounds
	foundImg = false;

	// iterate through column sums
	for (int i = 0; i < width; i++) {

		// find average RGB for each column
		TRIPLESUM colAvg = findAvg(&colSum[i], height);
		//printf("%i, blue: %i, red: %i, green: %i\n", i, colAvg.blue, colAvg.red, colAvg.green);
		// set left bound at first occurrence of colored pixels
		if (!isWhite(&colAvg) && !foundImg) {
			bounds.left = i;
			foundImg = true;
		}

		// set right bound at last occurrence of colored pixels
		if (!isWhite(&colAvg) && foundImg) {
			bounds.right = i;
		}
	}

	free(colSum);
	return bounds;
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