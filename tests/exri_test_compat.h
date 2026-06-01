#ifndef EXRI_TEST_COMPAT_H
#define EXRI_TEST_COMPAT_H

#ifndef EXRI_NO_STDIO
#include <stdio.h>
#endif

#if defined(__GNUC__) || defined(__clang__)
#define EXRI_TESTDEF static __attribute__((unused))
#else
#define EXRI_TESTDEF static
#endif

#define EXRI_WRITE_COMPRESSION_MASK 0x00ff
#define EXRI_WRITE_STORAGE_HALF     0x0100
#define EXRI_WRITE_TILED            0x0200
#define EXRI_WRITE_STORAGE_UINT     0x0400
#define EXRI_WRITE_TILED_MIPMAP     0x0800
#define EXRI_WRITE_TILED_RIPMAP     0x1000
#define EXRI_WRITE_TILED_ROUND_UP   0x2000
#define EXRI_WRITE_TILE_SIZE_SHIFT  16
#define EXRI_WRITE_TILE_SIZE_MASK   0x0fff0000

#define EXRI_COLOR_STRICT       EXRI_LOAD_SCRGB_STRICT
#define EXRI_COLOR_ASSUME_SCRGB EXRI_LOAD_SCRGB_ASSUME

EXRI_TESTDEF void exri_test_write_options_from_flags(exri_write_options *options, int flags)
{
   int compression;
   int known_flags;

   options->compression = EXRI_WRITE_COMPRESSION_NONE;
   options->pixel_type = EXRI_WRITE_PIXEL_FLOAT;
   options->tiled = 0;
   options->tile_size = 0;
   options->level_mode = EXRI_WRITE_LEVEL_ONE;
   options->level_rounding = EXRI_WRITE_ROUND_DOWN;

   compression = flags & EXRI_WRITE_COMPRESSION_MASK;
   known_flags = EXRI_WRITE_COMPRESSION_MASK |
                 EXRI_WRITE_STORAGE_HALF |
                 EXRI_WRITE_STORAGE_UINT |
                 EXRI_WRITE_TILED |
                 EXRI_WRITE_TILED_MIPMAP |
                 EXRI_WRITE_TILED_RIPMAP |
                 EXRI_WRITE_TILED_ROUND_UP |
                 EXRI_WRITE_TILE_SIZE_MASK;

   if ((flags & ~known_flags) != 0 ||
       ((flags & EXRI_WRITE_STORAGE_HALF) && (flags & EXRI_WRITE_STORAGE_UINT))) {
      options->compression = 99;
      return;
   }

   options->compression = compression;
   if (flags & EXRI_WRITE_STORAGE_HALF)
      options->pixel_type = EXRI_WRITE_PIXEL_HALF;
   if (flags & EXRI_WRITE_STORAGE_UINT)
      options->pixel_type = EXRI_WRITE_PIXEL_UINT;
   if (flags & EXRI_WRITE_TILED)
      options->tiled = 1;
   options->tile_size = (flags & EXRI_WRITE_TILE_SIZE_MASK) >> EXRI_WRITE_TILE_SIZE_SHIFT;
   if ((flags & EXRI_WRITE_TILED_MIPMAP) && (flags & EXRI_WRITE_TILED_RIPMAP))
      options->level_mode = 99;
   else if (flags & EXRI_WRITE_TILED_MIPMAP)
      options->level_mode = EXRI_WRITE_LEVEL_MIPMAP;
   else if (flags & EXRI_WRITE_TILED_RIPMAP)
      options->level_mode = EXRI_WRITE_LEVEL_RIPMAP;
   if (flags & EXRI_WRITE_TILED_ROUND_UP)
      options->level_rounding = EXRI_WRITE_ROUND_UP;
}

EXRI_TESTDEF int exri_test_writef_to_memory(exri_uc **out_data, int *out_len, int w, int h, int comp, float const *data, int flags)
{
   exri_write_options options;
   size_t len;
   int ok;

   exri_test_write_options_from_flags(&options, flags);
   len = 0;
   ok = exri_writef_to_memory(out_data, &len, w, h, comp, data, &options);
   if (out_len)
      *out_len = len > (size_t) 0x7fffffff ? 0 : (int) len;
   return ok && len <= (size_t) 0x7fffffff;
}

