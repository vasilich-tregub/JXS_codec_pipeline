#pragma once

typedef void (*filter_func_t)(std::vector<double>& im, const int start, const int level); // start pixel in either col (vert) or row (hor)

void dwt_inverse_filter(std::vector<double>& im, const int current, const int inc)
{
	int end = (int)im.size();
	assert(inc < end && "stepping outside source image");

	// low pass filter, {-1./4, 1./4, -1./4}
	int i = current;
	im[i] -= (im[inc] + 1) / 2;
	i += 2 * inc;
	for (; i < end - inc; i += 2 * inc)
	{
		im[i] -= (im[i - inc] + im[i + inc] + 2) / 4;
	}
	if (i < end)
	{
		im[i] -= (im[i - inc] + 1) / 2;
	}

	// high pass filter, {-1./8, 1./8, 6./8, 1./8 -1./8}
	// successive convolutions with {-1./4, 1./4, -1./4} for even pixels
	// and {1./2, 1., 1./2} for even pixels
	// for im[n] result in -im[n-2]/8 + im[n-1]/8 + 6*im[n]/8 + im[n+1]/8 - im[n+2]/8
	i = current + inc;
	for (; i < end - inc; i += 2 * inc)
	{
		im[i] += (im[i - inc] + im[i + inc]) / 2;
	}
	if (i < end)
	{
		im[i] += im[i - inc];
	}

}

void dwt_forward_filter(std::vector<double>& im, const int current, const int inc)
{
	int end = (int)im.size();
	assert(inc < end && "stepping outside source image");

	int i = current + inc;
	// high pass filter, {-1./2, 1., -1./2}
	for (; i < end - inc; i += 2 * inc)
	{
		im[i] -= (im[i - inc] + im[i + inc]) / 2;
	}
	if (i < end)
	{
		im[i] -= im[i - inc];
	}

	i = current;
	// low pass filter, 
	// successive convolutions with {-1./2, 1., -1./2} for odd pixels
	// and {1./4, 1., 1./4} for even pixels
	// for im[n] result in -im[n-2]/8 + im[n-1]/4 + 6*im[n]/8 + im[n+1]/4 - im[n+2]/8
	// i.e., {-1./8, 2./8, 6./8, 2./8, -1./8}
	im[i] += (im[inc] + 1) * 2;
	i += 2 * inc;
	for (; i < end - inc; i += 2 * inc)
	{
		im[i] += (im[i - inc] + im[i + inc] + 2) / 4;
	}
	if (i < end)
	{
		im[i] += (im[i - inc] + 1) / 2;
	}
}

void dwt_transform_vertical(std::vector<double>& im, int width, int height, const int h_level, const int v_level, filter_func_t filter)
{
	assert(h_level >= 0 && v_level >= 0);
	const int x_inc = 1 << h_level;
	const int y_inc = width << v_level;
	for (int ix = 0; ix < width; ix += x_inc)
	{
		filter(im, ix, y_inc);
	}
}

void dwt_transform_horizontal(std::vector<double>& im, int width, int height, const int h_level, const int v_level, filter_func_t filter)
{
	assert(h_level >= 0 && v_level >= 0);
	const int x_inc = 1 << h_level;
	const int y_inc = width << v_level;
	for (int iy = 0; iy < height; iy += y_inc)
	{
		filter(im, iy * width, x_inc);
	}
}

void dwt_forward_transform(std::vector<double>& im, int width, int height, const int h_level, const int v_level)
{
	assert(im.size() == width * height);
	assert(v_level <= h_level);
	int d = 0;
	for (; d < v_level; ++d)
	{
		dwt_transform_vertical(im, width, height, d, d, dwt_forward_filter);
		dwt_transform_horizontal(im, width, height, d, d, dwt_forward_filter);
	}
	for (; d < h_level; ++d)
	{
		dwt_transform_horizontal(im, width, height, d, d, dwt_forward_filter);
	}
}

void dwt_inverse_transform(std::vector<double>& im, int width, int height, const int h_level, const int v_level)
{
	assert(im.size() == width * height);
	assert(v_level <= h_level);
	int d = h_level - 1;
	for (; d >= v_level; --d)
	{
		dwt_transform_horizontal(im, width, height, d, d, dwt_inverse_filter);
	}
	for (; d >= 0; --d)
	{
		dwt_transform_horizontal(im, width, height, d, d, dwt_inverse_filter);
		dwt_transform_vertical(im, width, height, d, d, dwt_inverse_filter);
	}
}