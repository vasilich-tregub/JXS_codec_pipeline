// JXS_wavelet_transform.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

extern "C" {
#include "dwt.h"
#include "ids.h"
#include <stdio.h>
#include <malloc.h>
}
#include <vector>
#include "image_create.h"
#include "image_write.h"

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
    if (image_create(image) != 0)
    {
        printf("cannot create image, exit\n");
        return -1;
    }
    image_paint(image);

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


    std::vector<uint32_t> buf(image.width * image.height);
    for (int i = 0; i < image.width * image.height; ++i)
    {
        buf[i] = (image.comps_array[0][i] * 256 + image.comps_array[1][i]) * 256 + image.comps_array[2][i];
    }
    if (image_write(image.width, image.height, buf, L"source_image.png"))
        printf("cannot write image file 'source_image.png'\n");

    // JPEG XS workflow requires to upscale image data to 20 bit precision
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

    // JPEG XS workflow requires to apply reversible color transform (to YCbCr) 
    // for images like RGB 8 bit per component, no CFA (the image of this exercize 
    // can be classified into light444.12 profile with a XS_CPIH_RCT marker)
    const int len = image.width * image.height;
    xs_data_in_t* c0 = image.comps_array[0]; // R
    xs_data_in_t* c1 = image.comps_array[1]; // G
    xs_data_in_t* c2 = image.comps_array[2]; // B
    for (int i = 0; i < len; ++i)
    {
        const xs_data_in_t g = *c1;
        const xs_data_in_t tmp = (*c0 + 2 * g + *c2) >> 2;
        *c1 = *c2 - g;
        *c2 = *c0 - g;
        *c0 = tmp;
        ++c0; ++c1; ++c2;
    }

    // DWT forward
    dwt_forward_transform(&ids, &image);
    // create an illustration of DWT-transformed image by truncation of int32_t pixels into bytes
    for (int i = 0; i < image.width * image.height; ++i)
    {
        buf[i] = image.comps_array[0][i] / 256 / 4;
    }
    // and save this picture to image file
    if (image_write(image.width, image.height, buf, L"decomposition_after_inline_DWT.png"))
        printf("cannot write image file 'decomposition_after_inline_DWT.png'\n");
    // re-order picture with (ndecomp_h, ndecomp_v) stencils into classical decomposition picture, 
    // (AA, LH, HL, HH), or (AA, HD, VD, DD) in MATLAB convention. Only ndecomp_h = 1, ndecomp_v = 1 is shown, 
    // other (ndecomp_h, ndecomp_v) values may follow suit later
    if (ndecomp_h == 1 && ndecomp_v == 1)
    {
        int rs = 256 * 16; // right shift, 12 bit positions
        for (int iy = 0; iy < image.height / 2; ++iy)
        {
            for (int ix = 0; ix < image.width / 2; ++ix)
            {
                int stride = image.width;
                int iCC = 2 * iy * stride + 2 * ix; // top left stencil corner
                int iLL = iy * image.width + ix;
                int iLH = iy * image.width + ix + image.width / 2;
                int iHL = (iy + image.height / 2) * image.width + ix;
                int iHH = (iy + image.height / 2) * image.width + ix + image.width / 2;
                buf[iLL] = // Approximation coeffs
                    (uint8_t)(image.comps_array[0][iCC] / rs) * 256 * 256 + // red
                    (uint8_t)(image.comps_array[1][iCC] / rs) * 256 + // green
                    (uint8_t)(image.comps_array[2][iCC] / rs); // blue
                buf[iLH] = // horizontal detail coeffs
                    (uint8_t)(image.comps_array[0][iCC + 1] / rs) * 256 * 256 + // red
                    (uint8_t)(image.comps_array[1][iCC + 1] / rs) * 256 + // green
                    (uint8_t)(image.comps_array[2][iCC + 1] / rs); // blue
                buf[iHL] = // vertical detail coeffs
                    (uint8_t)(image.comps_array[0][iCC + stride] / rs) * 256 * 256 + // red
                    (uint8_t)(image.comps_array[1][iCC + stride] / rs) * 256 + // green
                    (uint8_t)(image.comps_array[2][iCC + stride] / rs); // blue
                buf[iHH] = // diagonal detail coeffs
                    (uint8_t)(image.comps_array[0][iCC + stride + 1] / rs) * 256 * 256 + // red
                    (uint8_t)(image.comps_array[1][iCC + stride + 1] / rs) * 256 + // green
                    (uint8_t)(image.comps_array[2][iCC + stride + 1] / rs); // blue
            }
        }
        if (image_write(image.width, image.height, buf, L"re-ordered_decomposition.png"))
            printf("cannot write image file 're-ordered_decomposition.png'\n");
    }
    else
    {
            printf("decomp_H/V!=1, image file 're-ordered_decomposition.png unaltered from previous run'\n");
    }
    // IDWT (inverse DWT)
    dwt_inverse_transform(&ids, &image);
    
    // inverse reversible color transform (to YCbCr) for light444.12 profile with XS_CPIH_RCT marker
    c0 = image.comps_array[0];
    c1 = image.comps_array[1];
    c2 = image.comps_array[2];
    for (int i = 0; i < len; ++i)
    {
        const xs_data_in_t tmp = *c0 - ((*c1 + *c2) >> 2);
        *c0 = tmp + *c2;
        *c2 = tmp + *c1;
        *c1 = tmp;
        ++c0; ++c1; ++c2;
    }
    
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
    
    printf("You see an example of lossless wavelet transform in this exercise.\n");
    printf("Using JPEG XS workflow as implemented in ISO 21122 reference software,\n");
    printf("DWT inline-transforms source_image into data in the xs_image_t image\n");
    printf("from which data IDWT generates a recovered_image.\n");
    printf("'image_after_DWT.png' is constructed from image.comps_array[0][i]/256/16 pixels ");
    printf("after inline DWT on source_image and is only created to illustrate ");
    printf("an uncoventional picture of approx and details coeffs with JPEG XS wavelet decomposition.\n");
    printf("A useful exercise is to vary ids.nlxy.y values (0, 1, 2) and see how the picture changes.\n");
    printf("In this exercise (only) you can also vary ids.nlxy.x values (0, ..., 7) provided ids.nlxy.x >= ids.nlxy.y.\n");
}
