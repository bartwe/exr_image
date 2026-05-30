#define EXR_IMAGE_IMPLEMENTATION
#include "../exr_image.h"
#include "exri_test_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
   unsigned char *data;
   int len;
   int cap;
   int short_write;
} writer_state;

typedef struct
{
   unsigned char const *data;
   int len;
   int pos;
} reader_state;

static int nearf(float a, float b)
{
   float d;

   d = a - b;
   if (d < 0.0f)
      d = -d;
   return d <= 0.00001f;
}

static int append_bytes(writer_state *w, void const *data, int size)
{
   unsigned char *p;
   int new_cap;

   if (size < 0 || w->len > 0x7fffffff - size)
      return 0;

   if (w->len + size > w->cap) {
      new_cap = w->cap ? w->cap : 256;
      while (new_cap < w->len + size) {
         if (new_cap > 0x7fffffff / 2)
            return 0;
         new_cap *= 2;
      }
      p = (unsigned char *) realloc(w->data, (size_t) new_cap);
      if (!p)
         return 0;
      w->data = p;
      w->cap = new_cap;
   }

   memcpy(w->data + w->len, data, (size_t) size);
   w->len += size;
   return 1;
}

static int EXRI_CALLBACK write_cb(void *user, void const *data, int size)
{
   writer_state *w;

   w = (writer_state *) user;
   if (w->short_write)
      return size > 0 ? size - 1 : 0;
   if (!append_bytes(w, data, size))
      return -1;
   return size;
}

static int EXRI_CALLBACK read_cb(void *user, char *data, int size)
{
   reader_state *r;
   int remaining;

   r = (reader_state *) user;
   if (r == NULL || data == NULL || size < 0)
      return -1;
   if (r->pos >= r->len)
      return 0;

   remaining = r->len - r->pos;
   if (size > remaining)
      size = remaining;
   memcpy(data, r->data + r->pos, (size_t) size);
   r->pos += size;
   return size;
}

static int EXRI_CALLBACK eof_cb(void *user)
{
   reader_state *r;

   r = (reader_state *) user;
   if (r == NULL)
      return 1;
   return r->pos >= r->len;
}

static void fill_pixels(float *pixels, int w, int h, int comp)
{
   int i;
   int total;

   total = w * h * comp;
   for (i = 0; i < total; ++i)
      pixels[i] = ((float) (i % 17) - 5.0f) * 0.25f;
}

static int compare_pixels(float const *a, float const *b, int count)
{
   int i;

   for (i = 0; i < count; ++i) {
      if (!nearf(a[i], b[i])) {
         fprintf(stderr, "pixel mismatch at %d: %g != %g\n", i, a[i], b[i]);
         return 0;
      }
   }

   return 1;
}

static int compare_region_pixels(float const *source, float const *region, int source_w, int comp, int rx, int ry, int rw, int rh)
{
   int x;
   int y;
   int c;
   int source_index;
   int region_index;

   for (y = 0; y < rh; ++y) {
      for (x = 0; x < rw; ++x) {
         for (c = 0; c < comp; ++c) {
            source_index = (((ry + y) * source_w + (rx + x)) * comp) + c;
            region_index = ((y * rw + x) * comp) + c;
            if (!nearf(source[source_index], region[region_index])) {
               fprintf(stderr, "region mismatch at %d,%d c=%d: %g != %g\n", x, y, c, source[source_index], region[region_index]);
               return 0;
            }
         }
      }
   }

   return 1;
}

static unsigned int get32(unsigned char const *p)
{
   return ((unsigned int) p[0]) |
          ((unsigned int) p[1] << 8) |
          ((unsigned int) p[2] << 16) |
          ((unsigned int) p[3] << 24);
}

static int compression_value(unsigned char const *data, int len, unsigned char *out)
{
   int pos;
   int start;
   int name_len;
   unsigned int attr_len;

   if (len < 9)
      return 0;

   pos = 8;
   for (;;) {
      if (pos >= len)
         return 0;

      start = pos;
      while (pos < len && data[pos] != 0)
         pos += 1;
      if (pos >= len)
         return 0;
      name_len = pos - start;
      pos += 1;
      if (name_len == 0)
         return 0;

      if (pos >= len)
         return 0;
      while (pos < len && data[pos] != 0)
         pos += 1;
      if (pos >= len)
         return 0;
      pos += 1;

      if (!exri__has_bytes_at(pos, len, 4))
         return 0;
      attr_len = get32(data + pos);
      pos += 4;
      if (attr_len > (unsigned int) (len - pos))
         return 0;

      if (name_len == 11 && memcmp(data + start, "compression", 11) == 0) {
         if (attr_len != 1u)
            return 0;
         *out = data[pos];
         return 1;
      }

      pos += (int) attr_len;
   }
}

static int check_memory_roundtrip(int comp, int compression)
{
   float source[4 * 3 * 4];
   unsigned char *bytes;
   float *loaded;
   int len;
   int x;
   int y;
   int c;
   int ok;
   unsigned char header_compression;

   fill_pixels(source, 4, 3, comp);
   bytes = NULL;
   len = -1;

   if (!exri_writef_to_memory(&bytes, &len, 4, 3, comp, source, compression)) {
      fprintf(stderr, "write memory failed comp=%d compression=%d: %s\n", comp, compression, exri_failure_reason());
      return 0;
   }
   if (!compression_value(bytes, len, &header_compression) || header_compression != (unsigned char) (compression & EXRI_WRITE_COMPRESSION_MASK)) {
      fprintf(stderr, "bad compression attr comp=%d compression=%d\n", comp, compression);
      exri_image_free(bytes);
      return 0;
   }

   x = y = c = 0;
   if (!exri_info_from_memory(bytes, len, &x, &y, &c)) {
      fprintf(stderr, "info failed comp=%d compression=%d: %s\n", comp, compression, exri_failure_reason());
      exri_image_free(bytes);
      return 0;
   }
   if (x != 4 || y != 3 || c != comp) {
      fprintf(stderr, "bad info comp=%d compression=%d got %dx%d c=%d\n", comp, compression, x, y, c);
      exri_image_free(bytes);
      return 0;
   }

   loaded = exri_loadf_from_memory(bytes, len, &x, &y, &c, comp);
   if (!loaded) {
      fprintf(stderr, "load failed comp=%d compression=%d: %s\n", comp, compression, exri_failure_reason());
      exri_image_free(bytes);
      return 0;
   }

   ok = compare_pixels(source, loaded, 4 * 3 * comp);
   exri_image_free(loaded);

   if (ok && (comp == 3 || comp == 4)) {
      loaded = exri_loadf_scrgb_from_memory(bytes, len, &x, &y, &c, comp, EXRI_COLOR_STRICT);
      if (!loaded) {
         fprintf(stderr, "strict scRGB load failed comp=%d compression=%d: %s\n", comp, compression, exri_failure_reason());
         ok = 0;
      } else {
         ok = compare_pixels(source, loaded, 4 * 3 * comp);
         exri_image_free(loaded);
      }
   }

   exri_image_free(bytes);
   return ok;
}

static int check_callbacks(void)
{
   float source[4 * 2 * 3];
   writer_state w;
   exri_write_callbacks cb;
   float *loaded;
   int x;
   int y;
   int c;
   int ok;

   fill_pixels(source, 4, 2, 3);
   memset(&w, 0, sizeof(w));
   cb.write = write_cb;

   if (!exri_writef_to_callbacks(&cb, &w, 4, 2, 3, source, EXRI_WRITE_COMPRESSION_NONE)) {
      fprintf(stderr, "callback write failed: %s\n", exri_failure_reason());
      free(w.data);
      return 0;
   }

   loaded = exri_loadf_from_memory(w.data, w.len, &x, &y, &c, 3);
   if (!loaded) {
      fprintf(stderr, "callback load failed: %s\n", exri_failure_reason());
      free(w.data);
      return 0;
   }

   ok = x == 4 && y == 2 && c == 3 && compare_pixels(source, loaded, 4 * 2 * 3);
   exri_image_free(loaded);
   free(w.data);

   memset(&w, 0, sizeof(w));
   if (!exri_writef_to_callbacks(&cb, &w, 4, 2, 3, source, EXRI_WRITE_COMPRESSION_RLE)) {
      fprintf(stderr, "callback rle write failed: %s\n", exri_failure_reason());
      return 0;
   }
   loaded = exri_loadf_from_memory(w.data, w.len, &x, &y, &c, 3);
   if (!loaded) {
      fprintf(stderr, "callback rle load failed: %s\n", exri_failure_reason());
      free(w.data);
      return 0;
   }
   ok = ok && x == 4 && y == 2 && c == 3 && compare_pixels(source, loaded, 4 * 2 * 3);
   exri_image_free(loaded);
   free(w.data);

   memset(&w, 0, sizeof(w));
   w.short_write = 1;
   ok = ok && !exri_writef_to_callbacks(&cb, &w, 4, 2, 3, source, EXRI_WRITE_COMPRESSION_RLE);
   free(w.data);
   return ok;
}

