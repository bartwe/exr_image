#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int live_allocs;

static void *test_malloc(size_t n)
{
   void *p;

   if (n == 0)
      n = 1;
   p = malloc(n);
   if (p)
      live_allocs += 1;
   return p;
}

static void *test_realloc(void *p, size_t n)
{
   void *q;

   if (n == 0)
      n = 1;
   q = realloc(p, n);
   if (p == NULL && q != NULL)
      live_allocs += 1;
   return q;
}

static void test_free(void *p)
{
   if (p)
      live_allocs -= 1;
   free(p);
}

#define EXRI_MALLOC(sz) test_malloc(sz)
#define EXRI_REALLOC(p,n) test_realloc((p),(n))
#define EXRI_FREE(p) test_free(p)
#define EXR_IMAGE_IMPLEMENTATION
#include "../exr_image.h"
#include "exri_test_compat.h"

static void put32(unsigned char *p, unsigned int v)
{
   p[0] = (unsigned char) (v & 255u);
   p[1] = (unsigned char) ((v >> 8) & 255u);
   p[2] = (unsigned char) ((v >> 16) & 255u);
   p[3] = (unsigned char) ((v >> 24) & 255u);
}

static unsigned int get32(unsigned char const *p)
{
   return ((unsigned int) p[0]) |
          ((unsigned int) p[1] << 8) |
          ((unsigned int) p[2] << 16) |
          ((unsigned int) p[3] << 24);
}

static void add_chlist(unsigned char *b, int *pp)
{
   int p;

   p = *pp;
   b[p++] = 'c'; b[p++] = 'h'; b[p++] = 'a'; b[p++] = 'n';
   b[p++] = 'n'; b[p++] = 'e'; b[p++] = 'l'; b[p++] = 's'; b[p++] = 0;
   b[p++] = 'c'; b[p++] = 'h'; b[p++] = 'l'; b[p++] = 'i';
   b[p++] = 's'; b[p++] = 't'; b[p++] = 0;
   put32(b + p, 19u); p += 4;
   b[p++] = 'R'; b[p++] = 0;
   put32(b + p, 1u); p += 4;
   put32(b + p, 0u); p += 4;
   put32(b + p, 1u); p += 4;
   put32(b + p, 1u); p += 4;
   b[p++] = 0;
   *pp = p;
}

static void add_chlist_sampling(unsigned char *b, int *pp, unsigned int xs, unsigned int ys)
{
   int p;

   p = *pp;
   b[p++] = 'c'; b[p++] = 'h'; b[p++] = 'a'; b[p++] = 'n';
   b[p++] = 'n'; b[p++] = 'e'; b[p++] = 'l'; b[p++] = 's'; b[p++] = 0;
   b[p++] = 'c'; b[p++] = 'h'; b[p++] = 'l'; b[p++] = 'i';
   b[p++] = 's'; b[p++] = 't'; b[p++] = 0;
   put32(b + p, 19u); p += 4;
   b[p++] = 'R'; b[p++] = 0;
   put32(b + p, 1u); p += 4;
   put32(b + p, 0u); p += 4;
   put32(b + p, xs); p += 4;
   put32(b + p, ys); p += 4;
   b[p++] = 0;
   *pp = p;
}

static void add_compression_value(unsigned char *b, int *pp, unsigned char compression)
{
   int p;

   p = *pp;
   b[p++] = 'c'; b[p++] = 'o'; b[p++] = 'm'; b[p++] = 'p';
   b[p++] = 'r'; b[p++] = 'e'; b[p++] = 's'; b[p++] = 's';
   b[p++] = 'i'; b[p++] = 'o'; b[p++] = 'n'; b[p++] = 0;
   b[p++] = 'c'; b[p++] = 'o'; b[p++] = 'm'; b[p++] = 'p';
   b[p++] = 'r'; b[p++] = 'e'; b[p++] = 's'; b[p++] = 's';
   b[p++] = 'i'; b[p++] = 'o'; b[p++] = 'n'; b[p++] = 0;
   put32(b + p, 1u); p += 4;
   b[p++] = compression;
   *pp = p;
}

static void add_compression(unsigned char *b, int *pp)
{
   add_compression_value(b, pp, 0);
}

static void add_bad_compression_type(unsigned char *b, int *pp)
{
   int p;

   p = *pp;
   b[p++] = 'c'; b[p++] = 'o'; b[p++] = 'm'; b[p++] = 'p';
   b[p++] = 'r'; b[p++] = 'e'; b[p++] = 's'; b[p++] = 's';
   b[p++] = 'i'; b[p++] = 'o'; b[p++] = 'n'; b[p++] = 0;
   b[p++] = 'i'; b[p++] = 'n'; b[p++] = 't'; b[p++] = 0;
   put32(b + p, 4u); p += 4;
   put32(b + p, 0u); p += 4;
   *pp = p;
}

