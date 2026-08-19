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

#include <cstdint>

namespace keylane::storage::internal {

struct TxGenerationReadiness {
  std::uint64_t active_transactions_ = 0;
  std::uint64_t live_tagged_bytes_ = 0;
  std::uint64_t dependency_pins_ = 0;
  bool sealed_and_durable_ = false;
};

constexpr bool CanReclaimTxGeneration(
    const TxGenerationReadiness& readiness) noexcept {
  return readiness.active_transactions_ == 0 &&
         readiness.live_tagged_bytes_ == 0 &&
         readiness.dependency_pins_ == 0 && readiness.sealed_and_durable_;
}

}  // namespace keylane::storage::internal