static int check_rle_fallback(void)
{
   float source[32 * 4 * 4];
   unsigned char *raw_bytes;
   unsigned char *rle_bytes;
   float *loaded;
   int raw_len;
   int rle_len;
   int x;
   int y;
   int c;
   int i;
   int ok;
   unsigned int state;
   unsigned int bits;

   state = 1u;
   for (i = 0; i < (int) (sizeof(source) / sizeof(source[0])); ++i) {
      state = state * 1664525u + 1013904223u;
      bits = state & 0x807fffffu;
      bits |= ((state >> 23) % 254u + 1u) << 23;
      memcpy(source + i, &bits, sizeof(bits));
   }

   raw_bytes = NULL;
   rle_bytes = NULL;
   raw_len = rle_len = 0;

   if (!exri_writef_to_memory(&raw_bytes, &raw_len, 32, 4, 4, source, EXRI_WRITE_COMPRESSION_NONE))
      return 0;
   if (!exri_writef_to_memory(&rle_bytes, &rle_len, 32, 4, 4, source, EXRI_WRITE_COMPRESSION_RLE)) {
      exri_image_free(raw_bytes);
      return 0;
   }

   if (rle_len != raw_len) {
      fprintf(stderr, "expected RLE raw fallback size to match raw: raw=%d rle=%d\n", raw_len, rle_len);
      exri_image_free(raw_bytes);
      exri_image_free(rle_bytes);
      return 0;
   }
   if (!exri_writef("/tmp/exri_write_rle_fallback.exr", 32, 4, 4, source, EXRI_WRITE_COMPRESSION_RLE)) {
      fprintf(stderr, "RLE fallback file write failed: %s\n", exri_failure_reason());
      exri_image_free(raw_bytes);
      exri_image_free(rle_bytes);
      return 0;
   }

   loaded = exri_loadf_from_memory(rle_bytes, rle_len, &x, &y, &c, 4);
   if (!loaded) {
      fprintf(stderr, "RLE fallback load failed: %s\n", exri_failure_reason());
      exri_image_free(raw_bytes);
      exri_image_free(rle_bytes);
      return 0;
   }

   ok = x == 32 && y == 4 && c == 4 && compare_pixels(source, loaded, 32 * 4 * 4);
   exri_image_free(loaded);
   exri_image_free(raw_bytes);
   exri_image_free(rle_bytes);
   return ok;
}

static int check_rle_smaller(void)
{
   float source[16 * 4 * 3];
   unsigned char *raw_bytes;
   unsigned char *rle_bytes;
   float *loaded;
   int raw_len;
   int rle_len;
   int x;
   int y;
   int c;
   int i;
   int ok;

   for (i = 0; i < (int) (sizeof(source) / sizeof(source[0])); ++i)
      source[i] = 0.0f;

   raw_bytes = NULL;
   rle_bytes = NULL;
   raw_len = rle_len = 0;

   if (!exri_writef_to_memory(&raw_bytes, &raw_len, 16, 4, 3, source, EXRI_WRITE_COMPRESSION_NONE))
      return 0;
   if (!exri_writef_to_memory(&rle_bytes, &rle_len, 16, 4, 3, source, EXRI_WRITE_COMPRESSION_RLE)) {
      exri_image_free(raw_bytes);
      return 0;
   }
   if (!exri_writef("/tmp/exri_write_rle_flat.exr", 16, 4, 3, source, EXRI_WRITE_COMPRESSION_RLE)) {
      exri_image_free(raw_bytes);
      exri_image_free(rle_bytes);
      return 0;
   }

   if (rle_len >= raw_len) {
      fprintf(stderr, "RLE did not reduce flat test image: raw=%d rle=%d\n", raw_len, rle_len);
      exri_image_free(raw_bytes);
      exri_image_free(rle_bytes);
      return 0;
   }

   loaded = exri_loadf_from_memory(rle_bytes, rle_len, &x, &y, &c, 3);
   if (!loaded) {
      fprintf(stderr, "RLE flat load failed: %s\n", exri_failure_reason());
      exri_image_free(raw_bytes);
      exri_image_free(rle_bytes);
      return 0;
   }

   ok = x == 16 && y == 4 && c == 3 && compare_pixels(source, loaded, 16 * 4 * 3);
   exri_image_free(loaded);
   exri_image_free(raw_bytes);
   exri_image_free(rle_bytes);
   return ok;
}

static int check_zip_blocks(void)
{
   float source[5 * 19 * 4];
   unsigned char *bytes;
   float *loaded;
   int len;
   int x;
   int y;
   int c;

   fill_pixels(source, 5, 19, 4);
   bytes = NULL;
   len = 0;

   if (!exri_writef_to_memory(&bytes, &len, 5, 19, 4, source, EXRI_WRITE_COMPRESSION_ZIP)) {
      fprintf(stderr, "ZIP memory write failed: %s\n", exri_failure_reason());
      return 0;
   }

   loaded = exri_loadf_from_memory(bytes, len, &x, &y, &c, 4);
   if (!loaded) {
      fprintf(stderr, "ZIP memory load failed: %s\n", exri_failure_reason());
      exri_image_free(bytes);
      return 0;
   }
   if (!(x == 5 && y == 19 && c == 4 && compare_pixels(source, loaded, 5 * 19 * 4))) {
      exri_image_free(loaded);
      exri_image_free(bytes);
      return 0;
   }
   exri_image_free(loaded);
   exri_image_free(bytes);

   if (!exri_writef("/tmp/exri_write_zip.exr", 5, 19, 4, source, EXRI_WRITE_COMPRESSION_ZIP)) {
      fprintf(stderr, "ZIP file write failed: %s\n", exri_failure_reason());
      return 0;
   }
   if (!exri_writef("/tmp/exri_write_zips.exr", 5, 19, 4, source, EXRI_WRITE_COMPRESSION_ZIPS)) {
      fprintf(stderr, "ZIPS file write failed: %s\n", exri_failure_reason());
      return 0;
   }
   if (!exri_writef("/tmp/exri_write_piz.exr", 5, 19, 4, source, EXRI_WRITE_COMPRESSION_PIZ)) {
      fprintf(stderr, "PIZ file write failed: %s\n", exri_failure_reason());
      return 0;
   }
   if (!exri_writef("/tmp/exri_write_pxr24.exr", 5, 19, 4, source, EXRI_WRITE_COMPRESSION_PXR24)) {
      fprintf(stderr, "PXR24 file write failed: %s\n", exri_failure_reason());
      return 0;
   }
   if (!exri_writef("/tmp/exri_write_half_zip.exr", 5, 19, 4, source, EXRI_WRITE_COMPRESSION_ZIP | EXRI_WRITE_STORAGE_HALF)) {
      fprintf(stderr, "HALF ZIP file write failed: %s\n", exri_failure_reason());
      return 0;
   }
   if (!exri_writef("/tmp/exri_write_half_piz.exr", 5, 19, 4, source, EXRI_WRITE_COMPRESSION_PIZ | EXRI_WRITE_STORAGE_HALF)) {
      fprintf(stderr, "HALF PIZ file write failed: %s\n", exri_failure_reason());
      return 0;
   }
   if (!exri_writef("/tmp/exri_write_half_pxr24.exr", 5, 19, 4, source, EXRI_WRITE_COMPRESSION_PXR24 | EXRI_WRITE_STORAGE_HALF)) {
      fprintf(stderr, "HALF PXR24 file write failed: %s\n", exri_failure_reason());
      return 0;
   }

   return 1;
}

static int check_b44_blocks(void)
{
   float source[9 * 35 * 4];
   unsigned char *bytes;
   float *loaded;
   int len;
   int x;
   int y;
   int c;

   fill_pixels(source, 9, 35, 4);
   bytes = NULL;
   len = 0;

   if (!exri_writef_to_memory(&bytes, &len, 9, 35, 4, source, EXRI_WRITE_COMPRESSION_B44)) {
      fprintf(stderr, "B44 memory write failed: %s\n", exri_failure_reason());
      return 0;
   }
   loaded = exri_loadf_from_memory(bytes, len, &x, &y, &c, 4);
   if (!loaded || x != 9 || y != 35 || c != 4) {
      fprintf(stderr, "B44 memory load failed: %s\n", loaded ? "bad info" : exri_failure_reason());
      if (loaded)
         exri_image_free(loaded);
      exri_image_free(bytes);
      return 0;
   }
   exri_image_free(loaded);
   exri_image_free(bytes);

   if (!exri_writef("/tmp/exri_write_b44.exr", 9, 35, 4, source, EXRI_WRITE_COMPRESSION_B44)) {
      fprintf(stderr, "B44 file write failed: %s\n", exri_failure_reason());
      return 0;
   }
   if (!exri_writef("/tmp/exri_write_b44a.exr", 9, 35, 4, source, EXRI_WRITE_COMPRESSION_B44A)) {
      fprintf(stderr, "B44A file write failed: %s\n", exri_failure_reason());
      return 0;
   }

   return 1;
}