EXRI_TESTDEF int exri_test_writef_channels_to_memory(exri_uc **out_data, int *out_len, int w, int h, int num_channels, float const *data, exri_write_channel const *channels, exri_write_attribute const *attributes, int num_attributes, int flags)
{
   exri_write_options options;
   size_t len;
   int ok;

   exri_test_write_options_from_flags(&options, flags);
   len = 0;
   ok = exri_writef_channels_to_memory(out_data, &len, w, h, num_channels, data, channels, attributes, num_attributes, &options);
   if (out_len)
      *out_len = len > (size_t) 0x7fffffff ? 0 : (int) len;
   return ok && len <= (size_t) 0x7fffffff;
}

EXRI_TESTDEF int exri_test_writef_to_callbacks(exri_write_callbacks const *clbk, void *user, int w, int h, int comp, float const *data, int flags)
{
   exri_write_options options;
   exri_test_write_options_from_flags(&options, flags);
   return exri_writef_to_callbacks(clbk, user, w, h, comp, data, &options);
}

EXRI_TESTDEF int exri_test_writef_channels_to_callbacks(exri_write_callbacks const *clbk, void *user, int w, int h, int num_channels, float const *data, exri_write_channel const *channels, exri_write_attribute const *attributes, int num_attributes, int flags)
{
   exri_write_options options;
   exri_test_write_options_from_flags(&options, flags);
   return exri_writef_channels_to_callbacks(clbk, user, w, h, num_channels, data, channels, attributes, num_attributes, &options);
}

#ifndef EXRI_NO_STDIO
EXRI_TESTDEF int exri_test_writef(char const *filename, int w, int h, int comp, float const *data, int flags)
{
   exri_write_options options;
   exri_test_write_options_from_flags(&options, flags);
   return exri_writef(filename, w, h, comp, data, &options);
}

EXRI_TESTDEF int exri_test_writef_channels(char const *filename, int w, int h, int num_channels, float const *data, exri_write_channel const *channels, exri_write_attribute const *attributes, int num_attributes, int flags)
{
   exri_write_options options;
   exri_test_write_options_from_flags(&options, flags);
   return exri_writef_channels(filename, w, h, num_channels, data, channels, attributes, num_attributes, &options);
}

EXRI_TESTDEF int exri_test_writef_to_file(FILE *f, int w, int h, int comp, float const *data, int flags)
{
   exri_uc *bytes;
   int len;
   size_t written;

   bytes = NULL;
   len = 0;
   if (!exri_test_writef_to_memory(&bytes, &len, w, h, comp, data, flags))
      return 0;
   written = fwrite(bytes, 1, (size_t) len, f);
   exri_image_free(bytes);
   return written == (size_t) len;
}

EXRI_TESTDEF int exri_test_writef_channels_to_file(FILE *f, int w, int h, int num_channels, float const *data, exri_write_channel const *channels, exri_write_attribute const *attributes, int num_attributes, int flags)
{
   exri_uc *bytes;
   int len;
   size_t written;

   bytes = NULL;
   len = 0;
   if (!exri_test_writef_channels_to_memory(&bytes, &len, w, h, num_channels, data, channels, attributes, num_attributes, flags))
      return 0;
   written = fwrite(bytes, 1, (size_t) len, f);
   exri_image_free(bytes);
   return written == (size_t) len;
}
#endif

EXRI_TESTDEF float *exri_test_loadf_from_memory(exri_uc const *buffer, size_t len, int *x, int *y, int *channels_in_file, int desired_channels)
{
   float *pixels;
   pixels = NULL;
   if (!exri_loadf_from_memory(&pixels, buffer, len, x, y, channels_in_file, desired_channels, EXRI_LOAD_DEFAULT))
      return NULL;
   return pixels;
}

