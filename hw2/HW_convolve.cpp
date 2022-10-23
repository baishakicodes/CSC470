#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_convolve:
//
// Convolve I1 with filter kernel in Ikernel.
// Output is in I2.
//
void
HW_convolve(ImagePtr I1, ImagePtr Ikernel, ImagePtr I2)
{

    // PUT YOUR CODE HERE
    // copy image header (width, height) of input image I1 to output image I2
	IP_copyImageHeader(I1, I2);

	// init vars for width, height, and total number of pixels
	int w = I1->width ();
	int h = I1->height();
	int total = w * h;

	// declarations for image channel pointers and datatype
	ChannelPtr<uchar> p1, p2, p;
    ChannelPtr<float> kernel;
	int type;

    // creating buffer dimensions
	int kernelW = Ikernel->width();
	int kernelH = Ikernel->height();
	int paddingW = kernelW / 2;
	int paddingH = kernelH / 2;
	int maxH = paddingH + h + paddingH;
	int maxW = paddingW + w + paddingW;

    // create buffer
    unsigned char **buffer = new unsigned char*[maxH];
    for(int i = 0; i < maxH; i++) buffer[i] = new unsigned char[maxW];

	// visit all image channels and evaluate output image
	for(int ch=0; IP_getChannel(I1, ch, p1, type); ch++) {	// get input  pointer for channel ch
		IP_getChannel(I2, ch, p2, type);		// get output pointer for channel ch

        // iterate through buffer
        // initially p points to start of input image
        p = p1;

        for(int i = 0; i < maxH; i++) {
			for(int j = 0; j < maxW; j++) {

                // initialize all pixels in the padded area with 0s
				if(i < paddingH || i >= paddingH + h || j < paddingW || j >= paddingW + w) {
					buffer[i][j] = 0;
				}
                // copy pixels from input into buffer
				else {
					buffer[i][j] = *p++;
				}
			}
		}

        for(int i = paddingH; i < h + paddingH; i++)  {
			for(int j = paddingW; j < w + paddingW; j++) {
				double sum = 0;
				IP_getChannel(Ikernel, 0, kernel, type);

				// rows of the kernel
				for(int kr = 0; kr < kernelH; kr++) {
					// columns of the kernel
					for(int kc = 0; kc < kernelW; kc++) {
						sum += buffer[i - paddingH + kr][j - paddingW + kc] * (*kernel++);
					}
				}
				*p2++ = CLIP(sum, 0, 255);
			}
		}
	}

}
