#include "markers.h"

#define XS_MARKER_SOC 0xff10
#define XS_MARKER_EOC 0xff11
#define XS_MARKER_PIH 0xff12
#define XS_MARKER_CDT 0xff13
#define XS_MARKER_WGT 0xff14
#define XS_MARKER_COM 0xff15
#define XS_MARKER_NLT 0xff16
#define XS_MARKER_CWD 0xff17
#define XS_MARKER_CTS 0xff18
#define XS_MARKER_CRG 0xff19
#define XS_MARKER_SLH 0xff20
#define XS_MARKER_CAP 0xff50

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

int write_capabilities_marker(bit_packer_t* bitstream, image_t* im, const config_t* cfg)
{
	//assert(cfg->cap_bits != XS_CAP_AUTO);

	// CAP marker
	int nbits = 0;
	nbits += bitpacker_write(bitstream, XS_MARKER_CAP, XS_MARKER_NBITS);

	if ((cfg->cap_bits & 0xffff) == 0)
	{
		nbits += bitpacker_write(bitstream, 2, XS_MARKER_NBITS);
	}
	else if ((cfg->cap_bits & 0xff) == 0)
	{
		nbits += bitpacker_write(bitstream, 3, XS_MARKER_NBITS);
		nbits += bitpacker_write(bitstream, cfg->cap_bits >> 8, 8);
	}
	else
	{
		nbits += bitpacker_write(bitstream, 4, XS_MARKER_NBITS);
		nbits += bitpacker_write(bitstream, cfg->cap_bits, 16);
	}
	return nbits;
}

int write_picture_header(bit_packer_t* bitstream, image_t* im, const config_t* cfg)
{
	int nbits = 0;
	const int lpih = 26;

	//assert(cfg->profile != XS_PROFILE_AUTO);
	//assert(cfg->level != XS_LEVEL_AUTO);
	//assert(cfg->sublevel != XS_SUBLEVEL_AUTO);

	nbits += bitpacker_write(bitstream, XS_MARKER_PIH, XS_MARKER_NBITS);
	nbits += bitpacker_write(bitstream, lpih, XS_MARKER_NBITS);
	if (cfg->bitstream_size_in_bytes == (size_t)-1)
	{
		// Write zeroes as we do not (yet) know the final codestream size.
		nbits += bitpacker_write(bitstream, 0, 32);
	}
	else
	{
		//assert(cfg->bitstream_size_in_bytes != 0 && cfg->bitstream_size_in_bytes != (size_t)-1);
		nbits += bitpacker_write(bitstream, cfg->bitstream_size_in_bytes, 32);
	}
	nbits += bitpacker_write(bitstream, cfg->profile, 16);  // Ppih
	nbits += bitpacker_write(bitstream, (((uint16_t)cfg->level) << 8) | ((uint16_t)cfg->sublevel), 16);  // Plev
	nbits += bitpacker_write(bitstream, im->width, 16);
	nbits += bitpacker_write(bitstream, im->height, 16);
	nbits += bitpacker_write(bitstream, cfg->p.Cw, 16);
	nbits += bitpacker_write(bitstream, cfg->p.slice_height / (1 << (cfg->p.NLy)), 16);
	nbits += bitpacker_write(bitstream, im->ncomps, 8);
	nbits += bitpacker_write(bitstream, cfg->p.N_g, 8);
	nbits += bitpacker_write(bitstream, cfg->p.S_s, 8);
	nbits += bitpacker_write(bitstream, cfg->p.Bw, 8);
	nbits += bitpacker_write(bitstream, cfg->p.Fq, 4);
	nbits += bitpacker_write(bitstream, cfg->p.B_r, 4);
	nbits += bitpacker_write(bitstream, cfg->p.Fslc, 1);
	nbits += bitpacker_write(bitstream, cfg->p.Ppoc, 3);
	nbits += bitpacker_write(bitstream, ((uint8_t)cfg->p.color_transform & 0xf), 4);
	nbits += bitpacker_write(bitstream, cfg->p.NLx, 4);
	nbits += bitpacker_write(bitstream, cfg->p.NLy, 4);
	nbits += bitpacker_write(bitstream, cfg->p.Lh, 1);
	nbits += bitpacker_write(bitstream, cfg->p.Rl, 1);
	nbits += bitpacker_write(bitstream, cfg->p.Qpih, 2);
	nbits += bitpacker_write(bitstream, cfg->p.Fs, 2);
	nbits += bitpacker_write(bitstream, cfg->p.Rm, 2);
	return nbits;
}

