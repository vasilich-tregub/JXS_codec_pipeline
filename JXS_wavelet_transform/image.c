#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "libjxs_for_XS_DWT_exercise.h"

bool xs_allocate_image(xs_image_t* im, const bool set_zero)
{
	for (int c = 0; c < im->ncomps; c++)
	{
		assert(im->comps_array[c] == NULL);
		assert(im->sx[c] == 1 || im->sx[c] == 2);
		assert(im->sy[c] == 1 || im->sy[c] == 2);
		const size_t sample_count = (size_t)(im->width / im->sx[c]) * (size_t)(im->height / im->sy[c]);
		im->comps_array[c] = (xs_data_in_t*)malloc(sample_count * sizeof(xs_data_in_t));
		if (im->comps_array[c] == NULL)
		{
			return false;
		}
		if (set_zero)
		{
			memset(im->comps_array[c], 0, sample_count * sizeof(xs_data_in_t));
		}
	}
	return true;
}

void xs_free_image(xs_image_t* im)
{
	for (int c = 0; c < im->ncomps; c++)
	{
		if (im->comps_array[c])
		{
			free(im->comps_array[c]);
		}
		im->comps_array[c] = NULL;
	}
}

//int xs_image_dump(xs_image_t* im, char* filename)
//{
//	//int min_val;
//	//int max_val;
//
//	FILE* f_out = fopen(filename, "wb");
//	if (f_out == NULL)
//	{
//		return -1;
//	}
//
//	//min_val = ~(1 << 31);
//	//max_val = (1 << 31);
//	//for (int c = 0; c < im->ncomps; ++c)
//	//{
//	//	for (i = 0; i < im->w[c] * im->h[c]; i++)
//	//	{
//	//		if (im->comps_array[comp][i] < min_val)
//	//			min_val = im->comps_array[comp][i];
//	//		if (im->comps_array[c][i] > max_val)
//	//			max_val = im->comps_array[c][i];
//	//	}
//	//}
//
//	//if (max_val == min_val)
//	//{
//	//	min_val = 0;
//	//	max_val = 255;
//	//}
//	//const bool use16 = max_val > 255;
//	const bool use16 = im->depth > 8;
//
//	//#define SCALE_TO_8BITS(v) (((v-min_val) * 256)/(max_val - min_val))
//
//	for (int c = 0; c < im->ncomps; ++c)
//	{
//		for (int i = 0; i < im->w[c] * im->h[c]; i++)
//		{
//			//const xs_data_in_t v = im->comps_array[comp][i] - min_val;
//			const xs_data_in_t v = im->comps_array[c][i];
//			putc(v & 0xff, f_out);
//			if (use16)
//			{
//				putc((v >> 8) & 0xff, f_out);
//			}
//		}
//	}
//	fclose(f_out);
//	return 0;
//}
