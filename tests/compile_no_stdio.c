#define EXRI_NO_STDIO
#define EXR_IMAGE_IMPLEMENTATION
#include "../exr_image.h"

int main(void)
{
   exri_uc data[8];
   exri_uc *out;
   exri_write_options options;
   float pixel[4];
   int out_len;

   data[0] = 0;
   data[1] = 0;
   data[2] = 0;
   data[3] = 0;
   data[4] = 0;
   data[5] = 0;
   data[6] = 0;
   data[7] = 0;

   pixel[0] = 1.0f;
   pixel[1] = 0.5f;
   pixel[2] = 0.25f;
   pixel[3] = 1.0f;
   out = NULL;
   out_len = 0;
   options.compression = EXRI_WRITE_COMPRESSION_RLE;
   options.pixel_type = EXRI_WRITE_PIXEL_FLOAT;
   options.tiled = 0;
   options.tile_size = 0;
   options.level_mode = EXRI_WRITE_LEVEL_ONE;
   options.level_rounding = EXRI_WRITE_ROUND_DOWN;
   if (!exri_writef_to_memory(&out, &out_len, 1, 1, 4, pixel, &options))
      return 1;
   exri_image_free(out);

   return exri_is_exr_from_memory(data, 8) ? 1 : 0;
}
