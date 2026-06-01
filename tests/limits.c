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

static int reason_is(char const *expected)
{
   return strcmp(exri_failure_reason(), expected) == 0;
}

static int probe_default_policy_caps(void)
{
   size_t expected_pixels;

   if (EXRI_MAX_INPUT_SIZE != (size_t) -1) {
      fprintf(stderr, "unexpected default input cap\n");
      return 0;
   }
   if (EXRI_MAX_OUTPUT_SIZE != (size_t) -1) {
      fprintf(stderr, "unexpected default output cap\n");
      return 0;
   }
   expected_pixels = ((size_t) -1) / (4u * sizeof(float));
   if (EXRI_MAX_PIXELS != expected_pixels) {
      fprintf(stderr, "unexpected default pixel cap\n");
      return 0;
   }
   if (sizeof(size_t) > 4u) {
      size_t dimension_pixels;

      dimension_pixels = (size_t) EXRI_MAX_DIMENSIONS * (size_t) EXRI_MAX_DIMENSIONS;
      if ((size_t) EXRI_MAX_PIXELS < dimension_pixels) {
         fprintf(stderr, "default pixel cap is below dimension cap\n");
         return 0;
      }
   }
   return 1;
}

static int probe_append_input_limit(void)
{
   exri_uc *buffer;
   exri_uc byte;
   size_t length;
   size_t capacity;

   buffer = NULL;
   byte = 0;
   length = EXRI_MAX_INPUT_SIZE;
   capacity = EXRI_MAX_INPUT_SIZE;

   if (exri__append(&buffer, &length, &capacity, &byte, 1)) {
      fprintf(stderr, "oversize appended input was accepted\n");
      return 0;
   }
   if (!reason_is("input too large")) {
      fprintf(stderr, "oversize appended input reason: %s\n", exri_failure_reason());
      return 0;
   }
   return 1;
}

static int probe_append_output_limit(void)
{
   exri_uc *bytes;
   exri_uc byte;
   size_t capacity;
   size_t len;

   bytes = NULL;
   byte = 0;
   len = EXRI_MAX_OUTPUT_SIZE;
   capacity = EXRI_MAX_OUTPUT_SIZE;

   if (exri__append_output(&bytes, &len, &capacity, &byte, 1)) {
      fprintf(stderr, "oversize appended output was accepted\n");
      return 0;
   }
   if (!reason_is("output too large")) {
      fprintf(stderr, "oversize appended output reason: %s\n", exri_failure_reason());
      return 0;
   }
   return 1;
}

static int probe_size_offset_limit(void)
{
   unsigned char data[8];
   size_t value;
   size_t size_bytes;

   put32(data, 0x7fffffffu);
   put32(data + 4, 0u);
   value = 0;
   if (!exri__get64le_as_size_at(data, &value) || value != (size_t) 0x7fffffff) {
      fprintf(stderr, "31-bit offset was rejected\n");
      return 0;
   }

   put32(data, 0x80000000u);
   put32(data + 4, 0u);
   if (!exri__get64le_as_size_at(data, &value) || value != (size_t) 0x80000000u) {
      fprintf(stderr, "32-bit offset was rejected\n");
      return 0;
   }

   put32(data, 0u);
   put32(data + 4, 1u);
   size_bytes = sizeof(size_t);
   if (size_bytes >= 8u) {
      if (!exri__get64le_as_size_at(data, &value) || value <= (size_t) 0xffffffffu) {
         fprintf(stderr, "64-bit offset was rejected\n");
         return 0;
      }
   } else {
      if (exri__get64le_as_size_at(data, &value)) {
         fprintf(stderr, "unrepresentable 64-bit offset was accepted\n");
         return 0;
      }
   }

   return 1;
}

int main(void)
{
   if (!probe_default_policy_caps())
      return 1;
   if (!probe_append_input_limit())
      return 1;
   if (!probe_append_output_limit())
      return 1;
   if (!probe_size_offset_limit())
      return 1;

   printf("limits regression ok\n");
   return 0;
}
