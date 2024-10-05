# JXS codec pipeline

## License
The JXS\_codec\_pipeline is a derivative work and includes in their entirety the files from the ISO-IEC 21122 (5)
software package individually specified in the projects of this solution.

## Overview

This is is a series of projects to help understand and effectively use ISO 21122 (5) reference software packet.

The first of the series, a JXS wavelet transform project, demonstrates an unconventional picture of wavelet decomposition 
approximation and details coefficients with the JPEG XS Le Gall 5/3 wavelet transform. To make my intention more pronounced, 
the project only includes a dwt.c source code file from the xs_ref_sw_ed2 CMake project. The code for upscaling 
image data to 20 bit precision and reversible color decorrelation transform, as well as the corresponding inverse transforms,
is included with the main() of the project.

Also, the image data is generated in the header image_create.h, the function int image\_create(xs_image\_t& image, ids\_t& ids),
which may later be extended to the use of function pointer as in image_create(xs_image\_t& image, ids\_t& ids, func\_t imagefunc)
to provide for a diversity of image data including image data read from image files.

The header image\_write.h writes the generated images to files, using Microsoft WIC component, and later can be modified 
to use a crossplatform cairo library for the purpose.
