#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_histoStretch:
//
// Apply histogram stretching to I1. Output is in I2.
// Stretch intensity values between t1 and t2 to fill the range [0,255].
//
void
HW_histoStretch(ImagePtr I1, int t1, int t2, ImagePtr I2)
{

// PUT YOUR CODE HERE
// copy image header (width, height) of input image I1 to output image I2
	IP_copyImageHeader(I1, I2);

	// init vars for width, height, and total number of pixels
	int w = I1->width ();
	int h = I1->height();
	int total = w * h;

	// init lookup table
    // clip histogram to t1 and t2 range
	int i, clipLUT[MXGRAY], scaleLUT[MXGRAY];
	for(i=0; i<MXGRAY; ++i) {
        if(i<t1) clipLUT[i] = t1;
        else if(i>t2) clipLUT[i] = t2;
        else clipLUT[i] = i;
    }

	// apply histostretch formula
	// first make sure t1 and t2 are different to avoide dividing by 0
	// then shift intensity values by t1, normalize to 0-1 range
	// bring back to 0-255 range by multiplying by 255
    if (t1 == t2) t2++;
    for(i=0; i<MXGRAY; ++i) {
        scaleLUT[i] = (double) ((i-t1) * 255.0) / (t2 - t1);
    }


	// declarations for image channel pointers and datatype
	ChannelPtr<uchar> p1, p2;
	int type;

	// Note: IP_getChannel(I, ch, p1, type) gets pointer p1 of channel ch in image I.
	// The pixel datatype (e.g., uchar, short, ...) of that channel is returned in type.
	// It is ignored here since we assume that our input images consist exclusively of uchars.
	// IP_getChannel() returns 1 when channel ch exists, 0 otherwise.

	// visit all image channels and evaluate output image
	for(int ch=0; IP_getChannel(I1, ch, p1, type); ch++) {	// get input  pointer for channel ch
		IP_getChannel(I2, ch, p2, type);		// get output pointer for channel ch

		for(i=0; i<total; i++) p2[i] = clipLUT[p1[i]];	// use clipLUT[] to eval output, first we clip

		// use scaleLUT[] to eval output, apply histo stretch function
		// to output image after it was clipped
        for(i=0; i<total; i++) p2[i] = scaleLUT[p2[i]];
	}

}
