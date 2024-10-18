// SvtJxs_breakdown.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "SvtJpegxs.h"
#include "SvtJpegxsEnc.h"
//#include "image_create.h"
#include "image_write.h"

int main()
{
    svt_jpeg_xs_encoder_api_t enc;
    enc.source_width = 4095;
    enc.source_height = 1743;
    enc.input_bit_depth = 8;
    enc.colour_format = COLOUR_FORMAT_PLANAR_YUV444_OR_RGB;
    enc.bpp_numerator = 3;

    uint32_t pixel_size = enc.input_bit_depth <= 8 ? 1 : 2;
    svt_jpeg_xs_image_buffer_t in_buf;
    in_buf.stride[0] = enc.source_width;
    in_buf.stride[1] = enc.source_width;
    in_buf.stride[2] = enc.source_width;
    for (uint8_t i = 0; i < 3; ++i) {
        in_buf.alloc_size[i] = in_buf.stride[i] * enc.source_height * pixel_size;
        in_buf.data_yuv[i] = malloc(in_buf.alloc_size[i]);
        if (!in_buf.data_yuv[i]) {
            return SvtJxsErrorInsufficientResources;
        }
    }
    for (uint32_t iy = 0; iy < enc.source_height; ++iy)
    {
        for (uint32_t ix = 0; ix < enc.source_width; ++ix)
        {
            ((uint8_t*)in_buf.data_yuv[0])[iy * enc.source_width + ix] = (uint8_t)((iy / 10 - ix * 3 / 20 + 40));  
        }
    }
    for (uint32_t iy = 0; iy < enc.source_height; ++iy)
    {
        for (uint32_t ix = 0; ix < enc.source_width; ++ix)
        {
            ((uint8_t*)in_buf.data_yuv[1])[iy * enc.source_width + ix] = (uint8_t)(iy * ix / 1600);  
        }
    }
    for (uint32_t iy = 0; iy < enc.source_height; ++iy)
    {
        for (uint32_t ix = 0; ix < enc.source_width; ++ix)
        {
            ((uint8_t*)in_buf.data_yuv[2])[iy * enc.source_width + ix] = (uint8_t)((iy / 10 + ix * 3 / 20)); 
        }
    }

    std::vector<uint32_t> buf(enc.source_width * enc.source_height);
    for (uint32_t iy = 0; iy < enc.source_height; ++iy)
    {
        for (uint32_t ix = 0; ix < enc.source_width; ++ix)
        buf[iy * enc.source_width + ix] = ((uint8_t*)in_buf.data_yuv[0])[iy * enc.source_width + ix] * 256 * 256 + 
            ((uint8_t*)in_buf.data_yuv[1])[iy * enc.source_width + ix] * 256 + 
            ((uint8_t*)in_buf.data_yuv[2])[iy * enc.source_width + ix];

    }
    if (image_write(enc.source_width, enc.source_height, buf, L"source_image.png"))
        printf("cannot write image file 'source_image.png'\n");

}

