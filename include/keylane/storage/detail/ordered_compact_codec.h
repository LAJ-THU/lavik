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

#include "keylane/storage/detail/grouped_collection.h"

namespace keylane::storage {

// Logical full-image bridge used by existing callback/RDB integrations.
// This is not the durable grouped representation: aggregate framing has its
// own caller-supplied limit, while each member still obeys the Redis limit.
absl::StatusOr<std::string> EncodeOrderedCompactValue(
    OrderedCollectionKind kind, std::span<const OrderedCollectionEntry> entries,
    std::size_t max_bytes = kMaxRecordPayloadBytes);
absl::StatusOr<std::vector<OrderedCollectionEntry>> DecodeOrderedCompactValue(
    OrderedCollectionKind kind, std::string_view encoded,
    std::uint64_t expected_count,
    std::size_t max_bytes = kMaxRecordPayloadBytes);

}  // namespace keylane::storage
