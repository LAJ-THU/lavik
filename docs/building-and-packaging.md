<!--
Copyright (C) 2026 EloqData Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
-->

# Building and packaging

## Local builds

Optimized local builds use the current machine's instruction set by default:

```bash
./scripts/build_release.sh
```

This configures `KEYLANE_MARCH=native`, including Celer and mimalloc. OpenSSL
is linked statically, so the resulting executable does not depend on
`libssl.so` or `libcrypto.so`. The build machine still needs the OpenSSL
headers and static archives (`libssl-dev` on Ubuntu, which provides `libssl.a`
and `libcrypto.a`).

For a different local CPU target, configure CMake directly with
`-DKEYLANE_MARCH=<target>`. An empty value disables the explicit `-march` flag.

## Downloadable release package

```bash
./scripts/package_release.sh
```

The packaging script performs a Release build, statically links OpenSSL plus
the GNU C++/compiler runtimes, strips a staged copy of the executable, verifies
that no dynamic OpenSSL or C++ runtime dependency remains, and writes a
versioned archive and SHA-256 checksum under `dist/`. The archive also carries
the Apache-2.0 license text required by the statically linked OpenSSL code.

Unlike a local build, a package uses a portable CPU baseline:

- `x86_64`: `-march=x86-64-v2`
- `aarch64`: `-march=armv8-a`

Override it with `KEYLANE_PACKAGE_MARCH` when producing a package for a more
specific fleet. Other useful overrides are `KEYLANE_PACKAGE_BUILD_DIR`,
`KEYLANE_PACKAGE_OUTPUT_DIR`, and `KEYLANE_PACKAGE_JOBS`.

The release remains a normal Linux ELF executable and therefore uses the
platform C library. Build official artifacts in the oldest supported Linux
environment so their glibc requirement remains compatible with newer systems.
