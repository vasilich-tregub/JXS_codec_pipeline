// CDF97_encoding.cpp : This file contains the 'main' function. Program execution begins and ends there.
// TODO: no png, read 12-bit image from JXS or PPM

extern "C" {
#include <stdio.h>
#include <malloc.h>
}
#include <vector>
#include <assert.h>
#include "image.h"
#include "CDF97dwt.h"
#include "png_readwrite.h"

int main()
{
    xs_image_t im;
    im.sx[2] = im.sx[1] = im.sx[0] = 1;
    im.sy[2] = im.sy[1] = im.sy[0] = 1;
    im.depth = -1;
    im.comps_array[2] = im.comps_array[1] = im.comps_array[0] = NULL;
    ppm_decode("../19.jxs.ppm", &im);
    int h_level = 1;
    int v_level = 1;

    std::vector<double> im0(im.width * im.height);
    std::vector<double> im1(im.width * im.height);
    std::vector<double> im2(im.width * im.height);
    for (int i = 0; i < im.width * im.height; ++i)
    {
        im0[i] = (double)im.comps_array[0][i];
        im1[i] = (double)im.comps_array[1][i];
        im2[i] = (double)im.comps_array[2][i];
    }
    dwt_forward_transform(im0, im.width, im.height, h_level, v_level);
    dwt_forward_transform(im1, im.width, im.height, h_level, v_level);
    dwt_forward_transform(im2, im.width, im.height, h_level, v_level);
    std::vector<uint32_t> buf(im.width * im.height);
    for (int i = 0; i < im.width * im.height; ++i)
    {
        int rs = (1 << (im.depth - 7)); // right shift, bitdepth - 8 bit positions
        buf[i] =
            (uint8_t)(im2[i] / rs) * 256 * 256 + // red
            (uint8_t)(im1[i] / rs) * 256 + // green
            (uint8_t)(im0[i] / rs); // blue
    }
    if (png_write(im.width, im.height, buf, L"decomposition_after_inline_DWT.png"))
        printf("cannot write image file 'decomposition_after_inline_DWT.png'\n");
    // de-interleave coefficients for test imaging into a buffer 'buf' without altering image 'im':     
    int stride = im.width;
    int rs = 1 << (im.depth - 7); // right shift, bitdepth positions - 1
    int dwidth = im.width / 2;
    int dheight = im.height / 2;
    for (int lvl = 1; lvl <= v_level; ++lvl)
    {
        int d = 1 << (lvl - 1);
        for (int iy = 0; iy < dheight / d; ++iy)
        {
            for (int ix = 0; ix < dwidth / d; ++ix)
            {
                int stride = im.width;
                int iCC = d * 2 * iy * stride + d * 2 * ix; // X, Y multiples of 1 << lvl
                int iLL = iy * stride + ix;
                int iLH = iy * stride + ix + dwidth / d;
                int iHL = (iy + dheight / d) * stride + ix;
                int iHH = (iy + dheight / d) * stride + ix + dwidth / d;
                buf[iLL] = // Approximation coeffs
                    (uint8_t)(im2[iCC] / rs) * 256 * 256 + // red
                    (uint8_t)(im1[iCC] / rs) * 256 + // green
                    (uint8_t)(im0[iCC] / rs); // blue
                buf[iLH] = // horizontal detail coeffs
                    (uint8_t)(16 * im2[iCC + d] / rs) * 256 * 256 + // red // *16: enhance details brightness
                    (uint8_t)(16 * im1[iCC + d] / rs) * 256 + // green // *16: enhance details brightness
                    (uint8_t)(16 * im0[iCC + d] / rs); // blue // *16: enhance details brightness
                buf[iHL] = // vertical detail coeffs
                    (uint8_t)(16 * im2[iCC + stride * d] / rs) * 256 * 256 + // red // *16: enhance details brightness
                    (uint8_t)(16 * im1[iCC + stride * d] / rs) * 256 + // green // *16: enhance details brightness
                    (uint8_t)(16 * im0[iCC + stride * d] / rs); // blue // *16: enhance details brightness
                buf[iHH] = // diagonal detail coeffs
                    (uint8_t)(16 * im2[iCC + stride * d + d] / rs) * 256 * 256 + // red // *16: enhance details brightness
                    (uint8_t)(16 * im1[iCC + stride * d + d] / rs) * 256 + // green // *16: enhance details brightness
                    (uint8_t)(16 * im0[iCC + stride * d + d] / rs); // blue // *16: enhance details brightness
            }
        }
    }
    for (int lvl = v_level + 1; lvl <= h_level; ++lvl)
    {
        int d = 1 << (lvl - 1);           // d only for horizontal coeffs
        if (v_level > 0)
        {
            int dvert = 1 << (v_level - 1); // no vertical decomposition from here
            for (int iy = 0; iy < dheight / dvert; ++iy)
            {
                for (int ix = 0; ix < dwidth / d; ++ix)
                {
                    int stride = im.width;
                    int iCC = dvert * 2 * iy * stride + 2 * d * ix; // X multiple of 1 << lvl
                    int iLL = iy * stride + ix;
                    int iLH = iy * stride + ix + dwidth / d;
                    buf[iLL] = // Approximation coeffs
                        (uint8_t)(im2[iCC] / rs) * 256 * 256 + // red
                        (uint8_t)(im1[iCC] / rs) * 256 + // green
                        (uint8_t)(im0[iCC] / rs); // blue
                    buf[iLH] = // horizontal detail coeffs
                        (uint8_t)(16 * im2[iCC + d] / rs) * 256 * 256 + // red
                        (uint8_t)(16 * im1[iCC + d] / rs) * 256 + // green
                        (uint8_t)(16 * im0[iCC + d] / rs); // blue
                }
            }
        }
        else // zero decomp_v is exceptional, none vertical bands like H(n)L, H(n)H so iy runs entire image.height
        {
            for (int iy = 0; iy < im.height; ++iy)
            {
                for (int ix = 0; ix < dwidth / d; ++ix)
                {
                    int stride = im.width;
                    int iCC = iy * stride + 2 * d * ix; // X multiple of 1 << lvl
                    int iLL = iy * stride + ix;
                    int iLH = iy * stride + ix + dwidth / d;
                    buf[iLL] = // Approximation coeffs
                        (uint8_t)(im2[iCC] / rs) * 256 * 256 + // red
                        (uint8_t)(im1[iCC] / rs) * 256 + // green
                        (uint8_t)(im0[iCC] / rs); // blue
                    buf[iLH] = // horizontal detail coeffs
                        (uint8_t)(16 * im2[iCC + d] / rs) * 256 * 256 + // red
                        (uint8_t)(16 * im1[iCC + d] / rs) * 256 + // green
                        (uint8_t)(16 * im0[iCC + d] / rs); // blue
                }
            }
        }
    }
    if (png_write(im.width, im.height, buf, L"de-interleaved_decomposition.png"))
        printf("cannot write image file 'de-interleaved_decomposition.png'\n");

    // IDWT (inverse DWT). De-interleave operation used to draw "de-interleaved_decomposition.png"
    // does not alter 'image', generated by DWT from source_image
    dwt_inverse_transform(im0, im.width, im.height, h_level, v_level);
    dwt_inverse_transform(im1, im.width, im.height, h_level, v_level);
    dwt_inverse_transform(im2, im.width, im.height, h_level, v_level);
    // save recovered image to file
    for (int i = 0; i < im.width * im.height; ++i)
    {
        int rs = 1 << (im.depth - 8); // right shift, bitdepth - 8 positions
        buf[i] =
            (uint8_t)(im2[i] / rs) * 256 * 256 + // red
            (uint8_t)(im1[i] / rs) * 256 + // green
            (uint8_t)(im0[i] / rs); // blue
    }
    if (png_write(im.width, im.height, buf, L"recovered_image.png"))
        printf("cannot write image file 'recovered_image.png'\n");
    return 0;
}

