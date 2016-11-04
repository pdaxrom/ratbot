#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/videodev2.h>
#include "cam/v4l2uvc.h"
#include <jpeglib.h>

int compress_yuyv_to_jpeg (struct vdIn *vd, unsigned char **outbuffer, long unsigned int *outlen, int quality)
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    JSAMPROW row_pointer[1];
    unsigned char *line_buffer, *yuyv;
    int z;

    *outbuffer = NULL;
    *outlen = 0;

    line_buffer = calloc (vd->width * 3, 1);
    yuyv = vd->framebuffer;

    cinfo.err = jpeg_std_error (&jerr);
    jpeg_create_compress (&cinfo);
    jpeg_mem_dest(&cinfo, outbuffer, outlen);

    cinfo.image_width = vd->width;
    cinfo.image_height = vd->height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;

    jpeg_set_defaults (&cinfo);
    jpeg_set_quality (&cinfo, quality, TRUE);

    jpeg_start_compress (&cinfo, TRUE);

    z = 0;
    while (cinfo.next_scanline < cinfo.image_height) {
	int x;
	unsigned char *ptr = line_buffer;

	for (x = 0; x < vd->width; x++) {
	    int r, g, b;
	    int y, u, v;

	    if (!z) {
		y = yuyv[0] << 8;
	    } else {
		y = yuyv[2] << 8;
	    }
	    u = yuyv[1] - 128;
	    v = yuyv[3] - 128;

	    r = (y + (359 * v)) >> 8;
	    g = (y - (88 * u) - (183 * v)) >> 8;
	    b = (y + (454 * u)) >> 8;

	    *(ptr++) = (r > 255) ? 255 : ((r < 0) ? 0 : r);
	    *(ptr++) = (g > 255) ? 255 : ((g < 0) ? 0 : g);
	    *(ptr++) = (b > 255) ? 255 : ((b < 0) ? 0 : b);

	    if (z++) {
		z = 0;
		yuyv += 4;
	    }
	}

	row_pointer[0] = line_buffer;
	jpeg_write_scanlines (&cinfo, row_pointer, 1);
    }

    jpeg_finish_compress (&cinfo);
    jpeg_destroy_compress (&cinfo);

    free (line_buffer);

    return (0);
}

