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

#include <memory>
#include <optional>

#include "keylane/meta/coordinator.h"
#include "keylane/meta/data_control_runtime_status.h"

namespace keylane::meta {

namespace detail {
// Plans at most one committed effect from an atomic recovered view. A missing
// command means wait for Data; incompatible state requires operator recovery,
// never another destructive initialization. No I/O or in-memory phase cursor.
absl::StatusOr<std::optional<MetaCommand>> PlanClusterCreateStep(
    const MetaCommittedView& view, const MetaOperationRecord& operation,
    const MetaDataControlRuntimeSnapshot& runtime);
}  // namespace detail

// Leader-scoped owner of durable cluster-create operations. Admin submits the
// complete intent before changing topology and merely waits for this owner.
// Every Start rescans the recovered operation journal. Demotion/shutdown joins
// local proposals, not remote Data completion, leaving durable work resumable.
class MetaClusterCreateReconciler final : public MetaReconciler {
 public:
  MetaClusterCreateReconciler(
      celer::ForeignExecutor executor,
      std::shared_ptr<MetaMembershipGate> membership_gate,
      std::shared_ptr<MetaDataControlRuntimeStatus> runtime_status);
  ~MetaClusterCreateReconciler() override;
  void Start(MetaLeaderContext& context) override;
  void CancelAndWait() override;
  // Permanent process stop; call before draining Admin/Data listeners while
  // the worker and proposal executor can still complete accepted proposals.
  void Shutdown();
  // Thread-safe ingress check; shutdown closes it before joining proposals.
  bool accepting() const;

 private:
  struct Core;
  static celer::Task<absl::Status> Run(std::shared_ptr<Core> core,
                                       MetaLeaderContext* context);
  void Stop(bool permanent);
  std::shared_ptr<Core> core_;
};

}  // namespace keylane::meta