static int check_tiled_writes(void)
{
   float source[7 * 5 * 4];
   unsigned char *bytes;
   float *loaded;
   int len;
   int x;
   int y;
   int c;
   int i;
   int ok;
   int flags;
   int nx;
   int ny;
   int version;
   int version_flags;
   FILE *f;
   reader_state r;
   exri_io_callbacks cb;
   int compressions[] = {
      EXRI_WRITE_COMPRESSION_NONE,
      EXRI_WRITE_COMPRESSION_RLE,
      EXRI_WRITE_COMPRESSION_ZIPS,
      EXRI_WRITE_COMPRESSION_ZIP,
      EXRI_WRITE_COMPRESSION_PIZ,
      EXRI_WRITE_COMPRESSION_PXR24,
      EXRI_WRITE_COMPRESSION_ZIP | EXRI_WRITE_STORAGE_HALF,
      EXRI_WRITE_COMPRESSION_PIZ | EXRI_WRITE_STORAGE_HALF,
      EXRI_WRITE_COMPRESSION_PXR24 | EXRI_WRITE_STORAGE_HALF
   };

   fill_pixels(source, 7, 5, 4);
   flags = EXRI_WRITE_TILED | (3 << EXRI_WRITE_TILE_SIZE_SHIFT);
   f = NULL;
   nx = 0;
   ny = 0;
   version = 0;
   version_flags = 0;
   memset(&r, 0, sizeof(r));
   memset(&cb, 0, sizeof(cb));

   for (i = 0; i < (int) (sizeof(compressions) / sizeof(compressions[0])); ++i) {
      bytes = NULL;
      len = 0;
      if (!exri_writef_to_memory(&bytes, &len, 7, 5, 4, source, compressions[i] | flags)) {
         fprintf(stderr, "tiled memory write failed compression=%d: %s\n", compressions[i], exri_failure_reason());
         return 0;
      }
      loaded = exri_loadf_from_memory(bytes, len, &x, &y, &c, 4);
      if (!loaded) {
         fprintf(stderr, "tiled memory load failed compression=%d: %s\n", compressions[i], exri_failure_reason());
         exri_image_free(bytes);
         return 0;
      }
      ok = x == 7 && y == 5 && c == 4 && compare_pixels(source, loaded, 7 * 5 * 4);
      exri_image_free(loaded);
      exri_image_free(bytes);
      if (!ok)
         return 0;
   }

   if (!exri_writef("/tmp/exri_write_tiled_zip.exr", 7, 5, 4, source, EXRI_WRITE_COMPRESSION_ZIP | flags)) {
      fprintf(stderr, "tiled ZIP file write failed: %s\n", exri_failure_reason());
      return 0;
   }
   if (!exri_writef("/tmp/exri_write_tiled_piz.exr", 7, 5, 4, source, EXRI_WRITE_COMPRESSION_PIZ | flags)) {
      fprintf(stderr, "tiled PIZ file write failed: %s\n", exri_failure_reason());
      return 0;
   }
   if (!exri_writef("/tmp/exri_write_tiled_half_zip.exr", 7, 5, 4, source, EXRI_WRITE_COMPRESSION_ZIP | EXRI_WRITE_STORAGE_HALF | flags)) {
      fprintf(stderr, "tiled HALF ZIP file write failed: %s\n", exri_failure_reason());
      return 0;
   }
   if (!exri_writef("/tmp/exri_write_tiled_b44.exr", 7, 5, 4, source, EXRI_WRITE_COMPRESSION_B44 | flags)) {
      fprintf(stderr, "tiled B44 file write failed: %s\n", exri_failure_reason());
      return 0;
   }
   if (!exri_writef("/tmp/exri_write_tiled_b44a.exr", 7, 5, 4, source, EXRI_WRITE_COMPRESSION_B44A | flags)) {
      fprintf(stderr, "tiled B44A file write failed: %s\n", exri_failure_reason());
      return 0;
   }
   if (!exri_version("/tmp/exri_write_tiled_zip.exr", &version, &version_flags) ||
       version != 2 ||
       (version_flags & EXRI_VERSION_FLAG_TILED) == 0 ||
       (version_flags & EXRI_VERSION_FLAG_MULTIPART) != 0) {
      fprintf(stderr, "tiled version filename parse failed version=%d flags=%d: %s\n", version, version_flags, exri_failure_reason());
      return 0;
   }
   f = fopen("/tmp/exri_write_tiled_zip.exr", "rb");
   if (f == NULL || !exri_version_from_file(f, &version, &version_flags) ||
       version != 2 ||
       (version_flags & EXRI_VERSION_FLAG_TILED) == 0) {
      if (f != NULL)
         fclose(f);
      fprintf(stderr, "tiled version FILE parse failed version=%d flags=%d: %s\n", version, version_flags, exri_failure_reason());
      return 0;
   }
   fclose(f);
   f = NULL;
   f = fopen("/tmp/exri_write_tiled_zip.exr", "rb");
   if (f == NULL || !exri_tiled_level_count_from_file(f, &nx, &ny) || nx != 1 || ny != 1) {
      if (f != NULL)
         fclose(f);
      fprintf(stderr, "tiled level FILE count failed nx=%d ny=%d: %s\n", nx, ny, exri_failure_reason());
      return 0;
   }
   fclose(f);
   f = NULL;
   if (!exri_part_tiled_level_count("/tmp/exri_write_tiled_zip.exr", 0, &nx, &ny) || nx != 1 || ny != 1) {
      fprintf(stderr, "part tiled level filename count failed nx=%d ny=%d: %s\n", nx, ny, exri_failure_reason());
      return 0;
   }
   f = fopen("/tmp/exri_write_tiled_zip.exr", "rb");
   if (f == NULL || !exri_part_tiled_level_count_from_file(f, 0, &nx, &ny) || nx != 1 || ny != 1) {
      if (f != NULL)
         fclose(f);
      fprintf(stderr, "part tiled level FILE count failed nx=%d ny=%d: %s\n", nx, ny, exri_failure_reason());
      return 0;
   }
   fclose(f);
   f = NULL;

   loaded = exri_loadf("/tmp/exri_write_tiled_b44.exr", &x, &y, &c, 4);
   if (!loaded || x != 7 || y != 5 || c != 4) {
      fprintf(stderr, "tiled B44 load failed: %s\n", loaded ? "bad info" : exri_failure_reason());
      if (loaded)
         exri_image_free(loaded);
      return 0;
   }
   exri_image_free(loaded);

   loaded = exri_loadf("/tmp/exri_write_tiled_b44a.exr", &x, &y, &c, 4);
   if (!loaded || x != 7 || y != 5 || c != 4) {
      fprintf(stderr, "tiled B44A load failed: %s\n", loaded ? "bad info" : exri_failure_reason());
      if (loaded)
         exri_image_free(loaded);
      return 0;
   }
   exri_image_free(loaded);

   bytes = NULL;
   len = 0;
   if (!exri_writef_to_memory(&bytes, &len, 7, 5, 4, source, EXRI_WRITE_COMPRESSION_ZIP | flags | EXRI_WRITE_TILED_MIPMAP | EXRI_WRITE_TILED_ROUND_UP)) {
      fprintf(stderr, "tiled mipmap ZIP write failed: %s\n", exri_failure_reason());
      return 0;
   }
   if (!exri_version_from_memory(bytes, len, &version, &version_flags) ||
       version != 2 ||
       (version_flags & EXRI_VERSION_FLAG_TILED) == 0) {
      fprintf(stderr, "tiled mipmap version parse failed version=%d flags=%d: %s\n", version, version_flags, exri_failure_reason());
      exri_image_free(bytes);
      return 0;
   }
   if (!exri_tiled_level_count_from_memory(bytes, len, &nx, &ny) || nx != 4 || ny != 4) {
      fprintf(stderr, "tiled mipmap level count failed nx=%d ny=%d: %s\n", nx, ny, exri_failure_reason());
      exri_image_free(bytes);
      return 0;
   }
   if (!exri_part_tiled_level_count_from_memory(bytes, len, 0, &nx, &ny) || nx != 4 || ny != 4) {
      fprintf(stderr, "part tiled mipmap level count failed nx=%d ny=%d: %s\n", nx, ny, exri_failure_reason());
      exri_image_free(bytes);
      return 0;
   }
   cb.read = read_cb;
   cb.skip = NULL;
   cb.eof = eof_cb;
   r.data = bytes;
   r.len = len;
   r.pos = 0;
   if (!exri_version_from_callbacks(&cb, &r, &version, &version_flags) ||
       version != 2 ||
       (version_flags & EXRI_VERSION_FLAG_TILED) == 0) {
      fprintf(stderr, "tiled mipmap callback version parse failed version=%d flags=%d: %s\n", version, version_flags, exri_failure_reason());
      exri_image_free(bytes);
      return 0;
   }
   r.pos = 0;
   if (!exri_tiled_level_count_from_callbacks(&cb, &r, &nx, &ny) || nx != 4 || ny != 4) {
      fprintf(stderr, "tiled mipmap callback level count failed nx=%d ny=%d: %s\n", nx, ny, exri_failure_reason());
      exri_image_free(bytes);
      return 0;
   }
   r.pos = 0;
   if (!exri_part_tiled_level_count_from_callbacks(&cb, &r, 0, &nx, &ny) || nx != 4 || ny != 4) {
      fprintf(stderr, "part tiled mipmap callback level count failed nx=%d ny=%d: %s\n", nx, ny, exri_failure_reason());
      exri_image_free(bytes);
      return 0;
   }
   loaded = exri_loadf_from_memory(bytes, len, &x, &y, &c, 4);
   if (!loaded || x != 7 || y != 5 || c != 4 || !compare_pixels(source, loaded, 7 * 5 * 4)) {
      fprintf(stderr, "tiled mipmap top level load failed: %s\n", loaded ? "bad data" : exri_failure_reason());
      if (loaded)
         exri_image_free(loaded);
      exri_image_free(bytes);
      return 0;
   }
   exri_image_free(loaded);
   loaded = exri_loadf_tiled_level_from_memory(bytes, len, 1, 1, &x, &y, &c, 4);
   if (!loaded || x != 4 || y != 3 || c != 4) {
      fprintf(stderr, "tiled mipmap selected level load failed x=%d y=%d c=%d: %s\n", x, y, c, loaded ? "bad info" : exri_failure_reason());
      if (loaded)
         exri_image_free(loaded);
      exri_image_free(bytes);
      return 0;
   }
   exri_image_free(loaded);
   exri_image_free(bytes);

   bytes = NULL;
   len = 0;
   if (!exri_writef_to_memory(&bytes, &len, 7, 5, 4, source, EXRI_WRITE_COMPRESSION_ZIP | flags | EXRI_WRITE_TILED_RIPMAP)) {
      fprintf(stderr, "tiled ripmap ZIP write failed: %s\n", exri_failure_reason());
      return 0;
   }
   if (!exri_tiled_level_count_from_memory(bytes, len, &nx, &ny) || nx != 3 || ny != 3) {
      fprintf(stderr, "tiled ripmap level count failed nx=%d ny=%d: %s\n", nx, ny, exri_failure_reason());
      exri_image_free(bytes);
      return 0;
   }
   loaded = exri_loadf_tiled_level_from_memory(bytes, len, 2, 1, &x, &y, &c, 4);
   if (!loaded || x != 1 || y != 2 || c != 4) {
      fprintf(stderr, "tiled ripmap selected level load failed x=%d y=%d c=%d: %s\n", x, y, c, loaded ? "bad info" : exri_failure_reason());
      if (loaded)
         exri_image_free(loaded);
      exri_image_free(bytes);
      return 0;
   }
   exri_image_free(loaded);
   exri_image_free(bytes);

   return 1;
}

