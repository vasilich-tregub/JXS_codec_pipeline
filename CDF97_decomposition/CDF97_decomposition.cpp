// CDF97_encoding.cpp : This file contains the 'main' function. Program execution begins and ends there.
// TODO: no png, read 12-bit image from JXS or PPM

#include <iostream>
#include <vector>
#include <assert.h>
#include "image.h"

/**
 *  fwt97 - Forward biorthogonal 9/7 wavelet transform (lifting implementation)
 *
 *  im is an input signal, which will be replaced by its output transform.
 */

void fwt97(std::vector<double>& im, const int level) {
    const int inc = (int)1 << level;
    int end = (int)im.size();
    assert(inc < end && "stepping outside source image");

    double a;
    int i;

    // Predict 1
    a = -1.586134342;
    for (i = inc; i < end - inc; i += 2 * inc) {
        im[i] += a * (im[i - inc] + im[i + inc]);
    }
    if (i < end)
        im[i] += 2 * a * im[i - inc];

    // Update 1
    a = -0.05298011854;
    im[0] += 2 * a * im[inc];
    for (i = 2 * inc; i < end - inc; i += 2 * inc) {
        im[i] += a * (im[i - inc] + im[i + inc]);
    }
    if (i < end)
        im[i] += 2 * a * im[i - inc];

    // Predict 2
    a = 0.8829110762;
    for (i = inc; i < end - inc; i += 2 * inc) {
        im[i] += a * (im[i - inc] + im[i + inc]);
    }
    if (i < end)
        im[i] += 2 * a * im[i - inc];

    // Update 2
    a = 0.4435068522;
    im[0] += 2 * a * im[inc];
    for (i = 2 * inc; i < end - inc; i += 2 * inc) {
        im[i] += a * (im[i - inc] + im[i + inc]);
    }
    if (i < end)
        im[i] += 2 * a * im[i - inc];

    // Scale
    a = 1 / 1.149604398;
    for (i = 0; i < end; i++) {
        if (i % (2 * inc)) im[i] /= a;
        else im[i] *= a;
    }

}

/**
 *  iwt97 - Inverse biorthogonal 9/7 wavelet transform
 *
 *  This is the inverse of fwt97 so that iwt97(fwt97(im,0),0)=im
 */

void iwt97(std::vector<double>& im, const int level) {
    const int inc = (int)1 << level;
    int end = (int)im.size();
    assert(inc < end && "stepping outside source image");

    double a;
    int i;

    // Undo scale
    a = 1.149604398;
    for (i = 0; i < end; i++) {
        if (i % (2 * inc)) im[i] /= a;
        else im[i] *= a;
    }

    // Undo update 2
    a = -0.4435068522;
    im[0] += 2 * a * im[inc];
    for (i = 2 * inc; i < end - inc; i += 2 * inc) {
        im[i] += a * (im[i - inc] + im[i + inc]);
    }
    if (i < end)
        im[i] += 2 * a * im[i - inc];

    // Undo predict 2
    a = -0.8829110762;
    for (i = inc; i < end - inc; i += 2 * inc) {
        im[i] += a * (im[i - inc] + im[i + inc]);
    }
    if (i < end)
        im[i] += 2 * a * im[i - inc];

    // Undo update 1
    a = 0.05298011854;
    im[0] += 2 * a * im[inc];
    for (i = 2 * inc; i < end - inc; i += 2 * inc) {
        im[i] += a * (im[i - inc] + im[i + inc]);
    }
    if (i < end)
        im[i] += 2 * a * im[i - inc];

    // Undo predict 1
    a = 1.586134342;
    for (i = inc; i < end - inc; i += 2 * inc) {
        im[i] += a * (im[i - inc] + im[i + inc]);
    }
    if (i < end)
        im[i] += 2 * a * im[i - inc];
}

int main()
{
    xs_image_t im;
    im.sx[2] = im.sx[1] = im.sx[0] = 1;
    im.sy[2] = im.sy[1] = im.sy[0] = 1;
    im.depth = -1;
    im.comps_array[2] = im.comps_array[1] = im.comps_array[0] = NULL;
    ppm_decode("../19.jxs.ppm", &im);
    std::vector<double> im0(im.width * im.height);
    std::vector<double> im1(im.width * im.height);
    std::vector<double> im2(im.width * im.height);
    for (int i = 0; i < im.width * im.height; ++i)
    {
        im0[i] = (double)im.comps_array[0][i];
        im1[i] = (double)im.comps_array[1][i];
        im2[i] = (double)im.comps_array[2][i];
    }
    fwt97(im0, 0);
    fwt97(im1, 0);
    fwt97(im2, 0);
    return 0;
}