EXRI_TESTDEF float *exri_test_loadf_scrgb_from_memory(exri_uc const *buffer, size_t len, int *x, int *y, int *channels_in_file, int desired_channels, int load_flags)
{
   float *pixels;
   pixels = NULL;
   if (!exri_loadf_from_memory(&pixels, buffer, len, x, y, channels_in_file, desired_channels, load_flags))
      return NULL;
   return pixels;
}

EXRI_TESTDEF float *exri_test_loadf_part_from_memory(exri_uc const *buffer, size_t len, int part_index, int *x, int *y, int *channels_in_file, int desired_channels)
{
   float *pixels;
   pixels = NULL;
   if (!exri_loadf_part_from_memory(&pixels, buffer, len, part_index, x, y, channels_in_file, desired_channels, EXRI_LOAD_DEFAULT))
      return NULL;
   return pixels;
}

EXRI_TESTDEF float *exri_test_loadf_channels_from_memory(exri_uc const *buffer, size_t len, int *x, int *y, int *num_channels)
{
   float *pixels;
   pixels = NULL;
   if (!exri_loadf_channels_from_memory(&pixels, buffer, len, x, y, num_channels))
      return NULL;
   return pixels;
}

EXRI_TESTDEF float *exri_test_loadf_part_channels_from_memory(exri_uc const *buffer, size_t len, int part_index, int *x, int *y, int *num_channels)
{
   float *pixels;
   pixels = NULL;
   if (!exri_loadf_part_channels_from_memory(&pixels, buffer, len, part_index, x, y, num_channels))
      return NULL;
   return pixels;
}

EXRI_TESTDEF float *exri_test_loadf_region_from_memory(exri_uc const *buffer, size_t len, int region_x, int region_y, int region_w, int region_h, int *x, int *y, int *channels_in_file, int desired_channels)
{
   float *pixels;
   pixels = NULL;
   if (!exri_loadf_region_from_memory(&pixels, buffer, len, region_x, region_y, region_w, region_h, x, y, channels_in_file, desired_channels, EXRI_LOAD_DEFAULT))
      return NULL;
   return pixels;
}

EXRI_TESTDEF float *exri_test_loadf_channels_region_from_memory(exri_uc const *buffer, size_t len, int region_x, int region_y, int region_w, int region_h, int *x, int *y, int *num_channels)
{
   float *pixels;
   pixels = NULL;
   if (!exri_loadf_channels_region_from_memory(&pixels, buffer, len, region_x, region_y, region_w, region_h, x, y, num_channels))
      return NULL;
   return pixels;
}

EXRI_TESTDEF float *exri_test_loadf_tiled_level_from_memory(exri_uc const *buffer, size_t len, int level_x, int level_y, int *x, int *y, int *channels_in_file, int desired_channels)
{
   float *pixels;
   pixels = NULL;
   if (!exri_loadf_tiled_level_from_memory(&pixels, buffer, len, level_x, level_y, x, y, channels_in_file, desired_channels, EXRI_LOAD_DEFAULT))
      return NULL;
   return pixels;
}

EXRI_TESTDEF float *exri_test_loadf_from_callbacks(exri_io_callbacks const *clbk, void *user, int *x, int *y, int *channels_in_file, int desired_channels)
{
   float *pixels;
   pixels = NULL;
   if (!exri_loadf_from_callbacks(&pixels, clbk, user, x, y, channels_in_file, desired_channels, EXRI_LOAD_DEFAULT))
      return NULL;
   return pixels;
}

#ifndef EXRI_NO_STDIO
EXRI_TESTDEF float *exri_test_loadf(char const *filename, int *x, int *y, int *channels_in_file, int desired_channels)
{
   float *pixels;
   pixels = NULL;
   if (!exri_loadf(&pixels, filename, x, y, channels_in_file, desired_channels, EXRI_LOAD_DEFAULT))
      return NULL;
   return pixels;
}

