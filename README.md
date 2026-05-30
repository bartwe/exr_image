# exr_image

`exr_image.h` is a standalone C header for loading and writing OpenEXR images.

The library is written in a conservative C89-compatible style, is usable from C
and C++, and exposes a small DLL-friendly C ABI for C# P/Invoke or other FFI
callers.

Status: pre-1.0 release candidate. The API is intended to be stable after the
first public `v0.1.0` tag, but downstream integration should still happen before
calling it final.

The header is intentionally large. It includes the parser, safety checks, image
and deep loaders, writer paths, tiled/multipart support, and the EXR compression
codecs needed for a dependency-free single-header integration.

## Quick Start

Put this in exactly one C or C++ translation unit:

```c
#define EXR_IMAGE_IMPLEMENTATION
#include "exr_image.h"
```

Then include `exr_image.h` normally everywhere else.

`exr_image.h` has no external runtime dependency. Build the implementation
translation unit like ordinary C:

```sh
cc -O2 your_file.c
```

## Load

```c
#define EXR_IMAGE_IMPLEMENTATION
#include "exr_image.h"

int load_rgba(char const *path)
{
   float *pixels;
   int w;
   int h;
   int comp;

   pixels = NULL;
   if (!exri_loadf(&pixels, path, &w, &h, &comp, 4, EXRI_LOAD_DEFAULT)) {
      /* exri_failure_reason() is a short static diagnostic string. */
      return 0;
   }

   /* pixels is w*h*4 float values. */

   exri_image_free(pixels);
   return 1;
}
```

For color-managed float output, use `EXRI_LOAD_SCRGB_STRICT` to require EXR
`chromaticities`, or `EXRI_LOAD_SCRGB_ASSUME` to treat missing metadata as
already linear sRGB/D65:

```c
if (!exri_loadf(&pixels, path, &w, &h, &comp, 4, EXRI_LOAD_SCRGB_STRICT)) {
   return 0;
}
```

## Write

```c
int write_zip_rgba(char const *path, int w, int h, float const *rgba)
{
   exri_write_options options;

   options.compression = EXRI_WRITE_COMPRESSION_ZIP;
   options.pixel_type = EXRI_WRITE_PIXEL_FLOAT;
   options.tiled = 0;
   options.tile_size = 0;
   options.level_mode = EXRI_WRITE_LEVEL_ONE;
   options.level_rounding = EXRI_WRITE_ROUND_DOWN;

   return exri_writef(path, w, h, 4, rgba, &options);
}
```

Passing `NULL` for `exri_write_options const *` writes FLOAT scanline EXR with
no compression.

## Feature Summary

Implemented baseline image workflows:

- Scanline and tiled decode to float pixels.
- NONE, RLE, ZIPS, ZIP, PIZ, PXR24, B44, and B44A compression.
- RGB/RGBA/Y/YA loading, named layers, explicit multipart part loading,
  regions, raw channel-order loading, tiled mip/rip level loading, and scRGB
  conversion for RGB-style loaders.
- Deep scanline loading for NONE, RLE, ZIPS, and ZIP deep blocks.
- Scanline, tiled, mipmap/ripmap, named-channel, custom-attribute, and multipart
  writing.
- FLOAT, HALF, and UINT channel output for supported writer modes.

Known non-goals for this release:

- ZFP decode/write.
- Deep tiled images.
- Progressive streaming decode.
- Decoding subsampled channels whose `xSampling` or `ySampling` is not `1`.

See [docs/feature_coverage.md](docs/feature_coverage.md) for the full coverage
matrix.

## ABI Notes

All public loaders return `int` status and write allocations through output
pointers. Free anything allocated by the library with `exri_image_free()`.

The public ABI uses `int`, `float`, pointer arguments, and plain structs. Define
`EXRI_DLL_EXPORT` when building a Windows DLL, `EXRI_DLL_IMPORT` when consuming
one, and `EXRI_STDCALL` if the exported functions and callbacks should use
`__stdcall`.

Define `EXRI_NO_STDIO` before including the header to remove filename helpers and
use memory/callback APIs only.

See [docs/abi.md](docs/abi.md) for more detail.

## Validation

The release gate is:

```sh
scripts/check-release.sh
```

It builds the header and tests with GCC and Clang using the strict C89 warning
set, runs ASan/UBSan-backed tests, and builds the fuzz harnesses.

On Windows, run this from a Visual Studio Developer Command Prompt:

```bat
scripts\check-msvc.cmd
```

See [docs/checks.md](docs/checks.md) for environment switches and fuzzing notes.

## License

`exr_image.h` is BSD-3-Clause. See [LICENSE](LICENSE) and
[THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).
