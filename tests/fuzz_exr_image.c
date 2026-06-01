#include <limits.h>
#include <stddef.h>
#include <string.h>

#define EXRI_NO_STDIO
#define EXR_IMAGE_IMPLEMENTATION
#include "../exr_image.h"

#ifdef EXRI_FUZZ_STANDALONE
#include <stdio.h>
#include <stdlib.h>
#endif

typedef struct
{
   unsigned char const *data;
   size_t len;
   size_t pos;
} exri_fuzz_reader;

static int EXRI_CALLBACK exri_fuzz_read(void *user, void *data, size_t size, size_t *bytes_read);
static int EXRI_CALLBACK exri_fuzz_skip(void *user, size_t n);
static int EXRI_CALLBACK exri_fuzz_eof(void *user);
static void exri_fuzz_call_callbacks(unsigned char const *data, size_t len);
static void exri_fuzz_metadata(unsigned char const *data, size_t len);
static void exri_fuzz_loaders(unsigned char const *data, size_t len);
int LLVMFuzzerTestOneInput(unsigned char const *data, size_t size);

static int EXRI_CALLBACK exri_fuzz_read(void *user, void *data, size_t size, size_t *bytes_read)
{
   exri_fuzz_reader *reader;
   size_t remaining;

   reader = (exri_fuzz_reader *) user;
   if (reader == NULL || data == NULL || bytes_read == NULL)
      return 0;
   if (reader->pos >= reader->len) {
      *bytes_read = 0;
      return 1;
   }

   remaining = reader->len - reader->pos;
   if (size > remaining)
      size = remaining;
   memcpy(data, reader->data + reader->pos, size);
   reader->pos += size;
   *bytes_read = size;
   return 1;
}

static int EXRI_CALLBACK exri_fuzz_skip(void *user, size_t n)
{
   exri_fuzz_reader *reader;

   reader = (exri_fuzz_reader *) user;
   if (reader == NULL)
      return 0;
   if (n > reader->len - reader->pos) {
      reader->pos = reader->len;
      return 1;
   }
   reader->pos += n;
   return 1;
}

static int EXRI_CALLBACK exri_fuzz_eof(void *user)
{
   exri_fuzz_reader *reader;

   reader = (exri_fuzz_reader *) user;
   if (reader == NULL)
      return 1;
   return reader->pos >= reader->len;
}

static void exri_fuzz_call_callbacks(unsigned char const *data, size_t len)
{
   exri_io_callbacks callbacks;
   exri_fuzz_reader reader;
   float *pixels;
   float *samples;
   int *offsets;
   int x;
   int y;
   int c;
   int total_samples;
   int version;
   int flags;

   callbacks.read = exri_fuzz_read;
   callbacks.skip = exri_fuzz_skip;
   callbacks.eof = exri_fuzz_eof;

   reader.data = data;
   reader.len = len;
   reader.pos = 0;
   (void) exri_is_exr_from_callbacks(&callbacks, &reader);

   reader.pos = 0;
   version = 0;
   flags = 0;
   (void) exri_version_from_callbacks(&callbacks, &reader, &version, &flags);

   reader.pos = 0;
   x = y = c = 0;
   (void) exri_info_from_callbacks(&callbacks, &reader, &x, &y, &c);

   reader.pos = 0;
   pixels = NULL;
   x = y = c = 0;
   if (exri_loadf_from_callbacks(&pixels, &callbacks, &reader, &x, &y, &c, 4, EXRI_LOAD_DEFAULT))
      exri_image_free(pixels);

   reader.pos = 0;
   samples = NULL;
   offsets = NULL;
   x = y = c = total_samples = 0;
   if (exri_load_deep_part_from_callbacks(&samples, &offsets, &callbacks, &reader, 0, &x, &y, &c, &total_samples)) {
      exri_image_free(samples);
      exri_image_free(offsets);
   }
}

