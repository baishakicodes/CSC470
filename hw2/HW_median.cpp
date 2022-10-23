#include <algorithm>
#include <vector>
#include "IP.h"
using namespace IP;
using std::vector;
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_median:
//
// Apply median filter of size sz x sz to I1.
// Clamp sz to 9.
// Output is p1 I2.
//

// find median in storage vector
int findMedian(vector<int> storage);

void
HW_median(ImagePtr I1, int sz, ImagePtr I2)
{

    // PUT YOUR CODE HERE
    // copy image header (width, height) of input image I4 to output image I2
	IP_copyImageHeader(I1, I2);

	// init vars for width, height, and total number of pixels
	int w = I1->width();
	int h = I1->height();
	int total = w * h;

	// declarations for image channel pointers and datatype
	ChannelPtr<uchar> p1, p2, p;
	int type;

    // enforce sz to be odd
	if (sz % 2 == 0) sz++;

    // calculate padding & max padding
	int padding = sz / 2;
	int paddingMax = (2 * padding) + w;

	// create buffers for padding
	short** buffer = new short*[sz];
	for (int i = 0; i < sz; i++) {
		buffer[i] = new short[paddingMax];
	}

	// visit all image channels and evaluate output image
	for (int ch = 0; IP_getChannel(I1, ch, p1, type); ch++) {	// get input  pointer for channel ch
		IP_getChannel(I2, ch, p2, type);		// get output pointer for channel ch

		// initialize starting from top
        // start padding rows
		for (int i = 0; i < padding; i++) {
			p = p1;
			for (int j = 0; j < padding; j++) buffer[i][j] = *p;
			for (int j = padding; j < padding + w; j++) buffer[i][j] = *p++; 
			for (int j = padding + w; j < paddingMax; j++) buffer[i][j] = p[-1];
		}

        // padding of columns
		for (int i = padding; i < sz; i++) {
			for (int j = 0; j < padding; j++) buffer[i][j] = *p1;
			for (int j = padding; j < padding + w; j++) buffer[i][j] = *p1++;
			for (int j = padding + w; j < paddingMax; j++) buffer[i][j] = p1[-1];
		}

        // storage vector used to find median
		vector<int> storage;
		for (int row = 0; row < h; row++) {
			for (int i = 0; i < sz; i++) {
				for (int j = 0; j < sz; j++) {
					storage.push_back(buffer[j][i]);
				}
			}
			for (int column = 0; column < w; column++) {
                // store median into output image
				*p2++ = findMedian(storage);
				if (column < w - 1) {
					for (int i = 0; i < sz; i++) {
						storage.push_back(buffer[i][column + sz]);
					}
					storage.erase(storage.begin(), storage.begin() + sz);
				}
			}
            // clear vector before starting on next row
			storage.clear();
			if (row < h - padding) {
				for (int j = 0; j < padding; j++) buffer[(row + sz - 1) % sz][j] = *p1;
				for (int j = padding; j < padding + w; j++) buffer[(row + sz - 1) % sz][j] = *p1++;
				for (int j = padding + w; j < paddingMax; j++) buffer[(row + sz - 1) % sz][j] = p1[-1];
			}
            // bottom padding; last few rows
			else {
				IP_getChannel(I1, ch, p, type);
				p += (h - 1) * w;
				for (int j = 0; j < padding; j++) buffer[(row + sz - 1) % sz][j] = *p;
				for (int j = padding; j < padding + w; j++) buffer[(row + sz - 1) % sz][j] = *p++;
				for (int j = padding + w; j < paddingMax; j++) buffer[(row + sz - 1) % sz][j] = p[-1];
			}
		}
	}

    // delete buffers w/in buffer
	for (int i = 0; i < sz; i++) {
		delete[] buffer[i];
	}

    // delete buffer itself
	delete[] buffer;
}

int findMedian(vector<int> storage)
{
    // first sort storage, median will be in the center index
	sort(storage.begin(), storage.end());
	return storage[storage.size() / 2];
}
