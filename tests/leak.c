#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int exri_live_allocs;

static void *leak_malloc(size_t n)
{
   void *p;

   if (n == 0)
      n = 1;
   p = malloc(n);
   if (p)
      exri_live_allocs += 1;
   return p;
}

static void *leak_realloc(void *p, size_t n)
{
   void *q;

   if (n == 0)
      n = 1;
   q = realloc(p, n);
   if (p == NULL && q != NULL)
      exri_live_allocs += 1;
   return q;
}

static void leak_free(void *p)
{
   if (p)
      exri_live_allocs -= 1;
   free(p);
}

#define EXRI_MALLOC(sz) leak_malloc(sz)
#define EXRI_REALLOC(p,n) leak_realloc((p),(n))
#define EXRI_FREE(p) leak_free(p)
#define EXR_IMAGE_IMPLEMENTATION
#include "../exr_image.h"
#include "exri_test_compat.h"

typedef struct
{
   unsigned char const *data;
   size_t len;
   size_t pos;
} memory_reader;

static int memory_read(void *user, void *data, size_t size, size_t *bytes_read)
{
   memory_reader *reader;
   size_t n;

   reader = (memory_reader *) user;
   n = reader->len - reader->pos;
   if (n > size)
      n = size;
   if (n > 0) {
      memcpy(data, reader->data + reader->pos, n);
      reader->pos += n;
   }
   *bytes_read = n;
   return 1;
}

static int short_write(void *user, void const *data, size_t size)
{
   (void) user;
   (void) data;
   (void) size;
   return 0;
}

static unsigned char *read_file(char const *path, int *out_len)
{
   FILE *f;
   unsigned char *data;
   long size;

   f = fopen(path, "rb");
   if (f == NULL)
      return NULL;
   if (fseek(f, 0, SEEK_END) != 0) {
      fclose(f);
      return NULL;
   }
   size = ftell(f);
   if (size < 0 || size > 100000000L) {
      fclose(f);
      return NULL;
   }
   if (fseek(f, 0, SEEK_SET) != 0) {
      fclose(f);
      return NULL;
   }

   data = (unsigned char *) malloc((size_t) size);
   if (data == NULL && size != 0) {
      fclose(f);
      return NULL;
   }
   if (size > 0 && fread(data, 1, (size_t) size, f) != (size_t) size) {
      free(data);
      fclose(f);
      return NULL;
   }
   fclose(f);
   *out_len = (int) size;
   return data;
}

static int check_clean(char const *path, char const *label)
{
   if (exri_live_allocs != 0) {
      fprintf(stderr, "leak after %s on %s: live=%d reason=%s\n", label, path, exri_live_allocs, exri_failure_reason());
      return 0;
   }
   return 1;
}

