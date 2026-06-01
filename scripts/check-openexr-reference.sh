#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR=$(cd "$(dirname "$0")/.." && pwd)
OPENEXR_SOURCE_DIR=${OPENEXR_SOURCE_DIR:-/tmp/openexr}
IMATH_SOURCE_DIR=${IMATH_SOURCE_DIR:-/tmp/imath}
OPENEXR_BUILD_DIR=${EXRI_OPENEXR_BUILD_DIR:-/tmp/exri-openexr-build}
OPENEXR_INSTALL_DIR=${EXRI_OPENEXR_INSTALL_DIR:-/tmp/exri-openexr-install}
HARNESS_BUILD_DIR=${EXRI_OPENEXR_HARNESS_BUILD_DIR:-/tmp/exri-openexr-reference-build}
INPUT_LIST=${EXRI_OPENEXR_INPUT_LIST:-/tmp/exri-openexr-reference-inputs.txt}
EPSILON=${EXRI_OPENEXR_EPSILON:-0.0001}
JOBS=${EXRI_JOBS:-}

if [ ! -d "$OPENEXR_SOURCE_DIR" ]; then
   echo "missing OPENEXR_SOURCE_DIR: $OPENEXR_SOURCE_DIR" >&2
   echo "clone https://github.com/AcademySoftwareFoundation/openexr there or set OPENEXR_SOURCE_DIR" >&2
   exit 2
fi

cmake_args=(
   -S "$OPENEXR_SOURCE_DIR"
   -B "$OPENEXR_BUILD_DIR"
   -DCMAKE_BUILD_TYPE=Release
   -DCMAKE_INSTALL_PREFIX="$OPENEXR_INSTALL_DIR"
   -DBUILD_TESTING=OFF
   -DBUILD_SHARED_LIBS=OFF
   -DOPENEXR_BUILD_LIBS=ON
   -DOPENEXR_BUILD_TOOLS=OFF
   -DOPENEXR_BUILD_EXAMPLES=OFF
   -DOPENEXR_BUILD_PYTHON=OFF
   -DBUILD_WEBSITE=OFF
   -DOPENEXR_INSTALL=ON
   -DOPENEXR_INSTALL_TOOLS=OFF
   -DOPENEXR_INSTALL_DEVELOPER_TOOLS=OFF
   -DOPENEXR_INSTALL_DOCS=OFF
   -DOPENEXR_INSTALL_PKG_CONFIG=ON
   -DOPENEXR_FORCE_INTERNAL_DEFLATE=ON
   -DOPENEXR_FORCE_INTERNAL_OPENJPH=ON
)

if [ -d "$IMATH_SOURCE_DIR/.git" ]; then
   cmake_args+=(
      -DOPENEXR_FORCE_INTERNAL_IMATH=ON
      -DOPENEXR_IMATH_REPO="file://$IMATH_SOURCE_DIR"
      -DOPENEXR_IMATH_TAG="${IMATH_GIT_TAG:-main}"
   )
fi

echo "configuring OpenEXR reference library"
cmake "${cmake_args[@]}"

echo "building and installing OpenEXR reference library"
if [ -n "$JOBS" ]; then
   cmake --build "$OPENEXR_BUILD_DIR" --target install --parallel "$JOBS"
else
   cmake --build "$OPENEXR_BUILD_DIR" --target install --parallel
fi

echo "configuring exr_image/OpenEXR reference harness"
cmake -S "$ROOT_DIR/tests/openexr_reference" \
   -B "$HARNESS_BUILD_DIR" \
   -DCMAKE_BUILD_TYPE=Release \
   -DEXRI_USE_OPENEXR_PACKAGE=OFF \
   -DEXRI_OPENEXR_INSTALL_DIR="$OPENEXR_INSTALL_DIR" \
   -DEXRI_OPENEXR_BUILD_DIR="$OPENEXR_BUILD_DIR" \
   -DEXRI_ROOT_DIR="$ROOT_DIR"

echo "building exr_image/OpenEXR reference harness"
if [ -n "$JOBS" ]; then
   cmake --build "$HARNESS_BUILD_DIR" --parallel "$JOBS"
else
   cmake --build "$HARNESS_BUILD_DIR" --parallel
fi

if [ "$#" -gt 0 ]; then
   "$HARNESS_BUILD_DIR/exri_openexr_reference" --epsilon "$EPSILON" "$@"
else
   corpus_dirs=${EXRI_OPENEXR_CORPUS_DIRS:-$OPENEXR_SOURCE_DIR}
   old_ifs=$IFS
   IFS=:
   # shellcheck disable=SC2086
   set -- $corpus_dirs
   IFS=$old_ifs
   find "$@" -type f -name '*.exr' | sort > "$INPUT_LIST"
   if [ ! -s "$INPUT_LIST" ]; then
      echo "no .exr files found in EXRI_OPENEXR_CORPUS_DIRS" >&2
      exit 2
   fi
   "$HARNESS_BUILD_DIR/exri_openexr_reference" --epsilon "$EPSILON" --list "$INPUT_LIST"
fi