static int check_region_loads(void)
{
   float source[7 * 5 * 4];
   unsigned char *bytes;
   float *loaded;
   int len;
   int x;
   int y;
   int c;
   int ok;
   int flags;

   fill_pixels(source, 7, 5, 4);
   bytes = NULL;
   len = 0;

   if (!exri_writef_to_memory(&bytes, &len, 7, 5, 4, source, EXRI_WRITE_COMPRESSION_ZIP)) {
      fprintf(stderr, "region scanline write failed: %s\n", exri_failure_reason());
      return 0;
   }

   loaded = exri_loadf_region_from_memory(bytes, len, 2, 1, 3, 2, &x, &y, &c, 4);
   if (!loaded) {
      fprintf(stderr, "region scanline load failed: %s\n", exri_failure_reason());
      exri_image_free(bytes);
      return 0;
   }
   ok = x == 3 && y == 2 && c == 4 && compare_region_pixels(source, loaded, 7, 4, 2, 1, 3, 2);
   exri_image_free(loaded);
   if (!ok) {
      exri_image_free(bytes);
      return 0;
   }

   loaded = (float *) 1;
   if (!exri_loadf_region_from_memory_to(&loaded, bytes, len, 0, 0, 7, 5, &x, &y, &c, 4) || loaded == NULL) {
      fprintf(stderr, "region memory _to wrapper failed: %s\n", exri_failure_reason());
      exri_image_free(bytes);
      return 0;
   }
   ok = x == 7 && y == 5 && c == 4 && compare_pixels(source, loaded, 7 * 5 * 4);
   exri_image_free(loaded);
   if (!ok) {
      exri_image_free(bytes);
      return 0;
   }

   loaded = exri_loadf_region_from_memory(bytes, len, 6, 0, 2, 1, &x, &y, &c, 4);
   if (loaded != NULL) {
      fprintf(stderr, "invalid region unexpectedly loaded\n");
      exri_image_free(loaded);
      exri_image_free(bytes);
      return 0;
   }
   exri_image_free(bytes);

   flags = EXRI_WRITE_TILED | (3 << EXRI_WRITE_TILE_SIZE_SHIFT);
   if (!exri_writef("/tmp/exri_write_region_tiled.exr", 7, 5, 4, source, EXRI_WRITE_COMPRESSION_ZIP | flags)) {
      fprintf(stderr, "region tiled file write failed: %s\n", exri_failure_reason());
      return 0;
   }
   loaded = exri_loadf_region("/tmp/exri_write_region_tiled.exr", 1, 1, 5, 3, &x, &y, &c, 4);
   if (!loaded) {
      fprintf(stderr, "region tiled file load failed: %s\n", exri_failure_reason());
      return 0;
   }
   ok = x == 5 && y == 3 && c == 4 && compare_region_pixels(source, loaded, 7, 4, 1, 1, 5, 3);
   exri_image_free(loaded);
   if (!ok)
      return 0;

   loaded = (float *) 1;
   if (!exri_loadf_region_to(&loaded, "/tmp/exri_write_region_tiled.exr", 3, 0, 2, 5, &x, &y, &c, 4) || loaded == NULL) {
      fprintf(stderr, "region file _to wrapper failed: %s\n", exri_failure_reason());
      return 0;
   }
   ok = x == 2 && y == 5 && c == 4 && compare_region_pixels(source, loaded, 7, 4, 3, 0, 2, 5);
   exri_image_free(loaded);
   return ok;
}

