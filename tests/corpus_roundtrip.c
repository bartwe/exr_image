#define EXR_IMAGE_IMPLEMENTATION
#include "../exr_image.h"

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
   int w;
   int h;
   int comp;
   float *pixels;
} exri_corpus_image;

typedef struct
{
   char const *name;
   int compression;
   double epsilon;
   int lossy;
   int half_limited;
   int compare_pixels;
} exri_corpus_mode;

typedef struct
{
   int cases_seen;
   int loaded;
   int skipped;
   int write_checks;
   size_t readable_only_samples;
   int failed;
} exri_corpus_stats;

static void exri_corpus_set_reason(char *reason, int reason_size, char const *text)
{
   size_t n;
   size_t max_n;

   if (reason_size <= 0)
      return;
   if (text == NULL)
      text = "unknown error";

   max_n = (size_t) reason_size - 1u;
   n = strlen(text);
   if (n > max_n)
      n = max_n;
   memcpy(reason, text, n);
   reason[n] = 0;
}

static void exri_corpus_free_image(exri_corpus_image *image)
{
   if (image->pixels != NULL)
      exri_image_free(image->pixels);
   image->pixels = NULL;
   image->w = 0;
   image->h = 0;
   image->comp = 0;
}

static int exri_corpus_take_pixels(exri_corpus_image *image, float *pixels, int w, int h, int comp)
{
   if (pixels == NULL || w <= 0 || h <= 0 || comp <= 0) {
      exri_image_free(pixels);
      return 0;
   }
   image->w = w;
   image->h = h;
   image->comp = comp;
   image->pixels = pixels;
   return 1;
}

static int exri_corpus_load_direct(exri_corpus_image *image, char const *path, char *reason, int reason_size)
{
   float *pixels;
   int w;
   int h;
   int comp;

   pixels = NULL;
   w = h = comp = 0;
   if (exri_loadf(&pixels, path, &w, &h, &comp, 4, EXRI_LOAD_DEFAULT)) {
      if (exri_corpus_take_pixels(image, pixels, w, h, 4))
         return 1;
      exri_corpus_set_reason(reason, reason_size, "invalid direct load metadata");
      return 0;
   }

   exri_corpus_set_reason(reason, reason_size, exri_failure_reason());
   return 0;
}

static int exri_corpus_load_layer(exri_corpus_image *image, char const *path, char *reason, int reason_size)
{
   char name[256];
   float *pixels;
   int layers;
   int layer;
   int name_len;
   int w;
   int h;
   int comp;

   layers = 0;
   if (!exri_layer_count(path, &layers) || layers <= 0)
      return 0;

   for (layer = 0; layer < layers; ++layer) {
      name_len = exri_layer_name(path, layer, name, (int) sizeof(name));
      if (name_len <= 0)
         continue;
      pixels = NULL;
      w = h = comp = 0;
      if (exri_loadf_layer(&pixels, path, name, &w, &h, &comp, 4, EXRI_LOAD_DEFAULT)) {
         if (exri_corpus_take_pixels(image, pixels, w, h, 4))
            return 1;
         exri_corpus_set_reason(reason, reason_size, "invalid layer load metadata");
         return 0;
      }
   }

   return 0;
}

static int exri_corpus_load_part(exri_corpus_image *image, char const *path, char *reason, int reason_size)
{
   float *pixels;
   int parts;
   int part;
   int w;
   int h;
   int comp;

   parts = 0;
   if (!exri_part_count(path, &parts) || parts <= 1)
      return 0;

   for (part = 0; part < parts; ++part) {
      pixels = NULL;
      w = h = comp = 0;
      if (exri_loadf_part(&pixels, path, part, &w, &h, &comp, 4, EXRI_LOAD_DEFAULT)) {
         if (exri_corpus_take_pixels(image, pixels, w, h, 4))
            return 1;
         exri_corpus_set_reason(reason, reason_size, "invalid part load metadata");
         return 0;
      }
   }

   return 0;
}

static int exri_corpus_load_any(exri_corpus_image *image, char const *path, char *reason, int reason_size)
{
   memset(image, 0, sizeof(*image));
   if (reason_size > 0)
      reason[0] = 0;

   if (exri_corpus_load_direct(image, path, reason, reason_size))
      return 1;
   if (exri_corpus_load_layer(image, path, reason, reason_size))
      return 1;
   if (exri_corpus_load_part(image, path, reason, reason_size))
      return 1;

   if (reason_size > 0 && reason[0] == 0)
      exri_corpus_set_reason(reason, reason_size, exri_failure_reason());
   return 0;
}