static void exri_fuzz_metadata(unsigned char const *data, size_t len)
{
   char name[128];
   char type[128];
   char units[128];
   unsigned char value[128];
   float wavelengths[16];
   int parts;
   int part_index;
   int part_limit;
   int channels;
   int channel_index;
   int channel_limit;
   int attributes;
   int attribute_index;
   int attribute_limit;
   int value_size;
   int bytes_written;
   int x;
   int y;
   int c;
   int total_samples;
   int pixel_type;
   int x_sampling;
   int y_sampling;
   int p_linear;
   int num_x_levels;
   int num_y_levels;
   int layers;
   int layer_limit;
   int layer_index;
   int spectrum_type;
   int num_wavelengths;
   int version;
   int flags;

   parts = 1;
   (void) exri_is_exr_from_memory(data, len);
   version = flags = 0;
   (void) exri_version_from_memory(data, len, &version, &flags);
   x = y = c = 0;
   (void) exri_info_from_memory(data, len, &x, &y, &c);
   if (exri_part_count_from_memory(data, len, &parts) == 0 || parts < 1)
      parts = 1;
   part_limit = parts < 4 ? parts : 4;

   for (part_index = 0; part_index < part_limit; ++part_index) {
      x = y = c = 0;
      (void) exri_part_info_from_memory(data, len, part_index, &x, &y, &c);

      channels = 0;
      (void) exri_part_channel_count_from_memory(data, len, part_index, &channels);
      if (channels < 0)
         channels = 0;
      channel_limit = channels < 8 ? channels : 8;
      for (channel_index = 0; channel_index < channel_limit; ++channel_index) {
         (void) exri_part_channel_name_from_memory(data, len, part_index, channel_index, name, (int) sizeof(name));
         pixel_type = 0;
         (void) exri_part_channel_pixel_type_from_memory(data, len, part_index, channel_index, &pixel_type);
         x_sampling = y_sampling = p_linear = 0;
         (void) exri_part_channel_sampling_from_memory(data, len, part_index, channel_index, &x_sampling, &y_sampling, &p_linear);
      }

      attributes = 0;
      (void) exri_part_attribute_count_from_memory(data, len, part_index, &attributes);
      if (attributes < 0)
         attributes = 0;
      attribute_limit = attributes < 8 ? attributes : 8;
      for (attribute_index = 0; attribute_index < attribute_limit; ++attribute_index) {
         (void) exri_part_attribute_name_from_memory(data, len, part_index, attribute_index, name, (int) sizeof(name));
         (void) exri_part_attribute_type_from_memory(data, len, part_index, attribute_index, type, (int) sizeof(type));
         value_size = 0;
         if (exri_part_attribute_value_size_from_memory(data, len, part_index, attribute_index, &value_size) && value_size > 0 && value_size <= (int) sizeof(value)) {
            bytes_written = 0;
            (void) exri_part_attribute_value_from_memory(data, len, part_index, attribute_index, value, (int) sizeof(value), &bytes_written);
         }
      }

      num_x_levels = num_y_levels = 0;
      (void) exri_part_tiled_level_count_from_memory(data, len, part_index, &num_x_levels, &num_y_levels);

      x = y = c = total_samples = 0;
      (void) exri_deep_part_info_from_memory(data, len, part_index, &x, &y, &c, &total_samples);
      channels = 0;
      (void) exri_deep_part_channel_count_from_memory(data, len, part_index, &channels);
      if (channels < 0)
         channels = 0;
      channel_limit = channels < 8 ? channels : 8;
      for (channel_index = 0; channel_index < channel_limit; ++channel_index) {
         (void) exri_deep_part_channel_name_from_memory(data, len, part_index, channel_index, name, (int) sizeof(name));
         pixel_type = 0;
         (void) exri_deep_part_channel_pixel_type_from_memory(data, len, part_index, channel_index, &pixel_type);
         x_sampling = y_sampling = p_linear = 0;
         (void) exri_deep_part_channel_sampling_from_memory(data, len, part_index, channel_index, &x_sampling, &y_sampling, &p_linear);
      }
   }

   layers = 0;
   if (exri_layer_count_from_memory(data, len, &layers) && layers > 0) {
      layer_limit = layers < 8 ? layers : 8;
      for (layer_index = 0; layer_index < layer_limit; ++layer_index)
         (void) exri_layer_name_from_memory(data, len, layer_index, name, (int) sizeof(name));
   }

   (void) exri_is_spectral_channel_name("R");
   (void) exri_parse_spectral_wavelength("S700");
   (void) exri_spectral_stokes_component("S700");
   (void) exri_is_spectral_from_memory(data, len);
   spectrum_type = 0;
   (void) exri_spectrum_type_from_memory(data, len, &spectrum_type);
   num_wavelengths = 0;
   (void) exri_spectral_wavelengths_from_memory(data, len, wavelengths, 16, &num_wavelengths);
   (void) exri_spectral_units_from_memory(data, len, units, (int) sizeof(units));
}

