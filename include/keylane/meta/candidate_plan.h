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
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "keylane/meta/observation_store.h"

namespace keylane::meta {

enum class CandidatePlanDisposition : std::uint8_t {
  kSelected,
  kGroupUnknown,
  kNoEligibleCandidates,
  kMultipleCompatibilityDomains,
};

enum class CandidateSelectionBasis : std::uint8_t {
  kUniqueGreatest,
  kEqualGreatestNodeTieBreak,
  kIncomparableEnvelopeDeficit,
};

// Immutable result of one internal failover-planning call. It is deliberately
// not an operation or RPC contract: future failover orchestration consumes the
// selected observation immediately and owns any later validation it needs.
struct CandidatePlan {
  CandidatePlanDisposition disposition_ =
      CandidatePlanDisposition::kNoEligibleCandidates;
  std::optional<CandidateSelectionBasis> selection_basis_;
  std::optional<MetaCandidateProgressObs> selected_;
  // Node-sorted maximal candidates retained for diagnostics and audit input.
  std::vector<std::string> maximal_node_ids_;
};

// Selects a completed replica population at one fixed receive-time cut. The
// selector depends only on committed group facts plus member-scoped semantic
// anchors and non-extendable observation TTLs; unrelated heartbeat traffic
// cannot invalidate or restart the calculation.
CandidatePlan CandidatePlanFor(std::string_view group_id,
                               const MetaCommittedFacts& facts,
                               const MetaObservationStore& observations,
                               std::int64_t now_unix_ms);

}  // namespace keylane::meta
