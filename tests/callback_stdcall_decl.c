#define EXRI_NO_STDIO
#define EXRI_STDCALL
#include "../exr_image.h"

static int EXRI_CALLBACK callback_stdcall_read(void *user, char *data, int size)
{
   (void) user;
   (void) data;
   return size;
}

int main(void)
{
   exri_io_callbacks cb;

   cb.read = callback_stdcall_read;
   cb.skip = 0;
   cb.eof = 0;

   return cb.read == 0;
}
