# Public API Freeze Notes

The intended exported C ABI is the set of `EXRIDEF` declarations in the public
section of `exr_image.h`.

Use this command to inspect that surface before a release:

```sh
scripts/list-public-api.sh
```

API policy for `v0.1.x`:

- Public names use the `exri_` function prefix and `EXRI_` macro/enum prefix.
- All public allocations returned by the library are freed with
  `exri_image_free()`.
- All public loaders and writers return `int` status. On failure,
  `exri_failure_reason()` returns a short diagnostic string.
- Memory APIs use `(exri_uc const *buffer, int len)` rather than `size_t` so the
  ABI stays simple for DLL and C# callers.
- Filename helpers are optional and are removed by `EXRI_NO_STDIO`.
- Callback APIs use `EXRI_CALLBACK` so `EXRI_STDCALL` can apply to callbacks and
  exported functions consistently.
- Structs are plain C data with no embedded ownership.

Do not add compatibility overloads before the first public release. If a name is
wrong, rename it before tagging.
