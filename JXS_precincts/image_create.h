#pragma once
#include "libjxs_for_XS_precincts_exercise.h"
#include "ids.h"
#include <malloc.h>

int image_create(xs_image_t& image)
{
    const int WIDTH = 3840;
    const int HEIGHT = 2160;

    image.ncomps = 3;
    image.depth = 8;
    image.width = WIDTH;
    image.height = HEIGHT;
    image.sx[0] = image.sx[1] = image.sx[2] = image.sy[0] = image.sy[1] = image.sy[2] = 1;
    for (int k = 0; k < image.ncomps; ++k)
    {
        image.comps_array[k] = (xs_data_in_t*)malloc(sizeof(xs_data_in_t) * image.width * image.height);
    }
    return 0;
}

int image_paint(xs_image_t& image)
{
    for (int k = 0; k < image.ncomps - 1; ++k)
    {
        for (int iy = 0; iy < image.height; ++iy)
        {
            for (int ix = 0; ix < image.width; ++ix)
            {
                image.comps_array[k][iy * image.width + ix] = (uint8_t)(iy * ix / 6400); // bands of const color intensity along hyperbolas
                // = ((iy * 8 + ix * 24 / 4)); //((iy % 32) > 16) ? 0 : 255; gives straight // straight 45 degree skew bands 
            }
        }
    }
    for (int iy = 0; iy < image.height; ++iy)
    {
        for (int ix = 0; ix < image.width; ++ix)
        {
            image.comps_array[2][iy * image.width + ix] = (uint8_t)((iy / 20 + ix * 3 / 40)); //(iy * ix / 64); // bands of const color intensity along hyperbolas
            // = ((iy % 32) > 16) ? 0 : 255; gives straight // straight 45 degree skew bands 
        }
    }
    return 0;
}