int write_component_table(bit_packer_t* bitstream, image_t* im)
{
	int nbits = 0;
	int lcdt = 2 * im->ncomps + 2;
	int comp;

	nbits += bitpacker_write(bitstream, XS_MARKER_CDT, XS_MARKER_NBITS);
	nbits += bitpacker_write(bitstream, lcdt, XS_MARKER_NBITS);

	for (comp = 0; comp < im->ncomps; comp++)
	{
		nbits += bitpacker_write(bitstream, im->depth, 8);
		nbits += bitpacker_write(bitstream, /*im->sx[comp]*/1, 4);
		nbits += bitpacker_write(bitstream, /*im->sy[comp]*/1, 4);
	}
	return nbits;
}

int write_weights_table(bit_packer_t* bitstream, image_t* im, const config_t* cfg)
{
	int nbits = 0;
	int Nl = 0;
	for (; Nl < MAX_NBANDS && cfg->p.lvl_gains[Nl] != 0xff; ++Nl);
	//assert(cfg->p.lvl_gains[Nl] == 0xff && cfg->p.lvl_priorities[Nl] == 0xff);

	nbits += bitpacker_write(bitstream, XS_MARKER_WGT, XS_MARKER_NBITS);
	nbits += bitpacker_write(bitstream, 2ull * Nl + 2, XS_MARKER_NBITS);
	for (int lvl = 0; lvl < Nl; ++lvl)
	{
		//assert(cfg->p.lvl_gains[lvl] != 0xff && cfg->p.lvl_priorities[lvl] != 0xff);
		nbits += bitpacker_write(bitstream, cfg->p.lvl_gains[lvl], 8);
		nbits += bitpacker_write(bitstream, cfg->p.lvl_priorities[lvl], 8);
	}

	return nbits;
}

int write_head(bit_packer_t* bitstream, image_t* im, const config_t* cfg)
{
	//assert(cfg != NULL && im != NULL);
	int nbits = 0;
	nbits += bitpacker_write(bitstream, XS_MARKER_SOC, XS_MARKER_NBITS);
	nbits += write_capabilities_marker(bitstream, im, cfg);
	nbits += write_picture_header(bitstream, im, cfg);
	nbits += write_component_table(bitstream, im);
	nbits += write_weights_table(bitstream, im, cfg);
	//nbits += xs_write_nlt_marker(bitstream, cfg);
	//nbits += xs_write_cwd_marker(bitstream, cfg);
	//nbits += xs_write_cts_marker(bitstream, cfg);
	//nbits += xs_write_crg_marker(bitstream, im, cfg);
	//nbits += xs_write_com_encoder_identification(bitstream);
	return nbits;
}

int write_slice_header(bit_packer_t* bitstream, int slice_idx)
{
	int nbits = 0;
	nbits += bitpacker_write(bitstream, XS_MARKER_SLH, XS_MARKER_NBITS);
	nbits += bitpacker_write(bitstream, 4, XS_MARKER_NBITS);
	nbits += bitpacker_write(bitstream, slice_idx, 16);
	//assert(nbits == XS_SLICE_HEADER_NBYTES * 8);
	return nbits;
}

