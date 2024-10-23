
#ifndef DWT_H
#define DWT_H

#include "ids.h"

void dwt_inverse_transform(const ids_t* ids, xs_image_t* im);
void dwt_forward_transform(const ids_t* ids, xs_image_t* im);

#endif
