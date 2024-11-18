// JXS_decode_rearrange.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <vector>
#include <wincodec.h>
#include "libjxs.h"
#include "PlanarToPackaged.h"
#include "image_readwrite.h"
extern "C" {
#include "file_io.h"
}
#ifdef _DEBUG
#pragma comment(lib, "jxsd.lib")
#else
#pragma comment(lib, "jxs.lib")
#endif // DEBUG

int main(int32_t argc, char* argv[])
{
    if (argc < 2) {
        printf("Not set input file!\n");
        return -1;
    }
    HRESULT hr = S_OK;
    xs_config_t xs_config{ 0 };
    char* input_seq_n = NULL;
    char* output_seq_n = NULL;
    const char* input_fn = argv[1];
    char* output_fn = NULL;
    xs_image_t image = { 0 };
    uint8_t* bitstream_buf = NULL;
    size_t bitstream_buf_size, bitstream_buf_max_size;
    xs_dec_context_t* ctx = NULL;
    int ret = 0;
    int file_idx = 0;

    IWICBitmap* jxsBitmap = nullptr;
    std::vector<uint8_t> pxlData;

    input_fn = argv[1];
    bitstream_buf_max_size = fileio_getsize(input_fn);
    if (bitstream_buf_max_size <= 0)
    {
        fprintf(stderr, "Unable to open file %s\n", input_seq_n);
        ret = -1;
        return ret;
    }

    bitstream_buf = (uint8_t*)malloc((bitstream_buf_max_size + 8) & (~0x7));
    if (!bitstream_buf)
    {
        fprintf(stderr, "Unable to allocate memory for codestream\n");
        ret = -1;
        return ret;
    }
    fileio_read(input_fn, bitstream_buf, &bitstream_buf_size, bitstream_buf_max_size);

    if (!xs_dec_probe(bitstream_buf, bitstream_buf_size, &xs_config, &image))
    {
        fprintf(stderr, "Unable to parse input codestream (%s)\n", input_fn);
        ret = -1;
        return ret;
    }

    if (!xs_allocate_image(&image, false))
    {
        fprintf(stderr, "Image memory allocation error\n");
        ret = -1;
        return ret;
    }

    ctx = xs_dec_init(&xs_config, &image);
    if (!ctx)
    {
        fprintf(stderr, "Error parsing the codestream\n");
        ret = -1;
        return ret;
    }

    if (!xs_dec_bitstream(ctx, bitstream_buf, bitstream_buf_max_size, &image))
    {
        fprintf(stderr, "Error while decoding the codestream\n");
        ret = -1;
        return ret;
    }

    if (!xs_dec_postprocess_image(&xs_config, &image))
    {
        fprintf(stderr, "Error postprocessing the image\n");
        ret = -1;
        return ret;
    }

    if (SUCCEEDED(hr))
    {
        if (!(image.ncomps == 3 || image.ncomps == 4))
        {
            printf("Viewing of other than 3 or 4 component images has yet to be implemented, come back later.");
            return E_NOTIMPL;
        }
        if (image.sx[0] != 1 || image.sx[1] != 1 || image.sx[2] != 1 ||
            image.sy[0] != 1 || image.sy[1] != 1 || image.sy[2] != 1)
        {
            if (!((image.sx[1] == 2 && image.sx[2] == 2) && (image.sy[1] == 1 && image.sy[2] == 1)) &&
                !((image.sx[1] == 2 && image.sx[2] == 2) && (image.sy[1] == 2 && image.sy[2] == 2)))
            {
                printf("This subsampling pattern has yet to be implemented in viewer, come back later.");
                return E_NOTIMPL;
            }
        }
    }

    //Step 3: Copy aligned image memory data to buffer (TODO: consider big endian case)
    if (SUCCEEDED(hr))
    {
        pxlData.resize(4 * image.width * image.height);
        PlanarToPackaged(xs_config, image, pxlData);
    }
    
    std::vector<uint32_t> buf(image.width * image.height);
    
    if (SUCCEEDED(hr))
    {
        int rs = (image.depth > 8) ? (image.depth - 8) : 0;
        int ds = (1 << rs);
        for (int i = 0; i < image.width * image.height; ++i)
        {
            buf[i] = (image.comps_array[2][i] / ds * 256 + image.comps_array[1][i] / ds) * 256 + image.comps_array[0][i] / ds;
        }
        if (image_write(image.width, image.height, buf, L"source_image.png"))
            printf("cannot write image file 'source_image.png'\n");

    }

    if (FAILED(hr))
        return hr;

    // de-interleave coefficients for test imaging:     
    int stride = image.width;
    int rs = (image.depth > 8) ? (image.depth - 8) : 0;
    int dwidth = image.width / 2;
    int dheight = image.height / 2;
    int ndecomp_v = 0;// xs_config.p.NLy;
    int ndecomp_h = 3;// xs_config.p.NLx;
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
}