// for JXS CODEC dissection only
void init_config(config_t* config)
{
	config->bitstream_size_in_bytes = -1;// (size_t)(bpp * image.width * image.height / 8);
	config->budget_report_lines = 20.0f;
	config->verbose = 255;
	config->gains_mode = 2;// XS_GAINS_OPT_EXPLICIT;
	config->profile = 0x6ec0;// XS_PROFILE_MLS_12; // XS_PROFILE_UNRESTRICTED;
	config->level = 0;// XS_LEVEL_UNRESTRICTED;
	config->sublevel = 0;// XS_SUBLEVEL_UNRESTRICTED;
	config->cap_bits = 0x0200;// XS_CAP_MLS;
	config->p.color_transform = 0; // XS_CPIH_NONE;
	config->p.Cw = 0;
	config->p.slice_height = 0x0010;
	config->p.N_g = 4;
	config->p.S_s = 8;
	config->p.Bw = 255;
	config->p.Fq = 0; // fractional bits; 8 in xs_enc_generated/monochrome; must be 0 in mls
	config->p.B_r = 4;
	config->p.Fslc = 0;
	config->p.Ppoc = 0;
	config->p.NLx = 1;
	config->p.NLy = 0;
	config->p.Lh = 0;
	config->p.Rl = 0; // 1 is raw mode selection per packet; 0 is no RAW_PER_PKT
	config->p.Qpih = 0; // 0 deadzone; 1 uniform
	config->p.Fs = 0;
	config->p.Rm = 0;
	config->p.Sd = 0;
	memset(config->p.lvl_gains, 255, (MAX_NBANDS + 1) * sizeof(uint8_t));
	if (config->p.NLx == 1)
		xs_parse_u8array_(config->p.lvl_gains, MAX_NBANDS, "1,1", 0); //  "1,0,0,1,0,0" "1,1,0,0,0,0"; "2,2,2,1,1,1"
	else if (config->p.NLx == 2)
		xs_parse_u8array_(config->p.lvl_gains, MAX_NBANDS, "1,1,1", 0); //  "1,0,0,1,0,0" "1,1,0,0,0,0"; "2,2,2,1,1,1"
	else if (config->p.NLx == 3)
		xs_parse_u8array_(config->p.lvl_gains, MAX_NBANDS, "1,1,1,1", 0); //  "1,0,0,1,0,0" "1,1,0,0,0,0"; "2,2,2,1,1,1"
	else if (config->p.NLx == 4)
		xs_parse_u8array_(config->p.lvl_gains, MAX_NBANDS, "1,1,1,1,1", 0); //  "1,0,0,1,0,0" "1,1,0,0,0,0"; "2,2,2,1,1,1"
	else if (config->p.NLx == 5)
		xs_parse_u8array_(config->p.lvl_gains, MAX_NBANDS, "0,0,0,0,0,0", 0); //  "1,0,0,1,0,0" "1,1,0,0,0,0"; "2,2,2,1,1,1"
	memset(config->p.lvl_priorities, 255, (MAX_NBANDS + 1) * sizeof(uint8_t));
	if (config->p.NLx == 1)
		xs_parse_u8array_(config->p.lvl_priorities, MAX_NBANDS, "0,1", 0); // "0,2,3,1,4,5" "0,1,2,4,3,5"; "8,7,6,5,3,4"
	else if (config->p.NLx == 2)
		xs_parse_u8array_(config->p.lvl_priorities, MAX_NBANDS, "0,1,2", 0); // "0,2,3,1,4,5" "0,1,2,4,3,5"; "8,7,6,5,3,4"
	else if (config->p.NLx == 3)
		xs_parse_u8array_(config->p.lvl_priorities, MAX_NBANDS, "0,1,2,3", 0); // "0,2,3,1,4,5" "0,1,2,4,3,5"; "8,7,6,5,3,4"
	else if (config->p.NLx == 4)
		xs_parse_u8array_(config->p.lvl_priorities, MAX_NBANDS, "0,1,2,3,4", 0); // "0,2,3,1,4,5" "0,1,2,4,3,5"; "8,7,6,5,3,4"
	else if (config->p.NLx == 5)
		xs_parse_u8array_(config->p.lvl_priorities, MAX_NBANDS, "0,0,0,0,0,0", 0); // "0,2,3,1,4,5" "0,1,2,4,3,5"; "8,7,6,5,3,4"
	config->p.Tnlt = 0;// XS_NLT_NONE;
	config->p.Tnlt_params.quadratic.sigma = 0; config->p.Tnlt_params.quadratic.alpha = 0;
	config->p.Tnlt_params.extended.T1 = 0; config->p.Tnlt_params.extended.T2 = 0; config->p.Tnlt_params.extended.E = 0;
	config->p.tetrix_params.Cf = /*XS_TETRIX_FULL*/0; config->p.tetrix_params.e1 = 0; config->p.tetrix_params.e2 = 0;
	config->p.cfa_pattern = 0;// XS_CFA_RGGB;

}
// end of `for JXS CODEC dissection only`
bool xs_parse_u8array_(uint8_t* values, int max_items, const char* cfg_str, int* num)
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