EXRI_TESTDEF float *exri_test_loadf_layer(char const *filename, char const *layer, int *x, int *y, int *channels_in_file, int desired_channels)
{
   float *pixels;
   pixels = NULL;
   if (!exri_loadf_layer(&pixels, filename, layer, x, y, channels_in_file, desired_channels, EXRI_LOAD_DEFAULT))
      return NULL;
   return pixels;
}

EXRI_TESTDEF float *exri_test_loadf_region(char const *filename, int region_x, int region_y, int region_w, int region_h, int *x, int *y, int *channels_in_file, int desired_channels)
{
   float *pixels;
   pixels = NULL;
   if (!exri_loadf_region(&pixels, filename, region_x, region_y, region_w, region_h, x, y, channels_in_file, desired_channels, EXRI_LOAD_DEFAULT))
      return NULL;
   return pixels;
}

EXRI_TESTDEF float *exri_test_loadf_layer_region(char const *filename, char const *layer, int region_x, int region_y, int region_w, int region_h, int *x, int *y, int *channels_in_file, int desired_channels)
{
   float *pixels;
   pixels = NULL;
   if (!exri_loadf_layer_region(&pixels, filename, layer, region_x, region_y, region_w, region_h, x, y, channels_in_file, desired_channels, EXRI_LOAD_DEFAULT))
      return NULL;
   return pixels;
}

EXRI_TESTDEF float *exri_test_loadf_channels(char const *filename, int *x, int *y, int *num_channels)
{
   float *pixels;
   pixels = NULL;
   if (!exri_loadf_channels(&pixels, filename, x, y, num_channels))
      return NULL;
   return pixels;
}

EXRI_TESTDEF float *exri_test_loadf_channels_region(char const *filename, int region_x, int region_y, int region_w, int region_h, int *x, int *y, int *num_channels)
{
   float *pixels;
   pixels = NULL;
   if (!exri_loadf_channels_region(&pixels, filename, region_x, region_y, region_w, region_h, x, y, num_channels))
      return NULL;
   return pixels;
}

EXRI_TESTDEF float *exri_test_loadf_tiled_level(char const *filename, int level_x, int level_y, int *x, int *y, int *channels_in_file, int desired_channels)
{
   float *pixels;
   pixels = NULL;
   if (!exri_loadf_tiled_level(&pixels, filename, level_x, level_y, x, y, channels_in_file, desired_channels, EXRI_LOAD_DEFAULT))
      return NULL;
   return pixels;
}

EXRI_TESTDEF float *exri_test_loadf_tiled_level_region(char const *filename, int level_x, int level_y, int region_x, int region_y, int region_w, int region_h, int *x, int *y, int *channels_in_file, int desired_channels)
{
   float *pixels;
   pixels = NULL;
   if (!exri_loadf_tiled_level_region(&pixels, filename, level_x, level_y, region_x, region_y, region_w, region_h, x, y, channels_in_file, desired_channels, EXRI_LOAD_DEFAULT))
      return NULL;
   return pixels;
}

EXRI_TESTDEF int exri_test_loadf_to(float **out_pixels, char const *filename, int *x, int *y, int *channels_in_file, int desired_channels)
{
   return exri_loadf(out_pixels, filename, x, y, channels_in_file, desired_channels, EXRI_LOAD_DEFAULT);
}

EXRI_TESTDEF int exri_test_loadf_part_to(float **out_pixels, char const *filename, int part_index, int *x, int *y, int *channels_in_file, int desired_channels)
{
   return exri_loadf_part(out_pixels, filename, part_index, x, y, channels_in_file, desired_channels, EXRI_LOAD_DEFAULT);
}

EXRI_TESTDEF int exri_test_loadf_layer_to(float **out_pixels, char const *filename, char const *layer, int *x, int *y, int *channels_in_file, int desired_channels)
{
   return exri_loadf_layer(out_pixels, filename, layer, x, y, channels_in_file, desired_channels, EXRI_LOAD_DEFAULT);
}

