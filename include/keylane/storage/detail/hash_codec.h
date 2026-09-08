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
#include <string>
#include <string_view>
#include <vector>

#include "absl/status/statusor.h"
#include "keylane/storage/format.h"

namespace keylane::storage {

inline constexpr std::size_t kHashValueHeaderBytes = 32;

struct HashEntry {
  Digest digest_{};
  std::string field_;
  std::string value_;
};

struct HashValue {
  std::vector<HashEntry> entries_;
};

// The compact encoding contains complete field/value pairs, never a mutation
// log or a process-local digest. Fixed-width header fields and entry lengths
// are little-endian; decoding reconstructs lookup digests.
// max_bytes bounds the aggregate encoding, independently of the 512 MiB
// limit on each field and value. Group framing needs room for a maximal
// individual value plus its field name and encoding overhead.
absl::StatusOr<HashValue> DecodeHashValue(
    std::string_view payload, std::size_t max_bytes = kMaxStringBytes);
absl::StatusOr<std::string> EncodeHashValue(
    const HashValue& value, std::size_t max_bytes = kMaxStringBytes);

// Checked size accumulation without allocating the supplied field/value.
// Used before serialization; failure leaves the caller's running size intact.
absl::StatusOr<std::size_t> AppendHashEntrySize(
    std::size_t encoded_bytes, std::size_t field_bytes, std::size_t value_bytes,
    std::size_t max_bytes = kMaxStringBytes);

}  // namespace keylane::storage
