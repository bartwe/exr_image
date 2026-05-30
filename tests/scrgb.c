#define EXR_IMAGE_IMPLEMENTATION
#include "../exr_image.h"

#include <stdio.h>
#include <string.h>

static void put32(unsigned char *p, unsigned int v)
{
   p[0] = (unsigned char) (v & 255u);
   p[1] = (unsigned char) ((v >> 8) & 255u);
   p[2] = (unsigned char) ((v >> 16) & 255u);
   p[3] = (unsigned char) ((v >> 24) & 255u);
}

static void put64(unsigned char *p, unsigned int lo, unsigned int hi)
{
   put32(p, lo);
   put32(p + 4, hi);
}

static void putf(unsigned char *p, float f)
{
   unsigned int u;

   memcpy(&u, &f, sizeof(u));
   put32(p, u);
}

static void put_string(unsigned char *b, int *pp, char const *s)
{
   int p;

   p = *pp;
   while (*s)
      b[p++] = (unsigned char) *s++;
   b[p++] = 0;
   *pp = p;
}

static void add_chlist_rgb(unsigned char *b, int *pp)
{
   int p;
   int start;
   int i;
   char const *names[3];

   names[0] = "R";
   names[1] = "G";
   names[2] = "B";

   p = *pp;
   put_string(b, &p, "channels");
   put_string(b, &p, "chlist");
   start = p;
   put32(b + p, 0u); p += 4;

   for (i = 0; i < 3; ++i) {
      put_string(b, &p, names[i]);
      put32(b + p, 2u); p += 4;
      put32(b + p, 0u); p += 4;
      put32(b + p, 1u); p += 4;
      put32(b + p, 1u); p += 4;
   }
   b[p++] = 0;

   put32(b + start, (unsigned int) (p - start - 4));
   *pp = p;
}

static void add_compression(unsigned char *b, int *pp)
{
   int p;

   p = *pp;
   put_string(b, &p, "compression");
   put_string(b, &p, "compression");
   put32(b + p, 1u); p += 4;
   b[p++] = 0;
   *pp = p;
}

static void add_data_window(unsigned char *b, int *pp)
{
   int p;

   p = *pp;
   put_string(b, &p, "dataWindow");
   put_string(b, &p, "box2i");
   put32(b + p, 16u); p += 4;
   put32(b + p, 0u); p += 4;
   put32(b + p, 0u); p += 4;
   put32(b + p, 0u); p += 4;
   put32(b + p, 0u); p += 4;
   *pp = p;
}

static void add_chromaticities(unsigned char *b, int *pp, float const c[8])
{
   int p;
   int i;

   p = *pp;
   put_string(b, &p, "chromaticities");
   put_string(b, &p, "chromaticities");
   put32(b + p, 32u); p += 4;
   for (i = 0; i < 8; ++i) {
      putf(b + p, c[i]);
      p += 4;
   }
   *pp = p;
}

static int make_rgb_exr(unsigned char *b, int add_chroma, float const chroma[8], float r, float g, float bl)
{
   int p;
   int header_end;
   int chunk_offset;

   p = 0;
   b[p++] = 0x76; b[p++] = 0x2f; b[p++] = 0x31; b[p++] = 0x01;
   put32(b + p, 2u); p += 4;
   add_chlist_rgb(b, &p);
   add_compression(b, &p);
   add_data_window(b, &p);
   if (add_chroma)
      add_chromaticities(b, &p, chroma);
   b[p++] = 0;

   header_end = p;
   chunk_offset = header_end + 8;
   put64(b + p, (unsigned int) chunk_offset, 0u); p += 8;
   put32(b + p, 0u); p += 4;
   put32(b + p, 12u); p += 4;
   putf(b + p, r); p += 4;
   putf(b + p, g); p += 4;
   putf(b + p, bl); p += 4;
   return p;
}

static int nearf(float a, float b)
{
   float d;

   d = a - b;
   if (d < 0.0f)
      d = -d;
   return d < 0.0001f;
}

