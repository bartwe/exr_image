#define EXR_IMAGE_IMPLEMENTATION
#include "../exr_image.h"
#include "exri_test_compat.h"

#include <stdio.h>

static int write_bytes(char const *path, unsigned char const *data, int len)
{
   FILE *f;
   size_t written;

   f = fopen(path, "wb");
   if (!f)
      return 0;
   written = fwrite(data, 1, (size_t) len, f);
   fclose(f);
   return written == (size_t) len;
}

static int check_exr(char const *path)
{
   int x;
   int y;
   int comp;

   x = y = comp = 0;
   if (!exri_is_exr(path)) {
      fprintf(stderr, "expected EXR: %s: %s\n", path, exri_failure_reason());
      return 0;
   }

   if (!exri_info(path, &x, &y, &comp)) {
      fprintf(stderr, "info failed: %s: %s\n", path, exri_failure_reason());
      return 0;
   }

   if (x <= 0 || y <= 0 || comp <= 0) {
      fprintf(stderr, "invalid info: %s: %d %d %d\n", path, x, y, comp);
      return 0;
   }

   printf("%s: %dx%d comp=%d\n", path, x, y, comp);
   return 1;
}

static int check_load(char const *path)
{
   int x;
   int y;
   int comp;
   float *pixels;

   x = y = comp = 0;
   pixels = exri_loadf(path, &x, &y, &comp, 4);
   if (!pixels) {
      fprintf(stderr, "load failed: %s: %s\n", path, exri_failure_reason());
      return 0;
   }

   if (x <= 0 || y <= 0 || comp <= 0) {
      fprintf(stderr, "invalid load info: %s: %d %d %d\n", path, x, y, comp);
      exri_image_free(pixels);
      return 0;
   }

   exri_image_free(pixels);
   return 1;
}

int main(int argc, char **argv)
{
   static float pixels[2 * 2 * 4] = {
      1.0f, 0.0f, 0.0f, 1.0f,
      0.0f, 1.0f, 0.0f, 1.0f,
      0.0f, 0.0f, 1.0f, 1.0f,
      2.0f, 3.0f, 4.0f, 1.0f
   };
   static unsigned char not_exr[8] = { 0x89u, 'P', 'N', 'G', 13u, 10u, 26u, 10u };
   char const *path;
   char const *bad_path;
   int i;

   if (argc > 1) {
      for (i = 1; i < argc; ++i) {
         if (!check_exr(argv[i]))
            return 1;
      }
      return 0;
   }

   path = "/tmp/exri_smoke.exr";
   bad_path = "/tmp/exri_smoke_not_exr.bin";

   if (!exri_writef(path, 2, 2, 4, pixels, EXRI_WRITE_COMPRESSION_ZIP)) {
      fprintf(stderr, "write failed: %s\n", exri_failure_reason());
      return 1;
   }

   if (!check_exr(path))
      return 1;
   if (!check_load(path))
      return 1;

   if (!write_bytes(bad_path, not_exr, (int) sizeof(not_exr))) {
      fprintf(stderr, "failed to write non-exr probe\n");
      return 1;
   }

   if (exri_is_exr(bad_path)) {
      fprintf(stderr, "non-EXR detected as EXR\n");
      return 1;
   }

   return 0;
}
