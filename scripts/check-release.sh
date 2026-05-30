#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR=$(cd "$(dirname "$0")/.." && pwd)
BUILD_DIR=${EXRI_BUILD_DIR:-/tmp/exri-check}
CC_CLANG=${CC_CLANG:-clang}
CC_GCC=${CC_GCC:-gcc}

C_TESTS=(
   smoke
   bounds
   leak
   safety
   negative
   scrgb
   spectral
   callback_abi
   callback_stdcall_decl
   compile_no_stdio
   abi_probe
   loadf_probe
   write
   write_fail
)

C_RUNTIME_TESTS=(
   smoke
   bounds
   leak
   safety
   negative
   scrgb
   spectral
   callback_abi
   callback_stdcall_decl
   compile_no_stdio
   abi_probe
   write
   write_fail
)

CFLAGS_BASE=(
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
   -Wstrict-overflow=5
   -fno-common
   -fno-builtin
   -funsigned-char
   -O2
   -g
)

CFLAGS_DEEP_NOSAN=(
   "${CFLAGS_BASE[@]}"
)

CFLAGS_DEEP=(
   "${CFLAGS_BASE[@]}"
   -fsanitize=undefined,address
   -fno-omit-frame-pointer
)

ASAN_OPTIONS=${ASAN_OPTIONS:-detect_leaks=0}
UBSAN_OPTIONS=${UBSAN_OPTIONS:-halt_on_error=1}
export ASAN_OPTIONS UBSAN_OPTIONS

cd "$ROOT_DIR"
mkdir -p "$BUILD_DIR"

check_tool()
{
   if ! command -v "$1" >/dev/null 2>&1; then
      echo "missing tool: $1" >&2
      exit 1
   fi
}

compile_header()
{
   compiler=$1
   name=$2
   source=$BUILD_DIR/header_$name.c
   object=$BUILD_DIR/header_$name.o

   printf '#define EXR_IMAGE_IMPLEMENTATION\n#include "exr_image.h"\nint main(void) { return 0; }\n' > "$source"
   "$compiler" "${CFLAGS_DEEP[@]}" -I. -c "$source" -o "$object"
}

compile_c_tests()
{
   compiler=$1
   name=$2
   test_name=

   for test_name in "${C_TESTS[@]}"; do
      "$compiler" "${CFLAGS_DEEP[@]}" -I. "tests/$test_name.c" -o "$BUILD_DIR/$test_name.$name"
   done
}

run_c_tests()
{
   name=$1
   test_name=

   for test_name in "${C_RUNTIME_TESTS[@]}"; do
      "$BUILD_DIR/$test_name.$name"
   done
}

run_corpus_probe()
{
   compiler=$1
   name=$2
   inputs=$BUILD_DIR/corpus_inputs.txt
   log=$BUILD_DIR/loadf_probe.$name.log
   corpus_dirs=()
   corpus_dir=
   old_ifs=$IFS

   if [ -z "${EXRI_CORPUS_DIRS:-}" ]; then
      echo "skipping EXR corpus probe: set EXRI_CORPUS_DIRS to external fixture directories"
      return
   fi

   IFS=:
   for corpus_dir in ${EXRI_CORPUS_DIRS:-}; do
      IFS=$old_ifs
      if [ -z "$corpus_dir" ]; then
         continue
      fi
      if [ ! -d "$corpus_dir" ]; then
         echo "missing EXRI_CORPUS_DIRS entry: $corpus_dir" >&2
         exit 1
      fi
      corpus_dirs+=("$corpus_dir")
      IFS=:
   done
   IFS=$old_ifs

   if [ "${#corpus_dirs[@]}" -eq 0 ]; then
      echo "EXRI_CORPUS_DIRS did not contain any directories" >&2
      exit 1
   fi

   find "${corpus_dirs[@]}" -type f -name '*.exr' | sort > "$inputs"
   if [ ! -s "$inputs" ]; then
      echo "skipping EXR corpus probe: no EXR corpus files present"
      return
   fi

   "$compiler" "${CFLAGS_DEEP_NOSAN[@]}" -I. tests/loadf_probe.c -o "$BUILD_DIR/loadf_probe_corpus.$name"
   while IFS= read -r input; do
      "$BUILD_DIR/loadf_probe_corpus.$name" "$input"
   done < "$inputs" > "$log"

   grep -E 'not-loaded|failed|error|unsupported|invalid' "$log" > "$BUILD_DIR/loadf_probe.$name.findings" || true
   grep -Ev 'issue-238-double-free-multipart|issue-238-double-free|piz-bug-issue-100|tiled_half_1x1_alpha|EyeCausticLUT16R' "$BUILD_DIR/loadf_probe.$name.findings" > "$BUILD_DIR/loadf_probe.$name.unexpected" || true
   if [ -s "$BUILD_DIR/loadf_probe.$name.unexpected" ]; then
      cat "$BUILD_DIR/loadf_probe.$name.unexpected" >&2
      exit 1
   fi
}

check_tool "$CC_CLANG"
check_tool "$CC_GCC"

echo "checking standalone implementation include"
compile_header "$CC_CLANG" clang
compile_header "$CC_GCC" gcc

echo "building C tests with clang"
compile_c_tests "$CC_CLANG" clang
echo "running C tests with clang"
run_c_tests clang

echo "building C tests with gcc"
compile_c_tests "$CC_GCC" gcc
echo "running C tests with gcc"
run_c_tests gcc

if [ "${EXRI_SKIP_CORPUS:-0}" != "1" ]; then
   echo "running EXR corpus probe"
   run_corpus_probe "$CC_CLANG" clang
fi

if [ "${EXRI_SKIP_FUZZ:-0}" != "1" ]; then
   echo "building fuzz harnesses"
   scripts/build-fuzzers.sh
fi

echo "release checks passed"
