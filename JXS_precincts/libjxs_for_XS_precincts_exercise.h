#pragma once
#include <stdbool.h>

#include <stdint.h>

typedef int32_t xs_data_in_t;

#define MAX_NDECOMP_H 5
#define MAX_NDECOMP_V 2
#define MAX_NCOMPS 4

#define MAX_NFILTER_TYPES (2 * (MAX_NDECOMP_V) + (MAX_NDECOMP_H) + 1)
#define MAX_NBANDS (MAX_NCOMPS * MAX_NFILTER_TYPES)

typedef struct xs_config_parameters_t xs_config_parameters_t;
struct xs_config_parameters_t
{
	// Refer to xs_config.c when making any changes to this struct!
	//xs_cpih_t color_transform;
	uint16_t Cw;  // column width
	uint16_t slice_height;  // slice height in lines
	uint8_t N_g;  // = 4
	uint8_t S_s;  // = 8
	uint8_t Bw;
	uint8_t Fq;  // 8, 6, or 0
	uint8_t B_r;  // = 4
	uint8_t Fslc;  // = 0
	uint8_t Ppoc;  // = 0
	uint8_t NLx;  // = 1 to 8
	uint8_t NLy;  // = maxi(log2(sy[i])) to min(NLx, 6)
	uint8_t Lh;  // long precinct header enforcement flag (0 or 1)
	uint8_t Rl;  // raw-mode selection per packet flag (0 or 1)
	uint8_t Qpih;  // = 0 (dead-zone), or 1 (uniform)
	uint8_t Fs;  // sign handling strategy, = 0 (jointly), or 1 (separate)
	uint8_t Rm;  // run mode, = 0 (zero prediction residuals), or 1 (zero coefficients)
	// CWD (optional)
	uint8_t Sd;  // wavelet decomposition supression
	// WGT
	uint8_t lvl_gains[MAX_NBANDS + 1];  // 0xff-terminated
	uint8_t lvl_priorities[MAX_NBANDS + 1];  // 0xff-terminated
	// NLT (optional)
	//xs_nlt_t Tnlt;
	//xs_nlt_parameters_t Tnlt_params;
	// CTS (when Cpih is TETRIX)
	//xs_cts_parameters_t tetrix_params;
	// CRG (when Cpih is TETRIX)
	//xs_cfa_pattern_t cfa_pattern;
};

typedef struct xs_config_t xs_config_t;
struct xs_config_t
{
	size_t bitstream_size_in_bytes;  // target size of complete codestream in bytes, (size_t)-1 is used for MLS (infinite budget)
	float budget_report_lines;  // used for rate allocation

	int verbose; // higher is more
	//xs_gains_mode_t gains_mode; // not really an XS option

	//xs_profile_t profile;
	//xs_level_t level;
	//xs_sublevel_t sublevel;
	//xs_cap_t cap_bits;

	xs_config_parameters_t p;
};


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

typedef struct xs_enc_context_t xs_enc_context_t;