#define EXR_IMAGE_IMPLEMENTATION
#include "../exr_image.h"

#include <stdio.h>

int main(int argc, char **argv)
{
   int i;

   for (i = 1; i < argc; ++i) {
      if (exri_is_exr(argv[i])) {
         fprintf(stderr, "non-EXR detected as EXR: %s\n", argv[i]);
         return 1;
      }
   }

   return 0;
}
