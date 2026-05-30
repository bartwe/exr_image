#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int fail_live_allocs;
static int fail_call_count;
static int fail_after_calls;

static int should_fail_alloc(void)
{
   int fail;

   fail = fail_call_count >= fail_after_calls;
   fail_call_count += 1;
   return fail;
}

static void *fail_malloc(size_t n)
{
   void *p;

   if (should_fail_alloc())
      return NULL;
   if (n == 0)
      n = 1;
   p = malloc(n);
   if (p)
      fail_live_allocs += 1;
   return p;
}

static void *fail_realloc(void *p, size_t n)
{
   void *q;

   if (p == NULL)
      return fail_malloc(n);
   if (should_fail_alloc())
      return NULL;
   if (n == 0)
      n = 1;
   q = realloc(p, n);
   return q;
}

static void fail_free(void *p)
{
   if (p)
      fail_live_allocs -= 1;
   free(p);
}

#define EXRI_MALLOC(sz) fail_malloc(sz)
#define EXRI_REALLOC(p,n) fail_realloc((p),(n))
#define EXRI_FREE(p) fail_free(p)
#define EXR_IMAGE_IMPLEMENTATION
#include "../exr_image.h"
#include "exri_test_compat.h"

static void fill_pixels(float *pixels, int count)
{
   int i;

   for (i = 0; i < count; ++i)
      pixels[i] = ((float) (i % 29) - 14.0f) * 0.0625f;
}

static int run_write_with_failure(int compression, int fail_after)
{
   float pixels[16 * 4 * 4];
   exri_uc *bytes;
   int len;
   int ok;

   fill_pixels(pixels, (int) (sizeof(pixels) / sizeof(pixels[0])));
   fail_live_allocs = 0;
   fail_call_count = 0;
   fail_after_calls = fail_after;
   bytes = (exri_uc *) 1;
   len = 123;

   ok = exri_writef_to_memory(&bytes, &len, 16, 4, 4, pixels, compression);
   if (ok) {
      if (bytes == NULL || len <= 0) {
         fprintf(stderr, "successful write returned empty output\n");
         return 0;
      }
      exri_image_free(bytes);
   } else {
      if (bytes != NULL || len != 0) {
         fprintf(stderr, "failed write did not clear outputs\n");
         if (bytes != NULL && bytes != (exri_uc *) 1)
            exri_image_free(bytes);
         return 0;
      }
   }

   if (fail_live_allocs != 0) {
      fprintf(stderr, "writer leaked allocations: compression=%d fail_after=%d live=%d reason=%s\n",
              compression, fail_after, fail_live_allocs, exri_failure_reason());
      return 0;
   }

   return 1;
}

static int run_channel_write_with_failure(int compression, int fail_after)
{
   exri_write_channel channels[3];
   exri_write_attribute attrs[2];
   char const *layout;
   char const *units;
   float pixels[8 * 4 * 3];
   exri_uc *bytes;
   int len;
   int ok;

   channels[0].name = "S0.550,000000nm";
   channels[1].name = "S0.610,000000nm";
   channels[2].name = "S1.550,000000nm";
   channels[0].pixel_type = (compression & EXRI_WRITE_STORAGE_UINT) ? EXRI_PIXEL_UINT : EXRI_PIXEL_FLOAT;
   channels[1].pixel_type = channels[0].pixel_type;
   channels[2].pixel_type = channels[0].pixel_type;
   if ((compression & EXRI_WRITE_COMPRESSION_MASK) == EXRI_WRITE_COMPRESSION_B44 ||
       (compression & EXRI_WRITE_COMPRESSION_MASK) == EXRI_WRITE_COMPRESSION_B44A ||
       (compression & EXRI_WRITE_STORAGE_HALF)) {
      channels[0].pixel_type = EXRI_PIXEL_HALF;
      channels[1].pixel_type = EXRI_PIXEL_HALF;
      channels[2].pixel_type = EXRI_PIXEL_HALF;
   }
   layout = "1.0";
   units = "W.m^-2.sr^-1.nm^-1";
   attrs[0].name = "spectralLayoutVersion";
   attrs[0].type = "string";
   attrs[0].value = (exri_uc const *) layout;
   attrs[0].value_size = (int) strlen(layout) + 1;
   attrs[1].name = "emissiveUnits";
   attrs[1].type = "string";
   attrs[1].value = (exri_uc const *) units;
   attrs[1].value_size = (int) strlen(units) + 1;

   fill_pixels(pixels, (int) (sizeof(pixels) / sizeof(pixels[0])));
   fail_live_allocs = 0;
   fail_call_count = 0;
   fail_after_calls = fail_after;
   bytes = (exri_uc *) 1;
   len = 123;

   ok = exri_writef_channels_to_memory(&bytes, &len, 8, 4, 3, pixels, channels, attrs, 2, compression);
   if (ok) {
      if (bytes == NULL || len <= 0) {
         fprintf(stderr, "successful channel write returned empty output\n");
         return 0;
      }
      exri_image_free(bytes);
   } else {
      if (bytes != NULL || len != 0) {
         fprintf(stderr, "failed channel write did not clear outputs\n");
         if (bytes != NULL && bytes != (exri_uc *) 1)
            exri_image_free(bytes);
         return 0;
      }
   }

   if (fail_live_allocs != 0) {
      fprintf(stderr, "channel writer leaked allocations: compression=%d fail_after=%d live=%d reason=%s\n",
              compression, fail_after, fail_live_allocs, exri_failure_reason());
      return 0;
   }

   return 1;
}

