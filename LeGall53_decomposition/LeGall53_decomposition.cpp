// LeGall53_encoding.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

extern "C" {
#include <stdio.h>
#include <malloc.h>
}
#include <vector>
#include <assert.h>
#include "image.h"
#include "LGdwt.h"

int main()
{
    xs_image_t im;
    im.sx[2] = im.sx[1] = im.sx[0] = 1;
    im.sy[2] = im.sy[1] = im.sy[0] = 1;
    im.depth = -1;
    im.comps_array[2] = im.comps_array[1] = im.comps_array[0] = NULL;
    ppm_decode("../19.jxs.ppm", &im);
    std::vector<int32_t> im0(&(im.comps_array[0][0]), &(im.comps_array[0][im.width * im.height]));
    std::vector<int32_t> im1(&(im.comps_array[1][0]), &(im.comps_array[1][im.width * im.height]));
    std::vector<int32_t> im2(&(im.comps_array[2][0]), &(im.comps_array[2][im.width * im.height]));
    dwt_forward(im0, 0);
    dwt_forward(im1, 0);
    dwt_forward(im2, 0);
    return 0;
}
