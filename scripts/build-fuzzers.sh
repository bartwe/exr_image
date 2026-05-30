#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR=$(cd "$(dirname "$0")/.." && pwd)
BUILD_DIR=${EXRI_FUZZ_BUILD_DIR:-/tmp/exri-fuzz}
CC_CLANG=${CC_CLANG:-clang}

CFLAGS_FUZZ=(
   -std=c89
   -Wall
   -Wextra
   -Wconversion
   -Wsign-conversion
   -Werror
   -O1
   -g
   -fsanitize=address,undefined
   -fno-omit-frame-pointer
)

CFLAGS_LIBFUZZER=(
   -std=c89
   -Wall
   -Wextra
   -Wconversion
   -Wsign-conversion
   -Werror
   -O1
   -g
   -fsanitize=fuzzer,address,undefined
   -fno-omit-frame-pointer
)

ASAN_OPTIONS=${ASAN_OPTIONS:-detect_leaks=0}
UBSAN_OPTIONS=${UBSAN_OPTIONS:-halt_on_error=1}
export ASAN_OPTIONS UBSAN_OPTIONS

cd "$ROOT_DIR"
mkdir -p "$BUILD_DIR"

if ! command -v "$CC_CLANG" >/dev/null 2>&1; then
   echo "missing tool: $CC_CLANG" >&2
   exit 1
fi

"$CC_CLANG" "${CFLAGS_FUZZ[@]}" -DEXRI_FUZZ_STANDALONE=1 -I. tests/fuzz_exr_image.c -o "$BUILD_DIR/fuzz_exr_image_standalone"

if [ "${EXRI_SKIP_LIBFUZZER:-0}" != "1" ]; then
   "$CC_CLANG" "${CFLAGS_LIBFUZZER[@]}" -I. tests/fuzz_exr_image.c -o "$BUILD_DIR/fuzz_exr_image"
fi

if [ "${EXRI_SKIP_FUZZ_SEEDS:-0}" != "1" ] && [ -n "${EXRI_FUZZ_SEED_DIRS:-}" ]; then
   SEEDS=$BUILD_DIR/fuzz_seed_inputs.txt
   seed_dirs=()
   seed_dir=
   old_ifs=$IFS

   IFS=:
   for seed_dir in ${EXRI_FUZZ_SEED_DIRS:-}; do
      IFS=$old_ifs
      if [ -z "$seed_dir" ]; then
         continue
      fi
      if [ ! -d "$seed_dir" ]; then
         echo "missing EXRI_FUZZ_SEED_DIRS entry: $seed_dir" >&2
         exit 1
      fi
      seed_dirs+=("$seed_dir")
      IFS=:
   done
   IFS=$old_ifs

   if [ "${#seed_dirs[@]}" -eq 0 ]; then
      echo "EXRI_FUZZ_SEED_DIRS did not contain any directories" >&2
      exit 1
   fi

   find "${seed_dirs[@]}" -type f | sort > "$SEEDS"
   if [ -s "$SEEDS" ]; then
      while IFS= read -r seed; do
         "$BUILD_DIR/fuzz_exr_image_standalone" "$seed"
      done < "$SEEDS"
   fi
fi

echo "fuzz harnesses built in $BUILD_DIR"
