#ifndef IDS_H
#define IDS_H

#include "common.h"

typedef struct ids_band_idx_to_comp_and_filter_type_t ids_band_idx_to_comp_and_filter_type_t;
typedef struct ids_xy_val_t ids_xy_val_t;
typedef struct ids_dim_val_t ids_dim_val_t;
typedef struct ids_packet_inclusion_t ids_packet_inclusion_t;
typedef struct ids_t ids_t;

struct ids_band_idx_to_comp_and_filter_type_t
{
	int8_t c : 3;
	int8_t b : 5;
};

struct ids_xy_val_t
{
	int8_t x : 4;
	int8_t y : 4;
};

struct ids_dim_val_t
{
	int w;
	int h;
};

struct ids_packet_inclusion_t
{
	int b;
	int y;
	int s;
};

struct ids_t
{
	int8_t ncomps;           /* number of (color?) components in an image*/
	int w;                   /* width of the image in sampling grid points*/
	int h;                   /* height of the image in sampling grid points*/
	int comp_w[MAX_NCOMPS];  /* width of the image component in sampling grid points*/
	int comp_h[MAX_NCOMPS];  /* height of the image component in sampling grid points*/

	int8_t nbands;           /* number of bands in the wavelet decomposition of the image (wavelet filter types times components) */
	int8_t sd;               /* Number of components with suppressed decomposition */
	int8_t nb;

	ids_xy_val_t nlxy;
	ids_xy_val_t nlxyp[MAX_NCOMPS];
	int8_t band_idx[MAX_NCOMPS][MAX_NFILTER_TYPES];  // band index
	ids_band_idx_to_comp_and_filter_type_t band_idx_to_c_and_b[MAX_NBANDS];
	ids_xy_val_t band_d[MAX_NCOMPS][MAX_NFILTER_TYPES];
	ids_xy_val_t band_is_high[MAX_NFILTER_TYPES];
	ids_dim_val_t band_dim[MAX_NCOMPS][MAX_NFILTER_TYPES];
	int band_max_width;

	int cs;
	int npx;
	int npy;
	int np;
	int pw[2];  // width of normal and last
	int ph;     // precinct height(?)
	int pwb[2][MAX_NBANDS];  // widths of normal and last
	int l0[MAX_NBANDS];
	int l1[2][MAX_NBANDS];  // normal and last

	ids_packet_inclusion_t pi[MAX_PACKETS];
	int npc;
	int npi;

	bool use_long_precinct_headers;
};

int ids_calculate_cs(const xs_image_t* im, const int ndecomp_h, const int cw);
int ids_calculate_nbands(const xs_image_t* im, const int ndecomp_h, const int ndecomp_v, const int Sd);

void ids_construct(ids_t* ids, const xs_image_t* im, const int ndecomp_h, const int ndecomp_v, const int sd, const int cw, const int lh);

#endif