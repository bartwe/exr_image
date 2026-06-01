# ABI Notes

`exr_image.h` exposes a small C ABI that is suitable for DLL exports and C#
P/Invoke:

- All public loaders return `int` status and write allocations through output
  pointers, for example `exri_loadf(&pixels, path, &w, &h, &comp, 4,
  EXRI_LOAD_DEFAULT)`.
- Pixel and deep-sample allocations are released with `exri_image_free()`.
- Failure details are returned by `exri_failure_reason()`.
- Public memory APIs accept `(unsigned char const *buffer, size_t len)`.
- Public streaming APIs use `exri_io_callbacks` and `exri_write_callbacks`.
  Read callbacks return status and report transferred bytes through
  `size_t *bytes_read`; write callbacks return status. There is no public
  `FILE *` API.
- Filename helpers are available unless `EXRI_NO_STDIO` is defined.
- Public metadata APIs are part-aware. Use part index `0` for ordinary
  single-part files.

## Size Limits

The public ABI uses `size_t` for memory byte counts, callback transfer sizes,
and in-memory writer output lengths. On 64-bit targets this allows inputs and
outputs larger than 2 GiB when allocation and file-system limits allow it. On
32-bit targets the native `size_t` limit remains the effective ceiling.

The default safety policy keeps byte-count caps near the native maximum and
relies on overflow checks plus allocation failures for the hard stop:

- `EXRI_MAX_INPUT_SIZE`: `(size_t)-1` for file, callback, and memory inputs.
- `EXRI_MAX_OUTPUT_SIZE`: `(size_t)-1` for memory/callback/file writer output.
- `EXRI_MAX_PIXELS`: `((size_t)-1) / (4 * sizeof(float))` decoded pixels or
  deep samples. For ordinary RGBA float output this is the largest pixel count
  whose final allocation size can be represented by `size_t`.
- `EXRI_MAX_DIMENSIONS`: 16,777,216 pixels per axis.

Define these macros before the implementation include to raise or lower the
policy caps for a build. The caps are checked before allocation growth and
before direct memory input parsing. Individual EXR attribute sizes, image
dimensions, channel counts, and decoded pixel counts still use bounded `int`
interfaces because those values are format or API scalar values rather than
whole-buffer byte counts.

On 64-bit builds, the default pixel cap is above the default dimension-product
cap, so the largest accepted ordinary image dimensions are governed by
`EXRI_MAX_DIMENSIONS` (`16,777,216 * 16,777,216`, or `2^48`, pixels). On 32-bit
builds, the native address space remains the practical cap.

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
