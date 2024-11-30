#pragma once
typedef struct xs_image_t xs_image_t;
struct xs_image_t
{
	int ncomps;
	int width;
	int height;
	int sx[4];
	int sy[4];
	int depth;
	uint32_t* comps_array[4];
};

bool xs_allocate_image(xs_image_t* im, const bool set_zero)
{
	for (int c = 0; c < im->ncomps; c++)
	{
		assert(im->comps_array[c] == NULL);
		assert(im->sx[c] == 1 || im->sx[c] == 2);
		assert(im->sy[c] == 1 || im->sy[c] == 2);
		const size_t sample_count = (size_t)(im->width / im->sx[c]) * (size_t)(im->height / im->sy[c]);
		im->comps_array[c] = (uint32_t*)malloc(sample_count * sizeof(uint32_t));
		if (im->comps_array[c] == NULL)
		{
			return false;
		}
		if (set_zero)
		{
			memset(im->comps_array[c], 0, sample_count * sizeof(uint32_t));
		}
	}
	return true;
}


#ifdef _MSC_VER
#include <intrin.h>
static unsigned int __inline BSR(unsigned long x)
{
	unsigned long r = 0;
	_BitScanReverse(&r, x);
	return r;
}
#else
#define BSR(x) (31 - __builtin_clz(x))
#endif

static int SkipBlanks(FILE* fp)
{
	int ch;

	do
	{
		ch = fgetc(fp);
	} while (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r');

	return ungetc(ch, fp);
}

static int ReadNumber(FILE* fp, int* result)
{
	int number = 0;
	int negative = 0;
	int valid = 0;
	int in;

	if (SkipBlanks(fp) == EOF)
		return 0;

	in = fgetc(fp);
	if (in == '+')
	{

	}
	else if (in == '-')
	{

		negative = 1;
	}
	else {

		if (ungetc(in, fp) == EOF)
			return 0;
	}

	if (SkipBlanks(fp) == EOF)
		return 0;

	do
	{
		in = fgetc(fp);
		if (in < '0' || in > '9')
			break;

		if (number >= 214748364)
		{
			fprintf(stderr, "Invalid number in PPM file, number %d too large.\n", number);
			return 0;
		}

		number = number * 10 + in - '0';
		valid = 1;
	} while (1);

	if (ungetc(in, fp) == EOF)
		return 0;

	if (!valid)
	{
		fprintf(stderr, "Invalid number in PPM file, found no valid digit. First digit code is %d\n", in);
		return 0;
	}

	if (negative)
		number = -number;

	*result = number;
	return 1;
}


static int SkipComment(FILE* fp)
{
	int c;

	do
	{
		c = fgetc(fp);

	} while (c == ' ' || c == '\t' || c == '\r');


	if (c == '\n')
	{

		do
		{
			c = fgetc(fp);
			if (c == '#')
			{

				do
				{
					c = fgetc(fp);
				} while (c != '\n' && c != '\r' && c != -1);
			}
			else {

				if (ungetc(c, fp) == EOF)
					return EOF;
				break;
			}
		} while (1);
	}
	else
	{
		return ungetc(c, fp);
	}
	return 0;
}

int ppm_decode(const char* filename, xs_image_t* im)
{
	int data;
	int result = -1;
	int precision = 0;
	int components = 0;
	int width = 0;
	int height = 0;
	int raw = 1;
	uint32_t* p[3] = { NULL };
	int x, y, c;
	FILE* fp;
	fopen_s(&fp, filename, "rb");

	if (fp == NULL)
	{
		perror("unable to open the source image stream");
		return -1;
	}

	data = fgetc(fp);
	if (data != 'P')
	{
		fprintf(stderr, "input stream %s is not a valid PPM file\n", filename);
		goto exit;
	}

	data = fgetc(fp);
	precision = 0;
	switch (data)
	{
	case '6':

		components = 3;
		break;
	case '5':

		components = 1;
		break;
	default:

		fprintf(stderr, "input stream %s is not P6, P5 PPM file\n", filename);
		goto exit;
	}


	if (SkipComment(fp) == EOF)
		goto exit;
	if (ReadNumber(fp, &width) == 0)
		goto exit;
	if (SkipComment(fp) == EOF)
		goto exit;
	if (ReadNumber(fp, &height) == 0)
		goto exit;

	if (precision == 0)
	{
		if (SkipComment(fp) == EOF)
			goto exit;
		if (ReadNumber(fp, &precision) == 0)
			goto exit;
	}


	im->ncomps = components;
	im->width = width;
	im->height = height;
	im->sx[0] = im->sx[1] = im->sx[2] = 1;
	im->sy[0] = im->sy[1] = im->sy[2] = 1;

	if (im->depth == -1)
		im->depth = BSR(precision) + 1;

	if (!xs_allocate_image(im, false))
		goto exit;

	for (c = 0; c < components; c++)
		p[c] = im->comps_array[c];

	data = fgetc(fp);

	if (data == '\r')
	{
		data = fgetc(fp);
		if (data != '\n')
		{

			if (ungetc(data, fp) == EOF)
				goto exit;
			data = '\r';
		}
	}

	if (data != ' ' && data != '\n' && data != '\r' && data != '\t')
	{
		fprintf(stderr, "Malformed PPM stream in %s.\n", filename);
		goto exit;
	}

	if (precision < 256)
	{

		for (y = 0; y < height; y++)
		{
			for (x = 0; x < width; x++)
			{
				for (c = 0; c < components; c++)
				{
					data = fgetc(fp);
					if (data == EOF)
					{
						fprintf(stderr, "Unexpected EOF in PPM stream %s.\n", filename);
						goto exit;
					}
					*p[c]++ = data;
				}
			}
		}
	}
	else if (precision < 65536)
	{

		for (y = 0; y < height; y++)
		{
			for (x = 0; x < width; x++)
			{
				for (c = 0; c < components; c++)
				{
					int lo, hi;
					hi = fgetc(fp);
					lo = fgetc(fp);
					if (lo == EOF || hi == EOF)
					{
						fprintf(stderr, "Unexpected EOF in PPM stream %s.\n", filename);
						goto exit;
					}
					*p[c]++ = lo | (hi << 8);
				}
			}
		}
	}
	else
	{
		fprintf(stderr, "Malformed PPM stream %s, invalid precision.\n", filename);
		goto exit;
	}
	result = 0;

exit:
	if (ferror(fp))
	{
		perror("error while reading the input stream");
		result = -1;
	}
	fclose(fp);

	return result;
}
