
#ifndef QUANT_H
#define QUANT_H

#include "common.h"

int apply_dq(int dq_type, int sig_mag_value, int gcli, int gtli);

int quant(sig_mag_data_t* buf, int buf_len, gcli_data_t* gclis, int group_size, int gtli, int dq_type);
int dequant(sig_mag_data_t* buf, int buf_len, gcli_data_t* gclis, int group_size, int gtli, int dq_type);

#endif
