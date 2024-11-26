// JXS_encode_decode.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "libjxs.h"
#include "image_open.h"
#include "cmdline_options.h"
#include "file_io.h"
#include "file_sequence.h"
#include <malloc.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _DEBUG
#pragma comment(lib, "jxsd.lib")
#pragma comment(lib, "jxs_utilsd.lib")
#else
#pragma comment(lib, "jxs.lib")
#pragma comment(lib, "jxs_utils.lib")
#endif

bool xs_parse_u8array_(uint8_t* values, int max_items, char* cfg_str, int* num)
{
	for (int i = 0; max_items > 0; ++i, --max_items)
	{
		if (*cfg_str == 0)
		{
			return true;
		}
		if (num != 0)
		{
			*num = i;
		}
		values[i] = (uint8_t)atoi(cfg_str);
		while (*cfg_str != ',' && *cfg_str != 0) ++cfg_str;
		if (*cfg_str == ',') ++cfg_str;
	}
	return *cfg_str == 0;
}

int main(int argc, char** argv)
{
	xs_config_t xs_config;
	xs_image_t image = { 0 };
	const char* output_fn = "fraunhofer-80x1-mono.jxs";
	uint8_t* bitstream_buf = NULL;
	size_t bitstream_buf_size, bitstream_buf_max_size;
	xs_enc_context_t* ctx = NULL;
	int ret = 0;
	int file_idx = 0;
	cmdline_options_t options;
	int optind;

	xs_dec_context_t* decctx = NULL;
	fprintf(stderr, "Learn JPEG XS msb packing(?)\n");


	do
	{
		const uint32_t width = 80;
		const uint32_t height = 1;
		const uint32_t depth = 8;
		const uint32_t bpp = 16;
		xs_config.verbose = 0; // options.verbose;

		image.width = width;
		image.height = height;
		image.depth = depth;
		image.ncomps = 1;

		image.sx[0] = 1;
		image.sy[0] = 1;
		//word_size = (bytesRead == nsamples444 * 2) ? 16 : 8;
		if (!xs_allocate_image(&image, false))
			return -1;

		uint32_t* ptr0 = image.comps_array[0];
		for (int iy = 0; iy < height; ++iy) {
			for (int ix = 0; ix < width; ++ix) {
				int halfw = width / 2;
				ptr0[iy * width + ix] = 128 - (0.25 * ((ix - halfw) * (ix - halfw) * (ix - halfw) / 125.));   // Y
				printf("%d ", ptr0[iy * width + ix]);
			}
			printf("\n");
			/*for (int ix = 0; ix < width / 4; ++ix) {
				ptr0[iy * width + ix] = 0;   // Y
			}
			for (int ix = width / 4; ix < 3 * width / 4; ++ix) {
				ptr0[iy * width + ix] = 255; // Y
			}
			for (int ix = 3 * width / 4; ix < width; ++ix) {
				ptr0[iy * width + ix] = 0;
			}*/
		}

		/*if (image_open_auto(input_fn, &image, width, height, depth) < 0)
		{
			fprintf(stderr, "Unable to open image %s!\n", input_fn);
			ret = -1;
			break;
		}*/

		xs_config.bitstream_size_in_bytes = (size_t)(bpp * image.width * image.height / 8);
		xs_config.budget_report_lines = 20.0f;
		xs_config.verbose = 255;
		xs_config.gains_mode = XS_GAINS_OPT_EXPLICIT;
		xs_config.profile = XS_PROFILE_UNRESTRICTED;
		xs_config.level = XS_LEVEL_UNRESTRICTED;
		xs_config.sublevel = XS_SUBLEVEL_UNRESTRICTED;
		xs_config.cap_bits = XS_CAP_RAW_PER_PKT;
		xs_config.p.color_transform = XS_CPIH_NONE;
		xs_config.p.Cw = 0;
		xs_config.p.slice_height = 0x0010;
		xs_config.p.N_g = 4;
		xs_config.p.S_s = 8;
		xs_config.p.Bw = 255;
		xs_config.p.Fq = 8;
		xs_config.p.B_r = 4;
		xs_config.p.Fslc = 0;
		xs_config.p.Ppoc = 0;
		xs_config.p.NLx = 1;
		xs_config.p.NLy = 0;
		xs_config.p.Lh = 0;
		xs_config.p.Rl = 1;
		xs_config.p.Qpih = 0; // 0 deadzone; 1 uniform
		xs_config.p.Fs = 0;
		xs_config.p.Rm = 0;
		xs_config.p.Sd = 0;
		memset(xs_config.p.lvl_gains, 255, (MAX_NBANDS + 1) * sizeof(uint8_t));
		xs_parse_u8array_(xs_config.p.lvl_gains, MAX_NBANDS, "1,1", 0); //  "1,0,0,1,0,0" "1,1,0,0,0,0"; "2,2,2,1,1,1"
		memset(xs_config.p.lvl_priorities, 255, (MAX_NBANDS + 1) * sizeof(uint8_t));
		xs_parse_u8array_(xs_config.p.lvl_priorities, MAX_NBANDS, "0,1", 0); // "0,2,3,1,4,5" "0,1,2,4,3,5"; "8,7,6,5,3,4"
		xs_config.p.Tnlt = XS_NLT_NONE;
		xs_config.p.Tnlt_params.quadratic.sigma = 0; xs_config.p.Tnlt_params.quadratic.alpha = 0;
		xs_config.p.Tnlt_params.extended.T1 = 0; xs_config.p.Tnlt_params.extended.T2 = 0; xs_config.p.Tnlt_params.extended.E = 0;
		xs_config.p.tetrix_params.Cf = XS_TETRIX_FULL; xs_config.p.tetrix_params.e1 = 0; xs_config.p.tetrix_params.e2 = 0;
		xs_config.p.cfa_pattern = XS_CFA_RGGB;

		if (!xs_enc_preprocess_image(&xs_config, &image))
		{
			fprintf(stderr, "Error preprocessing the image\n");
			ret = -1;
			break;
		}

		ctx = xs_enc_init(&xs_config, &image);
		if (!ctx)
		{
			fprintf(stderr, "Unable to allocate encoding context\n");
			ret = -1;
			break;
		}

		if (xs_config.bitstream_size_in_bytes == (size_t)-1)
		{
			// Take the RAW image size and add some extra for margin.
			bitstream_buf_max_size = image.width * image.height * image.ncomps * ((image.depth + 7) >> 3) + 1024 * 1024;
		}
		else
		{
			bitstream_buf_max_size = (xs_config.bitstream_size_in_bytes + 7) & (~0x7);
		}
		bitstream_buf = (uint8_t*)malloc(bitstream_buf_max_size);
		if (!bitstream_buf)
		{
			fprintf(stderr, "Unable to allocate codestream mem\n");
			ret = -1;
			break;
		}

		/*if (options.dump_xs_cfg > 0)
		{
			char dump_str[1024];
			memset(dump_str, 0, 1024);
			if (!xs_config_dump(&xs_config, image.depth, dump_str, 1024, options.dump_xs_cfg))
			{
				fprintf(stderr, "Unable to dump configuration for codestream\n");
				ret = -1;
				break;
			}
			fprintf(stdout, "Configuration: \"%s\"\n", dump_str);
		}*/
		do {
			if (!fileio_writable(output_fn))
			{
				fprintf(stderr, "Output file is not writable %s\n", output_fn);
				ret = -1;
				break;
			}
			if (!xs_enc_image(ctx, &image, (uint8_t*)bitstream_buf, bitstream_buf_max_size, &bitstream_buf_size))
			{
				fprintf(stderr, "Unable to encode image\n");
				ret = -1;
				break;
			}
			int32_t* ptr = image.comps_array[0];
			for (int iy = 0; iy < height; ++iy) {
				for (int ix = 0; ix < width / 2; ++ix) {
					printf("%d ", ptr[iy * width + 2 * ix] >> 12);
				}
				printf("\n");
				for (int ix = 0; ix < width / 2; ++ix) {
					printf("%d ", ptr[iy * width + 2 * ix + 1] >> 12);
				}
				printf("\n");
			}
			printf("\n");
			decctx = xs_dec_init(&xs_config, &image);
			if (!decctx)
			{
				fprintf(stderr, "Unable to allocate decoding context to decode encoded image\n");
				ret = -1;
				break;
			}
			if (!xs_dec_bitstream(decctx, bitstream_buf, bitstream_buf_size, &image))
			{
				fprintf(stderr, "Unable to decode encoded image\n");
				ret = -1;
				break;
			}
			for (int iy = 0; iy < height; ++iy) {
				for (int ix = 0; ix < width; ++ix) {
					printf("%d ", ptr[iy * width + ix]);
				}
				printf("\n");
			}
			printf("\n");
			xs_free_image(&image);
			if (fileio_write(output_fn, bitstream_buf, bitstream_buf_size) < 0)
			{
				fprintf(stderr, "Unable to write output encoded codestream to %s\n", output_fn);
				ret = -1;
				break;
			}
		} while (false);
	} while (false);

	// Cleanup.
	if (output_fn)
	{
		//free(output_fn);
	}
	xs_free_image(&image);
	if (ctx)
	{
		xs_enc_close(ctx);
	}
	if (bitstream_buf)
	{
		free(bitstream_buf);
	}
	return ret;
}
