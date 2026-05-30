# EXR Feature Coverage

This tracks the intended feature baseline for common OpenEXR image workflows.

## Implemented

- Single-header include/implementation split.
- C89-oriented implementation, C++ compatible declarations, custom allocator
  hooks, `EXRI_NO_STDIO`, `EXR_IMAGE_STATIC`, DLL export/import hooks, optional
  `EXRI_STDCALL`, and callback calling-convention hooks.
- Header detection, version flags, image info, part info, part channel metadata,
  part attributes, layer enumeration, tiled level metadata, spectral metadata,
  and failure strings.
- Scanline and tiled image decode to float pixels for NONE, RLE, ZIPS, ZIP, PIZ,
  PXR24, B44, and B44A compression.
- RGB/RGBA/Y/YA default loading, explicit layer loading, region loading,
  arbitrary channel-order loading, multipart part loading, and selected tiled
  mip/rip level loading.
- scRGB float output on RGB-style full-image, explicit-part, layer, region, and
  tiled-level loaders via `EXRI_LOAD_SCRGB_STRICT` or
  `EXRI_LOAD_SCRGB_ASSUME`.
- Deep scanline loading for single-part and multipart parts with NONE, RLE,
  ZIPS, and ZIP deep blocks.
- Scanline, tiled, mipmap/ripmap, named-channel, custom-attribute, and multipart
  writing. Writer compression supports NONE, RLE, ZIPS, ZIP, PIZ, PXR24, B44,
  and B44A through `exri_write_options`.
- FLOAT, HALF, UINT channel output where the EXR compression mode supports it.
- Strict validation of malformed headers, bad chunk tables, unsupported storage,
  corrupt compressed payloads, out-of-bounds regions, and unsupported flags.

## Not Implemented

- ZFP decode/write.
- Deep tiled images.
- Subsampled-channel pixel decode for channels whose `xSampling` or `ySampling`
  is not `1`; metadata is exposed, but image/deep loading fails explicitly.
- Raw requested-pixel-type load arrays. Public load output is float; storage
  types are exposed through metadata and writers can emit FLOAT/HALF/UINT.
- Progressive streaming decode.
- scRGB conversion for raw channel-order loaders. Those APIs return physical
  channels in file order and intentionally do not take color flags.

## Decode Contract

`exri_info*` and metadata queries parse headers without decoding pixel blocks.
`exri_loadf*` is strict: unsupported compression, unsupported storage,
multipart access without an explicit part, deep/non-image input, missing default
visible channels, subsampled channels, corrupt blocks, and unsupported load
flags fail and set `exri_failure_reason()`.

Part-aware APIs report one part for ordinary files and the declared count for
multipart files. Use part index `0` for single-part files.

scRGB loaders preserve high dynamic range float values. Strict mode converts
declared `chromaticities` to linear sRGB/D65 and fails when metadata is missing;
assume mode treats missing chromaticities as already linear sRGB/D65.

## Release Checks

The full local gate is documented in `docs/checks.md` and implemented by
`scripts/check-release.sh`.

The checked-in source tests cover load/write roundtrips, safety/bounds/leak
regressions, scRGB conversion, spectral helpers, callback ABI behavior,
`EXRI_NO_STDIO` builds, writer failure paths, fuzzer harness builds, and optional
external corpus probing.
