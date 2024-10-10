
#ifndef XS_CONFIG_H
#define XS_CONFIG_H

#include "common.h"
#include "ids.h"

//bool xs_config_init(xs_config_t* config, const xs_image_t* image, xs_profile_t profile, xs_level_t level, xs_sublevel_t sublevel);
bool xs_config_resolve_auto_values(xs_config_t* cfg, const xs_image_t* im);
bool xs_config_validate(const xs_config_t* cfg, const xs_image_t* im);
//void xs_config_resolve_bw_fq(xs_config_t* cfg, const xs_nlt_t target_Tnlt, const int im_depth);  // this is also done by xs_config_resolve_auto_values

#endif