static void exri_fuzz_loaders(unsigned char const *data, size_t len)
{
   float *pixels;
   float *samples;
   int *offsets;
   int x;
   int y;
   int c;
   int total_samples;
   int region_w;
   int region_h;
   int parts;
   int part_limit;
   int part_index;
   int levels_x;
   int levels_y;
   char layer[128];

   pixels = NULL;
   x = y = c = 0;
   if (exri_loadf_from_memory(&pixels, data, len, &x, &y, &c, 4, EXRI_LOAD_DEFAULT))
      exri_image_free(pixels);

   pixels = NULL;
   x = y = c = 0;
   if (exri_loadf_from_memory(&pixels, data, len, &x, &y, &c, 4, EXRI_LOAD_SCRGB_ASSUME))
      exri_image_free(pixels);

   pixels = NULL;
   x = y = c = 0;
   if (exri_loadf_channels_from_memory(&pixels, data, len, &x, &y, &c))
      exri_image_free(pixels);

   x = y = c = 0;
   if (exri_info_from_memory(data, len, &x, &y, &c) && x > 0 && y > 0) {
      region_w = x < 2 ? x : 2;
      region_h = y < 2 ? y : 2;
      pixels = NULL;
      if (exri_loadf_region_from_memory(&pixels, data, len, 0, 0, region_w, region_h, &x, &y, &c, 4, EXRI_LOAD_DEFAULT))
         exri_image_free(pixels);

      pixels = NULL;
      if (exri_loadf_channels_region_from_memory(&pixels, data, len, 0, 0, region_w, region_h, &x, &y, &c))
         exri_image_free(pixels);
   }

   if (exri_layer_name_from_memory(data, len, 0, layer, (int) sizeof(layer)) > 0) {
      pixels = NULL;
      x = y = c = 0;
      if (exri_loadf_layer_from_memory(&pixels, data, len, layer, &x, &y, &c, 4, EXRI_LOAD_SCRGB_ASSUME))
         exri_image_free(pixels);
   }

   parts = 1;
   if (exri_part_count_from_memory(data, len, &parts) == 0 || parts < 1)
      parts = 1;
   part_limit = parts < 3 ? parts : 3;
   for (part_index = 0; part_index < part_limit; ++part_index) {
      pixels = NULL;
      x = y = c = 0;
      if (exri_loadf_part_from_memory(&pixels, data, len, part_index, &x, &y, &c, 4, EXRI_LOAD_DEFAULT))
         exri_image_free(pixels);

      pixels = NULL;
      x = y = c = 0;
      if (exri_loadf_part_channels_from_memory(&pixels, data, len, part_index, &x, &y, &c))
         exri_image_free(pixels);

      levels_x = levels_y = 0;
      if (exri_part_tiled_level_count_from_memory(data, len, part_index, &levels_x, &levels_y) && levels_x > 0 && levels_y > 0) {
         pixels = NULL;
         x = y = c = 0;
         if (exri_loadf_part_tiled_level_from_memory(&pixels, data, len, part_index, 0, 0, &x, &y, &c, 4, EXRI_LOAD_DEFAULT))
            exri_image_free(pixels);
      }

      samples = NULL;
      offsets = NULL;
      x = y = c = total_samples = 0;
      if (exri_load_deep_part_from_memory(&samples, &offsets, data, len, part_index, &x, &y, &c, &total_samples)) {
         exri_image_free(samples);
         exri_image_free(offsets);
      }
   }

   pixels = NULL;
   x = y = c = 0;
   if (exri_loadf_tiled_level_from_memory(&pixels, data, len, 0, 0, &x, &y, &c, 4, EXRI_LOAD_DEFAULT))
      exri_image_free(pixels);
}

int LLVMFuzzerTestOneInput(unsigned char const *data, size_t size)
{
   size_t len;

   if (size > (size_t) (32 * 1024 * 1024))
      return 0;

   len = size;
   exri_fuzz_metadata(data, len);
   exri_fuzz_call_callbacks(data, len);
   exri_fuzz_loaders(data, len);
   return 0;
}

#ifdef EXRI_FUZZ_STANDALONE
static unsigned char *exri_fuzz_read_file(char const *path, size_t *size)
{
   FILE *f;
   long file_size;
   unsigned char *data;
   size_t read_size;

   f = fopen(path, "rb");
   if (f == NULL)
      return NULL;
   if (fseek(f, 0, SEEK_END) != 0) {
      fclose(f);
      return NULL;
   }
   file_size = ftell(f);
   if (file_size < 0) {
      fclose(f);
      return NULL;
   }
   if (fseek(f, 0, SEEK_SET) != 0) {
      fclose(f);
      return NULL;
   }

   read_size = (size_t) file_size;
   data = (unsigned char *) malloc(read_size == 0 ? (size_t) 1 : read_size);
   if (data == NULL) {
      fclose(f);
      return NULL;
   }
   if (read_size > 0 && fread(data, 1, read_size, f) != read_size) {
      free(data);
      fclose(f);
      return NULL;
   }
   fclose(f);
   *size = read_size;
   return data;
}

int main(int argc, char **argv)
{
   int i;
   unsigned char *data;
   size_t size;

   for (i = 1; i < argc; ++i) {
      size = 0;
      data = exri_fuzz_read_file(argv[i], &size);
      if (data == NULL) {
         fprintf(stderr, "read failed: %s\n", argv[i]);
         return 1;
      }
      (void) LLVMFuzzerTestOneInput(data, size);
      free(data);
   }

   return 0;
}
#endif