static void add_data_window(unsigned char *b, int *pp, unsigned int minx, unsigned int miny, unsigned int maxx, unsigned int maxy)
{
   int p;

   p = *pp;
   b[p++] = 'd'; b[p++] = 'a'; b[p++] = 't'; b[p++] = 'a';
   b[p++] = 'W'; b[p++] = 'i'; b[p++] = 'n'; b[p++] = 'd';
   b[p++] = 'o'; b[p++] = 'w'; b[p++] = 0;
   b[p++] = 'b'; b[p++] = 'o'; b[p++] = 'x'; b[p++] = '2';
   b[p++] = 'i'; b[p++] = 0;
   put32(b + p, 16u); p += 4;
   put32(b + p, minx); p += 4;
   put32(b + p, miny); p += 4;
   put32(b + p, maxx); p += 4;
   put32(b + p, maxy); p += 4;
   *pp = p;
}

static int bad_read(void *user, void *data, size_t size, size_t *bytes_read)
{
   (void) user;
   (void) data;
   if (size > 0)
      *bytes_read = size + 1u;
   else
      *bytes_read = 0;
   return 1;
}

static int probe_data_window(void)
{
   unsigned char b[160];
   int p;
   int x;
   int y;
   int c;

   p = 0;
   b[p++] = 0x76; b[p++] = 0x2f; b[p++] = 0x31; b[p++] = 0x01;
   put32(b + p, 2u); p += 4;
   add_chlist(b, &p);
   add_compression(b, &p);
   add_data_window(b, &p, 0x80000000u, 0u, 0x7fffffffu, 0u);
   b[p++] = 0;
   return exri_info_from_memory(b, (size_t) p, &x, &y, &c) == 0;
}

static int probe_sampling_range(void)
{
   unsigned char b[160];
   int p;
   int x;
   int y;
   int c;

   p = 0;
   b[p++] = 0x76; b[p++] = 0x2f; b[p++] = 0x31; b[p++] = 0x01;
   put32(b + p, 2u); p += 4;
   add_chlist_sampling(b, &p, 0xffffffffu, 1u);
   add_compression(b, &p);
   add_data_window(b, &p, 0u, 0u, 0u, 0u);
   b[p++] = 0;
   return exri_info_from_memory(b, (size_t) p, &x, &y, &c) == 0;
}

static int probe_known_attribute_type_mismatch(void)
{
   unsigned char b[200];
   int p;
   int x;
   int y;
   int c;

   p = 0;
   b[p++] = 0x76; b[p++] = 0x2f; b[p++] = 0x31; b[p++] = 0x01;
   put32(b + p, 2u); p += 4;
   add_chlist(b, &p);
   add_bad_compression_type(b, &p);
   add_compression(b, &p);
   add_data_window(b, &p, 0u, 0u, 0u, 0u);
   b[p++] = 0;
   return exri_info_from_memory(b, (size_t) p, &x, &y, &c) == 0;
}

static int probe_zlib_adler(void)
{
   unsigned char out[1];
   unsigned char bad_stream[] = { 0x78, 0x01, 0x01, 0x01, 0x00, 0xfe, 0xff, 0x41, 0x00, 0x42, 0x00, 0x43 };

   return exri__zlib_decode_buffer(out, 1, bad_stream, (int) sizeof(bad_stream)) < 0;
}

static int probe_callback_count(void)
{
   exri_io_callbacks cb;

   cb.read = bad_read;
   cb.skip = NULL;
   cb.eof = NULL;
   return exri_is_exr_from_callbacks(&cb, NULL) == 0;
}

static int probe_duplicate_channels(void)
{
   unsigned char b[256];
   int p;
   int x;
   int y;
   int c;
   int before;
   float *pixels;

   p = 0;
   b[p++] = 0x76; b[p++] = 0x2f; b[p++] = 0x31; b[p++] = 0x01;
   put32(b + p, 2u); p += 4;
   add_chlist(b, &p);
   add_chlist(b, &p);
   add_compression(b, &p);
   add_data_window(b, &p, 0u, 0u, 0u, 0u);
   b[p++] = 0;

   before = live_allocs;
   pixels = exri_loadf_from_memory(b, p, &x, &y, &c, 0);
   if (pixels)
      exri_image_free(pixels);
   return pixels == NULL && live_allocs == before;
}

static int probe_output_pointer_clear(void)
{
   unsigned char bad[8];
   int x;
   int y;
   int c;
   float sentinel;
   float *pixels;

   memset(bad, 0, sizeof(bad));
   pixels = &sentinel;
   if (exri_loadf_from_memory_to(&pixels, bad, (int) sizeof(bad), &x, &y, &c, 4))
      return 0;
   return pixels == NULL;
}