int main(void)
{
   int compressions[] = {
      EXRI_WRITE_COMPRESSION_NONE,
      EXRI_WRITE_COMPRESSION_RLE,
      EXRI_WRITE_COMPRESSION_ZIPS,
      EXRI_WRITE_COMPRESSION_ZIP,
      EXRI_WRITE_COMPRESSION_PIZ,
      EXRI_WRITE_COMPRESSION_PXR24,
      EXRI_WRITE_COMPRESSION_B44,
      EXRI_WRITE_COMPRESSION_B44A,
      EXRI_WRITE_COMPRESSION_ZIP | EXRI_WRITE_STORAGE_HALF,
      EXRI_WRITE_COMPRESSION_PIZ | EXRI_WRITE_STORAGE_HALF,
      EXRI_WRITE_COMPRESSION_PXR24 | EXRI_WRITE_STORAGE_HALF,
      EXRI_WRITE_COMPRESSION_ZIP | EXRI_WRITE_STORAGE_UINT,
      EXRI_WRITE_COMPRESSION_PIZ | EXRI_WRITE_STORAGE_UINT,
      EXRI_WRITE_COMPRESSION_PXR24 | EXRI_WRITE_STORAGE_UINT,
      EXRI_WRITE_COMPRESSION_ZIP | EXRI_WRITE_TILED | (4 << EXRI_WRITE_TILE_SIZE_SHIFT),
      EXRI_WRITE_COMPRESSION_PIZ | EXRI_WRITE_TILED | (4 << EXRI_WRITE_TILE_SIZE_SHIFT),
      EXRI_WRITE_COMPRESSION_B44 | EXRI_WRITE_TILED | (4 << EXRI_WRITE_TILE_SIZE_SHIFT),
      EXRI_WRITE_COMPRESSION_ZIP | EXRI_WRITE_STORAGE_UINT | EXRI_WRITE_TILED | (4 << EXRI_WRITE_TILE_SIZE_SHIFT)
   };
   int ci;
   int fail_after;

   for (ci = 0; ci < (int) (sizeof(compressions) / sizeof(compressions[0])); ++ci) {
      for (fail_after = 0; fail_after < 64; ++fail_after) {
         if (!run_write_with_failure(compressions[ci], fail_after))
            return 1;
      }
      if (!run_write_with_failure(compressions[ci], 1000000))
         return 1;
   }

   for (ci = 0; ci < 8; ++ci) {
      for (fail_after = 0; fail_after < 64; ++fail_after) {
         if (!run_channel_write_with_failure(compressions[ci], fail_after))
            return 1;
      }
      if (!run_channel_write_with_failure(compressions[ci], 1000000))
         return 1;
   }
   for (fail_after = 0; fail_after < 64; ++fail_after) {
      if (!run_channel_write_with_failure(EXRI_WRITE_COMPRESSION_ZIP | EXRI_WRITE_TILED | (3 << EXRI_WRITE_TILE_SIZE_SHIFT), fail_after))
         return 1;
   }
   if (!run_channel_write_with_failure(EXRI_WRITE_COMPRESSION_ZIP | EXRI_WRITE_TILED | (3 << EXRI_WRITE_TILE_SIZE_SHIFT), 1000000))
      return 1;
   for (fail_after = 0; fail_after < 64; ++fail_after) {
      if (!run_channel_write_with_failure(EXRI_WRITE_COMPRESSION_ZIP | EXRI_WRITE_STORAGE_UINT, fail_after))
         return 1;
   }
   if (!run_channel_write_with_failure(EXRI_WRITE_COMPRESSION_ZIP | EXRI_WRITE_STORAGE_UINT, 1000000))
      return 1;
   for (fail_after = 0; fail_after < 64; ++fail_after) {
      if (!run_channel_write_with_failure(EXRI_WRITE_COMPRESSION_ZIP | EXRI_WRITE_STORAGE_UINT | EXRI_WRITE_TILED | (3 << EXRI_WRITE_TILE_SIZE_SHIFT), fail_after))
         return 1;
   }
   if (!run_channel_write_with_failure(EXRI_WRITE_COMPRESSION_ZIP | EXRI_WRITE_STORAGE_UINT | EXRI_WRITE_TILED | (3 << EXRI_WRITE_TILE_SIZE_SHIFT), 1000000))
      return 1;

   printf("writer allocation-failure regression ok\n");
   return 0;
}
