#ifndef COMMON_H
#define COMMON_H

#include "libjxs_for_XS_precincts_exercise.h"

#include <stdbool.h>
#include <stdint.h>

#if defined(_MSC_VER)
#define INLINE _inline
#else
#define INLINE inline
#endif

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define ABS(a)    ((a) < 0 ? (-a) : (a))

#define MAX_PACKETS (MAX_NBANDS + MAX_NCOMPS * 3 * ((1 << (MAX_NDECOMP_V - 1)) - 1))
#define MAX_SUBPKTS MAX_PACKETS
#define MAX_PREC_COLS (130)
#define MAX_GCLI 15
#define RA_BUDGET_INVALID 0x8000000

typedef xs_data_in_t dwt_data_t;

typedef int8_t gcli_data_t;
typedef int32_t sig_mag_data_in_t;
typedef uint32_t sig_mag_data_t;

#define SIGN_BIT_POSITION (sizeof(sig_mag_data_t) * 8 - 1)
#define SIGN_BIT_MASK (1 << SIGN_BIT_POSITION)

#endif
