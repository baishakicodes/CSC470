#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_quantize:
//
// Quantize I1 to specified number of levels. Apply dither if flag is set.
// Output is in I2.
//
void
HW_quantize(ImagePtr I1, int levels, bool dither, ImagePtr I2)
{

// PUT YOUR CODE HERE
// copy image header (width, height) of input image I1 to output image I2
	IP_copyImageHeader(I1, I2);

	// init vars for width, height, and total number of pixels
	int w = I1->width ();
	int h = I1->height();
	int total = w * h;

	// init lookup table
	int i, lut[MXGRAY];
    double scale = MXGRAY / levels;
	for(i=0; i<MXGRAY; ++i) {
        lut[i] = scale * (int) (i / scale);
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

        if (dither) {
            // add noise to the input image
            // random number should be in the range [m, m] where m is MXGRAY / levels
            int m = (int) MXGRAY / levels;
            int randNum, val;

            for(i=0; i<total; i++) {
                randNum = (rand() % (2*m + 1)) - m;
                val = p1[i] + randNum;

                // make sure new value isn't outside of 0-255 range
                if (val > 255) val = 255;
                else if (val < 0) val = 0;

                p2[i] = lut[val];	// use lut[] to eval output
            }

        }
        else {
            for(i=0; i<total; i++) *p2++ = lut[*p1++];	// use lut[] to eval output
        }
	}

}
