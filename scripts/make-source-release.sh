#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR=$(cd "$(dirname "$0")/.." && pwd)
VERSION=${1:-$(sed -n '1p' "$ROOT_DIR/VERSION")}
PACKAGE=exr_image-$VERSION
OUT_DIR=${EXRI_RELEASE_DIR:-$ROOT_DIR/dist}
STAGE_ROOT=${EXRI_RELEASE_STAGE:-/tmp/exri-release}
STAGE=$STAGE_ROOT/$PACKAGE

cd "$ROOT_DIR"

if [ -z "$VERSION" ]; then
   echo "empty version" >&2
   exit 1
fi

case "$VERSION" in
   */*|*\\*)
      echo "version must not contain path separators: $VERSION" >&2
      exit 1
      ;;
esac

rm -rf "$STAGE"
mkdir -p "$STAGE"
mkdir -p "$STAGE/docs" "$STAGE/scripts" "$STAGE/tests"
mkdir -p "$OUT_DIR"

cp README.md CONTRIBUTING.md SECURITY.md LICENSE THIRD_PARTY_NOTICES.md VERSION exr_image.h "$STAGE/"
cp docs/*.md "$STAGE/docs/"
cp scripts/*.sh scripts/*.cmd "$STAGE/scripts/"
cp tests/*.c tests/*.h "$STAGE/tests/"

rm -f "$OUT_DIR/$PACKAGE.tar.gz" "$OUT_DIR/$PACKAGE.zip" "$OUT_DIR/exr_image.h" "$OUT_DIR/SHA256SUMS"
cp exr_image.h "$OUT_DIR/exr_image.h"
tar -czf "$OUT_DIR/$PACKAGE.tar.gz" -C "$STAGE_ROOT" "$PACKAGE"

if command -v zip >/dev/null 2>&1; then
   (cd "$STAGE_ROOT" && zip -qr "$OUT_DIR/$PACKAGE.zip" "$PACKAGE")
fi

if command -v sha256sum >/dev/null 2>&1; then
   (
      cd "$OUT_DIR"
      if [ -f "$PACKAGE.zip" ]; then
         sha256sum exr_image.h "$PACKAGE.tar.gz" "$PACKAGE.zip" > SHA256SUMS
      else
         sha256sum exr_image.h "$PACKAGE.tar.gz" > SHA256SUMS
      fi
   )
fi

echo "$OUT_DIR/exr_image.h"
echo "$OUT_DIR/$PACKAGE.tar.gz"
if [ -f "$OUT_DIR/$PACKAGE.zip" ]; then
   echo "$OUT_DIR/$PACKAGE.zip"
fi
if [ -f "$OUT_DIR/SHA256SUMS" ]; then
   echo "$OUT_DIR/SHA256SUMS"
fi
