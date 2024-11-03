#include "dwt.h"
#include <assert.h>

typedef void (*filter_func_t)(xs_data_in_t* const base, xs_data_in_t* const end, const ptrdiff_t inc);

// Bounds-safe extended 5x3 DWT.
void dwt_inverse_filter_1d_(xs_data_in_t* const base, xs_data_in_t* const end, const ptrdiff_t inc)
{
	// Even samples (low pass).
	xs_data_in_t* p = base;
	*p -= (*(p + inc) + 1) >> 1;
	p += 2 * inc;
	for (; p < end - inc; p += 2 * inc)
	{
		*p -= (*(p - inc) + *(p + inc) + 2) >> 2;
	}
	if (p < end)
	{
		*p -= (*(p - inc) + 1) >> 1;
	}
	// Odd samples (high pass).
	p = base + inc;
	for (; p < end - inc; p += 2 * inc)
	{
		*p += (*(p - inc) + *(p + inc)) >> 1;
	}
	if (p < end)
	{
		*p += *(p - inc);
	}
}

// Bounds-safe extended 5x3 DWT.
void dwt_forward_filter_1d_(xs_data_in_t* const base, xs_data_in_t* const end, const ptrdiff_t inc)
{
	// Odd samples (high pass).
	xs_data_in_t* p = base + inc;
	for (; p < end - inc; p += 2 * inc)
	{
		*p -= (*(p - inc) + *(p + inc)) >> 1;
	}
	if (p < end)
	{
		*p -= *(p - inc);
	}
	// Even samples (low pass).
	p = base;
	*p += (*(p + inc) + 1) >> 1;
	p += 2 * inc;
	for (; p < end - inc; p += 2 * inc)
	{
		*p += (*(p - inc) + *(p + inc) + 2) >> 2;
	}
	if (p < end)
	{
		*p += (*(p - inc) + 1) >> 1;
	}
}

void dwt_tranform_vertical_(const ids_t* ids, xs_image_t* im, const int k, const int h_level, const int v_level, filter_func_t filter)
{
	assert(h_level >= 0 && v_level >= 0);
	const ptrdiff_t x_inc = (ptrdiff_t)1 << h_level;
	const ptrdiff_t y_inc = (ptrdiff_t)ids->comp_w[k] << v_level;
	xs_data_in_t* base = im->comps_array[k];
	xs_data_in_t* const end = base + ids->comp_w[k];
	for (; base < end; base += x_inc)
	{
		xs_data_in_t* const col_end = base + (size_t)ids->comp_w[k] * (size_t)ids->comp_h[k];
		filter(base, col_end, y_inc);
	}
}

void dwt_tranform_horizontal_(const ids_t* ids, xs_image_t* im, const int k, const int h_level, const int v_level, filter_func_t filter)
{
	assert(h_level >= 0 && v_level >= 0);
	const ptrdiff_t x_inc = (ptrdiff_t)1 << h_level;
	const ptrdiff_t y_inc = (ptrdiff_t)ids->comp_w[k] << v_level;
	xs_data_in_t* base = im->comps_array[k];
	xs_data_in_t* const end = base + (size_t)ids->comp_w[k] * (size_t)ids->comp_h[k];
	for (; base < end; base += y_inc)
	{
		xs_data_in_t* const row_end = base + ids->comp_w[k];
		filter(base, row_end, x_inc);
	}
}

// Take interleaved grid of wavelet coefficients, and inverse transform in-place. Annex E.
void dwt_inverse_transform(const ids_t* ids, xs_image_t* im)
{
	for (int k = 0; k < ids->ncomps - ids->sd; ++k)
	{
		assert(ids->nlxyp[k].y <= ids->nlxyp[k].x);
		for (int d = ids->nlxyp[k].x - 1; d >= ids->nlxyp[k].y; --d)
		{
			dwt_tranform_horizontal_(ids, im, k, d, ids->nlxyp[k].y, dwt_inverse_filter_1d_);
		}
		for (int d = ids->nlxyp[k].y - 1; d >= 0; --d)
		{
			dwt_tranform_horizontal_(ids, im, k, d, d, dwt_inverse_filter_1d_);
			dwt_tranform_vertical_(ids, im, k, d, d, dwt_inverse_filter_1d_);
		}
	}
}

// Take image samples, and forward transform in-place into interleaved grid of coefficients. Annex E.
void dwt_forward_transform(const ids_t* ids, xs_image_t* im)
{
	for (int k = 0; k < ids->ncomps - ids->sd; ++k)
	{
		assert(ids->nlxyp[k].y <= ids->nlxyp[k].x);
		for (int d = 0; d < ids->nlxyp[k].y; ++d)
		{
			dwt_tranform_vertical_(ids, im, k, d, d, dwt_forward_filter_1d_);
			dwt_tranform_horizontal_(ids, im, k, d, d, dwt_forward_filter_1d_);
		}
		for (int d = ids->nlxyp[k].y; d < ids->nlxyp[k].x; ++d)
		{
			dwt_tranform_horizontal_(ids, im, k, d, ids->nlxyp[k].y, dwt_forward_filter_1d_);
		}
 	}
}
