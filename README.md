# JXS codec pipeline

## License
The JXS\_codec\_pipeline is a derivative work and includes in their entirety the files from the ISO-IEC 21122 (5)
and SVT-JPEG-XS (https://github.com/OpenVisualCloud/SVT-JPEG-XS) software packages individually specified 
in the projects of this solution.

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

In the second project, JXS\_precincts, operations of precinct creation from the DWT result data and restoring this data with 
precinct_to_image calls is inserted between dwt\_forward\_transform and dwt\_inverse\_transform calls. The image generated with 
the precinct_to_image call is written to the png file. The example image in this project is 3840x2160, so the processing can take 
a while and uses 694 MB of RAM. This memory consumption is the result of unfolding precincts in memory arrays instead of writing 
these to stream. It is made on purpose to help student examine the precinct operations. You can vary the dimensions of synthesized 
image in the image\_create.h header file.

So we have the model of encoder followed by the model of encoder in this project. The data from encoder to decoder are passed 
through memory here, rather than via stream.

This project may later be extended for examining the rate control mechanism in the ISO 21122 (5) reference software packet.

<!-- The project SvtJxs\_breakdown is based off another implementation of the ISO 21122 (ed.3) standard, this of Intel Corporation.
The point of this excursion is to show that both implementations are consistent with one another. Also, I believe that 
the Intel Corporation implementation gives more convenient structures representing precincts. Any way, the diagrams from 
their github repository, https://github.com/OpenVisualCloud/SVT-JPEG-XS/blob/main/documentation/encoder/svt-jpegxs-encoder-design.md 
and https://github.com/OpenVisualCloud/SVT-JPEG-XS/blob/main/documentation/decoder/svt-jpegxs-decoder-design.md contain clear, 
unambiguous visuals representing band arrangement after DWT decomposition. -->

The project JXS\_polyptych demonstrates an accidental art sometimes "spontaneously" created in the course of 
image wavelet decomposition. With lucky selection of parameters the landscape can become transformed into 
the tetraptych (or, in general, polyptich), sort of "Four Seasons" or "Morning, Afternoon, Evening, Night" 
or something else. Most often, however, decomposition images resemble Piet Mondrian's 
figurative paintings if any art at all.
