#pragma once
#include <malloc.h>
#include "SvtJpegxs.h"

int image_create(xs_image_t& image)
{
    const int WIDTH = 1920; // 3840;
    const int HEIGHT = 1080; // 2160;

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
    for (int iy = 0; iy < image.height; ++iy)
    {
        for (int ix = 0; ix < image.width; ++ix)
        {
            image.comps_array[0][iy * image.width + ix] = (uint8_t)((iy / 10 - ix * 3 / 20 + 40)); // bands of const color intensity along hyperbolas
            // = ((iy * 8 + ix * 24 / 4)); //((iy % 32) > 16) ? 0 : 255; gives straight // straight 45 degree skew bands 
        }
    }
    for (int iy = 0; iy < image.height; ++iy)
    {
        for (int ix = 0; ix < image.width; ++ix)
        {
            image.comps_array[1][iy * image.width + ix] = (uint8_t)(iy * ix / 1600); // bands of const color intensity along hyperbolas
            // = ((iy * 8 + ix * 24 / 4)); //((iy % 32) > 16) ? 0 : 255; gives straight // straight 45 degree skew bands 
        }
    }
    for (int iy = 0; iy < image.height; ++iy)
    {
        for (int ix = 0; ix < image.width; ++ix)
        {
            image.comps_array[2][iy * image.width + ix] = (uint8_t)((iy / 10 + ix * 3 / 20)); //(iy * ix / 64); // bands of const color intensity along hyperbolas
            // = ((iy % 32) > 16) ? 0 : 255; gives straight // straight 45 degree skew bands 
        }
    }
    return 0;
}

int image_clean(xs_image_t& image, uint8_t val = 0)
{
    for (int iy = 0; iy < image.height; ++iy)
    {
        for (int ix = 0; ix < image.width; ++ix)
        {
            image.comps_array[0][iy * image.width + ix] = val;
            image.comps_array[1][iy * image.width + ix] = val;
            image.comps_array[2][iy * image.width + ix] = val;
        }
    }
    return 0;
}