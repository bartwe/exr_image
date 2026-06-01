# Release Checks

`scripts/check-release.sh` is the local release gate. It builds the standalone
implementation include and every C test with GCC and Clang. The C build uses
the intentionally strict C89 warning set, `-Wconversion`, `-Wsign-conversion`,
ASan, and UBSan.

`exr_image.h` itself has no external runtime dependency. The checked-in release
gate does not require third-party source, third-party headers, zlib, or fixture
corpora.

The script can also run the EXR corpus probe across external fixture directories
listed in `EXRI_CORPUS_DIRS`. Expected non-image/color-metadata failures are
filtered by filename; any new failure is treated as a release failure.

Useful switches:

- `EXRI_BUILD_DIR=/path` changes the build directory.
- `EXRI_CORPUS_DIRS="/path/a:/path/b"` probes external EXR fixture
  directories.
- `EXRI_SKIP_CORPUS=1` skips the full test-data sweep.
- `EXRI_SKIP_FUZZ=1` skips fuzzer harness builds.
- `ASAN_OPTIONS=...` overrides the default `detect_leaks=0`. LeakSanitizer is
  disabled by default because some WSL/sandbox runners block the process
  inspection it uses; the dedicated allocation-count tests still run.

`scripts/build-fuzzers.sh` builds `tests/fuzz_exr_image.c` as both a standalone
seed runner and a Clang libFuzzer target. Set `EXRI_FUZZ_SEED_DIRS` to a
colon-separated directory list to run the standalone runner over external seed
directories.

`scripts/check-openexr-reference.sh` is an optional differential gate against
the Academy Software Foundation OpenEXR implementation. It expects an external
OpenEXR checkout via `OPENEXR_SOURCE_DIR` (default `/tmp/openexr`) and, when
available, an external Imath checkout via `IMATH_SOURCE_DIR` (default
`/tmp/imath`). The script builds OpenEXR into `/tmp`, builds
`tests/openexr_reference`, then compares OpenEXR fixture decode results against
`exr_image.h` with a small epsilon. For RGB fixtures that both libraries decode,
it also re-encodes the pixels through both writers and cross-decodes them.
`EXRI_OPENEXR_CORPUS_DIRS` chooses the fixture directories and
`EXRI_OPENEXR_EPSILON` changes the comparison tolerance.

`scripts/check-tinyexr-reference.sh` is an optional differential gate against
TinyEXR. It expects an external TinyEXR checkout via `TINYEXR_SOURCE_DIR`
(default `/tmp/tinyexr`) or a source archive via `TINYEXR_ARCHIVE` (default
`/tmp/tinyexr.tar.gz`). The script builds `tests/tinyexr_reference`, then
compares TinyEXR fixture decode results against `exr_image.h` with a small
epsilon. For fixtures that TinyEXR can decode, it also re-encodes through both
writers and cross-decodes them. `EXRI_TINYEXR_CORPUS_DIRS` chooses the fixture
directories and `EXRI_TINYEXR_EPSILON` changes the comparison tolerance.

`scripts/check-exrs-reference.sh` is an optional differential gate against the
Rust `exr` crate from the exrs repository. It expects an external checkout via
`EXRS_SOURCE_DIR` (default `/tmp/exrs`). The script builds
`tests/exrs_reference` with Cargo, compares exrs RGBA decode results against
`exr_image.h`, and cross-decodes files written by both writers for comparable
fixtures. `EXRI_EXRS_CORPUS_DIRS` chooses the fixture directories and
`EXRI_EXRS_EPSILON` changes the comparison tolerance.

`scripts/check-corpus-matrix.sh` is the heavyweight optional corpus matrix. It
builds one sorted union of all `.exr` fixtures from the external OpenEXR,
TinyEXR, and exrs checkouts, then runs the OpenEXR, TinyEXR, and exrs
differential gates against that exact same list. It also builds
`tests/corpus_roundtrip.c`, loads every fixture that `exr_image.h` can decode,
writes it back with every writer compression mode (`none`, `rle`, `zips`,
`zip`, `piz`, `pxr24`, `b44`, and `b44a`), and reads the bytes back with
`exr_image.h`. Lossless modes are compared tightly; PXR24 uses an explicit
lossy tolerance; B44/B44A are readability-only checks because their block
quantization is not pixel-preserving on arbitrary HDR inputs. Missing fixture
roots and reference mismatches are hard failures, not silent skips. Known
unsupported formats, such as DWAA/DWAB until implemented, therefore show up as
matrix failures in the relevant reference gate. `EXRI_MATRIX_BUILD_DIR`,
`EXRI_MATRIX_INPUT_LIST`,
`EXRI_MATRIX_EPSILON`, `EXRI_MATRIX_CC`, and `EXRI_MATRIX_SANITIZE=1` tune this
gate.

`scripts/check-msvc.cmd` is the Windows smoke gate. Run it from a Visual Studio
Developer Command Prompt after `vcvars64.bat`; it compiles the DLL/`__stdcall`
surface in C and C++ modes with `/W4 /WX`.

`.github/workflows/ci.yml` wires those checks into GitHub Actions: Linux runs
`scripts/check-release.sh` and creates the compact source package, while Windows
runs the MSVC smoke gate.

`scripts/make-source-release.sh` creates the source release archive documented
in `docs/release.md`.