EXRI_TESTDEF int exri_test_loadf_region_to(float **out_pixels, char const *filename, int region_x, int region_y, int region_w, int region_h, int *x, int *y, int *channels_in_file, int desired_channels)
{
   return exri_loadf_region(out_pixels, filename, region_x, region_y, region_w, region_h, x, y, channels_in_file, desired_channels, EXRI_LOAD_DEFAULT);
}

EXRI_TESTDEF int exri_test_loadf_layer_region_to(float **out_pixels, char const *filename, char const *layer, int region_x, int region_y, int region_w, int region_h, int *x, int *y, int *channels_in_file, int desired_channels)
{
   return exri_loadf_layer_region(out_pixels, filename, layer, region_x, region_y, region_w, region_h, x, y, channels_in_file, desired_channels, EXRI_LOAD_DEFAULT);
}

EXRI_TESTDEF int exri_test_loadf_channels_to(float **out_pixels, char const *filename, int *x, int *y, int *num_channels)
{
   return exri_loadf_channels(out_pixels, filename, x, y, num_channels);
}

EXRI_TESTDEF int exri_test_loadf_part_channels_to(float **out_pixels, char const *filename, int part_index, int *x, int *y, int *num_channels)
{
   return exri_loadf_part_channels(out_pixels, filename, part_index, x, y, num_channels);
}

EXRI_TESTDEF int exri_test_loadf_channels_region_to(float **out_pixels, char const *filename, int region_x, int region_y, int region_w, int region_h, int *x, int *y, int *num_channels)
{
   return exri_loadf_channels_region(out_pixels, filename, region_x, region_y, region_w, region_h, x, y, num_channels);
}

EXRI_TESTDEF int exri_test_loadf_tiled_level_to(float **out_pixels, char const *filename, int level_x, int level_y, int *x, int *y, int *channels_in_file, int desired_channels)
{
   return exri_loadf_tiled_level(out_pixels, filename, level_x, level_y, x, y, channels_in_file, desired_channels, EXRI_LOAD_DEFAULT);
}

EXRI_TESTDEF int exri_test_loadf_part_tiled_level_to(float **out_pixels, char const *filename, int part_index, int level_x, int level_y, int *x, int *y, int *channels_in_file, int desired_channels)
{
   return exri_loadf_part_tiled_level(out_pixels, filename, part_index, level_x, level_y, x, y, channels_in_file, desired_channels, EXRI_LOAD_DEFAULT);
}

EXRI_TESTDEF int exri_test_loadf_tiled_level_region_to(float **out_pixels, char const *filename, int level_x, int level_y, int region_x, int region_y, int region_w, int region_h, int *x, int *y, int *channels_in_file, int desired_channels)
{
   return exri_loadf_tiled_level_region(out_pixels, filename, level_x, level_y, region_x, region_y, region_w, region_h, x, y, channels_in_file, desired_channels, EXRI_LOAD_DEFAULT);
}
#endif

EXRI_TESTDEF int exri_test_loadf_from_memory_to(float **out_pixels, exri_uc const *buffer, size_t len, int *x, int *y, int *channels_in_file, int desired_channels)
{
   return exri_loadf_from_memory(out_pixels, buffer, len, x, y, channels_in_file, desired_channels, EXRI_LOAD_DEFAULT);
}

EXRI_TESTDEF int exri_test_loadf_scrgb_from_memory_to(float **out_pixels, exri_uc const *buffer, size_t len, int *x, int *y, int *channels_in_file, int desired_channels, int load_flags)
{
   return exri_loadf_from_memory(out_pixels, buffer, len, x, y, channels_in_file, desired_channels, load_flags);
}

EXRI_TESTDEF int exri_test_loadf_part_from_memory_to(float **out_pixels, exri_uc const *buffer, size_t len, int part_index, int *x, int *y, int *channels_in_file, int desired_channels)
{
   return exri_loadf_part_from_memory(out_pixels, buffer, len, part_index, x, y, channels_in_file, desired_channels, EXRI_LOAD_DEFAULT);
}

