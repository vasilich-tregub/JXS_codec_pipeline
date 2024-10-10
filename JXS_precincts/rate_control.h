
#ifndef SRC_RATE_CONTROL_H
#define SRC_RATE_CONTROL_H

#include "precinct.h"
//#include "predbuffer.h"
//#include "precinct_budget.h"
//#include "gcli_methods.h"

typedef struct rate_control_t rate_control_t;

typedef struct ra_params_t ra_params_t;
struct ra_params_t
{
	uint8_t Rl;

	int budget;
	int all_enabled_methods;

	const uint8_t* lvl_gains;
	const uint8_t* lvl_priorities;
};

typedef struct rc_results_t rc_results_t;
struct rc_results_t
{
	int quantization;
	int refinement;
	int gcli_method;
	int gcli_sb_methods[MAX_NBANDS];

	//precinct_budget_info_t pbinfo;

	int precinct_total_bits;
	int padding_bits;
	int bits_consumed;

	//predbuffer_t* pred_residuals;
	int* gtli_table_data;
	int* gtli_table_gcli;

	int rc_error;
};

rate_control_t* rate_control_open(const xs_config_t* xs_config, const ids_t* ids, int column);
void rate_control_init(rate_control_t* rc, int rate_bytes, int report_bytes);
int rate_control_process_precinct(rate_control_t* rc, precinct_t* precinct, rc_results_t* rc_results);
void rate_control_close(rate_control_t* rc);

#endif
