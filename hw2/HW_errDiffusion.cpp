#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_errDiffusion:
//
// Apply error diffusion algorithm to image I1.
//
// This procedure produces a black-and-white dithered version of I1.
// Each pixel is visited and if it + any error that has been diffused to it
// is greater than the threshold, the output pixel is white, otherwise it is black.
// The difference between this new value of the pixel from what it used to be
// (somewhere p1 between black and white) is diffused to the surrounding pixel
// intensities using different weighting systems.
//
// Use Floyd-Steinberg     weights if method=0.
// Use Jarvis-Judice-Ninke weights if method=1.
//
// Use raster scan (left-to-right) if serpentine=0.
// Use serpentine order (alternating left-to-right and right-to-left) if serpentine=1.
// Serpentine scan prevents errors from always being diffused p1 the same direction.
//
// A circular buffer is used to pad the edges of the image.
// Since a pixel + its error can exceed the 255 limit of uchar, shorts are used.
//
// Apply gamma correction to I1 prior to error diffusion.
// Output is saved p1 I2.
//

void copyRowToBufferFloyd(int row, short* buf[], ChannelPtr<uchar> start, int w, int h);
void copyRowToBufferJarvis(int row, short* buf[], ChannelPtr<uchar> start, int w, int h);

// from hw 1
void HW_gammaCorrect(ImagePtr I1, double gamma, ImagePtr I2);