static int probe_missing_requires_flag(void)
{
   unsigned char b[1024];
   int len;
   int x;
   int y;
   int c;
   float sentinel;
   float *pixels;

   len = make_rgb_exr(b, 0, NULL, 1.0f, 0.5f, 0.25f);
   pixels = &sentinel;
   if (exri_loadf_from_memory(&pixels, b, len, &x, &y, &c, 3, EXRI_LOAD_SCRGB_STRICT))
      return 0;
   if (pixels) {
      exri_image_free(pixels);
      return 0;
   }

   pixels = NULL;
   if (!exri_loadf_from_memory(&pixels, b, len, &x, &y, &c, 3, EXRI_LOAD_SCRGB_ASSUME))
      return 0;
   if (!nearf(pixels[0], 1.0f) || !nearf(pixels[1], 0.5f) || !nearf(pixels[2], 0.25f)) {
      exri_image_free(pixels);
      return 0;
   }
   exri_image_free(pixels);
   return 1;
}

static int probe_srgb_unchanged(void)
{
   static const float srgb[8] = { 0.6400f, 0.3300f, 0.3000f, 0.6000f, 0.1500f, 0.0600f, 0.3127f, 0.3290f };
   unsigned char b[1024];
   int len;
   int x;
   int y;
   int c;
   float *pixels;

   len = make_rgb_exr(b, 1, srgb, 2.0f, -0.5f, 0.25f);
   pixels = NULL;
   if (!exri_loadf_from_memory(&pixels, b, len, &x, &y, &c, 3, EXRI_LOAD_SCRGB_STRICT))
      return 0;
   if (x != 1 || y != 1 || c != 3 || !nearf(pixels[0], 2.0f) || !nearf(pixels[1], -0.5f) || !nearf(pixels[2], 0.25f)) {
      exri_image_free(pixels);
      return 0;
   }
   exri_image_free(pixels);
   return 1;
}

static int probe_primary_conversion(void)
{
   static const float swapped[8] = { 0.1500f, 0.0600f, 0.3000f, 0.6000f, 0.6400f, 0.3300f, 0.3127f, 0.3290f };
   unsigned char b[1024];
   int len;
   int x;
   int y;
   int c;
   float *pixels;

   len = make_rgb_exr(b, 1, swapped, 1.0f, 2.0f, 3.0f);
   pixels = NULL;
   if (!exri_loadf_from_memory(&pixels, b, len, &x, &y, &c, 3, EXRI_LOAD_SCRGB_STRICT))
      return 0;
   if (!nearf(pixels[0], 3.0f) || !nearf(pixels[1], 2.0f) || !nearf(pixels[2], 1.0f)) {
      exri_image_free(pixels);
      return 0;
   }
   exri_image_free(pixels);
   return 1;
}

static int probe_region_conversion(void)
{
   static const float swapped[8] = { 0.1500f, 0.0600f, 0.3000f, 0.6000f, 0.6400f, 0.3300f, 0.3127f, 0.3290f };
   unsigned char b[1024];
   int len;
   int x;
   int y;
   int c;
   float *pixels;

   len = make_rgb_exr(b, 1, swapped, 1.0f, 2.0f, 3.0f);
   pixels = NULL;
   if (!exri_loadf_region_from_memory(&pixels, b, len, 0, 0, 1, 1, &x, &y, &c, 3, EXRI_LOAD_SCRGB_STRICT))
      return 0;
   if (x != 1 || y != 1 || c != 3 || !nearf(pixels[0], 3.0f) || !nearf(pixels[1], 2.0f) || !nearf(pixels[2], 1.0f)) {
      exri_image_free(pixels);
      return 0;
   }
   exri_image_free(pixels);
   return 1;
}

static int probe_bad_options(void)
{
   static const float srgb[8] = { 0.6400f, 0.3300f, 0.3000f, 0.6000f, 0.1500f, 0.0600f, 0.3127f, 0.3290f };
   unsigned char b[1024];
   int len;
   int x;
   int y;
   int c;
   float *pixels;

   len = make_rgb_exr(b, 1, srgb, 1.0f, 1.0f, 1.0f);
   pixels = NULL;
   (void) exri_loadf_from_memory(&pixels, b, len, &x, &y, &c, 1, EXRI_LOAD_SCRGB_STRICT);
   if (pixels) {
      exri_image_free(pixels);
      return 0;
   }
   pixels = NULL;
   (void) exri_loadf_from_memory(&pixels, b, len, &x, &y, &c, 3, 99);
   if (pixels) {
      exri_image_free(pixels);
      return 0;
   }
   return 1;
}

int main(void)
{
   if (!probe_missing_requires_flag() ||
       !probe_srgb_unchanged() ||
       !probe_primary_conversion() ||
       !probe_region_conversion() ||
       !probe_bad_options()) {
      printf("scrgb regression failed: %s\n", exri_failure_reason());
      return 1;
   }

   printf("scrgb regression ok\n");
   return 0;
}
