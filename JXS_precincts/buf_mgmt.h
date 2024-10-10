
#ifndef BUF_MGMT_H
#define BUF_MGMT_H

#include <malloc.h>
#include <stdio.h>
#include <stdint.h>
#include "common.h"

enum {
	DUMP_MODE_NONE,
	DUMP_MODE_TXT,
	DUMP_MODE_BIN,
};

#define DEFAULT_DUMP_MODE DUMP_MODE_NONE

typedef struct multi_buf_t
{
	int magic;
	int n_buffers;
	int* sizes;
	int* sizes_byte;
	size_t element_size;
	union
	{
		int8_t** s8;
		uint8_t** u8;
		int16_t** s16;
		uint16_t** u16;
		int32_t** s32;
		uint32_t** u32;
	} bufs;
	FILE** fout;
	int dump_mode;

	void* storage;
} multi_buf_t;

multi_buf_t* multi_buf_create(int n_buffers, size_t element_size);
multi_buf_t* multi_buf_create_and_allocate(int n_buffers, int size, size_t element_size);
int multi_buf_enable_dump(multi_buf_t* mb, int binary);
int multi_buf_set_size(multi_buf_t* mb, int buf_idx, int size);
int multi_buf_allocate(multi_buf_t* mb, int alignment);
int multi_buf_reset(multi_buf_t* mb);
int multi_buf_destroy(multi_buf_t* mb);

enum
{
	MB_DUMP_U16,
	MB_DUMP_S16,
	MB_DUMP_HEX_16,
	MB_DUMP_HEX_32,
	MB_DUMP_S32,
	MB_DUMP_U32,
};

static INLINE int idx2d(int d1, int d2, int len2)
{
	return d1 * len2 + d2;
}

static INLINE int idx3d(int d1, int d2, int d3, int len2, int len3)
{
	return d1 * len2 * len3 + d2 * len3 + d3;
}

static INLINE int idx4d(int d1, int d2, int d3, int d4, int len2, int len3, int len4)
{
	return d1 * len2 * len3 * len4 + d2 * len3 * len4 + d3 * len4 + d4;
}

#endif
