#pragma once
#include "libjxs_for_XS_DWT_exercise.h"
#include "ids.h"
#include <malloc.h>

int image_create(xs_image_t& image, ids_t& ids)
{
    const int WIDTH = 320;
    const int HEIGHT = 240;
    image.ncomps = 3;
    image.depth = 8;
    image.width = WIDTH;
    image.height = HEIGHT;
    ids.ncomps = image.ncomps;
    ids.w = image.width;
    ids.h = image.height;
    ids.sd = 0;
    ids.nlxy.x = 5; // ndecomp_h;
    ids.nlxy.y = 2; // ndecomp_v;

    for (int k = 0; k < image.ncomps; ++k)
    {
        ids.nlxyp[k].x = ids.nlxy.x;
        ids.nlxyp[k].y = ids.nlxy.y; // -(im->sy[c] >> 1); // no CFA defined, all image.sx, image.sy = 1
        ids.comp_w[k] = ids.w;
        ids.comp_h[k] = ids.h;
        image.comps_array[k] = (xs_data_in_t*)malloc(sizeof(xs_data_in_t) * image.width * image.height);
        for (int iy = 0; iy < image.height; ++iy)
        {
            for (int ix = 0; ix < image.width; ++ix)
            {
                image.comps_array[k][iy * image.width + ix] = (uint8_t)(iy * ix / 32); // bands of const color intensity along hyperbolas
                // = ((iy * 8 + ix * 24 / 4)); //((iy % 32) > 16) ? 0 : 255; gives straight // straight 45 degree skew bands 
            }
        }
    }
    return 0;
}