static int check_named_channel_writes(void)
{
   exri_write_channel channels[3];
   exri_write_attribute attrs[2];
   char const *layout;
   char const *units;
   float source[4 * 3 * 3];
   float wavelengths[4];
   unsigned char *bytes;
   float *loaded;
   writer_state w;
   exri_write_callbacks cb;
   exri_io_callbacks read_callbacks;
   reader_state r;
   FILE *f;
   char name[64];
   char units_out[64];
   int len;
   int x;
   int y;
   int c;
   int count;
   int pixel_type;
   int x_sampling;
   int y_sampling;
   int p_linear;
   int spectrum_type;
   int i;
   int ok;

   channels[0].name = "S0.610,500000nm";
   channels[1].name = "S0.550,000000nm";
   channels[2].name = "S1.550,000000nm";
   channels[0].pixel_type = EXRI_PIXEL_FLOAT;
   channels[1].pixel_type = EXRI_PIXEL_FLOAT;
   channels[2].pixel_type = EXRI_PIXEL_FLOAT;
   layout = "1.0";
   units = "W.m^-2.sr^-1.nm^-1";
   attrs[0].name = "spectralLayoutVersion";
   attrs[0].type = "string";
   attrs[0].value = (unsigned char const *) layout;
   attrs[0].value_size = (int) strlen(layout) + 1;
   attrs[1].name = "emissiveUnits";
   attrs[1].type = "string";
   attrs[1].value = (unsigned char const *) units;
   attrs[1].value_size = (int) strlen(units) + 1;

   for (i = 0; i < (int) (sizeof(source) / sizeof(source[0])); ++i)
      source[i] = ((float) (i % 11) - 4.0f) * 0.125f;

   bytes = NULL;
   len = 0;
   if (!exri_writef_channels_to_memory(&bytes, &len, 4, 3, 3, source, channels, attrs, 2, EXRI_WRITE_COMPRESSION_ZIP)) {
      fprintf(stderr, "named channel memory write failed: %s\n", exri_failure_reason());
      return 0;
   }

   if (!exri_channel_count_from_memory(bytes, len, &count) || count != 3) {
      fprintf(stderr, "bad named channel count: %d reason=%s\n", count, exri_failure_reason());
      exri_image_free(bytes);
      return 0;
   }
   for (i = 0; i < 3; ++i) {
      x_sampling = y_sampling = p_linear = 0;
      if (!exri_channel_name_from_memory(bytes, len, i, name, (int) sizeof(name)) ||
          strcmp(name, channels[i].name) != 0 ||
          !exri_channel_pixel_type_from_memory(bytes, len, i, &pixel_type) ||
          pixel_type != EXRI_PIXEL_FLOAT ||
          !exri_channel_sampling_from_memory(bytes, len, i, &x_sampling, &y_sampling, &p_linear) ||
          x_sampling != 1 || y_sampling != 1 || p_linear != 0) {
         fprintf(stderr, "bad named channel metadata at %d: %s type=%d sampling=%d,%d pLinear=%d reason=%s\n",
                 i, name, pixel_type, x_sampling, y_sampling, p_linear, exri_failure_reason());
         exri_image_free(bytes);
         return 0;
      }
   }

   loaded = exri_loadf_channels_from_memory(bytes, len, &x, &y, &c);
   if (!loaded) {
      fprintf(stderr, "named channel load failed: %s\n", exri_failure_reason());
      exri_image_free(bytes);
      return 0;
   }
   ok = x == 4 && y == 3 && c == 3 && compare_pixels(source, loaded, 4 * 3 * 3);
   exri_image_free(loaded);
   if (!ok) {
      exri_image_free(bytes);
      return 0;
   }

   if (!exri_is_spectral_from_memory(bytes, len) ||
       !exri_spectrum_type_from_memory(bytes, len, &spectrum_type) ||
       spectrum_type != EXRI_SPECTRUM_POLARISED ||
       !exri_spectral_wavelengths_from_memory(bytes, len, wavelengths, 4, &count) ||
       count != 2 ||
       !nearf(wavelengths[0], 550.0f) ||
       !nearf(wavelengths[1], 610.5f) ||
       !exri_spectral_units_from_memory(bytes, len, units_out, (int) sizeof(units_out)) ||
       strcmp(units_out, units) != 0) {
      fprintf(stderr, "named channel spectral metadata failed: %s\n", exri_failure_reason());
      exri_image_free(bytes);
      return 0;
   }

   exri_image_free(bytes);

   bytes = NULL;
   len = 0;
   if (!exri_writef_channels_to_memory(&bytes, &len, 4, 3, 3, source, channels, attrs, 2, EXRI_WRITE_COMPRESSION_PIZ)) {
      fprintf(stderr, "named channel PIZ write failed: %s\n", exri_failure_reason());
      return 0;
   }
   loaded = exri_loadf_channels_from_memory(bytes, len, &x, &y, &c);
   if (!loaded) {
      fprintf(stderr, "named channel PIZ load failed: %s\n", exri_failure_reason());
      exri_image_free(bytes);
      return 0;
   }
   ok = x == 4 && y == 3 && c == 3 && compare_pixels(source, loaded, 4 * 3 * 3);
   exri_image_free(loaded);
   exri_image_free(bytes);
   if (!ok) {
      fprintf(stderr, "named channel PIZ metadata/data mismatch x=%d y=%d c=%d\n", x, y, c);
      return 0;
   }

   bytes = NULL;
   len = 0;
   if (!exri_writef_channels_to_memory(&bytes, &len, 4, 3, 3, source, channels, attrs, 2, EXRI_WRITE_COMPRESSION_ZIP | EXRI_WRITE_TILED | (2 << EXRI_WRITE_TILE_SIZE_SHIFT))) {
      fprintf(stderr, "named channel tiled ZIP write failed: %s\n", exri_failure_reason());
      return 0;
   }
   loaded = exri_loadf_channels_from_memory(bytes, len, &x, &y, &c);
   if (!loaded) {
      fprintf(stderr, "named channel tiled ZIP load failed: %s\n", exri_failure_reason());
      exri_image_free(bytes);
      return 0;
   }
   ok = x == 4 && y == 3 && c == 3 && compare_pixels(source, loaded, 4 * 3 * 3);
   exri_image_free(loaded);
   exri_image_free(bytes);
   if (!ok) {
      fprintf(stderr, "named channel tiled ZIP metadata/data mismatch x=%d y=%d c=%d\n", x, y, c);
      return 0;
   }

   bytes = NULL;
   len = 0;
   channels[0].pixel_type = EXRI_PIXEL_HALF;
   channels[1].pixel_type = EXRI_PIXEL_HALF;
   channels[2].pixel_type = EXRI_PIXEL_HALF;
   if (!exri_writef_channels_to_memory(&bytes, &len, 4, 3, 3, source, channels, attrs, 2, EXRI_WRITE_COMPRESSION_ZIP | EXRI_WRITE_STORAGE_HALF)) {
      fprintf(stderr, "named channel HALF ZIP write failed: %s\n", exri_failure_reason());
      return 0;
   }
   if (!exri_channel_pixel_type_from_memory(bytes, len, 0, &pixel_type) || pixel_type != EXRI_PIXEL_HALF) {
      fprintf(stderr, "bad named channel HALF metadata: %s\n", exri_failure_reason());
      exri_image_free(bytes);
      return 0;
   }
   loaded = exri_loadf_channels_from_memory(bytes, len, &x, &y, &c);
   if (!loaded || x != 4 || y != 3 || c != 3) {
      fprintf(stderr, "named channel HALF load failed: %s\n", loaded ? "bad info" : exri_failure_reason());
      if (loaded)
         exri_image_free(loaded);
      exri_image_free(bytes);
      return 0;
   }
   exri_image_free(loaded);
   exri_image_free(bytes);
   channels[0].pixel_type = EXRI_PIXEL_FLOAT;
   channels[1].pixel_type = EXRI_PIXEL_FLOAT;
   channels[2].pixel_type = EXRI_PIXEL_FLOAT;

   memset(&w, 0, sizeof(w));
   cb.write = write_cb;
   if (!exri_writef_channels_to_callbacks(&cb, &w, 4, 3, 3, source, channels, attrs, 2, EXRI_WRITE_COMPRESSION_RLE)) {
      fprintf(stderr, "named channel callback write failed: %s\n", exri_failure_reason());
      free(w.data);
      return 0;
   }
   loaded = exri_loadf_channels_from_memory(w.data, w.len, &x, &y, &c);
   if (!loaded || x != 4 || y != 3 || c != 3) {
      fprintf(stderr, "named channel callback load failed: %s\n", loaded ? "bad info" : exri_failure_reason());
      if (loaded)
         exri_image_free(loaded);
      free(w.data);
      return 0;
   }
   exri_image_free(loaded);
   read_callbacks.read = read_cb;
   read_callbacks.skip = NULL;
   read_callbacks.eof = eof_cb;
   r.data = w.data;
   r.len = w.len;
   r.pos = 0;
   if (!exri_channel_count_from_callbacks(&read_callbacks, &r, &count) || count != 3) {
      fprintf(stderr, "named channel callback metadata count failed: %s\n", exri_failure_reason());
      free(w.data);
      return 0;
   }
   r.pos = 0;
   x_sampling = y_sampling = p_linear = 0;
   if (exri_channel_name_from_callbacks(&read_callbacks, &r, 1, name, (int) sizeof(name)) != (int) strlen(channels[1].name) ||
       strcmp(name, channels[1].name) != 0) {
      fprintf(stderr, "named channel callback metadata name failed: %s\n", exri_failure_reason());
      free(w.data);
      return 0;
   }
   r.pos = 0;
   if (!exri_channel_pixel_type_from_callbacks(&read_callbacks, &r, 1, &pixel_type) ||
       pixel_type != EXRI_PIXEL_FLOAT) {
      fprintf(stderr, "named channel callback metadata type failed: %s\n", exri_failure_reason());
      free(w.data);
      return 0;
   }
   r.pos = 0;
   if (!exri_channel_sampling_from_callbacks(&read_callbacks, &r, 1, &x_sampling, &y_sampling, &p_linear) ||
       x_sampling != 1 || y_sampling != 1 || p_linear != 0) {
      fprintf(stderr, "named channel callback metadata sampling failed: %d,%d pLinear=%d reason=%s\n",
              x_sampling, y_sampling, p_linear, exri_failure_reason());
      free(w.data);
      return 0;
   }
   free(w.data);

   if (!exri_writef_channels("/tmp/exri_write_named_channels.exr", 4, 3, 3, source, channels, attrs, 2, EXRI_WRITE_COMPRESSION_ZIP)) {
      fprintf(stderr, "named channel file write failed: %s\n", exri_failure_reason());
      return 0;
   }
   if (!exri_channel_count("/tmp/exri_write_named_channels.exr", &count) || count != 3 ||
       exri_channel_name("/tmp/exri_write_named_channels.exr", 2, name, (int) sizeof(name)) != (int) strlen(channels[2].name) ||
       strcmp(name, channels[2].name) != 0 ||
       !exri_channel_pixel_type("/tmp/exri_write_named_channels.exr", 2, &pixel_type) ||
       pixel_type != EXRI_PIXEL_FLOAT ||
       !exri_channel_sampling("/tmp/exri_write_named_channels.exr", 2, &x_sampling, &y_sampling, &p_linear) ||
       x_sampling != 1 || y_sampling != 1 || p_linear != 0) {
      fprintf(stderr, "named channel filename metadata failed: %s\n", exri_failure_reason());
      return 0;
   }
   loaded = exri_loadf_channels("/tmp/exri_write_named_channels.exr", &x, &y, &c);
   if (!loaded || x != 4 || y != 3 || c != 3) {
      fprintf(stderr, "named channel file load failed: %s\n", loaded ? "bad info" : exri_failure_reason());
      if (loaded)
         exri_image_free(loaded);
      return 0;
   }
   exri_image_free(loaded);

   f = fopen("/tmp/exri_write_named_channels_fileptr.exr", "wb");
   if (f == NULL)
      return 0;
   ok = exri_writef_channels_to_file(f, 4, 3, 3, source, channels, attrs, 2, EXRI_WRITE_COMPRESSION_ZIPS);
   if (fclose(f) != 0)
      ok = 0;
   if (!ok) {
      fprintf(stderr, "named channel FILE* write failed: %s\n", exri_failure_reason());
      return 0;
   }
   f = fopen("/tmp/exri_write_named_channels_fileptr.exr", "rb");
   if (f == NULL)
      return 0;
   ok = exri_channel_count_from_file(f, &count) && count == 3;
   fclose(f);
   f = fopen("/tmp/exri_write_named_channels_fileptr.exr", "rb");
   if (f == NULL)
      return 0;
   ok = ok && exri_channel_name_from_file(f, 0, name, (int) sizeof(name)) == (int) strlen(channels[0].name) &&
        strcmp(name, channels[0].name) == 0;
   fclose(f);
   f = fopen("/tmp/exri_write_named_channels_fileptr.exr", "rb");
   if (f == NULL)
      return 0;
   ok = ok && exri_channel_pixel_type_from_file(f, 0, &pixel_type) && pixel_type == EXRI_PIXEL_FLOAT;
   fclose(f);
   f = fopen("/tmp/exri_write_named_channels_fileptr.exr", "rb");
   if (f == NULL)
      return 0;
   x_sampling = y_sampling = p_linear = 0;
   ok = ok && exri_channel_sampling_from_file(f, 0, &x_sampling, &y_sampling, &p_linear) &&
        x_sampling == 1 && y_sampling == 1 && p_linear == 0;
   fclose(f);
   if (!ok) {
      fprintf(stderr, "named channel FILE* metadata failed: %s\n", exri_failure_reason());
      return 0;
   }

   return 1;
}

