// JXS_mls_1Dcodec.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#pragma once
#include <iostream>
#include <vector>
#include "LGT.h"
extern "C" {
#include "markers.h"
}
#include <intrin.h>
static unsigned int __inline BSR(unsigned long x)
{
	unsigned long r = 0;
	_BitScanReverse(&r, x);
	return r;
}
#define GCLI(x) (((x) == 0) ? 0 : (BSR((x)) + 1))
#define SIGN_BIT_MASK (1 << (8 * sizeof(int32_t) - 1))

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
	config_t config{};
	init_config(&config);
	uint8_t NLx = 1; // Number of levels in x (decomp_h)
	if (NLx > 5)
	{
		std::cout << "NLx=" << (uint32_t)NLx << " is greater than max allowed decomp level (5)\n";
		return -1;
	}
	config.p.NLx = NLx;

	uint32_t width = 80;
	uint32_t height = 1;
	uint8_t ncomps = 1;
	std::vector<int32_t> img2enc(width);    // img2enc is later used to store details, so int32_t instead of uint32_t to make sign visible in printouts
	for (int ix = 0; ix < width; ++ix) {
		img2enc[ix] = 256 - 16 * (ix / 5);   // Y
	}
	int32_t depth = 8;

	image_t im{ ncomps, width, height, depth, nullptr };

	// xs_config_resolve_bw_fq
	if (config.profile == 0x6ec0/*XS_PROFILE_MLS_12*/ && config.p.Bw == 0xff)
		config.p.Bw = im.depth;

	int32_t dclev = ((1 << depth) >> 1);
	for (int i = 0; i < width; ++i)
		img2enc[i] = (img2enc[i] - dclev);

	size_t bitstream_buf_size, bitstream_buf_max_size;
	uint8_t* bitstream_buf = NULL;
	// Take the RAW image size and add some extra for margin.
	bitstream_buf_max_size = width * height * ncomps * ((depth + 7) >> 3) + 1024 * 1024;
	bitstream_buf = (uint8_t*)malloc(bitstream_buf_max_size);

	bit_packer_t* bitstream = bitpacker_init();

	bitpacker_set_buffer(bitstream, bitstream_buf, bitstream_buf_max_size);

	int slice_idx = 0;
	int markers_len = 0;
	const int header_len = write_head(bitstream, &im, &config);

	std::cout << "Forward\n";
	for (int i = 0; i < NLx; ++i)
	{
		dwt_forward(img2enc, i);
	}

	std::cout << "img2enc after NLx runs of dwt_forward(img2enc, lvl)\n";
	for (int i = 0; i < width; ++i)
	{
		std::cout << img2enc[i] << " ";
	}
	std::cout << "\n";

	//for (int line_idx = 0; line_idx < image->height; line_idx += ctx->ids.ph)
	//{
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
	precinct_gclis_bufs[0].resize((bufsize + group_size - 1) / group_size);    // precinct->gclis_mb->bufs[sig_mag_data_mb->n_buffers-1] is given by dwt_forward(im, 0)
	precinct_sig_mag_data_bufs[0].resize((bufsize + group_size - 1) / group_size * group_size); // precinct->sig_mag_data_mb->bufs[sig_mag_data_mb->n_buffers-1] is given by dwt_forward(im, 0)

	// precinct from image
	std::cout << "Precinct from image; images not altered\n";
	int d = 1; // fill precinct_sig_mag_data_bufs[NLx + 1 - lvl] and isolate value sign into msb
	for (int lvl = 1; lvl <= NLx; ++lvl)
	{
		d = 1 << lvl;
		for (int idst = 0, iximg = 0; iximg < width - (1 << (lvl - 1)); iximg += d, ++idst)
		{
			int band_idx = NLx + 1 - lvl;
			int32_t val = img2enc[iximg + (1 << (lvl - 1))]; // horizontal detail coeff
			if (val >= 0)
				precinct_sig_mag_data_bufs[band_idx][idst] = val;
			else
				precinct_sig_mag_data_bufs[band_idx][idst] = -val + SIGN_BIT_MASK;

		}
	}
	for (int idst = 0, iximg = 0; iximg < width; ++idst, iximg += d)
	{
		int32_t val = img2enc[iximg]; // approximation coeff
		if (val >= 0)
			precinct_sig_mag_data_bufs[0][idst] = val;
		else
			precinct_sig_mag_data_bufs[0][idst] = -val + SIGN_BIT_MASK;
	}
	// end of precinct from image

	// update gclis
	for (int lvl = 0; lvl <= NLx; ++lvl)
	{
		int32_t or_all, j, k, i, j_last;
		for (i = 0, k = 0; i < (precinct_sig_mag_data_bufs[lvl].size() / group_size) * group_size; i += group_size, ++k)
		{
			for (or_all = 0, j = 0; j < group_size; j++)
				or_all |= precinct_sig_mag_data_bufs[lvl][i + j];
			precinct_gclis_bufs[lvl][k] = GCLI(or_all & (~SIGN_BIT_MASK));
		}
		if (precinct_sig_mag_data_bufs[lvl].size() % group_size) // incomplete group of coeffs
		{
			for (or_all = 0, j_last = 0; j_last < precinct_sig_mag_data_bufs[lvl].size() % group_size; j_last++, j++)
				or_all |= precinct_sig_mag_data_bufs[lvl][i + j];
			precinct_gclis_bufs[lvl][k] = GCLI(or_all & (~SIGN_BIT_MASK));
		}
	}
	// end of update gclis

	//if (rate_control_process_precinct(ctx->rc[column], ctx->precinct[column], &rc_results) < 0) {
		//return false;
	//}
	// 
	// planned: quantization; dequantization etc. to be added here 
	//if (precinct_is_first_of_slice(ctx->precinct[column], ctx->xs_config->p.slice_height) && (column == 0))
	//{
	markers_len += write_slice_header(bitstream, slice_idx++);
	//}

	/* To back engineer how the ra_result fields of the below comments should be filled in, 
	* we need to analyze xs_ref_sw_ed2's routines:
	* xs_enc calls `rate control process precinct` which creates rc_results; 
	* rc_results`s quantization and refinement fields are filled with the _do_rate_allocation call; 
	* other fields are filled directly by the `rate control process precinct` function.
    * rate_control_t* rc param of this function is created in the xs_enc_init function call 
    * in line ctx->rc[column] = rate_control_open(xs_config, &ctx->ids, column); (126).
	*/
	// pack precinct
	const bool use_long_precinct_headers = false;// precinct_use_long_headers(precinct);

	bitpacker_write(bitstream, ((/*ra_result->precinct_total_bits*/672 - /*ra_result->pbinfo.prec_header_size*/48) >> 3), PREC_HDR_PREC_SIZE);
	//assert(ra_result->quantization < 16);
	bitpacker_write(bitstream, /*ra_result->quantization*/0, PREC_HDR_QUANTIZATION_SIZE);
	bitpacker_write(bitstream, /*ra_result->refinement*/1, PREC_HDR_REFINEMENT_SIZE);
	
	for (int band = 0; band < /*bands_count_of(precinct)*/2; ++band)
	{
		const int method_signaling = 0;// gcli_method_get_signaling(ra_result->gcli_sb_methods[band], ctx->enabled_methods);
		bitpacker_write(bitstream, method_signaling, GCLI_METHOD_NBITS) < 0;
	}
	bitpacker_align(bitstream, PREC_HDR_ALIGNMENT);
	
	const int position_count = 2;// line_count_of(precinct);
	int subpkt = 0;
	/*int len_after = 0;
	int lvl, ypos, idx_start, idx_stop, idx, gtli;*/
	int len_before_subpkt = 0;
	int lvl, ypos;
	int idx_start, idx_stop, idx;
	for (idx_start = idx_stop = 0; idx_stop < position_count; idx_stop++)
	{
		if ((idx_stop != (position_count - 1))/* && (precinct_subpkt_of(precinct, idx_stop) == precinct_subpkt_of(precinct, idx_stop + 1))*/)
		{
			continue;
		}
		lvl = 0;// precinct_band_index_of(precinct, idx_start);
		ypos = 0;// precinct_ypos_of(precinct, idx_start);
		/*if (!precinct_is_line_present(precinct, lvl, ypos))
		{
			++subpkt;
			idx_start = idx_stop + 1;
			continue;
		}*/

		bitpacker_write(bitstream, (uint64_t)/*ra_result->pbinfo.subpkt_uses_raw_fallback[subpkt]*/1, 1);
		bitpacker_write(bitstream, (uint64_t)/*ra_result->pbinfo.subpkt_size_data[subpkt]*/504 >> 3, use_long_precinct_headers ? PKT_HDR_DATA_SIZE_LONG : PKT_HDR_DATA_SIZE_SHORT);
		bitpacker_write(bitstream, (uint64_t)/*ra_result->pbinfo.subpkt_size_gcli[subpkt]*/80 >> 3, use_long_precinct_headers ? PKT_HDR_GCLI_SIZE_LONG : PKT_HDR_GCLI_SIZE_SHORT);
		bitpacker_write(bitstream, (uint64_t)/*ra_result->pbinfo.subpkt_size_sign[subpkt]*/0 >> 3, use_long_precinct_headers ? PKT_HDR_SIGN_SIZE_LONG : PKT_HDR_SIGN_SIZE_SHORT);
		bitpacker_align(bitstream, PKT_HDR_ALIGNMENT);

		// pack glis
		len_before_subpkt = bitpacker_get_len(bitstream);
		for (idx = idx_start; idx <= idx_stop; idx++)
		{
			//lvl = precinct_band_index_of(precinct, idx);
			//ypos = precinct_ypos_of(precinct, idx);
			//err = (err) || pack_gclis(ctx, bitstream, precinct, ra_result, idx);
			//return pack_raw_gclis(bitstream, precinct_gcli_of(precinct, lvl, ypos), (int)precinct_gcli_width_of(precinct, lvl)) < 0;
			//int pack_raw_gclis(bit_packer_t* bitstream, gcli_data_t* gclis, int len)
			int bit_len = 0, n = 1;
			for (int i = 0; (i < precinct_gclis_bufs[idx].size()) && (n > 0); i++)
			{
				n = bitpacker_write(bitstream, precinct_gclis_bufs[idx][i], 4);
				bit_len += n;
			}
		}
		bitpacker_align(bitstream, SUBPKT_ALIGNMENT);

		len_before_subpkt = bitpacker_get_len(bitstream);
		for (idx = idx_start; idx <= idx_stop; idx++)
		{
			//lvl = precinct_band_index_of(precinct, idx);
			//ypos = precinct_ypos_of(precinct, idx);
			/*if (precinct_is_line_present(precinct, lvl, ypos))
			{
				gtli = ra_result->gtli_table_data[lvl];
				if (ctx->xs_config->verbose > 3)
				{
					fprintf(stderr, "DATA (bitpos=%d) (lvl=%d ypos=%d gtli=%d)\n", bitpacker_get_len(bitstream), lvl, ypos, gtli);
				}
				err = err || (pack_data(bitstream, precinct_line_of(precinct, lvl, ypos), (int)precinct_width_of(precinct, lvl), precinct_gcli_of(precinct, lvl, ypos), ctx->xs_config->p.N_g, gtli, ctx->xs_config->p.Fs) < 0);
			}*/
		}
		bitpacker_align(bitstream, SUBPKT_ALIGNMENT);
	}
		// etc.
	// end of pack precinct
	
	//} end of for (int line_idx = 0; line_idx < image->height; line_idx += ctx->ids.ph)
	std::vector<int32_t> dcdimg(width);    // dcdimg, decoded image

	// precinct to image
	for (int lvl = 1; lvl <= NLx; ++lvl)
	{
		d = 1 << lvl;
		int band_idx = NLx + 1 - lvl;
		for (size_t idst = 0, isrc = 0; idst < width - (1 << (lvl - 1)); idst += d, ++isrc)
		{
			int32_t val = precinct_sig_mag_data_bufs[band_idx][isrc];
			dcdimg[idst + (1 << (lvl - 1))] = ((val & SIGN_BIT_MASK) ? -(val & ~SIGN_BIT_MASK) : val);
		}
	}
	for (int idst = 0, isrc = 0; idst < width; ++isrc, idst += d)
	{
		int32_t val = precinct_sig_mag_data_bufs[0][isrc];
		dcdimg[idst] = ((val & SIGN_BIT_MASK) ? -(val & ~SIGN_BIT_MASK) : val);
	}
	std::cout << "Precinct to image, 'dcdimg' printout\n";
	for (int i = 0; i < width; ++i)
		std::cout << dcdimg[i] << " ";
	std::cout << "\n";
	std::cout << "Inverse\n";
	for (int i = NLx - 1; i >= 0; --i)
	{
		dwt_inverse(dcdimg, i);
	}
	/*	const uint8_t s = Bw - (uint8_t)img2enc->depth;
	const xs_data_in_t dclev_and_rounding = ((1 << Bw) >> 1) + ((1 << s) >> 1);
	Bw = depth as Fq (fractional bits) is zero in this exercise*/
	int32_t dclev_and_rounding = ((1 << depth) >> 1);
	int32_t max_val = (1 << depth) - 1;
	for (int i = 0; i < width; ++i)
		dcdimg[i] = clamp((dcdimg[i] + dclev_and_rounding), max_val);
	for (int i = 0; i < width; ++i)
		std::cout << dcdimg[i] << " ";
	std::cout << "\n";

	/*std::vector<double> decomp(width);
	for (int d = 1; d <= width / 2; d *= 2)
	{
		for (int i = 0; i < width / 2 / d; ++i)
		{
			decomp[i] = img2enc[d * i];
			decomp[i + width / 2 / d] = img2enc[2 * d * i + d];
		}
	}
	std::cout << "Deinterleaved vector:\n";
	for (int i = 0; i < width; ++i)
	{
		std::cout << i << ": " << decomp[i] / 256 / 16 << "\n";
	}*/
	return 0;
}