static int probe_duplicate_chunk(void)
{
   exri_uc *buf;
   int len;
   exri__info info;
   exri__channel *channels;
   float source[2 * 2 * 4];
   unsigned int off0lo;
   unsigned int off0hi;
   float *pixels;
   int x;
   int y;
   int c;
   int i;

   for (i = 0; i < (int) (sizeof(source) / sizeof(source[0])); ++i)
      source[i] = (float) i * 0.25f;

   buf = NULL;
   len = 0;
   if (!exri_writef_to_memory(&buf, &len, 2, 2, 4, source, EXRI_WRITE_COMPRESSION_NONE))
      return 0;

   channels = NULL;
   if (!exri__parse_header(buf, (size_t) len, &info, &channels)) {
      exri_image_free(buf);
      return 0;
   }
   EXRI_FREE(channels);

   off0lo = get32(buf + info.header_end);
   off0hi = get32(buf + info.header_end + 4);
   put32(buf + info.header_end + 8, off0lo);
   put32(buf + info.header_end + 12, off0hi);

   pixels = exri_loadf_from_memory(buf, len, &x, &y, &c, 4);
   if (pixels)
      exri_image_free(pixels);
   exri_image_free(buf);
   return pixels == NULL;
}

static int probe_two_byte_rle_packet(void)
{
   unsigned char b[256];
   int p;
   int header_end;
   int chunk_offset;
   int x;
   int y;
   int c;
   float *pixels;

   p = 0;
   b[p++] = 0x76; b[p++] = 0x2f; b[p++] = 0x31; b[p++] = 0x01;
   put32(b + p, 2u); p += 4;
   add_chlist(b, &p);
   add_compression_value(b, &p, 1);
   add_data_window(b, &p, 0u, 0u, 1u, 0u);
   b[p++] = 0;

   header_end = p;
   chunk_offset = header_end + 8;
   put32(b + p, (unsigned int) chunk_offset); p += 4;
   put32(b + p, 0u); p += 4;
   put32(b + p, 0u); p += 4;
   put32(b + p, 2u); p += 4;
   b[p++] = 3;
   b[p++] = 128;

   pixels = exri_loadf_from_memory(b, p, &x, &y, &c, 1);
   if (pixels == NULL)
      return 0;
   exri_image_free(pixels);

   return x == 2 && y == 1 && c == 1;
}

static int probe_piz_trailing_payload(void)
{
   exri_uc *buf;
   unsigned char *mutated;
   int len;
   exri__info info;
   exri__channel *channels;
   float source[8 * 33 * 4];
   int block_lines;
   int num_blocks;
   int i;
   size_t chunk_offset;
   size_t max_offset;
   int data_len;
   int x;
   int y;
   int c;
   float *pixels;

   for (i = 0; i < (int) (sizeof(source) / sizeof(source[0])); ++i)
      source[i] = (float) ((i % 31) - 12) * 0.125f;

   buf = NULL;
   len = 0;
   if (!exri_writef_to_memory(&buf, &len, 8, 33, 4, source, EXRI_WRITE_COMPRESSION_PIZ))
      return 0;

   channels = NULL;
   if (!exri__parse_header(buf, (size_t) len, &info, &channels)) {
      exri_image_free(buf);
      return 0;
   }
   EXRI_FREE(channels);
   if (info.compression != 4) {
      exri_image_free(buf);
      return 0;
   }
   if (info.height <= 0) {
      exri_image_free(buf);
      return 0;
   }

   block_lines = 32;
   num_blocks = (int) (((size_t) info.height + (size_t) block_lines - 1u) / (size_t) block_lines);
   max_offset = 0;
   for (i = 0; i < num_blocks; ++i) {
      if (!exri__get64le_as_size_at(buf + info.header_end + i * 8, &chunk_offset)) {
         exri_image_free(buf);
         return 0;
      }
      if (chunk_offset > max_offset)
         max_offset = chunk_offset;
   }

   if (!exri__has_file_bytes_at(max_offset, (size_t) len, 8)) {
      exri_image_free(buf);
      return 0;
   }
   data_len = exri__i32_from_u32(exri__get32le_at(buf + max_offset + 4));
   if (data_len <= 0 || data_len == INT_MAX ||
       (size_t) max_offset + 8u + (size_t) data_len != (size_t) len) {
      exri_image_free(buf);
      return 0;
   }

   mutated = (unsigned char *) malloc((size_t) len + 1u);
   if (mutated == NULL) {
      exri_image_free(buf);
      return 0;
   }
   memcpy(mutated, buf, (size_t) len);
   mutated[len] = 0;
   put32(mutated + max_offset + 4, (unsigned int) (data_len + 1));

   pixels = exri_loadf_from_memory(mutated, len + 1, &x, &y, &c, 4);
   if (pixels)
      exri_image_free(pixels);

   free(mutated);
   exri_image_free(buf);
   return pixels == NULL;
}

int main(void)
{
   int ok;

   ok = 1;
   ok = ok && probe_data_window();
   ok = ok && probe_sampling_range();
   ok = ok && probe_known_attribute_type_mismatch();
   ok = ok && probe_zlib_adler();
   ok = ok && probe_callback_count();
   ok = ok && probe_duplicate_channels();
   ok = ok && probe_output_pointer_clear();
   ok = ok && probe_duplicate_chunk();
   ok = ok && probe_two_byte_rle_packet();
   ok = ok && probe_piz_trailing_payload();
   ok = ok && (live_allocs == 0);

   if (!ok) {
      printf("safety regression failed, live_allocs=%d, reason=%s\n", live_allocs, exri_failure_reason());
      return 1;
   }

   printf("safety regression ok\n");
   return 0;
}