static int check_uint_writes(void)
{
   exri_write_channel channels[3];
   float simple[5 * 4 * 4];
   float named[5 * 4 * 3];
   unsigned char *bytes;
   float *loaded;
   int len;
   int x;
   int y;
   int c;
   int pixel_type;
   int i;
   int ok;

   for (i = 0; i < (int) (sizeof(simple) / sizeof(simple[0])); ++i)
      simple[i] = (float) ((i * 37) & 65535);
   for (i = 0; i < (int) (sizeof(named) / sizeof(named[0])); ++i)
      named[i] = (float) ((i * 19 + 3) & 65535);

   bytes = NULL;
   len = 0;
   if (!exri_writef_to_memory(&bytes, &len, 5, 4, 4, simple, EXRI_WRITE_COMPRESSION_ZIP | EXRI_WRITE_STORAGE_UINT)) {
      fprintf(stderr, "UINT ZIP write failed: %s\n", exri_failure_reason());
      return 0;
   }
   pixel_type = -1;
   if (!exri_channel_pixel_type_from_memory(bytes, len, 0, &pixel_type) || pixel_type != EXRI_PIXEL_UINT) {
      fprintf(stderr, "bad UINT channel metadata: %d reason=%s\n", pixel_type, exri_failure_reason());
      exri_image_free(bytes);
      return 0;
   }
   loaded = exri_loadf_from_memory(bytes, len, &x, &y, &c, 4);
   if (!loaded) {
      fprintf(stderr, "UINT ZIP load failed: %s\n", exri_failure_reason());
      exri_image_free(bytes);
      return 0;
   }
   ok = x == 5 && y == 4 && c == 4 && compare_pixels(simple, loaded, 5 * 4 * 4);
   exri_image_free(loaded);
   exri_image_free(bytes);
   if (!ok)
      return 0;

   bytes = NULL;
   len = 0;
   if (!exri_writef_to_memory(&bytes, &len, 5, 4, 4, simple, EXRI_WRITE_COMPRESSION_PIZ | EXRI_WRITE_STORAGE_UINT)) {
      fprintf(stderr, "UINT PIZ write failed: %s\n", exri_failure_reason());
      return 0;
   }
   loaded = exri_loadf_from_memory(bytes, len, &x, &y, &c, 4);
   if (!loaded) {
      fprintf(stderr, "UINT PIZ load failed: %s\n", exri_failure_reason());
      exri_image_free(bytes);
      return 0;
   }
   ok = x == 5 && y == 4 && c == 4 && compare_pixels(simple, loaded, 5 * 4 * 4);
   exri_image_free(loaded);
   exri_image_free(bytes);
   if (!ok)
      return 0;

   bytes = NULL;
   len = 0;
   if (!exri_writef_to_memory(&bytes, &len, 5, 4, 4, simple, EXRI_WRITE_COMPRESSION_PXR24 | EXRI_WRITE_STORAGE_UINT)) {
      fprintf(stderr, "UINT PXR24 write failed: %s\n", exri_failure_reason());
      return 0;
   }
   loaded = exri_loadf_from_memory(bytes, len, &x, &y, &c, 4);
   if (!loaded) {
      fprintf(stderr, "UINT PXR24 load failed: %s\n", exri_failure_reason());
      exri_image_free(bytes);
      return 0;
   }
   ok = x == 5 && y == 4 && c == 4 && compare_pixels(simple, loaded, 5 * 4 * 4);
   exri_image_free(loaded);
   exri_image_free(bytes);
   if (!ok)
      return 0;

   bytes = NULL;
   len = 0;
   if (!exri_writef_to_memory(&bytes, &len, 5, 4, 4, simple, EXRI_WRITE_COMPRESSION_ZIP | EXRI_WRITE_STORAGE_UINT | EXRI_WRITE_TILED | (3 << EXRI_WRITE_TILE_SIZE_SHIFT))) {
      fprintf(stderr, "UINT tiled ZIP write failed: %s\n", exri_failure_reason());
      return 0;
   }
   loaded = exri_loadf_from_memory(bytes, len, &x, &y, &c, 4);
   if (!loaded) {
      fprintf(stderr, "UINT tiled ZIP load failed: %s\n", exri_failure_reason());
      exri_image_free(bytes);
      return 0;
   }
   ok = x == 5 && y == 4 && c == 4 && compare_pixels(simple, loaded, 5 * 4 * 4);
   exri_image_free(loaded);
   exri_image_free(bytes);
   if (!ok)
      return 0;

   channels[0].name = "object_id";
   channels[1].name = "material_id";
   channels[2].name = "sample_id";
   channels[0].pixel_type = EXRI_PIXEL_UINT;
   channels[1].pixel_type = EXRI_PIXEL_UINT;
   channels[2].pixel_type = EXRI_PIXEL_UINT;
   bytes = NULL;
   len = 0;
   if (!exri_writef_channels_to_memory(&bytes, &len, 5, 4, 3, named, channels, NULL, 0, EXRI_WRITE_COMPRESSION_ZIP | EXRI_WRITE_STORAGE_UINT)) {
      fprintf(stderr, "named UINT ZIP write failed: %s\n", exri_failure_reason());
      return 0;
   }
   for (i = 0; i < 3; ++i) {
      pixel_type = -1;
      if (!exri_channel_pixel_type_from_memory(bytes, len, i, &pixel_type) || pixel_type != EXRI_PIXEL_UINT) {
         fprintf(stderr, "bad named UINT channel metadata at %d: %d reason=%s\n", i, pixel_type, exri_failure_reason());
         exri_image_free(bytes);
         return 0;
      }
   }
   loaded = exri_loadf_channels_from_memory(bytes, len, &x, &y, &c);
   if (!loaded) {
      fprintf(stderr, "named UINT ZIP load failed: %s\n", exri_failure_reason());
      exri_image_free(bytes);
      return 0;
   }
   ok = x == 5 && y == 4 && c == 3 && compare_pixels(named, loaded, 5 * 4 * 3);
   exri_image_free(loaded);
   exri_image_free(bytes);
   if (!ok)
      return 0;

   bytes = NULL;
   len = 0;
   if (!exri_writef_channels_to_memory(&bytes, &len, 5, 4, 3, named, channels, NULL, 0, EXRI_WRITE_COMPRESSION_PXR24 | EXRI_WRITE_STORAGE_UINT)) {
      fprintf(stderr, "named UINT PXR24 write failed: %s\n", exri_failure_reason());
      return 0;
   }
   loaded = exri_loadf_channels_from_memory(bytes, len, &x, &y, &c);
   if (!loaded) {
      fprintf(stderr, "named UINT PXR24 load failed: %s\n", exri_failure_reason());
      exri_image_free(bytes);
      return 0;
   }
   ok = x == 5 && y == 4 && c == 3 && compare_pixels(named, loaded, 5 * 4 * 3);
   exri_image_free(loaded);
   exri_image_free(bytes);
   if (!ok)
      return 0;

   bytes = NULL;
   len = 0;
   if (!exri_writef_channels_to_memory(&bytes, &len, 5, 4, 3, named, channels, NULL, 0, EXRI_WRITE_COMPRESSION_ZIP | EXRI_WRITE_STORAGE_UINT | EXRI_WRITE_TILED | (3 << EXRI_WRITE_TILE_SIZE_SHIFT))) {
      fprintf(stderr, "named UINT tiled ZIP write failed: %s\n", exri_failure_reason());
      return 0;
   }
   loaded = exri_loadf_channels_from_memory(bytes, len, &x, &y, &c);
   if (!loaded) {
      fprintf(stderr, "named UINT tiled ZIP load failed: %s\n", exri_failure_reason());
      exri_image_free(bytes);
      return 0;
   }
   ok = x == 5 && y == 4 && c == 3 && compare_pixels(named, loaded, 5 * 4 * 3);
   exri_image_free(loaded);
   exri_image_free(bytes);
   if (!ok)
      return 0;

   channels[0].pixel_type = EXRI_PIXEL_FLOAT;
   channels[1].pixel_type = EXRI_PIXEL_HALF;
   channels[2].pixel_type = EXRI_PIXEL_UINT;
   bytes = NULL;
   len = 0;
   if (!exri_writef_channels_to_memory(&bytes, &len, 5, 4, 3, named, channels, NULL, 0, EXRI_WRITE_COMPRESSION_ZIP)) {
      fprintf(stderr, "mixed channel ZIP write failed: %s\n", exri_failure_reason());
      return 0;
   }
   for (i = 0; i < 3; ++i) {
      pixel_type = -1;
      if (!exri_channel_pixel_type_from_memory(bytes, len, i, &pixel_type) || pixel_type != channels[i].pixel_type) {
         fprintf(stderr, "bad mixed channel metadata at %d: %d reason=%s\n", i, pixel_type, exri_failure_reason());
         exri_image_free(bytes);
         return 0;
      }
   }
   loaded = exri_loadf_channels_from_memory(bytes, len, &x, &y, &c);
   if (!loaded) {
      fprintf(stderr, "mixed channel ZIP load failed: %s\n", exri_failure_reason());
      exri_image_free(bytes);
      return 0;
   }
   ok = x == 5 && y == 4 && c == 3 && compare_pixels(named, loaded, 5 * 4 * 3);
   exri_image_free(loaded);
   exri_image_free(bytes);
   if (!ok)
      return 0;

   bytes = NULL;
   len = 0;
   if (!exri_writef_channels_to_memory(&bytes, &len, 5, 4, 3, named, channels, NULL, 0, EXRI_WRITE_COMPRESSION_PIZ)) {
      fprintf(stderr, "mixed channel PIZ write failed: %s\n", exri_failure_reason());
      return 0;
   }
   loaded = exri_loadf_channels_from_memory(bytes, len, &x, &y, &c);
   if (!loaded) {
      fprintf(stderr, "mixed channel PIZ load failed: %s\n", exri_failure_reason());
      exri_image_free(bytes);
      return 0;
   }
   ok = x == 5 && y == 4 && c == 3 && compare_pixels(named, loaded, 5 * 4 * 3);
   exri_image_free(loaded);
   exri_image_free(bytes);
   if (!ok)
      return 0;

   bytes = NULL;
   len = 0;
   if (!exri_writef_channels_to_memory(&bytes, &len, 5, 4, 3, named, channels, NULL, 0, EXRI_WRITE_COMPRESSION_PXR24)) {
      fprintf(stderr, "mixed channel PXR24 write failed: %s\n", exri_failure_reason());
      return 0;
   }
   loaded = exri_loadf_channels_from_memory(bytes, len, &x, &y, &c);
   if (!loaded) {
      fprintf(stderr, "mixed channel PXR24 load failed: %s\n", exri_failure_reason());
      exri_image_free(bytes);
      return 0;
   }
   ok = x == 5 && y == 4 && c == 3 && compare_pixels(named, loaded, 5 * 4 * 3);
   exri_image_free(loaded);
   exri_image_free(bytes);
   if (!ok)
      return 0;

   bytes = NULL;
   len = 0;
   if (!exri_writef_channels_to_memory(&bytes, &len, 5, 4, 3, named, channels, NULL, 0, EXRI_WRITE_COMPRESSION_PIZ | EXRI_WRITE_TILED | (3 << EXRI_WRITE_TILE_SIZE_SHIFT))) {
      fprintf(stderr, "mixed channel tiled PIZ write failed: %s\n", exri_failure_reason());
      return 0;
   }
   loaded = exri_loadf_channels_from_memory(bytes, len, &x, &y, &c);
   if (!loaded) {
      fprintf(stderr, "mixed channel tiled PIZ load failed: %s\n", exri_failure_reason());
      exri_image_free(bytes);
      return 0;
   }
   ok = x == 5 && y == 4 && c == 3 && compare_pixels(named, loaded, 5 * 4 * 3);
   exri_image_free(loaded);
   exri_image_free(bytes);
   if (!ok)
      return 0;

   bytes = NULL;
   len = 0;
   if (!exri_writef_channels_to_memory(&bytes, &len, 5, 4, 3, named, channels, NULL, 0, EXRI_WRITE_COMPRESSION_ZIP | EXRI_WRITE_TILED | (3 << EXRI_WRITE_TILE_SIZE_SHIFT))) {
      fprintf(stderr, "mixed channel tiled ZIP write failed: %s\n", exri_failure_reason());
      return 0;
   }
   loaded = exri_loadf_channels_from_memory(bytes, len, &x, &y, &c);
   if (!loaded) {
      fprintf(stderr, "mixed channel tiled ZIP load failed: %s\n", exri_failure_reason());
      exri_image_free(bytes);
      return 0;
   }
   ok = x == 5 && y == 4 && c == 3 && compare_pixels(named, loaded, 5 * 4 * 3);
   exri_image_free(loaded);
   exri_image_free(bytes);
   return ok;
}

