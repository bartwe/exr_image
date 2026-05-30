# Contributing

Thanks for improving `exr_image.h`.

## Project Goals

- Keep the library dependency-free and single-header.
- Preserve conservative C89-compatible implementation code.
- Keep the public ABI friendly to C, C++, DLL exports, and C# P/Invoke.
- Fail explicitly for unsupported EXR features rather than silently producing
  partial or incorrect output.
- Keep the repository source-only. Do not commit third-party source, headers,
  generated release archives, analyzer reports, or fixture corpora.

## Development

Put implementation changes in `exr_image.h`. The public API is the declarations
in the non-implementation section of that header; update `docs/public_api.md`
when public names or contracts change.

Tests live in `tests/`. Prefer focused regression tests for parser safety,
allocation failure, roundtrips, color handling, and ABI behavior.

Useful commands:

```sh
scripts/check-release.sh
scripts/make-source-release.sh
```

On Windows, run this from a Visual Studio Developer Command Prompt:

```bat
scripts\check-msvc.cmd
```

The full release gate builds with GCC and Clang using strict C89 warning flags,
runs ASan/UBSan-backed tests, and builds the fuzz harnesses.

External EXR fixture corpora are optional local validation inputs. Use
`EXRI_CORPUS_DIRS=/path/a:/path/b scripts/check-release.sh` to include them.
Do not commit those files.

## Pull Requests

Before opening a pull request:

- Run `scripts/check-release.sh`.
- Run `scripts\check-msvc.cmd` when the change affects declarations, calling
  conventions, Windows builds, or the DLL-friendly API.
- Update docs for public API, feature coverage, release process, or behavior
  changes.
- Keep unrelated formatting, generated artifacts, and fixture files out of the
  commit.

## Security

Do not open a public issue with an actively exploitable vulnerability or
proof-of-concept input. Follow [SECURITY.md](SECURITY.md) instead.
