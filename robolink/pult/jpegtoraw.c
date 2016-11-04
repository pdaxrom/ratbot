#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <jpeglib.h>

static unsigned char *_outbuffer = NULL;
static unsigned int _outlen = 0;

int decompress_jpeg_to_raw(unsigned char *inbuffer, unsigned long inlen, unsigned char **outbuffer, unsigned int *width, unsigned int *height, int *pixelsize)
{
    int rc;
    unsigned int _oldoutlen;
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    int row_stride;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

    jpeg_mem_src(&cinfo, inbuffer, inlen);

    rc = jpeg_read_header(&cinfo, TRUE);
    if (rc != 1) {
	fprintf(stderr, "File does not seem to be a normal JPEG\n");
	return 1;
    }

    jpeg_start_decompress(&cinfo);

    *width = cinfo.output_width;
    *height = cinfo.output_height;
    *pixelsize = cinfo.output_components;

    fprintf(stderr, "image is %d by %d with %d components\n", *width, *height, *pixelsize);

    _oldoutlen = _outlen;
    _outlen = *width * *height * *pixelsize;

    if (_outlen > _oldoutlen) {
	if (_outbuffer) {
	    free(_outbuffer);
	}
	_outbuffer = malloc(_outlen);
    }

    row_stride = *width * *pixelsize;

    while (cinfo.output_scanline < cinfo.output_height) {
	unsigned char *buffer_array[1];
	buffer_array[0] = _outbuffer + (cinfo.output_scanline) * row_stride;
	jpeg_read_scanlines(&cinfo, buffer_array, 1);
    }

    fprintf(stderr, "jpeg decompressed!\n");

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    *outbuffer = _outbuffer;

    return 0;
}
