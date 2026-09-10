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

// Typed state carried inside the generic MetaOperationRecord for one
// top-level failover. The journal remains operation-kind agnostic; this codec
// gives the failover reconciler a strict, replayable contract without adding
// nested promotion operations.

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "absl/status/statusor.h"
#include "keylane/meta/commands.h"

namespace keylane::meta {

class MetaCommittedView;
class MetaObservationStore;

inline constexpr std::string_view kFailoverOperationKind = "failover";

struct FailoverIntent {
  std::string group_id_;
  std::string former_owner_node_id_;
  MetaAssignmentId former_owner_assignment_id_{};
  MetaBootIncarnation former_owner_boot_id_{};
  std::string candidate_node_id_;
  MetaAssignmentId candidate_assignment_id_{};
  MetaBootIncarnation candidate_boot_id_{};
  std::uint64_t group_term_ = 0;
  std::uint64_t authority_version_ = 0;
  std::uint64_t grant_revision_ = 0;
  std::uint64_t population_manifest_revision_ = 0;
  MetaHash256 population_manifest_digest_{};
  std::uint64_t partition_replication_epoch_ = 0;
  MetaReplicationHistoryId parent_history_id_{};

  bool operator==(const FailoverIntent&) const = default;
};

enum class FailoverPhaseStage : std::uint8_t {
  kOldAuthorityExcluded = 1,
  kCandidateCaughtUp = 2,
  kPromotionPreparing = 3,
  kPromotionPrepared = 4,
  // Reserved for #41. #40 validates and executes only through Prepared.
  kAuthorityActivated = 5,
  kServing = 6,
};

struct FailoverPhase {
  FailoverPhaseStage stage_ = FailoverPhaseStage::kOldAuthorityExcluded;
  MetaHash256 old_authority_exclusion_hash_{};
  std::vector<std::uint64_t> required_applied_next_lsns_;
  MetaHash256 prepared_result_hash_{};

  bool operator==(const FailoverPhase&) const = default;
};

absl::StatusOr<std::string> EncodeFailoverIntent(const FailoverIntent& intent);
absl::StatusOr<FailoverIntent> DecodeFailoverIntent(std::string_view encoded);
absl::StatusOr<std::string> EncodeFailoverPhase(const FailoverPhase& phase);
absl::StatusOr<FailoverPhase> DecodeFailoverPhase(std::string_view encoded);

// Coordinator hook for the #40 portion of the failover phase graph. It is a
// no-op for other operation kinds and rejects any failover transition that
// skips the committed authority-exclusion boundary, changes intent anchors,
// or claims prepared evidence without the exact successful directive receipt.
absl::Status ValidateFailoverProposal(
    const MetaCommand& command, const MetaCommittedView& view,
    const MetaObservationStore& observations);

}  // namespace keylane::meta
