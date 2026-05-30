#define EXR_IMAGE_IMPLEMENTATION
#include "../exr_image.h"

#include <stdio.h>
#include <stdlib.h>

static unsigned char *read_file(char const *path, int *len)
{
   FILE *f;
   long size;
   unsigned char *data;

   f = fopen(path, "rb");
   if (!f)
      return NULL;

   if (fseek(f, 0, SEEK_END) != 0) {
      fclose(f);
      return NULL;
   }

   size = ftell(f);
   if (size <= 0 || size > 0x7fffffffL) {
      fclose(f);
      return NULL;
   }

   if (fseek(f, 0, SEEK_SET) != 0) {
      fclose(f);
      return NULL;
   }

   data = (unsigned char *) malloc((size_t) size);
   if (!data) {
      fclose(f);
      return NULL;
   }

   if (fread(data, 1, (size_t) size, f) != (size_t) size) {
      free(data);
      fclose(f);
      return NULL;
   }

   fclose(f);
   *len = (int) size;
   return data;
}

static int write_file(char const *path, unsigned char const *data, int len)
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

int main(void)
{
   static float source[2 * 2 * 4] = {
      0.0f, 1.0f, 2.0f, 1.0f,
      3.0f, 4.0f, 5.0f, 1.0f,
      6.0f, 7.0f, 8.0f, 1.0f,
      9.0f, 10.0f, 11.0f, 1.0f
   };
   char const *path;
   exri_uc *bytes;
   unsigned char *data;
   float *pixels;
   int bytes_len;
   int len;
   int x;
   int y;
   int comp;

   path = "/tmp/exri_abi_probe.exr";
   bytes = NULL;
   bytes_len = 0;
   pixels = NULL;
   x = y = comp = 0;

   if (!exri_writef_to_memory(&bytes, &bytes_len, 2, 2, 4, source, NULL)) {
      fprintf(stderr, "memory write failed: %s\n", exri_failure_reason());
      return 1;
   }
   if (!write_file(path, bytes, bytes_len)) {
      fprintf(stderr, "file write failed\n");
      exri_image_free(bytes);
      return 1;
   }

   if (!exri_loadf(&pixels, path, &x, &y, &comp, 4, EXRI_LOAD_DEFAULT)) {
      fprintf(stderr, "file wrapper failed: %s\n", exri_failure_reason());
      exri_image_free(bytes);
      return 1;
   }

   if (!pixels || x != 2 || y != 2 || comp != 4) {
      fprintf(stderr, "unexpected file wrapper result\n");
      exri_image_free(pixels);
      exri_image_free(bytes);
      return 1;
   }
   exri_image_free(pixels);

   pixels = NULL;
   x = y = comp = 0;
   if (!exri_loadf_region(&pixels, path, 1, 1, 1, 1, &x, &y, &comp, 4, EXRI_LOAD_DEFAULT)) {
      fprintf(stderr, "file region wrapper failed: %s\n", exri_failure_reason());
      exri_image_free(bytes);
      return 1;
   }
   if (!pixels || x != 1 || y != 1 || comp != 4) {
      fprintf(stderr, "unexpected file region wrapper result\n");
      exri_image_free(pixels);
      exri_image_free(bytes);
      return 1;
   }
   exri_image_free(pixels);

   data = read_file(path, &len);
   if (!data) {
      fprintf(stderr, "read failed\n");
      exri_image_free(bytes);
      return 1;
   }

   pixels = NULL;
   x = y = comp = 0;
   if (!exri_loadf_from_memory(&pixels, data, len, &x, &y, &comp, 4, EXRI_LOAD_DEFAULT)) {
      fprintf(stderr, "memory wrapper failed: %s\n", exri_failure_reason());
      free(data);
      exri_image_free(bytes);
      return 1;
   }

   if (!pixels || x != 2 || y != 2 || comp != 4) {
      fprintf(stderr, "unexpected memory wrapper result\n");
      exri_image_free(pixels);
      free(data);
      exri_image_free(bytes);
      return 1;
   }

   exri_image_free(pixels);
   pixels = NULL;
   x = y = comp = 0;
   if (!exri_loadf_region_from_memory(&pixels, data, len, 0, 0, 1, 1, &x, &y, &comp, 4, EXRI_LOAD_DEFAULT)) {
      fprintf(stderr, "memory region wrapper failed: %s\n", exri_failure_reason());
      free(data);
      exri_image_free(bytes);
      return 1;
   }
   free(data);
   if (!pixels || x != 1 || y != 1 || comp != 4) {
      fprintf(stderr, "unexpected memory region wrapper result\n");
      exri_image_free(pixels);
      exri_image_free(bytes);
      return 1;
   }
   exri_image_free(pixels);
   exri_image_free(bytes);
   return 0;
}
