#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR=$(cd "$(dirname "$0")/.." && pwd)
TINYEXR_SOURCE_DIR=${TINYEXR_SOURCE_DIR:-/tmp/tinyexr}
TINYEXR_ARCHIVE=${TINYEXR_ARCHIVE:-/tmp/tinyexr.tar.gz}
TINYEXR_EXTRACT_DIR=${EXRI_TINYEXR_EXTRACT_DIR:-/tmp/exri-tinyexr-src}
HARNESS_BUILD_DIR=${EXRI_TINYEXR_HARNESS_BUILD_DIR:-/tmp/exri-tinyexr-reference-build}
INPUT_LIST=${EXRI_TINYEXR_INPUT_LIST:-/tmp/exri-tinyexr-reference-inputs.txt}
EPSILON=${EXRI_TINYEXR_EPSILON:-0.0001}
JOBS=${EXRI_JOBS:-}

if [ ! -d "$TINYEXR_SOURCE_DIR" ] && [ -d "$TINYEXR_EXTRACT_DIR" ]; then
   TINYEXR_SOURCE_DIR=$TINYEXR_EXTRACT_DIR
fi

if [ ! -d "$TINYEXR_SOURCE_DIR" ] && [ -f "$TINYEXR_ARCHIVE" ]; then
   echo "extracting TinyEXR source archive"
   rm -rf "$TINYEXR_EXTRACT_DIR"
   mkdir -p "$TINYEXR_EXTRACT_DIR"
   tar -xzf "$TINYEXR_ARCHIVE" -C "$TINYEXR_EXTRACT_DIR" --strip-components=1
   TINYEXR_SOURCE_DIR=$TINYEXR_EXTRACT_DIR
fi

if [ ! -f "$TINYEXR_SOURCE_DIR/tinyexr.h" ]; then
   echo "missing TinyEXR source: $TINYEXR_SOURCE_DIR/tinyexr.h" >&2
   echo "clone https://github.com/syoyo/tinyexr there, set TINYEXR_SOURCE_DIR, or set TINYEXR_ARCHIVE" >&2
   exit 2
fi

if [ ! -f "$TINYEXR_SOURCE_DIR/deps/miniz/miniz.c" ]; then
   echo "missing TinyEXR miniz dependency: $TINYEXR_SOURCE_DIR/deps/miniz/miniz.c" >&2
   echo "use a TinyEXR checkout/archive with submodules or vendored deps populated" >&2
   exit 2
fi

echo "configuring exr_image/TinyEXR reference harness"
cmake -S "$ROOT_DIR/tests/tinyexr_reference" \
   -B "$HARNESS_BUILD_DIR" \
   -DCMAKE_BUILD_TYPE=Release \
   -DEXRI_TINYEXR_SOURCE_DIR="$TINYEXR_SOURCE_DIR" \
   -DEXRI_ROOT_DIR="$ROOT_DIR"

echo "building exr_image/TinyEXR reference harness"
if [ -n "$JOBS" ]; then
   cmake --build "$HARNESS_BUILD_DIR" --parallel "$JOBS"
else
   cmake --build "$HARNESS_BUILD_DIR" --parallel
fi

if [ "$#" -gt 0 ]; then
   "$HARNESS_BUILD_DIR/exri_tinyexr_reference" --epsilon "$EPSILON" "$@"
else
   corpus_dirs=${EXRI_TINYEXR_CORPUS_DIRS:-$TINYEXR_SOURCE_DIR}
   old_ifs=$IFS
   IFS=:
   # shellcheck disable=SC2086
   set -- $corpus_dirs
   IFS=$old_ifs
   find "$@" -type f -name '*.exr' | sort > "$INPUT_LIST"
   if [ ! -s "$INPUT_LIST" ]; then
      echo "no .exr files found in EXRI_TINYEXR_CORPUS_DIRS" >&2
      exit 2
   fi
   "$HARNESS_BUILD_DIR/exri_tinyexr_reference" --epsilon "$EPSILON" --list "$INPUT_LIST"
fi
