#pragma once
#include <stdbool.h>

#include <stdint.h>

typedef int32_t xs_data_in_t;

#define MAX_NDECOMP_H 5
#define MAX_NDECOMP_V 2
#define MAX_NCOMPS 4

#define MAX_NFILTER_TYPES (2 * (MAX_NDECOMP_V) + (MAX_NDECOMP_H) + 1)
#define MAX_NBANDS (MAX_NCOMPS * MAX_NFILTER_TYPES)

typedef struct xs_image_t xs_image_t;
struct xs_image_t
{
	int ncomps;
	int width;
	int height;
	int sx[MAX_NCOMPS];
	int sy[MAX_NCOMPS];
	int depth;
	xs_data_in_t* comps_array[MAX_NCOMPS];
};