static int exercise_memory(char const *path, unsigned char const *data, int len)
{
   char units[64];
   float wavelengths[8];
   int x;
   int y;
   int comp;
   int count;
   float *pixels;

   (void) exri_is_exr_from_memory(data, (size_t) len);
   if (!check_clean(path, "memory is_exr"))
      return 0;

   (void) exri_info_from_memory(data, (size_t) len, &x, &y, &comp);
   if (!check_clean(path, "memory info"))
      return 0;
   (void) exri_channel_count_from_memory(data, (size_t) len, &comp);
   if (!check_clean(path, "memory channel_count"))
      return 0;
   (void) exri_attribute_count_from_memory(data, (size_t) len, &comp);
   if (!check_clean(path, "memory attribute_count"))
      return 0;
   (void) exri_is_spectral_from_memory(data, (size_t) len);
   if (!check_clean(path, "memory is_spectral"))
      return 0;
   (void) exri_spectrum_type_from_memory(data, (size_t) len, &comp);
   if (!check_clean(path, "memory spectrum_type"))
      return 0;
   (void) exri_spectral_wavelengths_from_memory(data, (size_t) len, wavelengths, 8, &count);
   if (!check_clean(path, "memory spectral_wavelengths"))
      return 0;
   (void) exri_spectral_units_from_memory(data, (size_t) len, units, (int) sizeof(units));
   if (!check_clean(path, "memory spectral_units"))
      return 0;

   pixels = exri_loadf_from_memory(data, len, &x, &y, &comp, 4);
   if (pixels)
      exri_image_free(pixels);
   if (!check_clean(path, "memory loadf"))
      return 0;

   pixels = (float *) 1;
   (void) exri_loadf_from_memory_to(&pixels, data, len, &x, &y, &comp, 4);
   if (pixels)
      exri_image_free(pixels);
   if (!check_clean(path, "memory loadf_to"))
      return 0;

   pixels = exri_loadf_region_from_memory(data, len, 0, 0, 1, 1, &x, &y, &comp, 4);
   if (pixels)
      exri_image_free(pixels);
   if (!check_clean(path, "memory loadf_region"))
      return 0;

   pixels = (float *) 1;
   (void) exri_loadf_region_from_memory_to(&pixels, data, len, 0, 0, 1, 1, &x, &y, &comp, 4);
   if (pixels)
      exri_image_free(pixels);
   if (!check_clean(path, "memory loadf_region_to"))
      return 0;

   pixels = exri_loadf_channels_from_memory(data, len, &x, &y, &comp);
   if (pixels)
      exri_image_free(pixels);
   if (!check_clean(path, "memory loadf_channels"))
      return 0;

   pixels = (float *) 1;
   (void) exri_loadf_channels_from_memory_to(&pixels, data, len, &x, &y, &comp);
   if (pixels)
      exri_image_free(pixels);
   if (!check_clean(path, "memory loadf_channels_to"))
      return 0;

   pixels = exri_loadf_channels_region_from_memory(data, len, 0, 0, 1, 1, &x, &y, &comp);
   if (pixels)
      exri_image_free(pixels);
   if (!check_clean(path, "memory loadf_channels_region"))
      return 0;

   pixels = (float *) 1;
   (void) exri_loadf_channels_region_from_memory_to(&pixels, data, len, 0, 0, 1, 1, &x, &y, &comp);
   if (pixels)
      exri_image_free(pixels);
   return check_clean(path, "memory loadf_channels_region_to");
}

static int exercise_callbacks(char const *path, unsigned char const *data, int len)
{
   exri_io_callbacks cb;
   memory_reader reader;
   char units[64];
   float wavelengths[8];
   int x;
   int y;
   int comp;
   int count;
   float *pixels;

   cb.read = memory_read;
   cb.skip = NULL;
   cb.eof = NULL;

   reader.data = data;
   reader.len = (size_t) len;
   reader.pos = 0;
   (void) exri_is_exr_from_callbacks(&cb, &reader);
   if (!check_clean(path, "callbacks is_exr"))
      return 0;

   reader.pos = 0;
   (void) exri_info_from_callbacks(&cb, &reader, &x, &y, &comp);
   if (!check_clean(path, "callbacks info"))
      return 0;

   reader.pos = 0;
   (void) exri_is_spectral_from_callbacks(&cb, &reader);
   if (!check_clean(path, "callbacks is_spectral"))
      return 0;

   reader.pos = 0;
   (void) exri_spectrum_type_from_callbacks(&cb, &reader, &comp);
   if (!check_clean(path, "callbacks spectrum_type"))
      return 0;

   reader.pos = 0;
   (void) exri_spectral_wavelengths_from_callbacks(&cb, &reader, wavelengths, 8, &count);
   if (!check_clean(path, "callbacks spectral_wavelengths"))
      return 0;

   reader.pos = 0;
   (void) exri_spectral_units_from_callbacks(&cb, &reader, units, (int) sizeof(units));
   if (!check_clean(path, "callbacks spectral_units"))
      return 0;

   reader.pos = 0;
   pixels = exri_loadf_from_callbacks(&cb, &reader, &x, &y, &comp, 4);
   if (pixels)
      exri_image_free(pixels);
   return check_clean(path, "callbacks loadf");
}

