#define EXR_IMAGE_IMPLEMENTATION
#include "../exr_image.h"

#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
   int i;
   int x;
   int y;
   int comp;
   int total_samples;
   int layers;
   int layer_index;
   int layer_loaded;
   int parts;
   int part_index;
   int multipart_loaded;
   int multipart_invalid;
   int name_len;
   int loaded;
   int allowed_not_loaded;
   char layer_name[256];
   char const *load_reason;
   char const *part_reason;
   char const *deep_reason;
   float *pixels;
   float *samples;
   int *offsets;

   loaded = 0;

   for (i = 1; i < argc; ++i) {
      x = y = comp = 0;
      pixels = NULL;
      if (exri_loadf(&pixels, argv[i], &x, &y, &comp, 4, EXRI_LOAD_SCRGB_ASSUME)) {
         if (x <= 0 || y <= 0 || comp <= 0) {
            fprintf(stderr, "invalid load info: %s: %d %d %d\n", argv[i], x, y, comp);
            exri_image_free(pixels);
            return 1;
         }
         printf("loaded %s: %dx%d comp=%d\n", argv[i], x, y, comp);
         exri_image_free(pixels);
         loaded += 1;
      } else {
         load_reason = exri_failure_reason();
         layer_loaded = 0;
         layers = 0;
         if (exri_layer_count(argv[i], &layers) && layers > 0) {
            for (layer_index = 0; layer_index < layers; ++layer_index) {
               name_len = exri_layer_name(argv[i], layer_index, layer_name, (int) sizeof(layer_name));
               if (name_len <= 0) {
                  fprintf(stderr, "invalid layer info: %s: layer=%d reason=%s\n", argv[i], layer_index, exri_failure_reason());
                  return 1;
               }
               x = y = comp = 0;
               pixels = NULL;
               if (!exri_loadf_layer(&pixels, argv[i], layer_name, &x, &y, &comp, 4, EXRI_LOAD_SCRGB_ASSUME)) {
                  fprintf(stderr, "layer not-loaded %s layer=%s: %s\n", argv[i], layer_name, exri_failure_reason());
                  return 1;
               }
               if (x <= 0 || y <= 0 || comp <= 0) {
                  fprintf(stderr, "invalid layer load info: %s layer=%s: %d %d %d\n", argv[i], layer_name, x, y, comp);
                  exri_image_free(pixels);
                  return 1;
               }
               printf("layer-loaded %s layer=%s: %dx%d comp=%d\n", argv[i], layer_name, x, y, comp);
               exri_image_free(pixels);
            }
            layer_loaded = 1;
            loaded += 1;
         }
         if (layer_loaded)
            continue;

         multipart_loaded = 0;
         multipart_invalid = 0;
         parts = 0;
         if (exri_part_count(argv[i], &parts) && parts > 1) {
            for (part_index = 0; part_index < parts; ++part_index) {
               x = y = comp = 0;
               pixels = NULL;
               if (exri_loadf_part(&pixels, argv[i], part_index, &x, &y, &comp, 4, EXRI_LOAD_SCRGB_ASSUME)) {
                  if (x <= 0 || y <= 0 || comp <= 0 || pixels == NULL) {
                     fprintf(stderr, "invalid part load info: %s part=%d: %d %d %d\n", argv[i], part_index, x, y, comp);
                     exri_image_free(pixels);
                     return 1;
                  }
                  printf("part-loaded %s part=%d: %dx%d comp=%d\n", argv[i], part_index, x, y, comp);
                  exri_image_free(pixels);
                  continue;
               }

               part_reason = exri_failure_reason();
               samples = NULL;
               offsets = NULL;
               x = y = comp = total_samples = 0;
               if (exri_load_deep_part(&samples, &offsets, argv[i], part_index, &x, &y, &comp, &total_samples)) {
                  if (x <= 0 || y <= 0 || comp <= 0 || total_samples < 0 || offsets == NULL || (total_samples > 0 && samples == NULL)) {
                     fprintf(stderr, "invalid deep part load info: %s part=%d: %d %d %d samples=%d\n", argv[i], part_index, x, y, comp, total_samples);
                     exri_image_free(samples);
                     exri_image_free(offsets);
                     return 1;
                  }
                  printf("deep-part-loaded %s part=%d: %dx%d channels=%d samples=%d\n", argv[i], part_index, x, y, comp, total_samples);
                  exri_image_free(samples);
                  exri_image_free(offsets);
                  continue;
               }

               deep_reason = exri_failure_reason();
               if (strcmp(part_reason, "invalid EXR") == 0 && strcmp(deep_reason, "invalid EXR") == 0) {
                  printf("not-loaded %s: invalid EXR\n", argv[i]);
                  multipart_invalid = 1;
                  break;
               }
               fprintf(stderr, "part not-loaded %s part=%d: image=%s deep=%s\n", argv[i], part_index, part_reason, deep_reason);
               return 1;
            }
            if (!multipart_invalid) {
               multipart_loaded = 1;
               loaded += 1;
            }
         }
         if (multipart_invalid)
            continue;
         if (multipart_loaded)
            continue;

         samples = NULL;
         offsets = NULL;
         x = y = comp = total_samples = 0;
         if (exri_load_deep_part(&samples, &offsets, argv[i], 0, &x, &y, &comp, &total_samples)) {
            if (x <= 0 || y <= 0 || comp <= 0 || total_samples < 0 || offsets == NULL || (total_samples > 0 && samples == NULL)) {
               fprintf(stderr, "invalid deep load info: %s: %d %d %d samples=%d\n", argv[i], x, y, comp, total_samples);
               exri_image_free(samples);
               exri_image_free(offsets);
               return 1;
            }
            printf("deep-loaded %s: %dx%d channels=%d samples=%d\n", argv[i], x, y, comp, total_samples);
            exri_image_free(samples);
            exri_image_free(offsets);
            loaded += 1;
         } else {
            deep_reason = exri_failure_reason();
            if (strcmp(deep_reason, "not deep") == 0)
               deep_reason = load_reason;
            allowed_not_loaded = strcmp(deep_reason, "invalid EXR") == 0 ||
                                 strcmp(deep_reason, "scRGB requires RGB channels") == 0;
            if (!allowed_not_loaded) {
               fprintf(stderr, "not-loaded %s: %s\n", argv[i], deep_reason);
               return 1;
            }
            printf("not-loaded %s: %s\n", argv[i], deep_reason);
         }
      }
   }

   return loaded > 0 ? 0 : 1;
}