static int exri_corpus_is_nan(float v)
{
   return v != v;
}

static int exri_corpus_is_finite(float v)
{
   return !exri_corpus_is_nan(v) && v <= FLT_MAX && v >= -FLT_MAX;
}

static int exri_corpus_nearly_equal(float a, float b, double epsilon)
{
   double da;
   double db;
   double scale;

   if (exri_corpus_is_nan(a) || exri_corpus_is_nan(b))
      return exri_corpus_is_nan(a) && exri_corpus_is_nan(b);
   if (!exri_corpus_is_finite(a) || !exri_corpus_is_finite(b))
      return a == b;

   da = (double) a;
   db = (double) b;
   scale = fabs(da);
   if (fabs(db) > scale)
      scale = fabs(db);
   if (scale < 1.0)
      scale = 1.0;
   return fabs(da - db) <= epsilon * scale;
}

static int exri_corpus_compare(exri_corpus_image const *a, exri_corpus_image const *b, exri_corpus_mode const *mode, char const *path, size_t *readable_only_samples)
{
   size_t count;
   size_t i;
   float av;
   float bv;

   if (a->w != b->w || a->h != b->h || b->comp != 4) {
      fprintf(stderr, "roundtrip-mismatch %s %s: dimensions %dx%d/%d != %dx%d/%d\n",
              path, mode->name, a->w, a->h, a->comp, b->w, b->h, b->comp);
      return 0;
   }

   count = (size_t) a->w * (size_t) a->h * 4u;
   if (!mode->compare_pixels) {
      *readable_only_samples += count;
      return 1;
   }

   for (i = 0; i < count; ++i) {
      av = a->pixels[i];
      bv = b->pixels[i];
      if (mode->lossy) {
         if (!exri_corpus_is_finite(av)) {
            *readable_only_samples += 1u;
            continue;
         }
         if (mode->half_limited && (av > 65000.0f || av < -65000.0f)) {
            *readable_only_samples += 1u;
            continue;
         }
      }
      if (!exri_corpus_nearly_equal(av, bv, mode->epsilon)) {
         fprintf(stderr, "roundtrip-mismatch %s %s: pixel[%lu] %.9g != %.9g epsilon=%.9g\n",
                 path, mode->name, (unsigned long) i, (double) av, (double) bv, mode->epsilon);
         return 0;
      }
   }

   return 1;
}

static int exri_corpus_roundtrip_mode(exri_corpus_image const *image, char const *path, exri_corpus_mode const *mode, exri_corpus_stats *stats)
{
   exri_write_options options;
   exri_uc *bytes;
   size_t len;
   float *pixels;
   exri_corpus_image loaded;
   int x;
   int y;
   int comp;
   size_t readable_only_samples;
   int ok;

   memset(&options, 0, sizeof(options));
   options.compression = mode->compression;
   options.pixel_type = EXRI_WRITE_PIXEL_FLOAT;
   options.tiled = 0;
   options.tile_size = 0;
   options.level_mode = EXRI_WRITE_LEVEL_ONE;
   options.level_rounding = EXRI_WRITE_ROUND_DOWN;

   bytes = NULL;
   len = 0;
   if (!exri_writef_to_memory(&bytes, &len, image->w, image->h, 4, image->pixels, &options)) {
      fprintf(stderr, "roundtrip-write-failed %s %s: %s\n", path, mode->name, exri_failure_reason());
      stats->failed += 1;
      return 0;
   }

   pixels = NULL;
   x = y = comp = 0;
   if (!exri_loadf_from_memory(&pixels, bytes, len, &x, &y, &comp, 4, EXRI_LOAD_DEFAULT)) {
      fprintf(stderr, "roundtrip-read-failed %s %s: %s\n", path, mode->name, exri_failure_reason());
      exri_image_free(bytes);
      stats->failed += 1;
      return 0;
   }

   memset(&loaded, 0, sizeof(loaded));
   ok = exri_corpus_take_pixels(&loaded, pixels, x, y, 4);
   if (!ok) {
      fprintf(stderr, "roundtrip-read-failed %s %s: invalid metadata\n", path, mode->name);
      exri_image_free(bytes);
      stats->failed += 1;
      return 0;
   }

   readable_only_samples = 0;
   ok = exri_corpus_compare(image, &loaded, mode, path, &readable_only_samples);
   stats->readable_only_samples += readable_only_samples;
   if (ok) {
      stats->write_checks += 1;
   } else {
      stats->failed += 1;
   }

   exri_corpus_free_image(&loaded);
   exri_image_free(bytes);
   return ok;
}

