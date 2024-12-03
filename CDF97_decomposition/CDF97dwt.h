#pragma once

typedef void (*filter_func_t)(std::vector<double>& im, const int start, const int level); // start pixel in either col (vert) or row (hor)

void dwt_inverse_filter(std::vector<double>& im, const int current, const int inc)
{
	int end = (int)im.size();
	assert(inc < end && "stepping outside source image");

	double a;
	int i;

	// Undo scale
	/*a = 1.149604398;
	for (i = current; i < end; i += inc) {
		if (i % (2 * inc)) im[i] *= a;
		else im[i] /= a;
	}*/

	// Undo update 2
	a = -0.4435068522;
	im[current] += 2 * a * im[current + inc];
	for (i = current + 2 * inc; i < end - inc; i += 2 * inc) {
		im[i] += a * (im[i - inc] + im[i + inc]);
	}
	if (i < end)
		im[i] += 2 * a * im[i - inc];

	// Undo predict 2
	a = -0.8829110762;
	for (i = current + inc; i < end - inc; i += 2 * inc) {
		im[i] += a * (im[i - inc] + im[i + inc]);
	}
	if (i < end)
		im[i] += 2 * a * im[i - inc];

	// Undo update 1
	a = 0.05298011854;
	im[current] += 2 * a * im[current + inc];
	for (i = current + 2 * inc; i < end - inc; i += 2 * inc) {
		im[i] += a * (im[i - inc] + im[i + inc]);
	}
	if (i < end)
		im[i] += 2 * a * im[i - inc];

	// Undo predict 1
	a = 1.586134342;
	for (i = current + inc; i < end - inc; i += 2 * inc) {
		im[i] += a * (im[i - inc] + im[i + inc]);
	}
	if (i < end)
		im[i] += 2 * a * im[i - inc];
}

void dwt_forward_filter(std::vector<double>& im, const int current, const int inc)
{
	int end = (int)im.size();
	assert(inc < end && "stepping outside source image");

	double a;
	int i;

	// Predict 1
	a = -1.586134342;
	for (i = current + inc; i < end - inc; i += 2 * inc) {
		im[i] += a * (im[i - inc] + im[i + inc]);
	}
	if (i < end)
		im[i] += 2 * a * im[i - inc];

	// Update 1
	a = -0.05298011854;
	im[current] += 2 * a * im[current + inc];
	for (i = current + 2 * inc; i < end - inc; i += 2 * inc) {
		im[i] += a * (im[i - inc] + im[i + inc]);
	}
	if (i < end)
		im[i] += 2 * a * im[i - inc];

	// Predict 2
	a = 0.8829110762;
	for (i = current + inc; i < end - inc; i += 2 * inc) {
		im[i] += a * (im[i - inc] + im[i + inc]);
	}
	if (i < end)
		im[i] += 2 * a * im[i - inc];

	// Update 2
	a = 0.4435068522;
	im[current] += 2 * a * im[current + inc];
	for (i = current + 2 * inc; i < end - inc; i += 2 * inc) {
		im[i] += a * (im[i - inc] + im[i + inc]);
	}
	if (i < end)
		im[i] += 2 * a * im[i - inc];

	// Scale
	/*a = 1 / 1.149604398;
	for (i = current; i < end; i += inc) {
		if (i % (2 * inc)) im[i] *= a;
		else im[i] /= a;
	}*/
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