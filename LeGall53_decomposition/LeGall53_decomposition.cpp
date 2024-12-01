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
#include "png_readwrite.h"

int main()
{
    xs_image_t im;
    im.sx[2] = im.sx[1] = im.sx[0] = 1;
    im.sy[2] = im.sy[1] = im.sy[0] = 1;
    im.depth = -1;
    im.comps_array[2] = im.comps_array[1] = im.comps_array[0] = NULL;
    ppm_decode("../19.jxs.ppm", &im);
    int h_level = 5;
    int v_level = 2;

    std::vector<int32_t> im0(&(im.comps_array[0][0]), &(im.comps_array[0][im.width * im.height]));
    std::vector<int32_t> im1(&(im.comps_array[1][0]), &(im.comps_array[1][im.width * im.height]));
    std::vector<int32_t> im2(&(im.comps_array[2][0]), &(im.comps_array[2][im.width * im.height]));
    // upscale image data to 20 bit precision
    uint8_t Bw = 20;
    const uint8_t s = Bw - (uint8_t)im.depth;
    const int32_t dclev = ((1 << Bw) >> 1); // DC level
    for (int i = 0; i < im.width * im.height; ++i)
    {
        im0[i] = im0[i] << s;
        im1[i] = im1[i] << s;
        im2[i] = im2[i] << s;
    }
    dwt_forward_transform(im0, im.width, im.height, h_level, v_level);
    dwt_forward_transform(im1, im.width, im.height, h_level, v_level);
    dwt_forward_transform(im2, im.width, im.height, h_level, v_level);
    for (int i = 0; i < im.width * im.height; ++i)
    {
        im0[i] = im0[i] >> s;
        im1[i] = im1[i] >> s;
        im2[i] = im2[i] >> s;
    }
    std::vector<uint32_t> buf(im.width * im.height);
    for (int i = 0; i < im.width * im.height; ++i)
    {
        int rs = (1 << im.depth) / 256; // right shift, 12 bit positions
        buf[i] =
            (uint8_t)(im2[i] / rs) * 256 * 256 + // red
            (uint8_t)(im1[i] / rs) * 256 + // green
            (uint8_t)(im0[i] / rs); // blue
    }
    if (png_write(im.width, im.height, buf, L"decomposition_after_inline_DWT.png"))
        printf("cannot write image file 'decomposition_after_inline_DWT.png'\n");
    return 0;
}
