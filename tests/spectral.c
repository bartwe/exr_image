#define EXR_IMAGE_IMPLEMENTATION
#include "../exr_image.h"

#include <stdio.h>
#include <string.h>

typedef struct
{
   unsigned char data[2048];
   int len;
} byte_builder;

typedef struct
{
   unsigned char const *data;
   size_t len;
   size_t pos;
} memory_reader;

static int EXRI_CALLBACK read_cb(void *user, void *data, size_t size, size_t *bytes_read)
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

static int nearf(float a, float b)
{
   float d;

   d = a - b;
   if (d < 0.0f)
      d = -d;
   return d < 0.001f;
}

static int append_bytes(byte_builder *b, void const *data, int size)
{
   if (size < 0 || b->len > (int) sizeof(b->data) - size)
      return 0;
   memcpy(b->data + b->len, data, (size_t) size);
   b->len += size;
   return 1;
}

static int append_u8(byte_builder *b, int v)
{
   unsigned char c;

   c = (unsigned char) v;
   return append_bytes(b, &c, 1);
}

static int append_u32(byte_builder *b, unsigned int v)
{
   unsigned char p[4];

   p[0] = (unsigned char) (v & 255u);
   p[1] = (unsigned char) ((v >> 8) & 255u);
   p[2] = (unsigned char) ((v >> 16) & 255u);
   p[3] = (unsigned char) ((v >> 24) & 255u);
   return append_bytes(b, p, 4);
}

static int append_cstr(byte_builder *b, char const *s)
{
   return append_bytes(b, s, (int) strlen(s) + 1);
}

static int append_attr(byte_builder *b, char const *name, char const *type, unsigned char const *value, int value_size)
{
   return append_cstr(b, name) &&
          append_cstr(b, type) &&
          append_u32(b, (unsigned int) value_size) &&
          append_bytes(b, value, value_size);
}

static int append_channel(byte_builder *b, char const *name)
{
   return append_cstr(b, name) &&
          append_u32(b, 2u) &&
          append_u32(b, 0u) &&
          append_u32(b, 1u) &&
          append_u32(b, 1u);
}

static int make_spectral_header(byte_builder *b, int include_layout, int reflective)
{
   byte_builder chlist;
   unsigned char one_byte[1];
   unsigned char box[16];
   char const *units;
   char const *layout;
   int i;

   b->len = 0;
   chlist.len = 0;
   for (i = 0; i < (int) sizeof(box); ++i)
      box[i] = 0;
   one_byte[0] = 0;
   units = "W.m^-2.sr^-1.nm^-1";
   layout = "1.0";

   if (reflective) {
      if (!append_channel(&chlist, "T.610,500000nm") ||
          !append_channel(&chlist, "T.550,000000nm"))
         return 0;
   } else {
      if (!append_channel(&chlist, "S0.610,500000nm") ||
          !append_channel(&chlist, "S1.550,000000nm") ||
          !append_channel(&chlist, "S0.550,000000nm"))
         return 0;
   }
   if (!append_u8(&chlist, 0))
      return 0;

   if (!append_u32(b, 20000630u) ||
       !append_u32(b, 2u) ||
       !append_attr(b, "channels", "chlist", chlist.data, chlist.len) ||
       !append_attr(b, "compression", "compression", one_byte, 1) ||
       !append_attr(b, "dataWindow", "box2i", box, 16) ||
       !append_attr(b, "displayWindow", "box2i", box, 16) ||
       !append_attr(b, "lineOrder", "lineOrder", one_byte, 1))
      return 0;

   if (include_layout) {
      if (!append_attr(b, "spectralLayoutVersion", "string", (unsigned char const *) layout, (int) strlen(layout) + 1))
         return 0;
   }

   if (reflective) {
      if (!append_attr(b, "ROOT/units", "string", (unsigned char const *) units, (int) strlen(units) + 1))
         return 0;
   } else {
      if (!append_attr(b, "emissiveUnits", "string", (unsigned char const *) units, (int) strlen(units) + 1))
         return 0;
   }

   return append_u8(b, 0);
}

static int write_file(char const *filename, unsigned char const *data, int len)
{
   FILE *f;
   size_t written;

   f = fopen(filename, "wb");
   if (f == NULL)
      return 0;
   written = fwrite(data, 1, (size_t) len, f);
   if (fclose(f) != 0)
      return 0;
   return written == (size_t) len;
}

