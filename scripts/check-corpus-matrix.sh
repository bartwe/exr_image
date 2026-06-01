#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR=$(cd "$(dirname "$0")/.." && pwd)

OPENEXR_SOURCE_DIR=${OPENEXR_SOURCE_DIR:-/tmp/openexr}
TINYEXR_SOURCE_DIR=${TINYEXR_SOURCE_DIR:-/tmp/tinyexr}
TINYEXR_ARCHIVE=${TINYEXR_ARCHIVE:-/tmp/tinyexr.tar.gz}
TINYEXR_EXTRACT_DIR=${EXRI_TINYEXR_EXTRACT_DIR:-/tmp/exri-tinyexr-src}
EXRS_SOURCE_DIR=${EXRS_SOURCE_DIR:-/tmp/exrs}

OPENEXR_CORPUS_DIR=${EXRI_MATRIX_OPENEXR_CORPUS_DIR:-$OPENEXR_SOURCE_DIR}
TINYEXR_CORPUS_DIR=${EXRI_MATRIX_TINYEXR_CORPUS_DIR:-$TINYEXR_SOURCE_DIR}
EXRS_CORPUS_DIR=${EXRI_MATRIX_EXRS_CORPUS_DIR:-$EXRS_SOURCE_DIR/tests/images}

BUILD_DIR=${EXRI_MATRIX_BUILD_DIR:-/tmp/exri-corpus-matrix}
INPUT_LIST=${EXRI_MATRIX_INPUT_LIST:-$BUILD_DIR/corpus-inputs.txt}
ROUNDTRIP_CC=${EXRI_MATRIX_CC:-clang}
EPSILON=${EXRI_MATRIX_EPSILON:-0.0001}

require_dir()
{
   label=$1
   path=$2
   if [ ! -d "$path" ]; then
      echo "missing $label directory: $path" >&2
      exit 2
   fi
}

require_tool()
{
   if ! command -v "$1" >/dev/null 2>&1; then
      echo "missing tool: $1" >&2
      exit 2
   fi
}

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

if [ "$TINYEXR_CORPUS_DIR" = "/tmp/tinyexr" ] && [ "$TINYEXR_SOURCE_DIR" != "/tmp/tinyexr" ]; then
   TINYEXR_CORPUS_DIR=$TINYEXR_SOURCE_DIR
fi

if [ ! -f "$TINYEXR_SOURCE_DIR/tinyexr.h" ]; then
   echo "missing TinyEXR source: $TINYEXR_SOURCE_DIR/tinyexr.h" >&2
   echo "clone https://github.com/syoyo/tinyexr there, set TINYEXR_SOURCE_DIR, or set TINYEXR_ARCHIVE" >&2
   exit 2
fi

require_tool "$ROUNDTRIP_CC"
require_dir "OpenEXR corpus" "$OPENEXR_CORPUS_DIR"
require_dir "TinyEXR corpus" "$TINYEXR_CORPUS_DIR"
require_dir "exrs corpus" "$EXRS_CORPUS_DIR"

mkdir -p "$BUILD_DIR"
RAW_LIST=$BUILD_DIR/corpus-inputs.raw
: > "$RAW_LIST"

append_corpus()
{
   label=$1
   path=$2
   before=$(wc -l < "$RAW_LIST")
   find "$path" -type f -name '*.exr' >> "$RAW_LIST"
   after=$(wc -l < "$RAW_LIST")
   echo "$label .exr files: $((after - before))"
}

append_corpus "OpenEXR" "$OPENEXR_CORPUS_DIR"
append_corpus "TinyEXR" "$TINYEXR_CORPUS_DIR"
append_corpus "exrs" "$EXRS_CORPUS_DIR"

sort -u "$RAW_LIST" > "$INPUT_LIST"
TOTAL_CASES=$(wc -l < "$INPUT_LIST")
if [ "$TOTAL_CASES" -eq 0 ]; then
   echo "no .exr files found in matrix corpus directories" >&2
   exit 2
fi
echo "matrix corpus cases: $TOTAL_CASES"

ROUNDTRIP_EXE=$BUILD_DIR/exri_corpus_roundtrip
CFLAGS=(
   -std=c89
   -pedantic-errors
   -Wall
   -Wextra
   -Wconversion
   -Wsign-conversion
   -Werror
   -Wstrict-prototypes
   -Wold-style-definition
   -Wmissing-prototypes
   -Wmissing-declarations
   -Wshadow
   -Wundef
   -Wpointer-arith
   -Wcast-align
   -Wcast-qual
   -Wwrite-strings
   -Waggregate-return
   -Wnested-externs
   -Wredundant-decls
   -Wbad-function-cast
   -Wswitch-enum
   -Wformat=2
   -Winit-self
   -Wuninitialized
   -fno-common
   -fno-builtin
   -funsigned-char
   -O2
   -g
)

if "$ROUNDTRIP_CC" --version 2>/dev/null | grep -qi clang; then
   CFLAGS+=(-Wstrict-overflow=5)
else
   CFLAGS+=(-Wstrict-overflow=2)
fi

if [ "${EXRI_MATRIX_SANITIZE:-0}" = "1" ]; then
   CFLAGS+=(-fsanitize=undefined,address -fno-omit-frame-pointer)
   ASAN_OPTIONS=${ASAN_OPTIONS:-detect_leaks=0}
   UBSAN_OPTIONS=${UBSAN_OPTIONS:-halt_on_error=1}
   export ASAN_OPTIONS UBSAN_OPTIONS
fi

echo "building corpus roundtrip harness with $ROUNDTRIP_CC"
"$ROUNDTRIP_CC" "${CFLAGS[@]}" -I"$ROOT_DIR" "$ROOT_DIR/tests/corpus_roundtrip.c" -lm -o "$ROUNDTRIP_EXE"

failed=0
run_gate()
{
   gate_name=$1
   shift
   echo
   echo "== $gate_name =="
   if "$@"; then
      echo "$gate_name passed"
   else
      gate_status=$?
      echo "$gate_name failed with exit code $gate_status" >&2
      failed=1
   fi
}

run_gate "OpenEXR reference" env EXRI_OPENEXR_EPSILON="$EPSILON" bash "$ROOT_DIR/scripts/check-openexr-reference.sh" --list "$INPUT_LIST"
run_gate "TinyEXR reference" env EXRI_TINYEXR_EPSILON="$EPSILON" TINYEXR_SOURCE_DIR="$TINYEXR_SOURCE_DIR" bash "$ROOT_DIR/scripts/check-tinyexr-reference.sh" --list "$INPUT_LIST"
run_gate "exrs reference" env EXRI_EXRS_EPSILON="$EPSILON" bash "$ROOT_DIR/scripts/check-exrs-reference.sh" --list "$INPUT_LIST"
run_gate "exr_image writer roundtrip" "$ROUNDTRIP_EXE" --list "$INPUT_LIST"

if [ "$failed" -ne 0 ]; then
   echo
   echo "corpus matrix failed" >&2
   exit 1
fi

echo
echo "corpus matrix passed"
