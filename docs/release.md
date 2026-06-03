# Release Process

This project is not ready for a `1.0` API promise until it has seen downstream
use, but it is ready for a source-based `v0.1.0-rc2` release candidate.

## Source Checklist

Before tagging a public release:

1. Run `scripts/check-release.sh` on Linux.
2. Run `scripts/check-msvc.cmd` from a Visual Studio Developer Command Prompt.
3. Run a longer fuzzing pass with `scripts/build-fuzzers.sh`, then the generated
   libFuzzer binary.
4. Build at least one downstream DLL/P/Invoke integration.
5. Review `docs/feature_coverage.md`, `docs/abi.md`, and `README.md` for stale
   claims.
6. Confirm `VERSION` matches the intended tag.
7. Prepare the GitHub release title and description from the changes since the
   previous tag. Include highlights, known non-goals, validation, and asset
   names.

## Source Package

Create a compact source archive with:

```sh
scripts/make-source-release.sh
```

By default this creates:

- `dist/exr_image.h`
- `dist/exr_image-<VERSION>.tar.gz`
- `dist/exr_image-<VERSION>.zip`, when `zip` is available
- `dist/SHA256SUMS`, when `sha256sum` is available

The package includes:

- `exr_image.h`
- project docs and license files
- test and check scripts

It excludes third-party source, headers, and fixture corpora. Full external
corpus validation should run from the repository checkout before the package is
created.

## Tagging

Use the semantic release name as the tag, for example:

```sh
git tag -a v0.1.0-rc2 -m "exr_image.h v0.1.0-rc2"
```

Attach the generated header, source archives, and checksum file. Avoid attaching
the full fixture corpora unless there is a specific reason to publish a separate
test-data archive.
