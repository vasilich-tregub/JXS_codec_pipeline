// JXS_polyptych.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

extern "C" {
#include "dwt.h"
#include "ids.h"
#include <stdio.h>
#include <malloc.h>
}
#include <vector>
#include "image_readwrite.h"

static INLINE xs_data_in_t clamp(xs_data_in_t v, xs_data_in_t max_v)
{
    if (v > max_v)
    {
        return max_v;
    }
    if (v < 0)
    {
        return 0;
    }
    return v;
}

int main()
{
    xs_image_t image{};
    ids_t ids{};

    // create a source 24-bit RGB image (8 bit depth)
    // and a corresponding ids structure of 
    // which only ncomps, w, h, sd and nlvy fields
    // are required in this exercise 
    // (see image_create.h header, in other projects 
    // a function parameter be used to create
    // a selectable image)
    unsigned int imwidth = 0;
    unsigned int imheight = 0;
    std::vector<uint8_t> buffer;
    image_read(L"IMG_0030.png", imwidth, imheight, buffer);
    image.ncomps = 3;
    image.depth = 8;
    image.width = imwidth;
    image.height = imheight;
    image.sx[0] = image.sx[1] = image.sx[2] = image.sy[0] = image.sy[1] = image.sy[2] = 1;
    for (int k = 0; k < image.ncomps; ++k)
    {
        image.comps_array[k] = (xs_data_in_t*)malloc(sizeof(xs_data_in_t) * image.width * image.height);
    }
    for (int i = 0; i < image.width * image.height; ++i)
    {
        image.comps_array[0][i] = buffer[4 * i + 0];
        image.comps_array[1][i] = buffer[4 * i + 1];
        image.comps_array[2][i] = buffer[4 * i + 2];
        //image.comps_array[3][i] = buffer[4 * i + 3];
    }

    //ids_construct(&ids, &image, cfgpNLx, cfgpNLy, cfgpSd, xs_config.p.Cw, xs_config.p.Lh);
    int8_t/*nibble*/ ndecomp_h = 1; // 5;
    int8_t/*nibble*/ ndecomp_v = 1; // 2;
    ids.ncomps = image.ncomps;
    ids.w = image.width;
    ids.h = image.height;
    ids.sd = 0; // suppressed decomp comp count sd;
    ids.nlxy.x = ndecomp_h;
    ids.nlxy.y = ndecomp_v;
    ids.nb = 2 * ids.nlxy.y + ids.nlxy.x + 1;
    for (int k = 0; k < image.ncomps; ++k)
    {
        ids.nlxyp[k].x = ids.nlxy.x;
        ids.nlxyp[k].y = ids.nlxy.y; // -(im->sy[c] >> 1); // no CFA defined, all image.sx, image.sy = 1
        ids.comp_w[k] = ids.w;
        ids.comp_h[k] = ids.h;
    }

    if (ndecomp_v > ndecomp_h)
        printf("ndecomp_v > ndecomp_h, exit\n");

    std::vector<uint32_t> buf(image.width * image.height);
    for (int i = 0; i < image.width * image.height; ++i)
    {
        buf[i] = (image.comps_array[0][i] * 256 + image.comps_array[1][i]) * 256 + image.comps_array[2][i];
    }
    if (image_write(image.width, image.height, buf, L"source_image.png"))
        printf("cannot write image file 'source_image.png'\n");

    // JPEG XS pipeline requires to upscale image data to 20 bit precision
    uint8_t Bw = 20;
    const uint8_t s = Bw - (uint8_t)image.depth;
    const xs_data_in_t dclev = ((1 << Bw) >> 1);
    for (int c = 0; c < image.ncomps; ++c)
    {
        const size_t sample_count = (size_t)(image.width) * (size_t)(image.height);
        xs_data_in_t* the_ptr = image.comps_array[c];
        for (size_t i = sample_count; i != 0; --i)
        {
            *the_ptr = (*the_ptr << s) - dclev;
            ++the_ptr;
        }
    }

    // JPEG XS pipeline requires to apply reversible color transform 
    // (to components similar to Y,Cb,Cr but reversible unlike transform to YCbCr) 
    // for images like RGB 8 bit per component, no CFA (the image of this exercize 
    // can be classified into light444.12 profile with a XS_CPIH_RCT marker)
    const int len = image.width * image.height;
    xs_data_in_t* c0 = image.comps_array[0]; // R
    xs_data_in_t* c1 = image.comps_array[1]; // G
    xs_data_in_t* c2 = image.comps_array[2]; // B
    /*for (int i = 0; i < len; ++i)
    {
        const xs_data_in_t g = *c1;
        const xs_data_in_t tmp = (*c0 + 2 * g + *c2) >> 2;
        *c1 = *c2 - g;
        *c2 = *c0 - g;
        *c0 = tmp;
        ++c0; ++c1; ++c2;
    }*/

    // DWT forward
    dwt_forward_transform(&ids, &image);
    // create an illustration of DWT-transformed image by truncation of int32_t component values into bytes
    for (int i = 0; i < image.width * image.height; ++i)
    {
        int rs = 1 << 12; // right shift, 12 bit positions
        buf[i] =
            (uint8_t)(image.comps_array[0][i] / rs) * 256 * 256 + // red
            (uint8_t)(image.comps_array[1][i] / rs) * 256 + // green
            (uint8_t)(image.comps_array[2][i] / rs); // blue
    }
    // and save this picture to image file. Notice that DWT of JPEGXS codec creates an INTERLEAVED tranform:
    // you would not readily discern the bands in the picture
    if (image_write(image.width, image.height, buf, L"decomposition_after_inline_DWT.png"))
        printf("cannot write image file 'decomposition_after_inline_DWT.png'\n");

    // de-interleave coefficients for test imaging:     
    int stride = image.width;
    int rs = 1 << 12; // right shift, 12 bit positions
    int dwidth = image.width / 2;
    int dheight = image.height / 2;
    for (int lvl = 1; lvl <= ndecomp_v; ++lvl)
    {
        int d = 1 << (lvl - 1);
        for (int iy = 0; iy < dheight / d; ++iy)
        {
            for (int ix = 0; ix < dwidth / d; ++ix)
            {
                int stride = image.width;
                int iCC = d * 2 * iy * stride + d * 2 * ix; // X, Y multiples of 1 << lvl
                int iLL = iy * stride + ix;
                int iLH = iy * stride + ix + dwidth / d;
                int iHL = (iy + dheight / d) * stride + ix;
                int iHH = (iy + dheight / d) * stride + ix + dwidth / d;
                buf[iLL] = // Approximation coeffs
                    (uint8_t)(image.comps_array[0][iCC] / rs) * 256 * 256 + // red
                    (uint8_t)(image.comps_array[1][iCC] / rs) * 256 + // green
                    (uint8_t)(image.comps_array[2][iCC] / rs); // blue
                buf[iLH] = // horizontal detail coeffs
                    (uint8_t)(image.comps_array[0][iCC + d] / rs) * 256 * 256 + // red
                    (uint8_t)(image.comps_array[1][iCC + d] / rs) * 256 + // green
                    (uint8_t)(image.comps_array[2][iCC + d] / rs); // blue
                buf[iHL] = // vertical detail coeffs
                    (uint8_t)(image.comps_array[0][iCC + stride * d] / rs) * 256 * 256 + // red
                    (uint8_t)(image.comps_array[1][iCC + stride * d] / rs) * 256 + // green
                    (uint8_t)(image.comps_array[2][iCC + stride * d] / rs); // blue
                buf[iHH] = // diagonal detail coeffs
                    (uint8_t)(image.comps_array[0][iCC + stride * d + d] / rs) * 256 * 256 + // red
                    (uint8_t)(image.comps_array[1][iCC + stride * d + d] / rs) * 256 + // green
                    (uint8_t)(image.comps_array[2][iCC + stride * d + d] / rs); // blue
            }
        }
    }
    for (int lvl = ndecomp_v + 1; lvl <= ndecomp_h; ++lvl)
    {
        int d = 1 << (lvl - 1);           // d only for horizontal coeffs
        if (ndecomp_v > 0)
        {
            int dvert = 1 << (ndecomp_v - 1); // no vertical decomposition from here
            for (int iy = 0; iy < dheight / dvert; ++iy)
            {
                for (int ix = 0; ix < dwidth / d; ++ix)
                {
                    int stride = image.width;
                    int iCC = dvert * 2 * iy * stride + 2 * d * ix; // X multiple of 1 << lvl
                    int iLL = iy * stride + ix;
                    int iLH = iy * stride + ix + dwidth / d;
                    buf[iLL] = // Approximation coeffs
                        (uint8_t)(image.comps_array[0][iCC] / rs) * 256 * 256 + // red
                        (uint8_t)(image.comps_array[1][iCC] / rs) * 256 + // green
                        (uint8_t)(image.comps_array[2][iCC] / rs); // blue
                    buf[iLH] = // horizontal detail coeffs
                        (uint8_t)(image.comps_array[0][iCC + d] / rs) * 256 * 256 + // red
                        (uint8_t)(image.comps_array[1][iCC + d] / rs) * 256 + // green
                        (uint8_t)(image.comps_array[2][iCC + d] / rs); // blue
                }
            }
        }
        else // zero decomp_v is exceptional, none vertical bands like H(n)L, H(n)H so iy runs entire image.height
        {
            for (int iy = 0; iy < image.height; ++iy)
            {
                for (int ix = 0; ix < dwidth / d; ++ix)
                {
                    int stride = image.width;
                    int iCC = iy * stride + 2 * d * ix; // X multiple of 1 << lvl
                    int iLL = iy * stride + ix;
                    int iLH = iy * stride + ix + dwidth / d;
                    buf[iLL] = // Approximation coeffs
                        (uint8_t)(image.comps_array[0][iCC] / rs) * 256 * 256 + // red
                        (uint8_t)(image.comps_array[1][iCC] / rs) * 256 + // green
                        (uint8_t)(image.comps_array[2][iCC] / rs); // blue
                    buf[iLH] = // horizontal detail coeffs
                        (uint8_t)(image.comps_array[0][iCC + d] / rs) * 256 * 256 + // red
                        (uint8_t)(image.comps_array[1][iCC + d] / rs) * 256 + // green
                        (uint8_t)(image.comps_array[2][iCC + d] / rs); // blue
                }
            }
        }
    }
    if (image_write(image.width, image.height, buf, L"de-interleaved_decomposition.png"))
        printf("cannot write image file 'de-interleaved_decomposition.png'\n");

    // IDWT (inverse DWT). De-interleave operation used to draw "de-interleaved_decomposition.png"
    // does not alter 'image', generated by DWT from source_image
    dwt_inverse_transform(&ids, &image);

    // inverse reversible color transform (from mct-aware encoded) for light444.12 profile with XS_CPIH_RCT marker
    /*c0 = image.comps_array[0];
    c1 = image.comps_array[1];
    c2 = image.comps_array[2];
    for (int i = 0; i < len; ++i)
    {
        const xs_data_in_t tmp = *c0 - ((*c1 + *c2) >> 2);
        *c0 = tmp + *c2;
        *c2 = tmp + *c1;
        *c1 = tmp;
        ++c0; ++c1; ++c2;
    }*/

    // downscale from 20 bit precision
    const xs_data_in_t dclev_and_rounding = ((1 << Bw) >> 1) + ((1 << s) >> 1);
    const xs_data_in_t max_val = (1 << image.depth) - 1;
    for (int c = 0; c < image.ncomps; ++c)
    {
        const size_t sample_count = (size_t)(image.width) * (size_t)(image.height);
        xs_data_in_t* the_ptr = image.comps_array[c];
        for (size_t i = sample_count; i != 0; --i)
        {
            *the_ptr = clamp((*the_ptr + dclev_and_rounding) >> s, max_val);
            ++the_ptr;
        }
    }

    // save recovered image to file
    for (int i = 0; i < len; ++i)
    {
        buf[i] = (image.comps_array[0][i] * 256 + image.comps_array[1][i]) * 256 + image.comps_array[2][i];
    }
    if (image_write(image.width, image.height, buf, L"recovered_image.png"))
        printf("cannot write image file 'recovered_image.png'\n");

    return 0;
}


