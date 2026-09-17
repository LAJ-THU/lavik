#!/usr/bin/env bash
# Copyright (C) 2026 EloqData Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set -euo pipefail

REPO_ROOT=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
HOST_ARCH=$(uname -m)

case "$HOST_ARCH" in
  x86_64|amd64)
    PACKAGE_ARCH=x86_64
    DEFAULT_MARCH=x86-64-v2
    ;;
  aarch64|arm64)
    PACKAGE_ARCH=aarch64
    DEFAULT_MARCH=armv8-a
    ;;
  *)
    echo "Unsupported packaging architecture: $HOST_ARCH" >&2
    exit 1
    ;;
esac

PACKAGE_MARCH=${LAVIK_PACKAGE_MARCH:-$DEFAULT_MARCH}
BUILD_DIR=${LAVIK_PACKAGE_BUILD_DIR:-$REPO_ROOT/build/package-$PACKAGE_ARCH}
OUTPUT_DIR=${LAVIK_PACKAGE_OUTPUT_DIR:-$REPO_ROOT/dist}
BUILD_JOBS=${LAVIK_PACKAGE_JOBS:-$(nproc)}
VERSION=$(git -C "$REPO_ROOT" describe --tags --always --dirty)
VERSION=${VERSION//\//-}
PACKAGE_NAME=lavik-$VERSION-linux-$PACKAGE_ARCH
STAGE_DIR=$BUILD_DIR/$PACKAGE_NAME
ARCHIVE=$OUTPUT_DIR/$PACKAGE_NAME.tar.gz

cmake -S "$REPO_ROOT" -B "$BUILD_DIR" \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_TESTING=OFF \
  -DLAVIK_BUILD_FAULT_SERVER=OFF \
  -DLAVIK_BUILD_META=ON \
  -DLAVIK_ENABLE_OPT=ON \
  -DLAVIK_MARCH="$PACKAGE_MARCH" \
  -DLAVIK_STATIC_OPENSSL=ON \
  -DLAVIK_STATIC_CXX_RUNTIME=ON
BINARIES=(lavik lavik-meta lavik-ctl)
cmake --build "$BUILD_DIR" --target "${BINARIES[@]}" -j"$BUILD_JOBS"

for binary_name in "${BINARIES[@]}"; do
  binary=$BUILD_DIR/$binary_name
  if [[ ! -x "$binary" ]]; then
    echo "Release build did not produce $binary" >&2
    exit 1
  fi

  dynamic_section=$(readelf -d "$binary")
  if grep -Eq 'Shared library: \[(libssl|libcrypto)\.so' <<<"$dynamic_section"; then
    echo "Packaging refused: $binary_name still dynamically links OpenSSL" >&2
    exit 1
  fi
  if grep -Eq 'Shared library: \[(libstdc\+\+|libgcc_s)\.so' \
      <<<"$dynamic_section"; then
    echo "Packaging refused: $binary_name still dynamically links the C++ runtime" >&2
    exit 1
  fi
done

cmake -E remove_directory "$STAGE_DIR"
cmake -E make_directory "$STAGE_DIR"
for binary_name in "${BINARIES[@]}"; do
  install -m 0755 "$BUILD_DIR/$binary_name" "$STAGE_DIR/$binary_name"
done
install -m 0644 "$REPO_ROOT/LICENSE" "$REPO_ROOT/NOTICE" "$STAGE_DIR/"
if command -v strip >/dev/null 2>&1; then
  for binary_name in "${BINARIES[@]}"; do
    strip --strip-unneeded "$STAGE_DIR/$binary_name"
  done
fi
install -m 0644 "$REPO_ROOT/docs/design-docs/tls-and-auth.md" \
  "$STAGE_DIR/tls-and-auth.md"
OPENSSL_LICENSE=${LAVIK_OPENSSL_LICENSE:-/usr/share/common-licenses/Apache-2.0}
if [[ ! -f "$OPENSSL_LICENSE" ]]; then
  echo "OpenSSL license text not found at $OPENSSL_LICENSE" >&2
  echo "Set LAVIK_OPENSSL_LICENSE to the Apache-2.0 license file." >&2
  exit 1
fi
install -m 0644 "$OPENSSL_LICENSE" "$STAGE_DIR/OPENSSL-LICENSE.txt"
printf '%s\n' "$VERSION" >"$STAGE_DIR/VERSION"

for binary_name in "${BINARIES[@]}"; do
  "$STAGE_DIR/$binary_name" --help >/dev/null
done

cmake -E make_directory "$OUTPUT_DIR"
tar -C "$BUILD_DIR" -czf "$ARCHIVE" "$PACKAGE_NAME"

echo "Release package: $ARCHIVE"
echo "CPU baseline: -march=$PACKAGE_MARCH"
echo "Dynamic dependencies:"
for binary_name in "${BINARIES[@]}"; do
  echo "$binary_name:"
  ldd "$STAGE_DIR/$binary_name" || true
done
sha256sum "$ARCHIVE"
