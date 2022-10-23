#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_sharpen:
//
// Sharpen image I1. Output is p1 I2.
//

//from hw 2
void HW_blur(ImagePtr I1, int filterW, int filterH, ImagePtr I2);

void
HW_sharpen(ImagePtr I1, int size, double factor, ImagePtr I2)
{
    // PUT YOUR CODE HERE
    // copy image header (width, height) of input image I1 to output image I2
	IP_copyImageHeader(I1, I2);

	// init vars for width, height, and total number of pixels
	int w = I1->width();
	int h = I1->height();
	int total = w * h;

    ImagePtr Iblur;

    // enforce size to be odd
	if (size % 2 == 0) size++;

    // apply p to input image and save to Iblur
	HW_blur(I1, size, size, Iblur);

    // declarations for image channel pointers and datatype
	ChannelPtr<uchar> p1, p2, p;
	int type;

	// visit all image channels and evaluate output image
	for (int ch = 0; IP_getChannel(I1, ch, p1, type); ch++) 
	{	
		IP_getChannel(I2, ch, p2, type);		// get output pointer for channel ch
		IP_getChannel(Iblur, ch, p, type);      // get p pointer for blurred input image

		for (int i = 0; i < total; i++, p1++, p++) 
		{
            // sharpen equation
			// (Current - Blur) * Factor + (Current), clip between 0-255
			*p2++ = CLIP((*p1 - *p) * factor + *p1, 0, 255);
		}
	}

}
