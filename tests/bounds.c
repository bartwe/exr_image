#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GUARD_SIZE 32
#define GUARD_HEAD 0xa5
#define GUARD_TAIL 0x5a
#define GUARD_MAGIC 0x45585242u

typedef union
{
   double d;
   void *p;
   size_t s;
   long l;
} guard_align;

typedef struct
{
   guard_align align;
   size_t size;
   unsigned int magic;
} guard_header;

static int guard_live_allocs;
static int guard_failures;

static unsigned char *guard_base_from_user(void *user)
{
   return (unsigned char *) user - GUARD_SIZE - sizeof(guard_header);
}

static void guard_store_header(unsigned char *base, size_t size, unsigned int magic)
{
   guard_header h;

   memset(&h, 0, sizeof(h));
   h.size = size;
   h.magic = magic;
   memcpy(base, &h, sizeof(h));
}

static void guard_load_header(void *user, guard_header *h)
{
   memcpy(h, guard_base_from_user(user), sizeof(*h));
}

static void guard_fill(unsigned char *p, size_t n, unsigned char value)
{
   size_t i;

   for (i = 0; i < n; ++i)
      p[i] = value;
}

static void guard_check_one(void *user)
{
   unsigned char *p;
   guard_header h;
   size_t i;

   if (user == NULL)
      return;

   p = (unsigned char *) user;
   guard_load_header(user, &h);
   if (h.magic != GUARD_MAGIC) {
      guard_failures += 1;
      return;
   }

   for (i = 0; i < GUARD_SIZE; ++i) {
      if (p[-(int) GUARD_SIZE + (int) i] != GUARD_HEAD)
         guard_failures += 1;
      if (p[h.size + i] != GUARD_TAIL)
         guard_failures += 1;
   }
}

static void *guard_malloc(size_t n)
{
   unsigned char *base;
   unsigned char *user;
   size_t total;

   if (n == 0)
      n = 1;
   if (n > ((size_t) -1) - sizeof(guard_header) - GUARD_SIZE * 2)
      return NULL;

   total = sizeof(guard_header) + GUARD_SIZE + n + GUARD_SIZE;
   base = (unsigned char *) malloc(total);
   if (base == NULL)
      return NULL;

   guard_store_header(base, n, GUARD_MAGIC);
   user = base + sizeof(guard_header) + GUARD_SIZE;
   guard_fill(user - GUARD_SIZE, GUARD_SIZE, GUARD_HEAD);
   guard_fill(user + n, GUARD_SIZE, GUARD_TAIL);
   guard_live_allocs += 1;
   return user;
}

static void guard_free(void *p)
{
   unsigned char *user;
   unsigned char *base;
   guard_header h;

   if (p == NULL)
      return;

   guard_check_one(p);
   user = (unsigned char *) p;
   base = guard_base_from_user(user);
   guard_load_header(p, &h);
   guard_store_header(base, h.size, 0);
   guard_live_allocs -= 1;
   free((void *) base);
}

static void *guard_realloc(void *p, size_t n)
{
   void *q;
   size_t old_size;
   guard_header h;

   if (p == NULL)
      return guard_malloc(n);
   if (n == 0)
      n = 1;

   guard_check_one(p);
   guard_load_header(p, &h);
   old_size = h.size;
   q = guard_malloc(n);
   if (q == NULL)
      return NULL;
   memcpy(q, p, old_size < n ? old_size : n);
   guard_free(p);
   return q;
}

#define EXRI_MALLOC(sz) guard_malloc(sz)
#define EXRI_REALLOC(p,n) guard_realloc((p),(n))
#define EXRI_FREE(p) guard_free(p)
#define EXR_IMAGE_IMPLEMENTATION
#include "../exr_image.h"
#include "exri_test_compat.h"

typedef struct
{
   unsigned char const *data;
   size_t len;
   size_t pos;
} bounds_reader;