void
HW_errDiffusion(ImagePtr I1, int method, bool serpentine, double gamma, ImagePtr I2)
{

    // PUT YOUR CODE HERE
    // appy gamma correction to I1
    ImagePtr Ig;
    HW_gammaCorrect(I1, gamma, Ig);

    // copy image header (width, height) of input image Ig to output image I2
	IP_copyImageHeader(Ig, I2);

	// init vars for width, height, and total number of pixels
	int w = I1->width ();
	int h = I1->height();
	int total = w * h;

	// init lookup table
	int i, lut[MXGRAY];
    int thr = MXGRAY / 2;
	for(i=0; i<thr && i<MXGRAY; ++i) lut[i] = 0;
	for(   ;          i<MXGRAY; ++i) lut[i] = MaxGray;

	// declarations for image channel pointers and datatype
	ChannelPtr<uchar> p1, p2, p;
	int type;

	// Floyd-Steinberg Method
	if (method == 0) {
		// buffers used for padding
		short *buf[2] = {}; //array of pointers
		buf[0] = new short[w + 2];
		buf[1] = new short[w + 2];

		// Initializing buffers to 0
		for (int i = 0; i < w + 2; i++) {
			buf[0][i] = 0;
			buf[1][i] = 0;
		}

		short *in1, *in2;

		// Raster-order scan
		if (!serpentine) {
			// visit all image channels of input and evaluate output image
			for (int ch = 0; IP_getChannel(Ig, ch, p1, type); ch++) {    // get input  pointer for channel ch
				IP_getChannel(I2, ch, p2, type);        // get output pointer for channel ch
				p = p1; //making p point at start of input image
				copyRowToBufferFloyd(0, buf, p, w, h);
				for (int y = 0; y < h; y++) {
					copyRowToBufferFloyd(y + 1, buf, p, w, h);
					in1 = buf[y % 2] + 1; //current row
					in2 = buf[(y + 1) % 2] + 1; //row below
					//Go through columns for row y
					for (int x = 0; x < w; x++) {
						int index = CLIP(*in1, 0, 255);
						*p2 = lut[index];
						int e = *in1 - *p2; //error to spread to neighboors
						in1[1] += (e * 7/16.0); 
						in2[-1] += (e * 3/16.0); 
						in2[0] += (e * 5/16.0);
						in2[1] += (e * 1/16.0);
						p2++;
						in1++;
						in2++;
					}

				}
			}
		}
        // serpentine Scan:
		else {
			// visit all image channels of input and evaluate output image
			for (int ch = 0; IP_getChannel(Ig, ch, p1, type); ch++) {    // get input  pointer for channel ch
				IP_getChannel(I2, ch, p2, type);        // get output pointer for channel ch
				p = p1; //making p point at start of input image
				copyRowToBufferFloyd(0, buf, p, w, h);
				for (int y = 0; y < h; y++) {
					copyRowToBufferFloyd(y + 1, buf, p, w, h);
					//Even row
					if (y % 2 == 0) {
						in1 = buf[y % 2] + 1; 
						in2 = buf[(y + 1) % 2] + 1;
						//Go through columns for row y
						for (int x = 0; x < w; x++) {
							int index = CLIP(*in1, 0, 255);
							*p2 = lut[index];
							int e = *in1 - *p2; 
							in1[1] += (e * 7/16.0);
							in2[-1] += (e * 3/16.0); 
							in2[0] += (e * 5/16.0);
							in2[1] += (e * 1/16.0); 
							p2++;
							in1++;
							in2++;
						}
					} 
                    //Odd row
					else {
						in1 = buf[y % 2] + w; 
						in2 = buf[(y + 1) % 2] + w;
						p2 += w - 1; //Making output pointer go to the end of the row
						//Go through columns for row y
						for (int x = 0; x < w; x++) {
							int index = CLIP(*in1, 0, 255);
							*p2 = lut[index];
							int e = *in1 - *p2;
							in1[-1] += (e * 7/16.0);
							in2[-1] += (e * 1/16.0); 
							in2[0] += (e * 5/16.0);
							in2[1] += (e * 3/16.0); 
							p2--;
							in1--;
							in2--;
						}
                        //Making p2 point to the next ro
						p2 += w + 1;
					}
				}
			}
		}
	}
    //Jarvis-Judice-Ninke Method
	else {

		// buffers for padding
		short *buf[3] = {}; //array of pointers
		buf[0] = new short[w + 4];
		buf[1] = new short[w + 4];
		buf[2] = new short[w + 4];
		short *in1, *in2, *in3;

		//Initializing buffers to 0
		for (int i = 0; i < w + 2; i++) {
			buf[0][i] = 0;
			buf[1][i] = 0;
			buf[2][i] = 0;
		}

		//raster scan:
		if (!serpentine) {
			// visit all image channels of input and evaluate output image
			for (int ch = 0; IP_getChannel(Ig, ch, p1, type); ch++) {    // get input  pointer for channel ch
				IP_getChannel(I2, ch, p2, type);        // get output pointer for channel ch
				p = p1; //making p point at start of input image
				copyRowToBufferJarvis(0, buf, p, w, h);
				copyRowToBufferJarvis(1, buf, p, w, h);
				for (int y = 0; y < h; y++) {
					copyRowToBufferJarvis(y + 2, buf, p, w, h);
					in1 = buf[y % 3] + 2;
					in2 = buf[(y + 1) % 3] + 2;
					in3 = buf[(y + 2) % 3] + 2;
					//Go through columns for row y
					for (int x = 0; x < w; x++) {
						int index = CLIP(*in1, 0, 255);
						*p2 = lut[index];
						int e = *in1 - *p2; //error to spread to neighboors
						in1[1] += (e * 7/48.0);
						in1[2] += (e * 5/48.0);

						in2[-2] += (e * 3/48.0);
						in2[-1] += (e * 5/48.0);
						in2[0] += (e * 7/48.0);
						in2[1] += (e * 5/48.0);
						in2[2] += (e * 3/48.0);

						in3[-2] += (e * 1/48.0); 
						in3[-1] += (e * 3/48.0);
						in3[0] += (e * 5/48.0);
						in3[1] += (e * 3/48.0);
						in3[2] += (e * 1/48.0); 

						p2++;
						in1++;
						in2++;
						in3++;
					}

				}
			}
		}
        //Serpentine scan
		else {
			// visit all image channels of input and evaluate output image
			for (int ch = 0; IP_getChannel(Ig, ch, p1, type); ch++) {    // get input  pointer for channel ch
				IP_getChannel(I2, ch, p2, type);        // get output pointer for channel ch
				p = p1; //making p point at start of input image
				copyRowToBufferJarvis(0, buf, p, w, h);
				copyRowToBufferJarvis(1, buf, p, w, h);
				for (int y = 0; y < h; y++) {
					copyRowToBufferJarvis(y + 2, buf, p, w, h);
					//Even row
					if (y % 2 == 0) {
						in1 = buf[y % 3] + 2;
						in2 = buf[(y + 1) % 3] + 2;
						in3 = buf[(y + 2) % 3] + 2; 
						//Go through columns for row y
						for (int x = 0; x < w; x++) {
							int index = CLIP(*in1, 0, 255);
							*p2 = lut[index];
							int e = *in1 - *p2; //error to spread to neighboors
							in1[1] += (e * 7/48.0);
							in1[2] += (e * 5/48.0);

							in2[-2] += (e * 3/48.0);
							in2[-1] += (e * 5/48.0);
							in2[0] += (e * 7/48.0);
							in2[1] += (e * 5/48.0);
							in2[2] += (e * 3/48.0);

							in3[-2] += (e * 1/48.0);
							in3[-1] += (e * 3/48.0);
							in3[0] += (e * 5/48.0);
							in3[1] += (e * 3/48.0);
							in3[2] += (e * 1/48.0);

							p2++;
							in1++;
							in2++;
							in3++;
						}
					} 
                    //Odd row
					else {
						in1 = buf[y % 3] + w + 1;
						in2 = buf[(y + 1) % 3] + w + 1;
						in3 = buf[(y + 2) % 3] + w + 1;
						p2 += w - 1;
						//Go through columns for row y
						for (int x = 0; x < w; x++) {
							int index = CLIP(*in1, 0, 255);
							*p2 = lut[index];
							int e = *in1 - *p2; //error to spread to neighboors

							in1[-1] += (e * 7/48.0);
							in1[-2] += (e * 5/48.0);

							in2[-2] += (e * 3/48.0);
							in2[-1] += (e * 5/48.0);
							in2[0] += (e * 7/48.0);
							in2[1] += (e * 5/48.0);
							in2[2] += (e * 3/48.0);

							in3[-2] += (e * 1/48.0); 
							in3[-1] += (e * 3/48.0);
							in3[0] += (e * 5/48.0);
							in3[1] += (e * 3/48.0);
							in3[2] += (e * 1/48.0); 

							p2--;
							in1--;
							in2--;
							in3--;
						}
						p2 += w + 1;
                    }
				}
            }	
        }
	}
}


void copyRowToBufferFloyd(int row, short* buf[], ChannelPtr<uchar> p, int w, int h) {
	//row out of valid range [0,h-1]
    if(row >= h) return;

	ChannelPtr<uchar> original = p;

    //pointer to correct buffer
	short *ptr = buf[row % 2];

    //p points to correct row
	p = (p + w * row);
	for (int i = 1; i < w + 1; i++) {
		ptr[i] = *p++;
	}

    // bringing p back
	p = original;
}

void copyRowToBufferJarvis(int row, short* buf[], ChannelPtr<uchar> p, int w, int h) {
	//row out of valid range [0,h-1]
    if (row >= h) return;

	ChannelPtr<uchar> original = p;
    
    //pointer to correct buffer
	short *ptr = buf[row % 3];

    //p points to correct row
	p = (p + w * row);
	for (int i = 2; i < w + 2; i++) {
		ptr[i] = *p++;
	}

    // bringing p back
	p = original;
}