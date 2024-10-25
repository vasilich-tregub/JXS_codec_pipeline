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
    int8_t/*nibble*/ ndecomp_h = 5; // 5;
    int8_t/*nibble*/ ndecomp_v = 2; // 2;
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
        int rs = 256 * 16; // right shift, 12 bit positions
        buf[i] = 
            (uint8_t)(image.comps_array[0][i] / rs) * 256 * 256 + // red
            (uint8_t)(image.comps_array[1][i] / rs) * 256 + // green
            (uint8_t)(image.comps_array[2][i] / rs); // blue
    }
    // and save this picture to image file
    if (image_write(image.width, image.height, buf, L"decomposition_after_inline_DWT.png"))
        printf("cannot write image file 'decomposition_after_inline_DWT.png'\n");

    // re-order picture with (ndecomp_h, ndecomp_v) stencils into classical decomposition picture: 
    // for example, for ndecomp_h = 1, ndecomp_v = 1 it would look like
    // (AA, LH, HL, HH), or (AA, HD, VD, DD) in MATLAB convention.
    
    int stride = image.width;
    int rs = 256 * 16; // right shift, 12 bit positions
    int cellw = image.width / (1 << ndecomp_h); // cell width
    if (ndecomp_v == 0)
    {
        for (int lvlhor = 0; lvlhor <= (1 << ndecomp_h) - 1; ++lvlhor)
        {
            int cell0x = lvlhor * cellw; // cell origin X
            for (int iy = 0; iy < image.height; ++iy)
            {
                for (int ix = 0, ixs = ix + lvlhor; ix < cellw && ixs < image.width; ++ix, ixs += (1 << ndecomp_h))
                {
                    buf[iy * stride + cell0x + ix] =
                        (uint8_t)(image.comps_array[0][iy * stride + ixs] / rs) * 256 * 256 + // red
                        (uint8_t)(image.comps_array[1][iy * stride + ixs] / rs) * 256 + // green
                        (uint8_t)(image.comps_array[2][iy * stride + ixs] / rs); // blue
                }
            }
        }
    }
    else
    {
        int cellh = image.height / (1 << ndecomp_v); // cell height
        for (int lvlver = 0; lvlver <= (1 << ndecomp_v) - 1; ++lvlver) // lvlver is a vertical decomp level
        {
            int cell0y = lvlver * cellh; // cell origin Y
            for (int lvlhor = 0; lvlhor <= (1 << ndecomp_h) - 1; ++lvlhor) // lvlver is a horizontal decomp level
            {
                int cell0x = lvlhor * cellw; // cell origin X
                for (int iy = 0, iys = iy + lvlver; iy < cellh && iys < image.height; ++iy, iys += (1 << ndecomp_v))
                {
                    for (int ix = 0, ixs = ix + lvlhor; ix < cellw && ixs < image.width; ++ix, ixs += (1 << ndecomp_h))
                    {
                        buf[(cell0y + iy) * stride + (cell0x + ix)] =
                            (uint8_t)(image.comps_array[0][iys * stride + ixs] / rs) * 256 * 256 + // red
                            (uint8_t)(image.comps_array[1][iys * stride + ixs] / rs) * 256 + // green
                            (uint8_t)(image.comps_array[2][iys * stride + ixs] / rs); // blue
                    }
                }
            }
        }
    }
    if (image_write(image.width, image.height, buf, L"re-ordered_decomposition.png"))
        printf("cannot write image file 're-ordered_decomposition.png'\n");

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
    printf("Image 'decomposition_after_inline_DWT.png' is constructed \n");
    printf("from image.comps_array[0][i]/256/16 pixels after inline DWT \n");
    printf("on source_image and is only created to illustrate an uncoventional picture\n ");
    printf("of approx and details coeffs with JPEG XS wavelet decomposition.\n");
    printf("Code lines 124-167 re-order this unconventional picture of approx/detail coeffs \n");
    printf("and draw the conventional picture into 're-ordered_decomposition.png'\n");
    printf("A useful exercise is to vary ids.nlxy.y values (0, 1, 2) and see how the picture changes.\n");
    printf("In this exercise (only) you can also vary ids.nlxy.x values (0, ..., 7), provided ids.nlxy.x >= ids.nlxy.y.\n");
}
