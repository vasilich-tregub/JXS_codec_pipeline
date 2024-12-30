// JXS_mls_1Dcodec.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <vector>
#include "LGT.h"

#include <intrin.h>
static unsigned int __inline BSR(unsigned long x)
{
	unsigned long r = 0;
	_BitScanReverse(&r, x);
	return r;
}
#define GCLI(x) (((x) == 0) ? 0 : (BSR((x)) + 1))
#define SIGN_BIT_MASK (1 << 31)

int32_t clamp(int32_t v, int32_t max_v)
{
	if (v > max_v)
	{
		return max_v;
	}
	if (v < 0)
	{
		return 0;
	}
	return v;
}

int main()
{
	uint8_t NLx = 5; // Number of levels in x (decomp_h)
	if (NLx > 5)
	{
		std::cout << "NLx=" << (uint32_t)NLx << " is greater than max allowed decomp level (5)\n";
		return -1;
	}
	uint32_t width = 80;
	std::vector<int32_t> im(width);    // im is later used to store details, so int32_t instead of uint32_t to make sign visible in printouts
	for (int ix = 0; ix < width; ++ix) {
		im[ix] = 256 - 16 * (ix / 5);   // Y
	}
	int32_t depth = 8;
	int32_t dclev = ((1 << depth) >> 1);
	for (int i = 0; i < width; ++i)
		im[i] = (im[i] - dclev);

	std::cout << "Forward\n";
	for (int i = 0; i < NLx; ++i)
	{
		dwt_forward(im, i);
	}

	int group_size = 4;
	std::vector<std::vector<int32_t>> precinct_sig_mag_data_bufs(NLx + 1);// Mimics ref codec's prec sig_mag_data buffer, where level indices are in reverse
	std::vector<std::vector<int8_t>> precinct_gclis_bufs(NLx + 1);// Mimics ref codec's prec gclis buffer, where level indices are in reverse
	size_t bufsize = width;
	for (int i = NLx; i > 0; --i)
	{
		bufsize /= 2;
		precinct_gclis_bufs[i].resize((bufsize + group_size - 1) / group_size);
		precinct_sig_mag_data_bufs[i].resize((bufsize + group_size - 1) / group_size * group_size);
	}                                            // the last array has the earliest details, array of size of half source data length
	precinct_sig_mag_data_bufs[0].resize((bufsize + group_size) / group_size * group_size); // precinct->sig_mag_data_mb->bufs[sig_mag_data_mb->n_buffers-1] is given by dwt_forward(im, 0)
	precinct_gclis_bufs[0].resize((bufsize + group_size) / group_size);    // precinct->gclis_mb->bufs[sig_mag_data_mb->n_buffers-1] is given by dwt_forward(im, 0)

	int d = 1; // fill precinct_sig_mag_data_bufs[NLx + 1 - lvl] and isolate value sign into msb
	for (int lvl = 1; lvl <= NLx; ++lvl)
	{
		d = 1 << lvl;
		for (int idst = 0, iximg = 0; iximg < width - (1 << (lvl - 1)); iximg += d, ++idst)
		{
			int32_t val = im[iximg + (1 << (lvl - 1))]; // horizontal detail coeff
			if (val >= 0)
				precinct_sig_mag_data_bufs[NLx + 1 - lvl][idst] = val;
			else
				precinct_sig_mag_data_bufs[NLx + 1 - lvl][idst] = -val + SIGN_BIT_MASK;

		}
	}
	for (int idst = 0, iximg = 0; iximg < width; ++idst, iximg += d)
	{
		int32_t val = im[iximg]; // approximation coeff
		if (val >= 0)
			precinct_sig_mag_data_bufs[0][idst] = val;
		else
			precinct_sig_mag_data_bufs[0][idst] = -val + SIGN_BIT_MASK;
	}

	for (int lvl = 1; lvl <= NLx; ++lvl)
	{
		int32_t or_all, j, k, i, j_last;
		for (i = 0, k = 0; i < (precinct_sig_mag_data_bufs[lvl].size() / 4) * group_size; i += group_size, ++k)
		{
			for (or_all = 0, j = 0; j < group_size; j++)
				or_all |= precinct_sig_mag_data_bufs[lvl][j];
			precinct_gclis_bufs[lvl][k] = GCLI(or_all & (~SIGN_BIT_MASK));
		}
		if (precinct_sig_mag_data_bufs[lvl].size() % group_size) // incomplete group of coeffs
		{
			for (or_all = 0, j_last = 0; j_last < precinct_sig_mag_data_bufs[lvl].size() % group_size; j_last++, j++)
				or_all |= precinct_sig_mag_data_bufs[lvl][j];
			precinct_gclis_bufs[lvl][k] = GCLI(or_all & (~SIGN_BIT_MASK));
		}
	}

	for (int i = 0; i < width; ++i)
	{
		std::cout << im[i] << " ";
	}
	std::cout << "\n";

	std::cout << "Inverse\n";
	for (int i = NLx - 1; i >= 0; --i)
	{
		dwt_inverse(im, i);
	}
	/*	const uint8_t s = Bw - (uint8_t)im->depth;
	const xs_data_in_t dclev_and_rounding = ((1 << Bw) >> 1) + ((1 << s) >> 1);
	Bw = depth as Fq (fractional bits) is zero in this exercise*/
	int32_t dclev_and_rounding = ((1 << depth) >> 1);
	int32_t max_val = (1 << depth) - 1;
	for (int i = 0; i < width; ++i)
		im[i] = clamp((im[i] + dclev_and_rounding), max_val);
	for (int i = 0; i < width; ++i)
		std::cout << im[i] << " ";
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