static int bounds_read(void *user, void *data, size_t size, size_t *bytes_read)
{
   bounds_reader *reader;
   size_t n;

   reader = (bounds_reader *) user;
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

static unsigned char *read_file(char const *path, int *out_len)
{
   FILE *f;
   unsigned char *data;
   long size;

   f = fopen(path, "rb");
   if (f == NULL)
      return NULL;
   fseek(f, 0, SEEK_END);
   size = ftell(f);
   fseek(f, 0, SEEK_SET);
   if (size <= 0 || size > 100000000L) {
      fclose(f);
      return NULL;
   }
   data = (unsigned char *) malloc((size_t) size);
   if (data == NULL) {
      fclose(f);
      return NULL;
   }
   if (fread(data, 1, (size_t) size, f) != (size_t) size) {
      free(data);
      fclose(f);
      return NULL;
   }
   fclose(f);
   *out_len = (int) size;
   return data;
}

static int clean(char const *label, char const *path)
{
   if (guard_failures || guard_live_allocs != 0) {
      fprintf(stderr, "bounds failure after %s on %s: failures=%d live=%d reason=%s\n",
              label, path, guard_failures, guard_live_allocs, exri_failure_reason());
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
   if (!clean("memory is_exr", path))
      return 0;
   (void) exri_info_from_memory(data, (size_t) len, &x, &y, &comp);
   if (!clean("memory info", path))
      return 0;
   (void) exri_channel_count_from_memory(data, (size_t) len, &comp);
   if (!clean("memory channel_count", path))
      return 0;
   (void) exri_attribute_count_from_memory(data, (size_t) len, &comp);
   if (!clean("memory attribute_count", path))
      return 0;
   (void) exri_is_spectral_from_memory(data, (size_t) len);
   if (!clean("memory is_spectral", path))
      return 0;
   (void) exri_spectrum_type_from_memory(data, (size_t) len, &comp);
   if (!clean("memory spectrum_type", path))
      return 0;
   (void) exri_spectral_wavelengths_from_memory(data, (size_t) len, wavelengths, 8, &count);
   if (!clean("memory spectral_wavelengths", path))
      return 0;
   (void) exri_spectral_units_from_memory(data, (size_t) len, units, (int) sizeof(units));
   if (!clean("memory spectral_units", path))
      return 0;

   pixels = exri_loadf_from_memory(data, len, &x, &y, &comp, 4);
   if (pixels)
      exri_image_free(pixels);
   if (!clean("memory loadf", path))
      return 0;

   pixels = (float *) 1;
   (void) exri_loadf_from_memory_to(&pixels, data, len, &x, &y, &comp, 4);
   if (pixels)
      exri_image_free(pixels);
   if (!clean("memory loadf_to", path))
      return 0;

   pixels = exri_loadf_region_from_memory(data, len, 0, 0, 1, 1, &x, &y, &comp, 4);
   if (pixels)
      exri_image_free(pixels);
   if (!clean("memory loadf_region", path))
      return 0;

   pixels = (float *) 1;
   (void) exri_loadf_region_from_memory_to(&pixels, data, len, 0, 0, 1, 1, &x, &y, &comp, 4);
   if (pixels)
      exri_image_free(pixels);
   if (!clean("memory loadf_region_to", path))
      return 0;

   pixels = exri_loadf_channels_from_memory(data, len, &x, &y, &comp);
   if (pixels)
      exri_image_free(pixels);
   if (!clean("memory loadf_channels", path))
      return 0;

   pixels = (float *) 1;
   (void) exri_loadf_channels_from_memory_to(&pixels, data, len, &x, &y, &comp);
   if (pixels)
      exri_image_free(pixels);
   if (!clean("memory loadf_channels_to", path))
      return 0;

   pixels = exri_loadf_channels_region_from_memory(data, len, 0, 0, 1, 1, &x, &y, &comp);
   if (pixels)
      exri_image_free(pixels);
   if (!clean("memory loadf_channels_region", path))
      return 0;

   pixels = (float *) 1;
   (void) exri_loadf_channels_region_from_memory_to(&pixels, data, len, 0, 0, 1, 1, &x, &y, &comp);
   if (pixels)
      exri_image_free(pixels);
   return clean("memory loadf_channels_region_to", path);
}

static int exercise_callbacks(char const *path, unsigned char const *data, int len)
{
   exri_io_callbacks cb;
   bounds_reader reader;
   char units[64];
   float wavelengths[8];
   int x;
   int y;
   int comp;
   int count;
   float *pixels;

   cb.read = bounds_read;
   cb.skip = NULL;
   cb.eof = NULL;
   reader.data = data;
   reader.len = (size_t) len;

   reader.pos = 0;
   (void) exri_is_exr_from_callbacks(&cb, &reader);
   if (!clean("callback is_exr", path))
      return 0;

   reader.pos = 0;
   (void) exri_info_from_callbacks(&cb, &reader, &x, &y, &comp);
   if (!clean("callback info", path))
      return 0;

   reader.pos = 0;
   (void) exri_is_spectral_from_callbacks(&cb, &reader);
   if (!clean("callback is_spectral", path))
      return 0;

   reader.pos = 0;
   (void) exri_spectrum_type_from_callbacks(&cb, &reader, &comp);
   if (!clean("callback spectrum_type", path))
      return 0;

   reader.pos = 0;
   (void) exri_spectral_wavelengths_from_callbacks(&cb, &reader, wavelengths, 8, &count);
   if (!clean("callback spectral_wavelengths", path))
      return 0;

   reader.pos = 0;
   (void) exri_spectral_units_from_callbacks(&cb, &reader, units, (int) sizeof(units));
   if (!clean("callback spectral_units", path))
      return 0;

   reader.pos = 0;
   pixels = exri_loadf_from_callbacks(&cb, &reader, &x, &y, &comp, 4);
   if (pixels)
      exri_image_free(pixels);
   return clean("callback loadf", path);
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
   if (!clean("file is_exr", path))
      return 0;
   (void) exri_info(path, &x, &y, &comp);
   if (!clean("file info", path))
      return 0;
   (void) exri_channel_count(path, &comp);
   if (!clean("file channel_count", path))
      return 0;
   (void) exri_attribute_count(path, &comp);
   if (!clean("file attribute_count", path))
      return 0;
   (void) exri_is_spectral(path);
   if (!clean("file is_spectral", path))
      return 0;
   (void) exri_spectrum_type(path, &comp);
   if (!clean("file spectrum_type", path))
      return 0;
   (void) exri_spectral_wavelengths(path, wavelengths, 8, &count);
   if (!clean("file spectral_wavelengths", path))
      return 0;
   (void) exri_spectral_units(path, units, (int) sizeof(units));
   if (!clean("file spectral_units", path))
      return 0;

   pixels = exri_loadf(path, &x, &y, &comp, 4);
   if (pixels)
      exri_image_free(pixels);
   if (!clean("file loadf", path))
      return 0;

   pixels = (float *) 1;
   (void) exri_loadf_to(&pixels, path, &x, &y, &comp, 4);
   if (pixels)
      exri_image_free(pixels);
   if (!clean("file loadf_to", path))
      return 0;

   pixels = exri_loadf_region(path, 0, 0, 1, 1, &x, &y, &comp, 4);
   if (pixels)
      exri_image_free(pixels);
   if (!clean("file loadf_region", path))
      return 0;

   pixels = (float *) 1;
   (void) exri_loadf_region_to(&pixels, path, 0, 0, 1, 1, &x, &y, &comp, 4);
   if (pixels)
      exri_image_free(pixels);
   if (!clean("file loadf_region_to", path))
      return 0;

   pixels = exri_loadf_channels(path, &x, &y, &comp);
   if (pixels)
      exri_image_free(pixels);
   if (!clean("file loadf_channels", path))
      return 0;

   pixels = (float *) 1;
   (void) exri_loadf_channels_to(&pixels, path, &x, &y, &comp);
   if (pixels)
      exri_image_free(pixels);
   if (!clean("file loadf_channels_to", path))
      return 0;

   pixels = exri_loadf_channels_region(path, 0, 0, 1, 1, &x, &y, &comp);
   if (pixels)
      exri_image_free(pixels);
   if (!clean("file loadf_channels_region", path))
      return 0;

   pixels = (float *) 1;
   (void) exri_loadf_channels_region_to(&pixels, path, 0, 0, 1, 1, &x, &y, &comp);
   if (pixels)
      exri_image_free(pixels);
   return clean("file loadf_channels_region_to", path);
}

static unsigned int rng_state = 1u;

static unsigned int rnd(void)
{
   rng_state = rng_state * 1664525u + 1013904223u;
   return rng_state;
}

static int exercise_mutations(char const *path, unsigned char const *data, int len)
{
   unsigned char *copy;
   int iterations;
   int i;
   int j;
   int pos;

   copy = (unsigned char *) malloc((size_t) len);
   if (copy == NULL)
      return 0;

   iterations = 2000;
   if (len > 2 * 1024 * 1024)
      iterations = 4;
   else if (len > 512 * 1024)
      iterations = 20;

   for (i = 0; i < iterations; ++i) {
      memcpy(copy, data, (size_t) len);
      for (j = 0; j < 4; ++j) {
         pos = (int) (rnd() % (unsigned int) len);
         copy[pos] = (unsigned char) rnd();
      }
      if (!exercise_memory(path, copy, len)) {
         free(copy);
         return 0;
      }
   }

   free(copy);
   return 1;
}

static int exercise_bad(void)
{
   unsigned char bad[32];
   int x;
   int y;
   int comp;
   float *pixels;

   memset(bad, 0, sizeof(bad));
   (void) exri_info_from_memory(bad, (int) sizeof(bad), &x, &y, &comp);
   if (!clean("bad info", "<bad>"))
      return 0;
   pixels = exri_loadf_from_memory(bad, (int) sizeof(bad), &x, &y, &comp, 4);
   if (pixels)
      exri_image_free(pixels);
   if (!clean("bad load", "<bad>"))
      return 0;
   return 1;
}

int main(int argc, char **argv)
{
   int i;
   int len;
   unsigned char *data;

   if (!exercise_bad())
      return 1;

   for (i = 1; i < argc; ++i) {
      data = read_file(argv[i], &len);
      if (data == NULL) {
         fprintf(stderr, "could not read %s\n", argv[i]);
         return 1;
      }

      if (!exercise_memory(argv[i], data, len) ||
          !exercise_callbacks(argv[i], data, len) ||
          !exercise_files(argv[i]) ||
          !exercise_mutations(argv[i], data, len)) {
         free(data);
         return 1;
      }

      free(data);
   }

   if (guard_failures || guard_live_allocs != 0) {
      fprintf(stderr, "bounds final failure: failures=%d live=%d\n", guard_failures, guard_live_allocs);
      return 1;
   }

   printf("bounds regression ok: %d files\n", argc - 1);
   return 0;
}
