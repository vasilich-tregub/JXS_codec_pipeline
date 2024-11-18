#pragma once

uint8_t clamp(double arg)
{
    if (arg < 0.0)
        return 0;
    else if (arg > 255.0)
        return 255;
    return (uint8_t)arg;
}
void PlanarToPackaged(xs_config_t& xs_config, xs_image_t& image, std::vector<uint8_t>& pxldata)
{
    int rightshift = (image.depth > 8) ? (image.depth - 8) : 0;
    if (image.sx[0] == 1 && image.sx[1] == 1 && image.sx[2] == 1 &&
        image.sy[0] == 1 && image.sy[1] == 1 && image.sy[2] == 1)
    {
        if (xs_config.p.color_transform == XS_CPIH_RCT)
        {
            for (int ix = 0; ix < image.width * image.height; ++ix)
            {
                pxldata[4 * ix] = image.comps_array[0][ix] >> rightshift;
                pxldata[4 * ix + 1] = image.comps_array[1][ix] >> rightshift;
                pxldata[4 * ix + 2] = image.comps_array[2][ix] >> rightshift;
                pxldata[4 * ix + 3] = (image.ncomps == 4) ? (image.comps_array[3][ix] >> rightshift) : 255;
            }
        }
        else
        {
            for (int ix = 0; ix < image.width * image.height; ++ix)
            {
                uint8_t y = image.comps_array[0][ix] >> rightshift;
                uint8_t u = image.comps_array[1][ix] >> rightshift;
                uint8_t v = image.comps_array[2][ix] >> rightshift;
                pxldata[4 * ix + 0] = clamp(y + 1.28033 * (v - 128));
                pxldata[4 * ix + 1] = clamp(y - 0.21482 * (u - 128) - 0.38059 * (v - 128));
                pxldata[4 * ix + 2] = clamp(y + 2.12798 * (u - 128));
                pxldata[4 * ix + 3] = (image.ncomps == 4) ? (image.comps_array[3][ix] >> rightshift) : 255;
            }
        }
    }
    else 
        if (image.sx[0] == 1 && image.sx[1] == 2 && image.sx[2] == 2 &&
            image.sy[0] == 1 && image.sy[1] == 1 && image.sy[2] == 1)
        {
            for (int ix = 0; ix < image.width * image.height; ++ix)
            {
                uint8_t y = image.comps_array[0][ix] >> rightshift;
                uint8_t u = image.comps_array[1][ix / 2] >> rightshift;
                uint8_t v = image.comps_array[2][ix / 2] >> rightshift;
                pxldata[4 * ix + 0] = clamp(y + 1.28033 * (v - 128));
                pxldata[4 * ix + 1] = clamp(y - 0.21482 * (u - 128) - 0.38059 * (v - 128));
                pxldata[4 * ix + 2] = clamp(y + 2.12798 * (u - 128));
                pxldata[4 * ix + 3] = (image.ncomps == 4) ? (image.comps_array[3][ix] >> rightshift) : 255;
            }
        }
    else 
        if (image.sx[0] == 1 && image.sx[1] == 2 && image.sx[2] == 2 &&
            image.sy[0] == 1 && image.sy[1] == 2 && image.sy[2] == 2)
        {
            bool oddrow = false; // only one chroma value (U or V) per two rows; hor pattern is YUYV
            int row_1stpxl = 0;
            int chrrow_1stpxl = 0;
            int ycurrpxl = 0;
            int chrcurrpxl = 0;
            for (; row_1stpxl < image.height; ++row_1stpxl, ycurrpxl += image.width)
            {
                for (int ix = 0; ix < image.width; ++ix)
                {
                    int yix = ycurrpxl + ix;
                    int chrix = chrcurrpxl + ix / 2; // here, algo may become broken for odd width value (like 12287 of 27.jxs)
                    uint8_t y = image.comps_array[0][yix] >> rightshift;
                    uint8_t u = image.comps_array[1][chrix] >> rightshift;
                    uint8_t v = image.comps_array[2][chrix] >> rightshift;
                    pxldata[4 * yix + 0] = clamp(y + 1.28033 * (v - 128));
                    pxldata[4 * yix + 1] = clamp(y - 0.21482 * (u - 128) - 0.38059 * (v - 128));
                    pxldata[4 * yix + 2] = clamp(y + 2.12798 * (u - 128));
                    pxldata[4 * yix + 3] = (image.ncomps == 4) ? (image.comps_array[3][ix] >> rightshift) : 255;
                }
                if (oddrow)
                {
                    ++chrrow_1stpxl;
                    chrcurrpxl += image.width / 2;
                }
                oddrow = !oddrow;
            }
        }

};

