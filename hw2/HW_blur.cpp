#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_blur:
//
// Blur image I1 with a box filter (unweighted averaging).
// The filter has width filterW and height filterH.
// We force the kernel dimensions to be odd.
// Output is in I2.
//

// blur input image with a 1 dimensional box filter with width of ww
void blur1D(ChannelPtr<uchar> src, int len, int stride, int ww, ChannelPtr<uchar> dst); 

void
HW_blur(ImagePtr I1, int filterW, int filterH, ImagePtr I2)
{
	// PUT YOUR CODE HERE
	// copy image header (width, height) of input image I1 to output image I2
	IP_copyImageHeader(I1, I2);

	// init vars for width, height, and total number of pixels
	int w = I1->width ();
	int h = I1->height();
	int total = w * h;

	// make kernel size odd
	if((filterW % 2) == 0) filterW++; 
	if((filterW % 2) == 0) filterH++;

	// declarations for image channel pointers and datatype
	ChannelPtr<uchar> p1, p2;
	int type;

	// visit all image channels and evaluate output image
	for(int ch=0; IP_getChannel(I1, ch, p1, type); ch++) {	// get input  pointer for channel ch
		IP_getChannel(I2, ch, p2, type);		// get output pointer for channel ch
		
		// iterate over rows
		if(filterW > 1) {
			// blur each row one by one
			for(int y = 0; y<h; y++) {
				blur1D(p1, w, 1, filterW, p2);
				p1 = p1 + w; // move to next row in input image
				p2 = p2 + w; // move to next row in output image
			}

			// get new input
			IP_getChannel(I2, ch, p1, type);
		}
		else {
			// otherwise copy input image to output image
			IP_copyImage(I1, I2);
		}

		// iterate over columns
		if(filterH > 1) {
			IP_getChannel(I2, ch, p2, type);

			// blur each column one by one
			for (int x = 0; x < w; x++) {
				blur1D(p1, h, w, filterH, p2);
				p1 = p1 + 1;
				p2 = p2 + 1;
			}
		}		
	}
}

void blur1D(ChannelPtr<uchar> src, int len, int stride, int ww, ChannelPtr<uchar> dst)
{
	// create buffer to hold padded rows/cols
	uchar* buffer = new uchar[len + ww];

	// length of padding (ie num of pixels) to the right & left 
	int padding = ww / 2;
	uchar *p;

	// skip past padding
	p = buffer + padding;

	// copy input pixels to buffer
	for (int i = 0; i < len; i++) p[i] = src[i*stride];

	uchar *p1, *p2;
	p1 = buffer;
	p2 = buffer + len + padding;

	for (int i = 0; i < padding; i++)
	{
		// pad left side
		p1[i] = p[0];

		//  pad right side
		p2[i] = p[len - 1];
	}

	// calculate sum of pixel values in the neighborhood
	int sum = 0;
	for (int i = 0; i < ww - 1; i++)
		sum += buffer[i];

	int i = ww - 1;
	for (int x = 0; x < len; x++)
	{
		// add incoming pixel
		sum += buffer[i++];

		// calculate average
		dst[x*stride] = sum / ww;

		// subtract outgoing pixel
		sum -= buffer[i - ww];

	}

	// free memory
	delete[] buffer; 

}
