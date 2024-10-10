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

void swap_ptrs(precinct_t** p1, precinct_t** p2)
{
	precinct_t* tmp = *p1;
	*p1 = *p2;
	*p2 = tmp;
}

struct xs_dec_context_t
{
	const xs_config_t* xs_config;
	ids_t ids;

	precinct_t* precinct[MAX_PREC_COLS];
	precinct_t* precinct_top[MAX_PREC_COLS];
	int	gtlis_table_top[MAX_PREC_COLS][MAX_NBANDS];

	//bit_unpacker_t* bitstream;
	//unpacking_context_t* unpack_ctx;

#ifdef PACKING_GENERATE_FRAGMENT_CODE
	xs_fragment_info_cb_t user_fragment_info_cb;
	void* user_fragment_info_context;
	xs_buffering_fragment_t fragment_info_buf;
#endif
};
