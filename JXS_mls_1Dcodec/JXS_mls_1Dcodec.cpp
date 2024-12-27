// JXS_mls_1Dcodec.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <vector>
#include "LGT.h"

int main()
{
	uint8_t NLx = 1; // Number of levels in x (decomp_h)
	if (NLx > 5)
	{
		std::cout << "NLx=" << (uint32_t)NLx << " is greater than max allowed decomp level (5)\n";
		return -1;
	}
	uint32_t width = 160;
	std::vector<int32_t> im(width);    // im is later used to store details, so int32_t instead of uint32_t to make sign visible in printouts
	for (int ix = 0; ix < width; ++ix) {
		im[ix] = 256 - 8 * (ix / 5);   // Y
	}

	for (int i = 0; i < width; ++i)
		im[i] *= 256 * 16;

	std::cout << "Forward\n";
	for (int i = 0; i < NLx; ++i)
	{
		dwt_forward(im, i);
	}

	std::vector<std::vector<int32_t>> precinct_bufs(NLx + 1);
	size_t bufsize = width;
	for (int i = NLx; i > 0; --i)
	{
		bufsize /= 2;
		precinct_bufs[i].resize(bufsize);// Mimics ref codec prec buffers, where level indices are in reverse:
	}                                            // the last array has the earliest details, array of size of half source data length
	precinct_bufs[0].resize(bufsize);    // precinct->sig_mag_data_mb->bufs[sig_mag_data_mb->n_buffers-1] is given by dwt_forward(im, 0)

	/*int d = 1 << NLx;
	for (int ix = 0; ix < width / d; ++ix)
	{
		for (int lvl = 1; lvl <= NLx; ++lvl)
		{
			precinct_bufs[lvl][ix] = im[d * ix + (1 << (lvl - 1))]; // horizontal detail coeffs
		}
	}*/
	for (int i = 0; i < width; ++i)
	{

		std::cout << im[i] / 256 / 16 << " ";
	}
	std::cout << "\n";

	std::cout << "Inverse\n";
	for (int i = NLx - 1; i >= 0; --i)
	{
		dwt_inverse(im, i);
	}
	for (int i = 0; i < width; ++i)
		std::cout << im[i] / 256 / 16 << " ";
	std::cout << "\n";

	/*std::vector<double> decomp(width);
	for (int d = 1; d <= width / 2; d *= 2)
	{
		for (int i = 0; i < width / 2 / d; ++i)
		{
			decomp[i] = im[d * i];
			decomp[i + width / 2 / d] = im[2 * d * i + d];
		}
	}
	std::cout << "Deinterleaved vector:\n";
	for (int i = 0; i < width; ++i)
	{
		std::cout << i << ": " << decomp[i] / 256 / 16 << "\n";
	}*/
	return 0;
}