static int exri_corpus_check_file(char const *path, exri_corpus_stats *stats)
{
   static exri_corpus_mode const modes[] = {
      { "none",  EXRI_WRITE_COMPRESSION_NONE,  0.000001, 0, 0, 1 },
      { "rle",   EXRI_WRITE_COMPRESSION_RLE,   0.000001, 0, 0, 1 },
      { "zips",  EXRI_WRITE_COMPRESSION_ZIPS,  0.000001, 0, 0, 1 },
      { "zip",   EXRI_WRITE_COMPRESSION_ZIP,   0.000001, 0, 0, 1 },
      { "piz",   EXRI_WRITE_COMPRESSION_PIZ,   0.000001, 0, 0, 1 },
      { "pxr24", EXRI_WRITE_COMPRESSION_PXR24, 0.001000, 1, 0, 1 },
      { "b44",   EXRI_WRITE_COMPRESSION_B44,   0.000000, 1, 1, 0 },
      { "b44a",  EXRI_WRITE_COMPRESSION_B44A,  0.000000, 1, 1, 0 }
   };
   exri_corpus_image image;
   char reason[256];
   size_t i;
   int ok;

   stats->cases_seen += 1;
   memset(&image, 0, sizeof(image));
   reason[0] = 0;

   if (!exri_corpus_load_any(&image, path, reason, (int) sizeof(reason))) {
      printf("roundtrip-skip %s: %s\n", path, reason);
      stats->skipped += 1;
      return 1;
   }

   stats->loaded += 1;
   ok = 1;
   for (i = 0; i < sizeof(modes) / sizeof(modes[0]); ++i) {
      if (!exri_corpus_roundtrip_mode(&image, path, modes + i, stats))
         ok = 0;
   }

   if (ok)
      printf("roundtrip-ok %s: %dx%d modes=%lu\n", path, image.w, image.h, (unsigned long) (sizeof(modes) / sizeof(modes[0])));

   exri_corpus_free_image(&image);
   return ok;
}

static int exri_corpus_run_list(char const *path, exri_corpus_stats *stats)
{
   FILE *f;
   char line[4096];
   size_t len;
   int ok;

   f = fopen(path, "rb");
   if (f == NULL) {
      fprintf(stderr, "could not open list file: %s\n", path);
      return 0;
   }

   ok = 1;
   while (fgets(line, (int) sizeof(line), f) != NULL) {
      len = strlen(line);
      while (len > 0u && (line[len - 1u] == '\n' || line[len - 1u] == '\r')) {
         line[len - 1u] = 0;
         len -= 1u;
      }
      if (line[0] == 0)
         continue;
      if (!exri_corpus_check_file(line, stats))
         ok = 0;
   }

   fclose(f);
   return ok;
}

static void exri_corpus_usage(char const *program)
{
   fprintf(stderr, "usage: %s [--list file] file.exr ...\n", program);
}

int main(int argc, char **argv)
{
   exri_corpus_stats stats;
   int i;
   int ok;

   memset(&stats, 0, sizeof(stats));
   ok = 1;

   if (argc <= 1) {
      exri_corpus_usage(argv[0]);
      return 2;
   }

   i = 1;
   while (i < argc) {
      if (strcmp(argv[i], "--list") == 0) {
         i += 1;
         if (i >= argc) {
            exri_corpus_usage(argv[0]);
            return 2;
         }
         if (!exri_corpus_run_list(argv[i], &stats))
            ok = 0;
      } else if (strncmp(argv[i], "--", 2) == 0) {
         exri_corpus_usage(argv[0]);
         return 2;
      } else {
         if (!exri_corpus_check_file(argv[i], &stats))
            ok = 0;
      }
      i += 1;
   }

   printf("roundtrip-summary cases=%d loaded=%d skipped=%d write_checks=%d readable_only_samples=%lu failed=%d\n",
          stats.cases_seen, stats.loaded, stats.skipped, stats.write_checks, (unsigned long) stats.readable_only_samples, stats.failed);
   return ok && stats.failed == 0 ? 0 : 1;
}