int main(void)
{
   byte_builder header;
   byte_builder no_layout;
   exri_io_callbacks cb;
   memory_reader reader;
   char name[64];
   char small[8];
   char units[64];
   float wavelengths[2];
   int count;
   int spectrum_type;
   int units_len;
   char const *path;

   if (!exri_format_wavelength(name, (int) sizeof(name), 550.0f))
      return 1;
   if (strcmp(name, "550,000000") != 0) {
      fprintf(stderr, "bad wavelength format: %s\n", name);
      return 1;
   }

   if (!exri_make_emissive_spectral_channel_name(name, (int) sizeof(name), 550.0f, 0))
      return 1;
   if (strcmp(name, "S0.550,000000nm") != 0) {
      fprintf(stderr, "bad emissive channel: %s\n", name);
      return 1;
   }
   if (!exri_is_spectral_channel_name(name) || !nearf(exri_parse_spectral_wavelength(name), 550.0f))
      return 1;

   if (!exri_make_reflective_spectral_channel_name(name, (int) sizeof(name), 610.5f))
      return 1;
   if (strcmp(name, "T.610,500000nm") != 0) {
      fprintf(stderr, "bad reflective channel: %s\n", name);
      return 1;
   }
   if (!exri_is_spectral_channel_name(name) || !nearf(exri_parse_spectral_wavelength(name), 610.5f))
      return 1;

   if (exri_is_spectral_channel_name("S4.550,000000nm"))
      return 1;
   if (exri_parse_spectral_wavelength("R") >= 0.0f)
      return 1;
   if (exri_make_emissive_spectral_channel_name(name, (int) sizeof(name), 550.0f, 4))
      return 1;
   if (exri_make_reflective_spectral_channel_name(small, (int) sizeof(small), 610.5f))
      return 1;
   if (small[sizeof(small) - 1] != 0)
      return 1;

   if (exri_spectral_stokes_component("S2.550,000000nm") != 2)
      return 1;
   if (exri_spectral_stokes_component("T.550,000000nm") != -1)
      return 1;

   if (!make_spectral_header(&header, 1, 0)) {
      fprintf(stderr, "failed to build spectral header\n");
      return 1;
   }
   if (!make_spectral_header(&no_layout, 0, 0)) {
      fprintf(stderr, "failed to build non-spectral header\n");
      return 1;
   }

   if (!exri_is_spectral_from_memory(header.data, (size_t) header.len)) {
      fprintf(stderr, "spectral detection failed: %s\n", exri_failure_reason());
      return 1;
   }
   if (exri_is_spectral_from_memory(no_layout.data, (size_t) no_layout.len)) {
      fprintf(stderr, "non-spectral header was accepted\n");
      return 1;
   }

   spectrum_type = -1;
   if (!exri_spectrum_type_from_memory(header.data, (size_t) header.len, &spectrum_type) ||
       spectrum_type != EXRI_SPECTRUM_POLARISED) {
      fprintf(stderr, "bad spectrum type: %d reason=%s\n", spectrum_type, exri_failure_reason());
      return 1;
   }

   count = -1;
   if (!exri_spectral_wavelengths_from_memory(header.data, (size_t) header.len, NULL, 0, &count) || count != 2) {
      fprintf(stderr, "bad wavelength count: %d reason=%s\n", count, exri_failure_reason());
      return 1;
   }
   if (exri_spectral_wavelengths_from_memory(header.data, (size_t) header.len, wavelengths, 1, &count)) {
      fprintf(stderr, "too-small wavelength buffer accepted\n");
      return 1;
   }
   if (count != 2) {
      fprintf(stderr, "too-small wavelength count not reported: %d\n", count);
      return 1;
   }
   if (!exri_spectral_wavelengths_from_memory(header.data, (size_t) header.len, wavelengths, 2, &count) ||
       count != 2 ||
       !nearf(wavelengths[0], 550.0f) ||
       !nearf(wavelengths[1], 610.5f)) {
      fprintf(stderr, "bad wavelengths: count=%d first=%g second=%g reason=%s\n", count, wavelengths[0], wavelengths[1], exri_failure_reason());
      return 1;
   }

   units_len = exri_spectral_units_from_memory(header.data, (size_t) header.len, NULL, 0);
   if (units_len != (int) strlen("W.m^-2.sr^-1.nm^-1")) {
      fprintf(stderr, "bad units length: %d reason=%s\n", units_len, exri_failure_reason());
      return 1;
   }
   if (!exri_spectral_units_from_memory(header.data, (size_t) header.len, units, (int) sizeof(units)) ||
       strcmp(units, "W.m^-2.sr^-1.nm^-1") != 0) {
      fprintf(stderr, "bad units string: %s reason=%s\n", units, exri_failure_reason());
      return 1;
   }
   strcpy(units, "stale");
   if (exri_spectral_units_from_memory(header.data, (size_t) header.len, units, 4) || units[0] != 0) {
      fprintf(stderr, "too-small units buffer accepted or left stale data\n");
      return 1;
   }

   if (!make_spectral_header(&header, 1, 1)) {
      fprintf(stderr, "failed to build reflective header\n");
      return 1;
   }
   spectrum_type = -1;
   if (!exri_spectrum_type_from_memory(header.data, (size_t) header.len, &spectrum_type) ||
       spectrum_type != EXRI_SPECTRUM_REFLECTIVE) {
      fprintf(stderr, "bad reflective spectrum type: %d reason=%s\n", spectrum_type, exri_failure_reason());
      return 1;
   }
   if (!exri_spectral_units_from_memory(header.data, (size_t) header.len, units, (int) sizeof(units)) ||
       strcmp(units, "W.m^-2.sr^-1.nm^-1") != 0) {
      fprintf(stderr, "bad reflective units string: %s reason=%s\n", units, exri_failure_reason());
      return 1;
   }

   cb.read = read_cb;
   cb.skip = NULL;
   cb.eof = NULL;
   reader.data = header.data;
   reader.len = (size_t) header.len;
   reader.pos = 0;
   if (!exri_is_spectral_from_callbacks(&cb, &reader)) {
      fprintf(stderr, "callback spectral detection failed: %s\n", exri_failure_reason());
      return 1;
   }
   reader.pos = 0;
   spectrum_type = -1;
   if (!exri_spectrum_type_from_callbacks(&cb, &reader, &spectrum_type) ||
       spectrum_type != EXRI_SPECTRUM_REFLECTIVE) {
      fprintf(stderr, "bad callback spectrum type: %d reason=%s\n", spectrum_type, exri_failure_reason());
      return 1;
   }
   reader.pos = 0;
   if (!exri_spectral_wavelengths_from_callbacks(&cb, &reader, wavelengths, 2, &count) ||
       count != 2 ||
       !nearf(wavelengths[0], 550.0f) ||
       !nearf(wavelengths[1], 610.5f)) {
      fprintf(stderr, "bad callback wavelengths: count=%d reason=%s\n", count, exri_failure_reason());
      return 1;
   }
   reader.pos = 0;
   if (!exri_spectral_units_from_callbacks(&cb, &reader, units, (int) sizeof(units)) ||
       strcmp(units, "W.m^-2.sr^-1.nm^-1") != 0) {
      fprintf(stderr, "bad callback units string: %s reason=%s\n", units, exri_failure_reason());
      return 1;
   }

   path = "/tmp/exri_spectral_header.exr";
   if (!write_file(path, header.data, header.len)) {
      fprintf(stderr, "failed to write spectral header fixture\n");
      return 1;
   }
   if (!exri_is_spectral(path)) {
      fprintf(stderr, "file spectral detection failed: %s\n", exri_failure_reason());
      return 1;
   }
   spectrum_type = -1;
   if (!exri_spectrum_type(path, &spectrum_type) ||
       spectrum_type != EXRI_SPECTRUM_REFLECTIVE) {
      fprintf(stderr, "bad file spectrum type: %d reason=%s\n", spectrum_type, exri_failure_reason());
      return 1;
   }
   if (!exri_spectral_wavelengths(path, wavelengths, 2, &count) ||
       count != 2 ||
       !nearf(wavelengths[0], 550.0f) ||
       !nearf(wavelengths[1], 610.5f)) {
      fprintf(stderr, "bad file wavelengths: count=%d reason=%s\n", count, exri_failure_reason());
      return 1;
   }
   if (!exri_spectral_units(path, units, (int) sizeof(units)) ||
       strcmp(units, "W.m^-2.sr^-1.nm^-1") != 0) {
      fprintf(stderr, "bad file units string: %s reason=%s\n", units, exri_failure_reason());
      return 1;
   }

   printf("spectral helper regression ok\n");
   return 0;
}
