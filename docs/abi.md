# ABI Notes

`exr_image.h` exposes a small C ABI that is suitable for DLL exports and C#
P/Invoke:

- All public loaders return `int` status and write allocations through output
  pointers, for example `exri_loadf(&pixels, path, &w, &h, &comp, 4,
  EXRI_LOAD_DEFAULT)`.
- Pixel and deep-sample allocations are released with `exri_image_free()`.
- Failure details are returned by `exri_failure_reason()`.
- Public memory APIs accept `(unsigned char const *buffer, int len)`.
- Public streaming APIs use `exri_io_callbacks` and `exri_write_callbacks`;
  there is no public `FILE *` API.
- Filename helpers are available unless `EXRI_NO_STDIO` is defined.
- Public metadata APIs are part-aware. Use part index `0` for ordinary
  single-part files.

## Load Flags

`load_flags` is used by image loaders that return RGB-style pixels:

- `EXRI_LOAD_DEFAULT`: decode file values without color conversion.
- `EXRI_LOAD_SCRGB_STRICT`: require RGB data plus `chromaticities`, then convert
  declared primaries to linear sRGB/D65 floats.
- `EXRI_LOAD_SCRGB_ASSUME`: treat missing chromaticities as already linear
  sRGB/D65.

Layer, region, explicit-part, and tiled-level RGB-style loaders accept the same
scRGB flags. Raw channel-order loaders do not take color flags because they
return physical channels in file order.

## Write Options

Writers take `exri_write_options const *options`. Passing `NULL` uses defaults:
FLOAT scanline output with `EXRI_WRITE_COMPRESSION_NONE`.

Relevant fields:

- `compression`: one of `EXRI_WRITE_COMPRESSION_NONE`, `RLE`, `ZIPS`, `ZIP`,
  `PIZ`, `PXR24`, `B44`, or `B44A`.
- `pixel_type`: `EXRI_WRITE_PIXEL_FLOAT`, `EXRI_WRITE_PIXEL_HALF`, or
  `EXRI_WRITE_PIXEL_UINT`.
- `tiled`: nonzero writes tiled output.
- `tile_size`: square tile size, or `0` for the default.
- `level_mode`: `EXRI_WRITE_LEVEL_ONE`, `MIPMAP`, or `RIPMAP`.
- `level_rounding`: `EXRI_WRITE_ROUND_DOWN` or `EXRI_WRITE_ROUND_UP`.

Multipart writes use `exri_write_part.options` per part.

## Export Surface

The intended exported symbols are exactly the `EXRIDEF` declarations in the
public section of `exr_image.h`. Implementation-only compatibility helpers are
not declared and are not exported from DLL builds.
