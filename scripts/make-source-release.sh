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

cp README.md LICENSE THIRD_PARTY_NOTICES.md VERSION exr_image.h "$STAGE/"
cp docs/*.md "$STAGE/docs/"
cp scripts/*.sh scripts/*.cmd "$STAGE/scripts/"
cp tests/*.c tests/*.h "$STAGE/tests/"

rm -f "$OUT_DIR/$PACKAGE.tar.gz" "$OUT_DIR/$PACKAGE.zip"
tar -czf "$OUT_DIR/$PACKAGE.tar.gz" -C "$STAGE_ROOT" "$PACKAGE"

if command -v zip >/dev/null 2>&1; then
   (cd "$STAGE_ROOT" && zip -qr "$OUT_DIR/$PACKAGE.zip" "$PACKAGE")
fi

echo "$OUT_DIR/$PACKAGE.tar.gz"
if [ -f "$OUT_DIR/$PACKAGE.zip" ]; then
   echo "$OUT_DIR/$PACKAGE.zip"
fi
