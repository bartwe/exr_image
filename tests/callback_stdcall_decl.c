#define EXRI_NO_STDIO
#define EXRI_STDCALL
#include "../exr_image.h"

static int EXRI_CALLBACK callback_stdcall_read(void *user, void *data, size_t size, size_t *bytes_read)
{
   (void) user;
   (void) data;
   *bytes_read = size;
   return 1;
}

int main(void)
{
   exri_io_callbacks cb;

   cb.read = callback_stdcall_read;
   cb.skip = 0;
   cb.eof = 0;

   return cb.read == 0;
}
