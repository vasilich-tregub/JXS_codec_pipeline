
#ifndef SRC_PRECINCT_H
#define SRC_PRECINCT_H

#include "assert.h"

#include "buf_mgmt.h"
#include "libjxs_for_XS_precincts_exercise.h"
#include "ids.h"

#define MAX_PRECINCT_HEIGHT (1 << MAX_NDECOMP_V)

typedef struct precinct_t precinct_t;

precinct_t* precinct_open_column(const ids_t* ids, int group_size, int column);
void precinct_close(precinct_t* prec);

void copy_gclis(precinct_t* dest, const precinct_t* src);

void precinct_from_image(precinct_t* prec, xs_image_t* img, const uint8_t Fq);
void precinct_to_image(const precinct_t* prec, xs_image_t* target, const uint8_t Fq);

void precinct_set_y_idx_of(precinct_t* prec, int y_idx);

int bands_count_of(const precinct_t* prec);
int line_count_of(const precinct_t* prec);

sig_mag_data_t* precinct_line_of(const precinct_t* prec, int band_index, int ypos);
size_t precinct_width_of(const precinct_t* prec, int band_index);

gcli_data_t* precinct_gcli_of(const precinct_t* prec, int band_index, int ypos);
gcli_data_t* precinct_gcli_top_of(const precinct_t* prec, const precinct_t* prec_top, int lvl, int ypos);

int precinct_gcli_width_of(const precinct_t* prec, int band_index);
int precinct_max_gcli_width_of(const precinct_t* prec);

int precinct_get_gcli_group_size(const precinct_t* prec);

void update_gclis(precinct_t* prec);

void quantize_precinct(precinct_t* prec, const int* gtli, int dq_type);
void dequantize_precinct(precinct_t* prec, const int* gtli, int dq_type);

int precinct_copy(precinct_t* dest, const precinct_t* src);

int precinct_is_first_of_slice(const precinct_t* prec, int slice_height);
int precinct_is_last_of_image(const precinct_t* prec, int im_height);

int precinct_spacial_lines_of(const precinct_t* prec, int im_height);
int precinct_is_line_present(precinct_t* prec, int lvl, int ypos);

int precinct_band_index_of(const precinct_t* prec, int position);
int precinct_ypos_of(const precinct_t* prec, int position);
int precinct_subpkt_of(const precinct_t* prec, int position);
int precinct_nb_subpkts_of(const precinct_t* prec);
int precinct_position_of(const precinct_t* prec, int lvl, int ypos);
int precinct_in_band_height_of(const precinct_t* prec, int band_index);

bool precinct_use_long_headers(const precinct_t* prec);

#endif