static int exercise_files(char const *path)
{
   char units[64];
   float wavelengths[8];
   int x;
   int y;
   int comp;
   int count;
   float *pixels;

   (void) exri_is_exr(path);
   if (!check_clean(path, "file is_exr"))
      return 0;

   (void) exri_info(path, &x, &y, &comp);
   if (!check_clean(path, "file info"))
      return 0;
   (void) exri_is_spectral(path);
   if (!check_clean(path, "file is_spectral"))
      return 0;
   (void) exri_spectrum_type(path, &comp);
   if (!check_clean(path, "file spectrum_type"))
      return 0;
   (void) exri_spectral_wavelengths(path, wavelengths, 8, &count);
   if (!check_clean(path, "file spectral_wavelengths"))
      return 0;
   (void) exri_spectral_units(path, units, (int) sizeof(units));
   if (!check_clean(path, "file spectral_units"))
      return 0;

   pixels = exri_loadf(path, &x, &y, &comp, 4);
   if (pixels)
      exri_image_free(pixels);
   if (!check_clean(path, "file loadf"))
      return 0;

   pixels = (float *) 1;
   (void) exri_loadf_to(&pixels, path, &x, &y, &comp, 4);
   if (pixels)
      exri_image_free(pixels);
   if (!check_clean(path, "file loadf_to"))
      return 0;

   pixels = exri_loadf_region(path, 0, 0, 1, 1, &x, &y, &comp, 4);
   if (pixels)
      exri_image_free(pixels);
   if (!check_clean(path, "file loadf_region"))
      return 0;

   pixels = (float *) 1;
   (void) exri_loadf_region_to(&pixels, path, 0, 0, 1, 1, &x, &y, &comp, 4);
   if (pixels)
      exri_image_free(pixels);
   return check_clean(path, "file loadf_region_to");
}

static int exercise_bad_inputs(void)
{
   unsigned char bad[16];
   int x;
   int y;
   int comp;
   float *pixels;

   memset(bad, 0, sizeof(bad));
   (void) exri_is_exr_from_memory(bad, (int) sizeof(bad));
   if (!check_clean("<bad>", "bad is_exr"))
      return 0;
   (void) exri_info_from_memory(bad, (int) sizeof(bad), &x, &y, &comp);
   if (!check_clean("<bad>", "bad info"))
      return 0;
   pixels = exri_loadf_from_memory(bad, (int) sizeof(bad), &x, &y, &comp, 4);
   if (pixels)
      exri_image_free(pixels);
   if (!check_clean("<bad>", "bad loadf"))
      return 0;
   pixels = (float *) 1;
   (void) exri_loadf_from_memory_to(&pixels, bad, (int) sizeof(bad), &x, &y, &comp, 4);
   if (pixels)
      exri_image_free(pixels);
   if (!check_clean("<bad>", "bad loadf_to"))
      return 0;
   pixels = (float *) 1;
   (void) exri_loadf_region_from_memory_to(&pixels, bad, (int) sizeof(bad), 0, 0, 1, 1, &x, &y, &comp, 4);
   if (pixels)
      exri_image_free(pixels);
   if (!check_clean("<bad>", "bad loadf_region_to"))
      return 0;
   (void) exri_is_exr(NULL);
   if (!check_clean("<null>", "null filename is_exr"))
      return 0;
   pixels = (float *) 1;
   (void) exri_loadf_to(&pixels, NULL, &x, &y, &comp, 4);
   if (pixels)
      exri_image_free(pixels);
   if (!check_clean("<null>", "null filename loadf_to"))
      return 0;
   pixels = (float *) 1;
   (void) exri_loadf_region_to(&pixels, NULL, 0, 0, 1, 1, &x, &y, &comp, 4);
   if (pixels)
      exri_image_free(pixels);
   return check_clean("<null>", "null filename loadf_region_to");
}

