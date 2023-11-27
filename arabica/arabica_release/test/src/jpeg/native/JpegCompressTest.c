#include <jni.h>
#include "cdjpeg.h"		/* Common decls for cjpeg/djpeg applications */
#include "jversion.h"		
#include "JpegCompressTest.h"

static const char * const cdjpeg_message_table[] = {
#include "cderror.h"
  NULL
};

static boolean is_targa;	/* records user -targa switch */


LOCAL(cjpeg_source_ptr)
select_file_type (j_compress_ptr cinfo, FILE * infile)
{
  int c;

  if (is_targa) {
#ifdef TARGA_SUPPORTED
    return jinit_read_targa(cinfo);
#else
    ERREXIT(cinfo, JERR_TGA_NOTCOMP);
#endif
  }

  if ((c = getc(infile)) == EOF)
    ERREXIT(cinfo, JERR_INPUT_EMPTY);
  if (ungetc(c, infile) == EOF)
    ERREXIT(cinfo, JERR_UNGETC_FAILED);

  switch (c) {
#ifdef BMP_SUPPORTED
  case 'B':
    return jinit_read_bmp(cinfo);
#endif
#ifdef GIF_SUPPORTED
  case 'G':
    return jinit_read_gif(cinfo);
#endif
#ifdef PPM_SUPPORTED
  case 'P':
    return jinit_read_ppm(cinfo);
#endif
#ifdef RLE_SUPPORTED
  case 'R':
    return jinit_read_rle(cinfo);
#endif
#ifdef TARGA_SUPPORTED
  case 0x00:
    return jinit_read_targa(cinfo);
#endif
  default:
    ERREXIT(cinfo, JERR_UNKNOWN_FORMAT);
    break;
  }

  return NULL;			/* suppress compiler warnings */
}

JNIEXPORT void JNICALL Java_jpeg_JpegCompressTest_compress(JNIEnv *env, jobject obj)
{
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;

  	int file_index;
  	cjpeg_source_ptr src_mgr;
  	FILE * input_file;
  	FILE * output_file;
  	JDIMENSION num_scanlines;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);

	jerr.addon_message_table = cdjpeg_message_table;
	jerr.first_addon_message = JMSG_FIRSTADDONCODE;
	jerr.last_addon_message = JMSG_LASTADDONCODE;
	
	//initialize the file structures
	cinfo.in_color_space = JCS_RGB; /* arbitrary guess */
	jpeg_set_defaults(&cinfo);

	//open the input file
	if ((input_file = fopen(".images/testimg.bmp", READ_BINARY)) == NULL) 
	{
		//error opening the input file...	
		printf("Error opening the input file...\n");
		fflush(stdout);
		return;
	}

	//open the output file
	if ((output_file = fopen("./testimg.jpg", WRITE_BINARY)) == NULL) 
	{
		//error opening the output file...
		printf("Error opening the output file...\n");
		fflush(stdout);
		return;
	}

	/* Figure out the input file format, and set up to read it. */
  	src_mgr = select_file_type(&cinfo, input_file);
	src_mgr->input_file = input_file;

	/* Read the input file header to obtain file size & colorspace. */
	(*src_mgr->start_input) (&cinfo, src_mgr);

	/* Now that we know input colorspace, fix colorspace-dependent defaults */
	jpeg_default_colorspace(&cinfo);

	/* Specify data destination for compression */
	jpeg_stdio_dest(&cinfo, output_file);

	/* Start compressor */
	jpeg_start_compress(&cinfo, TRUE);

	/* Process data */
	while (cinfo.next_scanline < cinfo.image_height) 
	{
		num_scanlines = (*src_mgr->get_pixel_rows) (&cinfo, src_mgr);
		(void) jpeg_write_scanlines(&cinfo, src_mgr->buffer, num_scanlines);
	}	

	/* Finish compression and release memory */
	(*src_mgr->finish_input) (&cinfo, src_mgr);
	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);

	fclose(input_file);
	fclose(output_file);
}