static int check_internal_rle_packets(void)
{
   unsigned char source[260];
   unsigned char scratch[260];
   unsigned char encoded[512];
   unsigned char decoded[260];
   unsigned int state;
   int seed;
   int i;
   int encoded_len;

   for (seed = 0; seed < 256; ++seed) {
      state = (unsigned int) seed + 1u;
      for (i = 0; i < (int) sizeof(source); ++i) {
         state = state * 1103515245u + 12345u;
         source[i] = (unsigned char) (state >> 16);
      }

      if (!exri__compress_rle_exr_block(encoded, (int) sizeof(encoded), scratch, source, (int) sizeof(source), &encoded_len)) {
         fprintf(stderr, "internal RLE encode failed at seed=%d: %s\n", seed, exri_failure_reason());
         return 0;
      }
      if (!exri__unrle_exr_block(decoded, (int) sizeof(decoded), encoded, encoded_len)) {
         fprintf(stderr, "internal RLE decode failed at seed=%d: %s\n", seed, exri_failure_reason());
         return 0;
      }
      if (memcmp(source, decoded, sizeof(source)) != 0) {
         fprintf(stderr, "internal RLE roundtrip mismatch at seed=%d\n", seed);
         return 0;
      }
   }

   return 1;
}

static int check_file(void)
{
   char const *path;
   float source[3 * 2 * 4];
   float *loaded;
   int x;
   int y;
   int c;
   int ok;
   FILE *f;

   path = "/tmp/exri_write_roundtrip.exr";
   fill_pixels(source, 3, 2, 4);

   if (!exri_writef(path, 3, 2, 4, source, EXRI_WRITE_COMPRESSION_NONE)) {
      fprintf(stderr, "file write failed: %s\n", exri_failure_reason());
      return 0;
   }

   loaded = exri_loadf(path, &x, &y, &c, 4);
   if (!loaded) {
      fprintf(stderr, "file load failed: %s\n", exri_failure_reason());
      return 0;
   }

   ok = x == 3 && y == 2 && c == 4 && compare_pixels(source, loaded, 3 * 2 * 4);
   exri_image_free(loaded);
   if (!ok)
      return 0;

   path = "/tmp/exri_write_fileptr.exr";
   f = fopen(path, "wb");
   if (!f)
      return 0;
   ok = exri_writef_to_file(f, 3, 2, 4, source, EXRI_WRITE_COMPRESSION_RLE);
   if (fclose(f) != 0)
      ok = 0;
   if (!ok) {
      fprintf(stderr, "FILE* write failed: %s\n", exri_failure_reason());
      return 0;
   }
   loaded = exri_loadf(path, &x, &y, &c, 4);
   if (!loaded) {
      fprintf(stderr, "FILE* load failed: %s\n", exri_failure_reason());
      return 0;
   }
   ok = x == 3 && y == 2 && c == 4 && compare_pixels(source, loaded, 3 * 2 * 4);
   exri_image_free(loaded);
   if (!ok)
      return 0;

   path = "/tmp/exri_write_rle.exr";
   if (!exri_writef(path, 3, 2, 4, source, EXRI_WRITE_COMPRESSION_RLE)) {
      fprintf(stderr, "rle file write failed: %s\n", exri_failure_reason());
      return 0;
   }

   loaded = exri_loadf(path, &x, &y, &c, 4);
   if (!loaded) {
      fprintf(stderr, "rle file load failed: %s\n", exri_failure_reason());
      return 0;
   }

   ok = x == 3 && y == 2 && c == 4 && compare_pixels(source, loaded, 3 * 2 * 4);
   exri_image_free(loaded);
   return ok;
}

