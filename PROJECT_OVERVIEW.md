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

# Celer Redis Overview

## Goal

`keylane` is a Redis/Valkey-protocol server built on top of the `celer` core runtime.

This repository should own:

- RESP parsing and serialization
- command dispatch
- shard-local in-memory database structures
- cross-worker request routing
- protocol-level connection/session state

This repository should not own:

- worker loop semantics
- TCP transport runtime
- generic I/O primitives

Those belong to `celer`.

## Intended First Milestone

A multi-worker, in-memory server with:

- RESP2 parsing
- one thread per core
- key-based routing to shards/workers
- no `MULTI/EXEC/WATCH` yet
- no replication yet
- no persistence yet

## Expected Layout

Repository layout:

- this repo contains a `celer/` git submodule
- initialize it with `git submodule update --init --recursive`

## Build

```bash
cmake -S . -B build
cmake --build build -j4
```

This repository currently uses `add_subdirectory(celer ...)` against the submodule checkout.
