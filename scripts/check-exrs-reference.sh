#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR=$(cd "$(dirname "$0")/.." && pwd)
EXRS_SOURCE_DIR=${EXRS_SOURCE_DIR:-/tmp/exrs}
HARNESS_BUILD_DIR=${EXRI_EXRS_HARNESS_BUILD_DIR:-/tmp/exri-exrs-reference-build}
HARNESS_PROJECT_DIR="$HARNESS_BUILD_DIR/project"
INPUT_LIST=${EXRI_EXRS_INPUT_LIST:-/tmp/exri-exrs-reference-inputs.txt}
EPSILON=${EXRI_EXRS_EPSILON:-0.0001}

if [ ! -f "$EXRS_SOURCE_DIR/Cargo.toml" ]; then
   echo "missing exrs source: $EXRS_SOURCE_DIR/Cargo.toml" >&2
   echo "clone https://github.com/johannesvollmer/exrs there or set EXRS_SOURCE_DIR" >&2
   exit 2
fi

rm -rf "$HARNESS_PROJECT_DIR"
mkdir -p "$HARNESS_PROJECT_DIR"
cp "$ROOT_DIR/tests/exrs_reference/Cargo.toml" "$HARNESS_PROJECT_DIR/Cargo.toml"
cp "$ROOT_DIR/tests/exrs_reference/build.rs" "$HARNESS_PROJECT_DIR/build.rs"
cp "$ROOT_DIR/tests/exrs_reference/exri_impl.c" "$HARNESS_PROJECT_DIR/exri_impl.c"
mkdir -p "$HARNESS_PROJECT_DIR/src"
cp "$ROOT_DIR/tests/exrs_reference/src/main.rs" "$HARNESS_PROJECT_DIR/src/main.rs"
ln -s "$EXRS_SOURCE_DIR" "$HARNESS_PROJECT_DIR/exrs-source"

echo "building exr_image/exrs reference harness"
EXRI_ROOT_DIR="$ROOT_DIR" cargo build \
   --manifest-path "$HARNESS_PROJECT_DIR/Cargo.toml" \
   --release

if [ "$#" -gt 0 ]; then
   EXRI_ROOT_DIR="$ROOT_DIR" cargo run \
      --manifest-path "$HARNESS_PROJECT_DIR/Cargo.toml" \
      --release \
      -- --epsilon "$EPSILON" "$@"
else
   corpus_dirs=${EXRI_EXRS_CORPUS_DIRS:-$EXRS_SOURCE_DIR/tests/images}
   old_ifs=$IFS
   IFS=:
   # shellcheck disable=SC2086
   set -- $corpus_dirs
   IFS=$old_ifs
   find "$@" -type f -name '*.exr' | sort > "$INPUT_LIST"
   if [ ! -s "$INPUT_LIST" ]; then
      echo "no .exr files found in EXRI_EXRS_CORPUS_DIRS" >&2
      exit 2
   fi
   EXRI_ROOT_DIR="$ROOT_DIR" cargo run \
      --manifest-path "$HARNESS_PROJECT_DIR/Cargo.toml" \
      --release \
      -- --epsilon "$EPSILON" --list "$INPUT_LIST"
fi