static int check_invalid(void)
{
   float source[4];
   unsigned char *bytes;
   exri_write_channel channels[2];
   exri_write_attribute attrs[2];
   char const *layout;
   int len;

   source[0] = 0.0f;
   channels[0].name = "A";
   channels[0].pixel_type = EXRI_PIXEL_FLOAT;
   channels[1].name = "A";
   channels[1].pixel_type = EXRI_PIXEL_FLOAT;
   layout = "1.0";
   attrs[0].name = "channels";
   attrs[0].type = "string";
   attrs[0].value = (unsigned char const *) layout;
   attrs[0].value_size = (int) strlen(layout) + 1;
   attrs[1].name = "spectralLayoutVersion";
   attrs[1].type = "string";
   attrs[1].value = (unsigned char const *) layout;
   attrs[1].value_size = (int) strlen(layout) + 1;

   bytes = (unsigned char *) 1;
   len = 123;
   if (exri_writef_to_memory(&bytes, &len, 0, 1, 1, source, EXRI_WRITE_COMPRESSION_NONE))
      return 0;
   if (bytes != NULL || len != 0)
      return 0;

   bytes = (unsigned char *) 1;
   len = 123;
   if (exri_writef_to_memory(&bytes, &len, 1, 1, 1, source, 99))
      return 0;
   if (bytes != NULL || len != 0)
      return 0;

   bytes = (unsigned char *) 1;
   len = 123;
   if (exri_writef_to_memory(&bytes, &len, 1, 1, 1, source, EXRI_WRITE_COMPRESSION_ZIP | 0x4000))
      return 0;
   if (bytes != NULL || len != 0)
      return 0;

   bytes = (unsigned char *) 1;
   len = 123;
   if (exri_writef_to_memory(&bytes, &len, 1, 1, 1, source, EXRI_WRITE_COMPRESSION_ZIP | (3 << EXRI_WRITE_TILE_SIZE_SHIFT)))
      return 0;
   if (bytes != NULL || len != 0)
      return 0;

   bytes = (unsigned char *) 1;
   len = 123;
   if (exri_writef_to_memory(&bytes, &len, 1, 1, 1, source, EXRI_WRITE_COMPRESSION_ZIP | EXRI_WRITE_TILED_MIPMAP))
      return 0;
   if (bytes != NULL || len != 0)
      return 0;

   bytes = (unsigned char *) 1;
   len = 123;
   if (exri_writef_to_memory(&bytes, &len, 1, 1, 1, source, EXRI_WRITE_COMPRESSION_ZIP | EXRI_WRITE_TILED | EXRI_WRITE_TILED_MIPMAP | EXRI_WRITE_TILED_RIPMAP))
      return 0;
   if (bytes != NULL || len != 0)
      return 0;

   bytes = (unsigned char *) 1;
   len = 123;
   if (exri_writef_to_memory(&bytes, &len, 1, 1, 1, source, EXRI_WRITE_COMPRESSION_ZIP | EXRI_WRITE_TILED | EXRI_WRITE_TILED_ROUND_UP))
      return 0;
   if (bytes != NULL || len != 0)
      return 0;

   bytes = (unsigned char *) 1;
   len = 123;
   if (exri_writef_to_memory(&bytes, &len, 1, 1, 1, source, EXRI_WRITE_COMPRESSION_ZIP | EXRI_WRITE_STORAGE_HALF | EXRI_WRITE_STORAGE_UINT))
      return 0;
   if (bytes != NULL || len != 0)
      return 0;

   bytes = (unsigned char *) 1;
   len = 123;
   if (exri_writef_channels_to_memory(&bytes, &len, 1, 1, 2, source, channels, NULL, 0, EXRI_WRITE_COMPRESSION_NONE))
      return 0;
   if (bytes != NULL || len != 0)
      return 0;

   channels[1].name = "B";
   bytes = (unsigned char *) 1;
   len = 123;
   if (exri_writef_channels_to_memory(&bytes, &len, 1, 1, 2, source, channels, attrs, 1, EXRI_WRITE_COMPRESSION_NONE))
      return 0;
   if (bytes != NULL || len != 0)
      return 0;

   attrs[0] = attrs[1];
   bytes = (unsigned char *) 1;
   len = 123;
   if (exri_writef_channels_to_memory(&bytes, &len, 1, 1, 2, source, channels, attrs, 2, EXRI_WRITE_COMPRESSION_NONE))
      return 0;
   if (bytes != NULL || len != 0)
      return 0;

   bytes = (unsigned char *) 1;
   len = 123;
   if (exri_writef_channels_to_memory(&bytes, &len, 1, 1, 2, source, channels, NULL, 0, EXRI_WRITE_COMPRESSION_ZIP | (3 << EXRI_WRITE_TILE_SIZE_SHIFT)))
      return 0;
   if (bytes != NULL || len != 0)
      return 0;

   bytes = (unsigned char *) 1;
   len = 123;
   if (exri_writef_channels_to_memory(&bytes, &len, 1, 1, 2, source, channels, NULL, 0, EXRI_WRITE_COMPRESSION_B44 | EXRI_WRITE_STORAGE_UINT))
      return 0;
   if (bytes != NULL || len != 0)
      return 0;

   return 1;
}

static float multipart_source_value(int part, int x, int y, int c)
{
   if (c == 3)
      return 1.0f;
   return (float) (part * 100 + c * 10 + y * 3 + x);
}

static int check_multipart_writes(void)
{
   exri_write_part parts[2];
   float part0[3 * 2 * 4];
   float part1[3 * 2 * 4];
   float *arrays[2];
   unsigned char *bytes;
   unsigned char *bad_bytes;
   float *pixels;
   int len;
   int part_count;
   int x;
   int y;
   int comp;
   int part;
   int i;
   int c;
   int version;
   int version_flags;

   arrays[0] = part0;
   arrays[1] = part1;
   for (part = 0; part < 2; ++part) {
      for (y = 0; y < 2; ++y) {
         for (x = 0; x < 3; ++x) {
            for (c = 0; c < 4; ++c)
               arrays[part][((y * 3 + x) * 4) + c] = multipart_source_value(part, x, y, c);
         }
      }
   }

   memset(parts, 0, sizeof(parts));
   parts[0].name = "left";
   parts[0].width = 3;
   parts[0].height = 2;
   parts[0].num_channels = 4;
   parts[0].data = part0;
   exri_test_write_options_from_flags(&parts[0].options, EXRI_WRITE_COMPRESSION_ZIP);
   parts[1] = parts[0];
   parts[1].name = "right";
   parts[1].data = part1;
   exri_test_write_options_from_flags(&parts[1].options, EXRI_WRITE_COMPRESSION_RLE);

   bytes = NULL;
   bad_bytes = NULL;
   len = 0;
   version = 0;
   version_flags = 0;
   if (!exri_writef_multipart_to_memory(&bytes, &len, parts, 2))
      return 0;
   if (!exri_version_from_memory(bytes, len, &version, &version_flags) ||
       version != 2 ||
       (version_flags & EXRI_VERSION_FLAG_MULTIPART) == 0) {
      exri_image_free(bytes);
      return 0;
   }
   if (!exri_part_count_from_memory(bytes, len, &part_count) || part_count != 2) {
      exri_image_free(bytes);
      return 0;
   }
   pixels = exri_loadf_from_memory(bytes, len, &x, &y, &comp, 4);
   if (pixels != NULL) {
      exri_image_free(pixels);
      exri_image_free(bytes);
      return 0;
   }

   for (part = 0; part < 2; ++part) {
      x = y = comp = 0;
      pixels = exri_loadf_part_from_memory(bytes, len, part, &x, &y, &comp, 4);
      if (pixels == NULL || x != 3 || y != 2 || comp != 4) {
         if (pixels)
            exri_image_free(pixels);
         exri_image_free(bytes);
         return 0;
      }
      for (i = 0; i < 3 * 2 * 4; ++i) {
         if (!nearf(pixels[i], arrays[part][i])) {
            exri_image_free(pixels);
            exri_image_free(bytes);
            return 0;
         }
      }
      exri_image_free(pixels);
   }

   if (!exri_writef_multipart("/tmp/exri_write_multipart.exr", parts, 2)) {
      exri_image_free(bytes);
      return 0;
   }
   if (!exri_part_count("/tmp/exri_write_multipart.exr", &part_count) || part_count != 2) {
      exri_image_free(bytes);
      return 0;
   }
   if (!exri_version("/tmp/exri_write_multipart.exr", &version, &version_flags) ||
       version != 2 ||
       (version_flags & EXRI_VERSION_FLAG_MULTIPART) == 0) {
      exri_image_free(bytes);
      return 0;
   }
   pixels = NULL;
   x = y = comp = 0;
   if (!exri_loadf_part_to(&pixels, "/tmp/exri_write_multipart.exr", 1, &x, &y, &comp, 4) || pixels == NULL || x != 3 || y != 2 || comp != 4) {
      if (pixels)
         exri_image_free(pixels);
      exri_image_free(bytes);
      return 0;
   }
   for (i = 0; i < 3 * 2 * 4; ++i) {
      if (!nearf(pixels[i], part1[i])) {
         exri_image_free(pixels);
         exri_image_free(bytes);
         return 0;
      }
   }
   exri_image_free(pixels);

   parts[1].name = "left";
   if (exri_writef_multipart_to_memory(&bad_bytes, &len, parts, 2)) {
      exri_image_free(bad_bytes);
      exri_image_free(bytes);
      return 0;
   }

   exri_image_free(bytes);
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
      EXRI_WRITE_COMPRESSION_NONE | EXRI_WRITE_STORAGE_HALF,
      EXRI_WRITE_COMPRESSION_RLE | EXRI_WRITE_STORAGE_HALF,
      EXRI_WRITE_COMPRESSION_ZIPS | EXRI_WRITE_STORAGE_HALF,
      EXRI_WRITE_COMPRESSION_ZIP | EXRI_WRITE_STORAGE_HALF,
      EXRI_WRITE_COMPRESSION_PIZ | EXRI_WRITE_STORAGE_HALF,
      EXRI_WRITE_COMPRESSION_PXR24 | EXRI_WRITE_STORAGE_HALF
   };
   int comp;
   int ci;

   for (comp = 1; comp <= 4; ++comp) {
      for (ci = 0; ci < (int) (sizeof(compressions) / sizeof(compressions[0])); ++ci) {
         if (!check_memory_roundtrip(comp, compressions[ci]))
            return 1;
      }
   }

   if (!check_callbacks())
      return 1;
   if (!check_rle_fallback())
      return 1;
   if (!check_rle_smaller())
      return 1;
   if (!check_zip_blocks())
      return 1;
   if (!check_b44_blocks())
      return 1;
   if (!check_tiled_writes())
      return 1;
   if (!check_region_loads())
      return 1;
   if (!check_named_channel_writes())
      return 1;
   if (!check_uint_writes())
      return 1;
   if (!check_internal_rle_packets())
      return 1;
   if (!check_file())
      return 1;
   if (!check_multipart_writes())
      return 1;
   if (!check_invalid())
      return 1;

   printf("write regression ok\n");
   return 0;
}