EXRI_TESTDEF int exri_test_loadf_region_from_memory_to(float **out_pixels, exri_uc const *buffer, size_t len, int region_x, int region_y, int region_w, int region_h, int *x, int *y, int *channels_in_file, int desired_channels)
{
   return exri_loadf_region_from_memory(out_pixels, buffer, len, region_x, region_y, region_w, region_h, x, y, channels_in_file, desired_channels, EXRI_LOAD_DEFAULT);
}

EXRI_TESTDEF int exri_test_loadf_channels_from_memory_to(float **out_pixels, exri_uc const *buffer, size_t len, int *x, int *y, int *num_channels)
{
   return exri_loadf_channels_from_memory(out_pixels, buffer, len, x, y, num_channels);
}

EXRI_TESTDEF int exri_test_loadf_part_channels_from_memory_to(float **out_pixels, exri_uc const *buffer, size_t len, int part_index, int *x, int *y, int *num_channels)
{
   return exri_loadf_part_channels_from_memory(out_pixels, buffer, len, part_index, x, y, num_channels);
}

EXRI_TESTDEF int exri_test_loadf_channels_region_from_memory_to(float **out_pixels, exri_uc const *buffer, size_t len, int region_x, int region_y, int region_w, int region_h, int *x, int *y, int *num_channels)
{
   return exri_loadf_channels_region_from_memory(out_pixels, buffer, len, region_x, region_y, region_w, region_h, x, y, num_channels);
}

#define exri_writef_to_memory(out_data,out_len,w,h,comp,data,flags) exri_test_writef_to_memory(out_data,out_len,w,h,comp,data,flags)
#define exri_writef_channels_to_memory(out_data,out_len,w,h,num_channels,data,channels,attributes,num_attributes,flags) exri_test_writef_channels_to_memory(out_data,out_len,w,h,num_channels,data,channels,attributes,num_attributes,flags)
#define exri_writef_to_callbacks(clbk,user,w,h,comp,data,flags) exri_test_writef_to_callbacks(clbk,user,w,h,comp,data,flags)
#define exri_writef_channels_to_callbacks(clbk,user,w,h,num_channels,data,channels,attributes,num_attributes,flags) exri_test_writef_channels_to_callbacks(clbk,user,w,h,num_channels,data,channels,attributes,num_attributes,flags)

#define exri_loadf_from_memory(buffer,len,x,y,channels_in_file,desired_channels) exri_test_loadf_from_memory(buffer,(size_t) (len),x,y,channels_in_file,desired_channels)
#define exri_loadf_scrgb_from_memory(buffer,len,x,y,channels_in_file,desired_channels,load_flags) exri_test_loadf_scrgb_from_memory(buffer,(size_t) (len),x,y,channels_in_file,desired_channels,load_flags)
#define exri_loadf_part_from_memory(buffer,len,part_index,x,y,channels_in_file,desired_channels) exri_test_loadf_part_from_memory(buffer,(size_t) (len),part_index,x,y,channels_in_file,desired_channels)
#define exri_loadf_channels_from_memory(buffer,len,x,y,num_channels) exri_test_loadf_channels_from_memory(buffer,(size_t) (len),x,y,num_channels)
#define exri_loadf_part_channels_from_memory(buffer,len,part_index,x,y,num_channels) exri_test_loadf_part_channels_from_memory(buffer,(size_t) (len),part_index,x,y,num_channels)
#define exri_loadf_region_from_memory(buffer,len,rx,ry,rw,rh,x,y,channels_in_file,desired_channels) exri_test_loadf_region_from_memory(buffer,(size_t) (len),rx,ry,rw,rh,x,y,channels_in_file,desired_channels)
#define exri_loadf_channels_region_from_memory(buffer,len,rx,ry,rw,rh,x,y,num_channels) exri_test_loadf_channels_region_from_memory(buffer,(size_t) (len),rx,ry,rw,rh,x,y,num_channels)
#define exri_loadf_tiled_level_from_memory(buffer,len,level_x,level_y,x,y,channels_in_file,desired_channels) exri_test_loadf_tiled_level_from_memory(buffer,(size_t) (len),level_x,level_y,x,y,channels_in_file,desired_channels)
#define exri_loadf_from_callbacks(clbk,user,x,y,channels_in_file,desired_channels) exri_test_loadf_from_callbacks(clbk,user,x,y,channels_in_file,desired_channels)

