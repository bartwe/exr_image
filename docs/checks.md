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

`scripts/check-msvc.cmd` is the Windows smoke gate. Run it from a Visual Studio
Developer Command Prompt after `vcvars64.bat`; it compiles the DLL/`__stdcall`
surface in C and C++ modes with `/W4 /WX`.

`.github/workflows/ci.yml` wires those checks into GitHub Actions: Linux runs
`scripts/check-release.sh` and creates the compact source package, while Windows
runs the MSVC smoke gate.

`scripts/make-source-release.sh` creates the source release archive documented
in `docs/release.md`.
