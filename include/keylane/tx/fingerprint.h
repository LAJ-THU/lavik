/*
 * Copyright (C) 2026 EloqData Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "keylane/storage/format.h"

namespace keylane::tx {

// Lock fingerprint: the first 8 bytes of the key's SHA-1 digest, which the
// engine computes for index lookups anyway. Collisions are correctness-neutral
// (two keys sharing a lock entry only causes false contention): execution
// order is arbitrated by the per-shard TxQueue and data access still compares
// the full digest and key.
using LockFp = std::uint64_t;

inline LockFp FingerprintOf(const storage::Digest& digest) noexcept {
  LockFp fp;
  std::memcpy(&fp, digest.bytes_.data(), sizeof(fp));
  return fp;
}

// The fingerprint is already uniformly distributed; hash maps keyed by it use
// the identity function.
struct LockFpIdentityHash {
  std::size_t operator()(LockFp fp) const noexcept { return fp; }
};

enum class LockMode : std::uint8_t {
  kShared,
  kExclusive,
};

// One key of a transaction's lock set: fingerprint, requested mode, and the
// logical database whose lock table arbitrates it. Carrying the database per
// key lets one transaction span databases (SELECT inside MULTI).
struct KeyRef {
  LockFp fp_ = 0;
  LockMode mode_ = LockMode::kShared;
  std::uint8_t db_ = 0;
};

}  // namespace keylane::tx
