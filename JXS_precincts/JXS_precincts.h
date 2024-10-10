#pragma once

struct xs_enc_context_t
{
	const xs_config_t* xs_config;
	ids_t ids;

	precinct_t* precinct[MAX_PREC_COLS];
	//packing_context_t* packer;

	//bit_packer_t* bitstream;
	int bitstream_len;

	rate_control_t* rc[MAX_PREC_COLS];
};

struct rate_control_t
{
	const xs_config_t* xs_config;
	int image_height;
	precinct_t* precinct;
	//precinct_budget_table_t* pbt;
	//predbuffer_t* pred_residuals;
	precinct_t* precinct_top;
	int gc_enabled_modes;

	int nibbles_image;
	int nibbles_report;
	int nibbles_consumed;
	int lines_consumed;

	//ra_params_t ra_params;
	int* gtli_table_data;
	int* gtli_table_gcli;
	int* gtli_table_gcli_prec;
	int gcli_methods_table;
	int* gcli_sb_methods;

	//precinct_budget_info_t* pbinfo;
};