#define exri_loadf_from_memory_to(out,buffer,len,x,y,channels_in_file,desired_channels) exri_test_loadf_from_memory_to(out,buffer,(size_t) (len),x,y,channels_in_file,desired_channels)
#define exri_loadf_scrgb_from_memory_to(out,buffer,len,x,y,channels_in_file,desired_channels,load_flags) exri_test_loadf_scrgb_from_memory_to(out,buffer,(size_t) (len),x,y,channels_in_file,desired_channels,load_flags)
#define exri_loadf_part_from_memory_to(out,buffer,len,part_index,x,y,channels_in_file,desired_channels) exri_test_loadf_part_from_memory_to(out,buffer,(size_t) (len),part_index,x,y,channels_in_file,desired_channels)
#define exri_loadf_region_from_memory_to(out,buffer,len,rx,ry,rw,rh,x,y,channels_in_file,desired_channels) exri_test_loadf_region_from_memory_to(out,buffer,(size_t) (len),rx,ry,rw,rh,x,y,channels_in_file,desired_channels)
#define exri_loadf_channels_from_memory_to(out,buffer,len,x,y,num_channels) exri_test_loadf_channels_from_memory_to(out,buffer,(size_t) (len),x,y,num_channels)
#define exri_loadf_part_channels_from_memory_to(out,buffer,len,part_index,x,y,num_channels) exri_test_loadf_part_channels_from_memory_to(out,buffer,(size_t) (len),part_index,x,y,num_channels)
#define exri_loadf_channels_region_from_memory_to(out,buffer,len,rx,ry,rw,rh,x,y,num_channels) exri_test_loadf_channels_region_from_memory_to(out,buffer,(size_t) (len),rx,ry,rw,rh,x,y,num_channels)

#define exri_load_deep_from_memory_to(out_samples,out_offsets,buffer,len,x,y,num_channels,total_samples) exri_load_deep_part_from_memory(out_samples,out_offsets,buffer,len,0,x,y,num_channels,total_samples)
#define exri_load_deep_part_from_memory_to(out_samples,out_offsets,buffer,len,part_index,x,y,num_channels,total_samples) exri_load_deep_part_from_memory(out_samples,out_offsets,buffer,len,part_index,x,y,num_channels,total_samples)
#define exri_load_deep_part_from_callbacks_to(out_samples,out_offsets,clbk,user,part_index,x,y,num_channels,total_samples) exri_load_deep_part_from_callbacks(out_samples,out_offsets,clbk,user,part_index,x,y,num_channels,total_samples)

#ifndef EXRI_NO_STDIO
#define exri_writef(filename,w,h,comp,data,flags) exri_test_writef(filename,w,h,comp,data,flags)
#define exri_writef_channels(filename,w,h,num_channels,data,channels,attributes,num_attributes,flags) exri_test_writef_channels(filename,w,h,num_channels,data,channels,attributes,num_attributes,flags)
#define exri_writef_to_file(file,w,h,comp,data,flags) exri_test_writef_to_file(file,w,h,comp,data,flags)
#define exri_writef_channels_to_file(file,w,h,num_channels,data,channels,attributes,num_attributes,flags) exri_test_writef_channels_to_file(file,w,h,num_channels,data,channels,attributes,num_attributes,flags)

