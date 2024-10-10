#pragma once
#include "libjxs_for_XS_precincts_exercise.h"
#include "ids.h"
#include <malloc.h>

int image_create(xs_image_t& image, ids_t& ids)
{
    const int WIDTH = 48;
    const int HEIGHT = 36;

    image.ncomps = 3;
    image.depth = 8;
    image.width = WIDTH;
    image.height = HEIGHT;
    image.sx[0] = image.sx[1] = image.sx[2] = image.sy[0] = image.sy[1] = image.sy[2] = 1;
    for (int k = 0; k < image.ncomps; ++k)
    {
        image.comps_array[k] = (xs_data_in_t*)malloc(sizeof(xs_data_in_t) * image.width * image.height);
        for (int iy = 0; iy < image.height; ++iy)
        {
            for (int ix = 0; ix < image.width; ++ix)
            {
                image.comps_array[k][iy * image.width + ix] = (uint8_t)(iy * ix / 1); // bands of const color intensity along hyperbolas
                // = ((iy * 8 + ix * 24 / 4)); //((iy % 32) > 16) ? 0 : 255; gives straight // straight 45 degree skew bands 
            }
        }
    }

    ids.ncomps = image.ncomps;
    ids.w = image.width;
    ids.h = image.height;
    ids.nbands = 24;
    ids.sd = 0;
    ids.nb = 8;
    ids.nlxy = { 5, 1 }; // { ndecomp_h, ndecomp_v };
    ids.nlxyp[0] = {5, 1};
    ids.nlxyp[1] = {5, 1};
    ids.nlxyp[2] = {5, 1};
    ids.nlxyp[3] = {0, 0};
    for (int i = 0; i < MAX_NFILTER_TYPES - 2; ++i)
    {
        ids.band_idx[0][i] = 3 * i;
        ids.band_idx[1][i] = 3 * i + 1;
        ids.band_idx[2][i] = 3 * i + 2;
        ids.band_idx[3][i] = 0;
    }
    for (int k = 0; k < ids.ncomps; ++k)
    {
        ids.band_d[k][0] = { 5,1 };
        ids.band_d[k][1] = { 5,1 };
        ids.band_d[k][2] = { 4,1 };
        ids.band_d[k][3] = { 3,1 };
        ids.band_d[k][4] = { 2,1 };
        ids.band_d[k][5] = { 1,1 };
        ids.band_d[k][6] = { 1,1 };
        ids.band_d[k][7] = { 1,1 };
        ids.band_dim[k][0] = { 2,18 };
        ids.band_dim[k][1] = { 1,18 };
        ids.band_dim[k][2] = { 3,18 };
        ids.band_dim[k][3] = { 6,18 };
        ids.band_dim[k][4] = { 12,18 };
        ids.band_dim[k][5] = { 24,18 };
        ids.band_dim[k][6] = { 24,18 };
        ids.band_dim[k][7] = { 24,18 };
    }
    ids.band_is_high[0] = { 0,0 };
    ids.band_is_high[1] = { 1,0 };
    ids.band_is_high[2] = { 1,0 };
    ids.band_is_high[3] = { 1,0 };
    ids.band_is_high[4] = { 1,0 };
    ids.band_is_high[5] = { 1,0 };
    ids.band_is_high[6] = { 0,1 };
    ids.band_is_high[7] = { 1,1 };
    for (int k = 0; k < ids.nbands; ++k)
    {
        ids.band_idx_to_c_and_b[k] = { (int8_t)(k % 3), (int8_t)(k / 3) };
    }
    for (int k = 0; k < image.ncomps; ++k)
    {
        ids.comp_w[k] = ids.w;
        ids.comp_h[k] = ids.h;
    }
    ids.band_max_width = 24;
    ids.cs = 48;
    ids.npx = 1;
    ids.npy = 18;
    ids.np = 18;
    ids.pw[0] = 48;
    ids.pw[1] = 48;
    ids.ph = 2;
    
    for (int i = 0; i < 18; ++i)
    {
        ids.pwb[0][i] = 1;
        ids.pwb[1][i] = 1;
        ids.l0[i] = 0;
        ids.l1[0][i] = 1;
        ids.l1[1][i] = 1;
    }
    for (int i = 18; i < ids.nbands; ++i)
    {
        ids.pwb[0][i] = 2;
        ids.pwb[1][i] = 2;
        ids.l0[i] = 1;
        ids.l1[0][i] = 2;
        ids.l1[1][i] = 2;
    }
    // packet inclusion
    for (int i = 0; i < 12; ++i)
    {
        ids.pi[i] = { i, 0, 0 };
    }
    for (int i = 15; i < ids.nbands; ++i)
    {
        ids.pi[i] = { i, (i - ids.nbands / 2) / 6, (i - ids.nbands / 2) / 3 };
    }
    ids.npc = 4;
    ids.npi = 24;
    return 0;
}