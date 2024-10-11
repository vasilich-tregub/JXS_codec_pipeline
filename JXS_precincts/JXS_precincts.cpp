// JXS_precincts.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

extern "C" {
#include "dwt.h"
#include "ids.h"
#include "precinct.h"
#include "rate_control.h"
}
#include <vector>
#include "image_create.h"
#include "image_write.h"
#include "JXS_precincts.h"

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

    // create a source 8-bit depth RGB image of
    if (image_create(image) != 0)
    {
        printf("cannot create image, exit\n");
        return -1;
    }
    image_paint(image);

    std::vector<uint32_t> buf(image.width * image.height);
    for (int i = 0; i < image.width * image.height; ++i)
    {
        buf[i] = (image.comps_array[0][i] * 256 + image.comps_array[1][i]) * 256 + image.comps_array[2][i];
    }
    if (image_write(image.width, image.height, buf, L"source_image.png"))
        printf("cannot write image file 'source_image.png'\n");

    //create xs_config specific for example images
    xs_config_t xs_config{};
    xs_config.p.NLx = 5; 
    xs_config.p.NLy = 1; // can be 0 or 2
    xs_config.p.Sd = 0;
    xs_config.p.N_g = 4;
    xs_config.p.Fq = 8; 
    xs_config.p.Lh = 0;
    xs_config.p.slice_height = 16;

    ids_construct(&ids, &image, xs_config.p.NLx, xs_config.p.NLy, xs_config.p.Sd, xs_config.p.Cw, xs_config.p.Lh);

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
        buf[i] = (image.comps_array[0][i] / 256 / 16 * 256 + image.comps_array[1][i] / 256 / 16) * 256 + image.comps_array[2][i] / 256 / 16;
    }
    // and save this picture to png file
    if (image_write(image.width, image.height, buf, L"image_after_inline_DWT.png"))
        printf("cannot write image file 'image_after_inline_DWT.png'\n");

    // create precinct(s) from image
    std::vector<std::vector<precinct_t*>> precinct;
    std::vector<std::vector<rate_control_t*>> rc;

    std::vector<std::vector<rc_results_t>> rc_results; // rc_results[..]
    int slice_idx = 0;
    int markers_len = 0;

    precinct.resize(image.height);
    rc.resize(image.height);
    rc_results.resize(image.height);
    for (int i = 0; i < image.height; ++i)
    {
        precinct[i].resize(ids.npx);
        rc[i].resize(ids.npx);
        rc_results[i].resize(ids.npx);
    }
    for (int line_idx = 0; line_idx < image.height; line_idx += ids.ph)
    {
        for (int column = 0; column < ids.npx; column++)
        {
            rc[line_idx][column] = rate_control_open(&xs_config, &ids, column);
            precinct[line_idx][column] = precinct_open_column(&ids, xs_config.p.N_g, column);
        }
    }

    for (int line_idx = 0; line_idx < image.height; line_idx += ids.ph)
    {
        const int prec_y_idx = (line_idx / ids.ph);
        for (int column = 0; column < ids.npx; ++column)
        {
            precinct_set_y_idx_of(precinct[line_idx][column], prec_y_idx);
            precinct_from_image(precinct[line_idx][column], &image, xs_config.p.Fq); //ctx.xs_config->p.Fq

            update_gclis(precinct[line_idx][column]);

            if (rate_control_process_precinct(rc[0][column], precinct[line_idx][column], &rc_results[line_idx][column]) < 0) { // rc_results[..]
                return false;
            }

            quantize_precinct(precinct[line_idx][column], rc_results[line_idx][column].gtli_table_data, xs_config.p.Qpih);

            if (precinct_is_first_of_slice(precinct[line_idx][column], xs_config.p.slice_height) && (column == 0))
            {
                if (xs_config.verbose > 1)
                {
                    fprintf(stderr, "Write Slice Header (slice_idx=%d)\n", slice_idx);
                }
                //markers_len += xs_write_slice_header(ctx->bitstream, slice_idx++);
            }

            /*if (pack_precinct(ctx->packer, ctx->bitstream, ctx->precinct[line_idx][column], &rc_results) < 0)
            {
                return false;
            }*/

            if (rc_results[line_idx][column].rc_error == 1)
                break;
        }
    }
    //xs_write_tail(ctx->bitstream);
    //assert((ctx->xs_config->bitstream_size_in_bytes == (size_t)-1) || bitpacker_get_len(ctx->bitstream) / 8 == ctx->xs_config->bitstream_size_in_bytes);

    //*codestream_byte_size = ((bitpacker_get_len(ctx->bitstream) + 7) / 8);
    //bitpacker_flush(ctx->bitstream);

    // draw precincts as image file

    // recover image from precinct(s)
    xs_image_t image_out{}; // create empty image with identical structure to source umage
    if (image_create(image_out) != 0)
    {
        printf("cannot create target image, exit\n");
        return -1;
    }

    std::vector<precinct_t*> precinct_top;
    precinct_top.resize(ids.npx);

    //ids_construct(&ids, &image, xs_config.p.NLx, xs_config.p.NLy, xs_config.p.Sd, xs_config.p.Cw, xs_config.p.Lh);

    for (int column = 0; column < ids.npx; column++)
    {
        //precinct[0][column] = precinct_open_column(&ids, xs_config.p.N_g, column);
        precinct_top[column] = precinct_open_column(&ids, xs_config.p.N_g, column);
    }

    for (int line_idx = 0; line_idx < ids.h; line_idx += ids.ph)
    {
        //unpacked_info_t unpack_out;
        const int prec_y_idx = (line_idx / ids.ph);
        for (int column = 0; column < ids.npx; column++)
        {
            precinct_set_y_idx_of(precinct[line_idx][column], prec_y_idx);
            const int first_of_slice = precinct_is_first_of_slice(precinct[line_idx][column], xs_config.p.slice_height);

            /*if (first_of_slice && column == 0)
            {
                int slice_idx_check;
                xs_parse_slice_header(ctx->bitstream, &slice_idx_check);
                assert(slice_idx_check == (slice_idx++));
                if (ctx->xs_config->verbose > 1)
                {
                    fprintf(stderr, "Read Slice Header (slice_idx=%d)\n", slice_idx_check);
                }
            }

#ifdef PACKING_GENERATE_FRAGMENT_CODE
            const int extra_bits_before_precinct = (int)(bitunpacker_consumed_bits(ctx->bitstream) - bitstream_pos);
#else
            const int extra_bits_before_precinct = 0;
#endif
            if (unpack_precinct(ctx->unpack_ctx, ctx->bitstream, ctx->precinct[line_idx][column],
                (!first_of_slice) ? ctx->precinct_top[column] : NULL, ctx->gtlis_table_top[column],
                &unpack_out, extra_bits_before_precinct) < 0)
            {
                if (ctx->xs_config->verbose) fprintf(stderr, "Corrupted codestream! line number %d\n", line_idx);
                return false;
            }
            bitstream_pos = bitunpacker_consumed_bits(ctx->bitstream);*/

            dequantize_precinct(precinct[line_idx][column], /*unpack_out*/rc_results[line_idx][column].gtli_table_data, xs_config.p.Qpih); // rc_results[..]

            precinct_to_image(precinct[line_idx][column], &image_out, xs_config.p.Fq); // image_out_create !!!

            //swap_ptrs(&precinct_top[column], &precinct[line_idx][column]);
            //memcpy(ctx->gtlis_table_top[column], unpack_out.gtli_table_gcli, MAX_NBANDS * sizeof(int));
        }
    }

    // save precinct_to_image to png file
    for (int i = 0; i < len; ++i)
    {
        buf[i] = (image_out.comps_array[0][i] / 256 / 16 * 256 + image_out.comps_array[1][i] / 256 / 16) * 256 + image_out.comps_array[2][i] / 256 / 16;
    }
    if (image_write(image_out.width, image_out.height, buf, L"precinct_to_image.png"))
        printf("cannot write image file 'precinct_to_image.png'\n");

    // IDWT (inverse DWT)
    dwt_inverse_transform(&ids, &image_out);

    // inverse reversible color transform (to YCbCr) for light444.12 profile with XS_CPIH_RCT marker
    c0 = image_out.comps_array[0];
    c1 = image_out.comps_array[1];
    c2 = image_out.comps_array[2];
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
    const xs_data_in_t max_val = (1 << image_out.depth) - 1;
    for (int c = 0; c < image_out.ncomps; ++c)
    {
        const size_t sample_count = (size_t)(image_out.width) * (size_t)(image_out.height);
        xs_data_in_t* the_ptr = image_out.comps_array[c];
        for (size_t i = sample_count; i != 0; --i)
        {
            *the_ptr = clamp((*the_ptr + dclev_and_rounding) >> s, max_val);
            ++the_ptr;
        }
    }

    // save recovered image to png file
    for (int i = 0; i < len; ++i)
    {
        buf[i] = (image_out.comps_array[0][i] * 256 + image_out.comps_array[1][i]) * 256 + image_out.comps_array[2][i];
    }
    if (image_write(image_out.width, image_out.height, buf, L"recovered_image.png"))
        printf("cannot write target image file 'recovered_image.png'\n");

    printf("You see an example of precinct operations in this exercise.\n");
    printf("Precincts are created after DWT, later image_out is retrieved \n");
    printf("from the precincts, is saved to the png file, \n");
    printf("and the JXS pipeline continues to recover the source image. \n");
}

