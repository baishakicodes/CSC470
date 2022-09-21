#include "IP.h"
using namespace IP   ;

void histoMatchApprox(ImagePtr, ImagePtr, ImagePtr)   ;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_histoMatch:
//
// Apply histogram matching to I1. Output is in I2.
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void HW_histoMatch(ImagePtr I1, ImagePtr targetHisto, bool approxAlg, ImagePtr I2)
{
	if (approxAlg) 
	{
		histoMatchApprox(I1, targetHisto, I2)  ; 
		return  ;
	}

	// copy image header (width, height) of input image I1 to output image I2
	IP_copyImageHeader(I1, I2)  ;

	//init vars for width, height, and total number of pixels
	int w = I1->width();
	int h = I1->height();
	int total1 = w * h;

	// scale dynamic range of I1 to full intensity range [0,255]
	IP_embedRange(I1, 0., (double)MaxGray, I2);

	// declarations for Image channel and lut pointers, datatype, and misc vars
	ChannelPtr<uchar> p, lutp;
	int   type, histo[MXGRAY], histo1[MXGRAY], *histo2;
	int   map[MXGRAY], cap[MXGRAY], val[MXGRAY];
	double hmin, hmax;

	/* Note: IP_getChannel(I1, ch, p1, type) gets pointer p1 of channel ch in image I1.
		The pixel datatype (e.g., uchar, short, ...) of that channel is returned in type.
		It is ignored here since we assume that our input images consist exclusively of uchars.
		IP_getChannel() returns 1 when channel ch exists, 0 otherwise.
	*/
	
	// visit all image channels and evaluate output image
	for (int ch = 0; IP_getChannel(I2, ch, p, type); ch++)
	{
		IP_getChannel(targetHisto, 0, lutp, type);
		histo2 = (int *)&lutp[0];

		// compute sum of target histogram for normalization
		int total2 = 0;
		for (int i = 0; i < MXGRAY; i++)
		{
			total2 += histo2[i];
		}

		// scale histo2 to conform with dimensions of I1
		double scale = (double)total1 / total2;

		if (scale != 1)
		{
			int sum = 0;
			for (int i = 0; i < MXGRAY; i++)
			{
				histo2[i] = ROUND(histo2[i] * scale);
				
				// update histo2[] if cumulative histogram overshoots due to rounding operations
				sum += histo2[i];
				if (sum > total1) // check for overshoot
				{
					histo2[i] -= (sum - total1); // clamp last non-zero histo2[]
					for (; i < MXGRAY; i++) histo2[i] = 0; // clear the remainder of histo2[]
				}
			}
		}

		// compute histogram of channel ch of input
		IP_histogram(I2, ch, histo1, MXGRAY, hmin, hmax);

		// init map[] and cap[] for each graylevel
		int ii = 0, sum = 0;
		for (int i = 0; i < MXGRAY; i++)
		{
			map[i] = ii;           // left end of mapping range
			cap[i] = histo2[ii] - sum; //remaining amount (cap) on left end of mapping range
			sum += histo1[i];    // cumulative histogram value
			
			// widen mapping range if sum > histo2[ii]
			while (ii < MaxGray && sum > histo2[ii])
			{
				sum -= histo2[ii];
				ii++;
			}

		}

		// init output histogram
		for (int i = 0; i < MXGRAY; i++)
		{
			histo[i] = 0;
		}

		// init val[] to initial mapping of each input graylevel
		for (int i = 0; i < MXGRAY; i++)
		{
			val[i] = map[i];
		}
		// remap all pixels based on graylevel output intervals computed above
		IP_getChannel(I2, ch, p, type);

		for (int i = 0; i < total1; i++, p++)
		{
			int n = val[*p];   //desired mapping of input graylevel
			if (n == map[*p])  // verify if it corresponds to initial mapping
			{
				if (cap[*p]-- <= 0) // check not to exceed cap allowed at initial mapping
				{
					// exceeded cap: update val[] to next graylevel in range
					n = val[*p] = MIN(n + 1, MaxGray);
				}
				*p = n; // assign graylevel mapping to output
			}
			else // we passed rangeL so no need to check for cap[]
			{
				if (histo[n] < histo2[n]) // verify that we have not exceeded target histogram
				{
					*p = n;  // all good: assign n to output
				}
				else
				{
					*p = val[*p] = MIN(n + 1, MaxGray); //update val[] to next graylevel and assign to output
				}

			}
			
			//update output histogram
			histo[n]++;

		}

	}


		
}




void histoMatchApprox(ImagePtr I1, ImagePtr targetHisto, ImagePtr I2)
{
	// copy image header (width, height) of input image I1 to output image I2
	IP_copyImageHeader(I1, I2);

	// init vars for width, height, and total number of pixels
	int w = I1->width();
	int h = I1->height();
	int total1 = w * h;

	// scale dynamic range of I1 to full intensity range [0,255]
	IP_embedRange(I1, 0., (double)MaxGray, I2);

	// declarations for image channel and lut pointers, datatype, and misc vars
	ChannelPtr<uchar> p1, lutp;
	int type, lut[MXGRAY];
	int histo1[MXGRAY], *histo2;
	double histoSum1[MXGRAY], histoSum2[MXGRAY];
	double hmin, hmax;

	// visit all image channels and compute normalized histogram
	for (int ch = 0; IP_getChannel(I2, ch, p1, type); ch++)
	{
		IP_getChannel(targetHisto, 0, lutp, type);
		histo2 = (int *)&lutp[0];

		// compute sum of target histogram for normalization
		int total2 = 0;
		for (int i = 0; i < MXGRAY; i++) total2 += histo2[i];

		// compute histogram of channel ch of input
		IP_histogram(I2, ch, histo1, MXGRAY, hmin, hmax);

		// compute normalized cumulative distribution function (CDF) of input and output
		histoSum1[0] = (double)histo1[0] / total1;
		histoSum2[0] = (double)histo2[0] / total2;

		for (int i = 1; i < MXGRAY; i++)
		{
			histoSum1[i] = histoSum1[i - 1] + (double)histo1[i] / total1;
			histoSum2[i] = histoSum2[i - 1] + (double)histo2[i] / total2;
		}

		for (int i = 0; i < MXGRAY; i++)
		{
			int ii = MXGRAY - 1;
			lut[i] = ii;
			for (; ii >= 0 && histoSum2[ii] > histoSum1[i]; ii--)
			{
				lut[i] = ii; 
			}

		}

		// apply lut to input image
		for (int i = 0; i < total1; i++)
		{
			p1[i] = lut[p1[i]]; 
		}

	}



}