static int exercise_write(void)
{
   exri_write_callbacks cb;
   exri_write_attribute attrs[1];
   exri_write_channel channels[2];
   char const *layout;
   unsigned char *bytes;
   float pixels[4 * 2 * 3];
   float channel_pixels[4 * 2 * 2];
   int len;
   int i;

   for (i = 0; i < (int) (sizeof(pixels) / sizeof(pixels[0])); ++i)
      pixels[i] = (float) i * 0.125f;
   for (i = 0; i < (int) (sizeof(channel_pixels) / sizeof(channel_pixels[0])); ++i)
      channel_pixels[i] = (float) i * 0.25f;
   channels[0].name = "S0.550,000000nm";
   channels[1].name = "S0.610,000000nm";
   channels[0].pixel_type = EXRI_PIXEL_FLOAT;
   channels[1].pixel_type = EXRI_PIXEL_FLOAT;
   layout = "1.0";
   attrs[0].name = "spectralLayoutVersion";
   attrs[0].type = "string";
   attrs[0].value = (unsigned char const *) layout;
   attrs[0].value_size = (int) strlen(layout) + 1;

   bytes = NULL;
   len = 0;
   if (!exri_writef_to_memory(&bytes, &len, 4, 2, 3, pixels, EXRI_WRITE_COMPRESSION_RLE))
      return 0;
   exri_image_free(bytes);
   if (!check_clean("<write>", "write rle memory"))
      return 0;

   bytes = NULL;
   len = 0;
   if (!exri_writef_to_memory(&bytes, &len, 4, 2, 3, pixels, EXRI_WRITE_COMPRESSION_ZIP | EXRI_WRITE_STORAGE_UINT))
      return 0;
   exri_image_free(bytes);
   if (!check_clean("<write>", "write uint memory"))
      return 0;

   bytes = (unsigned char *) 1;
   len = 123;
   (void) exri_writef_to_memory(&bytes, &len, 0, 2, 3, pixels, EXRI_WRITE_COMPRESSION_NONE);
   if (bytes)
      exri_image_free(bytes);
   if (!check_clean("<write>", "bad write memory"))
      return 0;

   cb.write = short_write;
   (void) exri_writef_to_callbacks(&cb, NULL, 4, 2, 3, pixels, EXRI_WRITE_COMPRESSION_NONE);
   if (!check_clean("<write>", "short callback write"))
      return 0;

   bytes = NULL;
   len = 0;
   if (!exri_writef_channels_to_memory(&bytes, &len, 4, 2, 2, channel_pixels, channels, attrs, 1, EXRI_WRITE_COMPRESSION_ZIP))
      return 0;
   exri_image_free(bytes);
   if (!check_clean("<write>", "named channel write memory"))
      return 0;

   bytes = NULL;
   len = 0;
   channels[0].pixel_type = EXRI_PIXEL_UINT;
   channels[1].pixel_type = EXRI_PIXEL_UINT;
   if (!exri_writef_channels_to_memory(&bytes, &len, 4, 2, 2, channel_pixels, channels, attrs, 1, EXRI_WRITE_COMPRESSION_ZIP | EXRI_WRITE_STORAGE_UINT))
      return 0;
   exri_image_free(bytes);
   if (!check_clean("<write>", "named channel uint write memory"))
      return 0;

   cb.write = short_write;
   channels[0].pixel_type = EXRI_PIXEL_FLOAT;
   channels[1].pixel_type = EXRI_PIXEL_FLOAT;
   (void) exri_writef_channels_to_callbacks(&cb, NULL, 4, 2, 2, channel_pixels, channels, attrs, 1, EXRI_WRITE_COMPRESSION_NONE);
   return check_clean("<write>", "named channel short callback write");
}

int main(int argc, char **argv)
{
   int i;
   int len;
   unsigned char *data;

   if (!exercise_bad_inputs())
      return 1;
   if (!exercise_write())
      return 1;

   for (i = 1; i < argc; ++i) {
      data = read_file(argv[i], &len);
      if (data == NULL) {
         fprintf(stderr, "could not read %s\n", argv[i]);
         return 1;
      }

      if (!exercise_memory(argv[i], data, len) ||
          !exercise_callbacks(argv[i], data, len) ||
          !exercise_files(argv[i])) {
         free(data);
         return 1;
      }

      free(data);
   }

   if (exri_live_allocs != 0) {
      fprintf(stderr, "final leak count: %d\n", exri_live_allocs);
      return 1;
   }

   printf("leak regression ok: %d files\n", argc - 1);
   return 0;
}
