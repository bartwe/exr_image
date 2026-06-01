#if !defined(EXRI_CALLBACK)
#if defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
#define EXRI_CALLBACK __attribute__((ms_abi))
#endif
#endif

#define EXR_IMAGE_IMPLEMENTATION
#include "../exr_image.h"

typedef struct
{
   unsigned char const *data;
   size_t len;
   size_t pos;
} callback_abi_reader;

static int EXRI_CALLBACK callback_abi_read(void *user, void *data, size_t size, size_t *bytes_read)
{
   callback_abi_reader *reader;
   unsigned char *dst;
   size_t n;
   size_t i;

   reader = (callback_abi_reader *) user;
   dst = (unsigned char *) data;
   n = reader->len - reader->pos;
   if (n > size)
      n = size;

   for (i = 0; i < n; ++i)
      dst[i] = reader->data[reader->pos + i];
   reader->pos += n;
   *bytes_read = n;
   return 1;
}

static int EXRI_CALLBACK callback_abi_write(void *user, void const *data, size_t size)
{
   size_t *written;

   (void) data;
   written = (size_t *) user;
   *written += size;
   return 1;
}

int main(void)
{
   unsigned char data[8];
   callback_abi_reader reader;
   exri_io_callbacks cb;
   exri_write_callbacks wcb;
   exri_write_options options;
   float pixel[4];
   size_t written;

   data[0] = 0x76;
   data[1] = 0x2f;
   data[2] = 0x31;
   data[3] = 0x01;
   data[4] = 0x02;
   data[5] = 0x00;
   data[6] = 0x00;
   data[7] = 0x00;

   reader.data = data;
   reader.len = sizeof(data);
   reader.pos = 0;

   cb.read = callback_abi_read;
   cb.skip = NULL;
   cb.eof = NULL;

   pixel[0] = 1.0f;
   pixel[1] = 0.5f;
   pixel[2] = 0.25f;
   pixel[3] = 1.0f;
   options.compression = EXRI_WRITE_COMPRESSION_NONE;
   options.pixel_type = EXRI_WRITE_PIXEL_FLOAT;
   options.tiled = 0;
   options.tile_size = 0;
   options.level_mode = EXRI_WRITE_LEVEL_ONE;
   options.level_rounding = EXRI_WRITE_ROUND_DOWN;
   written = 0;
   wcb.write = callback_abi_write;
   if (!exri_writef_to_callbacks(&wcb, &written, 1, 1, 4, pixel, &options))
      return 1;
   if (written <= 0)
      return 1;

   return exri_is_exr_from_callbacks(&cb, &reader) ? 1 : 0;
}
