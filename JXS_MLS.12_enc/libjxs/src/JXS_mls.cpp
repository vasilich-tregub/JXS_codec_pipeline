/***************************************************************************
** intoPIX SA & Fraunhofer IIS (hereinafter the "Software Copyright       **
** Holder") hold or have the right to license copyright with respect to   **
** the accompanying software (hereinafter the "Software").                **
**                                                                        **
** Copyright License for Evaluation and Testing                           **
** --------------------------------------------                           **
**                                                                        **
** The code for this snippet is based on and mostly borrowed from         **
** the module xs_ref_sw_ed2/programs/xs_enc.main.c of the original        **
** Fraunhofer's/IntoPIX's repository                                      **
***************************************************************************/

extern "C" {
#include "libjxs.h"
#include "dwt.h"
#include "ids.h"
#include "precinct.h"
#include "rate_control.h"
#include "xs_markers.h"
#include "packing.h"
#include "bitpacking.h"
}
#include <vector>
#include "image_create.h"
#include "image_write.h"

int main(int argc, char** argv)
{
	xs_config_t xs_config;
	xs_image_t image = { 0 };
	char* input_seq_n = NULL;
	char* output_seq_n = NULL;
	char* input_fn = NULL;
	char* output_fn = NULL;
	uint8_t* bitstream_buf = NULL;
	size_t bitstream_buf_size, bitstream_buf_max_size;
	xs_enc_context_t* ctx = NULL;
	int ret = 0;
	int file_idx = 0;
	int optind;

	fprintf(stderr, "JPEG XS test model (XSM) version %s\n", xs_get_version_str());

	do
	{
		xs_config.verbose = 3;

		//create xs_config specific for example images
		xs_config_t xs_config{};

		xs_config.bitstream_size_in_bytes = -1;
		xs_config.budget_report_lines = 20.0000000;
		xs_config.profile = XS_PROFILE_MLS_12;
		xs_config.level = XS_LEVEL_AUTO;
		xs_config.sublevel = XS_SUBLEVEL_AUTO;
		xs_config.cap_bits = XS_CAP_AUTO;
		xs_config.p.color_transform = XS_CPIH_AUTO;
		xs_config.p.slice_height = 16;
		xs_config.p.N_g = 4;
		xs_config.p.S_s = 8;
		xs_config.p.Bw = 255;
		xs_config.p.B_r = 4;
		xs_config.p.NLx = 5;
		xs_config.p.NLy = 2; // can be 0, 1 or 2
		xs_config.p.Rl = 1;
		xs_config.p.Qpih = 1;
		xs_config.p.lvl_gains[30] = 255;
		xs_config.p.lvl_priorities[30] = 255;

		if (image_create(image) != 0)
		{
			printf("cannot create image, exit\n");
			return -1;
		}
		image_paint(image);

		std::vector<uint32_t> buf(image.width * image.height);
		for (int i = 0; i < image.width * image.height; ++i)
		{
			buf[i] = (image.comps_array[0][i] * 256 + image.comps_array[1][i]) * 256 + image.comps_array[2][i];
		}

		if (image_write(image.width, image.height, buf, L"source_image.png"))
			printf("cannot write source image file 'source_image.png'\n");

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

		if (!xs_enc_image(ctx, &image, (uint8_t*)bitstream_buf, bitstream_buf_max_size, &bitstream_buf_size))
		{
			fprintf(stderr, "Unable to encode image %s\n", input_fn);
			ret = -1;
			break;
		}
		xs_free_image(&image);

		FILE* file_out = fopen("source_image.jxs", "wb");
		if (file_out)
		{
			fwrite(bitstream_buf, bitstream_buf_size, 1, file_out);
			fclose(file_out);
			ret = 0;
		}
		else
		{
			fprintf(stderr, "Cannot write to source_image.jxs");
			ret = -1;
		}
	
	} while (false);

	// Cleanup.
	if (input_fn)
	{
		free(input_fn);
	}
	if (output_fn)
	{
		free(output_fn);
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
