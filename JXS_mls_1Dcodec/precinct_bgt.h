#pragma once
#include <stdint.h>
#include "markers.h"

#define MAX_NDECOMP_H 5
#define MAX_NFILTER_TYPES (2 * (MAX_NDECOMP_V) + (MAX_NDECOMP_H) + 1)
#define MAX_NBANDS (MAX_NCOMPS * MAX_NFILTER_TYPES)
#define MAX_PACKETS (MAX_NBANDS + MAX_NCOMPS * 3 * ((1 << (MAX_NDECOMP_V - 1)) - 1))
#define MAX_SUBPKTS MAX_PACKETS

typedef struct precinct_budget_table_t precinct_budget_table_t;
struct precinct_budget_table_t
{
	uint32_t* sigf_budget_table;
	uint32_t* gcli_budget_table;
	uint32_t* data_budget_table;
	uint32_t* sign_budget_table;
	int position_count;
	int method_count;
};

typedef struct precinct_budget_info_t
{
	int precinct_bits;
	uint32_t subpkt_uses_raw_fallback[MAX_SUBPKTS];
	uint32_t prec_header_size;
	uint32_t pkt_header_size[MAX_PACKETS];

	uint32_t subpkt_size_sigf[MAX_SUBPKTS];
	uint32_t subpkt_size_gcli[MAX_SUBPKTS];
	uint32_t subpkt_size_gcli_raw[MAX_SUBPKTS];
	uint32_t subpkt_size_data[MAX_SUBPKTS];
	uint32_t subpkt_size_sign[MAX_SUBPKTS];
} precinct_budget_info_t;

//void precinct_get_best_gcli_method(precinct_t* precinct, precinct_budget_table_t* pbt, int* gtli_table_gcli, int* res_gcli_sb_methods);

void precinct_get_budget(
	//precinct_t* precinct,
	precinct_budget_table_t* pbt,
	int* gtli_table_gcli,
	int* gtli_table_data,
	//const uint8_t Rl,
	//int* res_gcli_sb_methods,
	precinct_budget_info_t* result);

precinct_budget_table_t* pbt_open(int position_count, int method_count);