#define exri_loadf(filename,x,y,channels_in_file,desired_channels) exri_test_loadf(filename,x,y,channels_in_file,desired_channels)
#define exri_loadf_layer(filename,layer,x,y,channels_in_file,desired_channels) exri_test_loadf_layer(filename,layer,x,y,channels_in_file,desired_channels)
#define exri_loadf_region(filename,rx,ry,rw,rh,x,y,channels_in_file,desired_channels) exri_test_loadf_region(filename,rx,ry,rw,rh,x,y,channels_in_file,desired_channels)
#define exri_loadf_layer_region(filename,layer,rx,ry,rw,rh,x,y,channels_in_file,desired_channels) exri_test_loadf_layer_region(filename,layer,rx,ry,rw,rh,x,y,channels_in_file,desired_channels)
#define exri_loadf_channels(filename,x,y,num_channels) exri_test_loadf_channels(filename,x,y,num_channels)
#define exri_loadf_channels_region(filename,rx,ry,rw,rh,x,y,num_channels) exri_test_loadf_channels_region(filename,rx,ry,rw,rh,x,y,num_channels)
#define exri_loadf_tiled_level(filename,level_x,level_y,x,y,channels_in_file,desired_channels) exri_test_loadf_tiled_level(filename,level_x,level_y,x,y,channels_in_file,desired_channels)
#define exri_loadf_tiled_level_region(filename,level_x,level_y,rx,ry,rw,rh,x,y,channels_in_file,desired_channels) exri_test_loadf_tiled_level_region(filename,level_x,level_y,rx,ry,rw,rh,x,y,channels_in_file,desired_channels)

#define exri_loadf_to(out,filename,x,y,channels_in_file,desired_channels) exri_test_loadf_to(out,filename,x,y,channels_in_file,desired_channels)
#define exri_loadf_part_to(out,filename,part_index,x,y,channels_in_file,desired_channels) exri_test_loadf_part_to(out,filename,part_index,x,y,channels_in_file,desired_channels)
#define exri_loadf_layer_to(out,filename,layer,x,y,channels_in_file,desired_channels) exri_test_loadf_layer_to(out,filename,layer,x,y,channels_in_file,desired_channels)
#define exri_loadf_region_to(out,filename,rx,ry,rw,rh,x,y,channels_in_file,desired_channels) exri_test_loadf_region_to(out,filename,rx,ry,rw,rh,x,y,channels_in_file,desired_channels)
#define exri_loadf_layer_region_to(out,filename,layer,rx,ry,rw,rh,x,y,channels_in_file,desired_channels) exri_test_loadf_layer_region_to(out,filename,layer,rx,ry,rw,rh,x,y,channels_in_file,desired_channels)
#define exri_loadf_channels_to(out,filename,x,y,num_channels) exri_test_loadf_channels_to(out,filename,x,y,num_channels)
#define exri_loadf_part_channels_to(out,filename,part_index,x,y,num_channels) exri_test_loadf_part_channels_to(out,filename,part_index,x,y,num_channels)
#define exri_loadf_channels_region_to(out,filename,rx,ry,rw,rh,x,y,num_channels) exri_test_loadf_channels_region_to(out,filename,rx,ry,rw,rh,x,y,num_channels)
#define exri_loadf_tiled_level_to(out,filename,level_x,level_y,x,y,channels_in_file,desired_channels) exri_test_loadf_tiled_level_to(out,filename,level_x,level_y,x,y,channels_in_file,desired_channels)
#define exri_loadf_part_tiled_level_to(out,filename,part_index,level_x,level_y,x,y,channels_in_file,desired_channels) exri_test_loadf_part_tiled_level_to(out,filename,part_index,level_x,level_y,x,y,channels_in_file,desired_channels)
#define exri_loadf_tiled_level_region_to(out,filename,level_x,level_y,rx,ry,rw,rh,x,y,channels_in_file,desired_channels) exri_test_loadf_tiled_level_region_to(out,filename,level_x,level_y,rx,ry,rw,rh,x,y,channels_in_file,desired_channels)
#define exri_load_deep_to(out_samples,out_offsets,filename,x,y,num_channels,total_samples) exri_load_deep_part(out_samples,out_offsets,filename,0,x,y,num_channels,total_samples)
#define exri_load_deep_part_to(out_samples,out_offsets,filename,part_index,x,y,num_channels,total_samples) exri_load_deep_part(out_samples,out_offsets,filename,part_index,x,y,num_channels,total_samples)
#endif

#endif
