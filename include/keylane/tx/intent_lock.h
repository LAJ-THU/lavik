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

#include <cassert>
#include <cstdint>
#include <span>

#include "absl/container/flat_hash_map.h"
#include "keylane/tx/fingerprint.h"

namespace keylane::tx {

// VLL-style non-blocking lock table, one per (worker, logical DB). All methods
// run on the owning worker thread in non-suspending sections — no atomics.
//
// Two counter layers per fingerprint:
//  - intents: recorded from scheduling until the transaction's release hop.
//    AcquireIntent never blocks; it records the intent and reports whether it
//    was granted (sole/compatible owner). Intents arbitrate scheduling: a
//    fully-granted transaction may run immediately, anything else queues.
//  - holds: marked only while a callback is actually executing (possibly
//    suspended on disk I/O). The queue head may start only when CanHold passes
//    for its whole key set, i.e. every conflicting suspended runner drained.
//    Traditional VLL has no hold layer because callbacks there
//    run to completion; keylane callbacks suspend, so "currently executing"
//    must be visible to the scheduler. Invariant: holds ⊆ intents.
class LockTable {
 public:
  // Records the intent unconditionally. Returns true iff granted immediately:
  // shared — no exclusive intent; exclusive — no other intent at all.
  bool AcquireIntent(LockFp fp, LockMode mode) {
    IntentLock& lock = map_[fp];
    if (mode == LockMode::kShared) {
      ++lock.shared_intent_;
      return lock.exclusive_intent_ == 0;
    }
    ++lock.exclusive_intent_;
    return lock.shared_intent_ == 0 && lock.exclusive_intent_ == 1;
  }

  void ReleaseIntent(LockFp fp, LockMode mode) {
    auto it = map_.find(fp);
    assert(it != map_.end());
    IntentLock& lock = it->second;
    if (mode == LockMode::kShared) {
      assert(lock.shared_intent_ > 0);
      --lock.shared_intent_;
    } else {
      assert(lock.exclusive_intent_ > 0);
      --lock.exclusive_intent_;
    }
    if (lock.IsFree()) {
      map_.erase(it);
    }
  }

  bool CanHold(LockFp fp, LockMode mode) const {
    auto it = map_.find(fp);
    if (it == map_.end()) {
      return true;
    }
    const IntentLock& lock = it->second;
    if (mode == LockMode::kShared) {
      return lock.exclusive_held_ == 0;
    }
    return lock.shared_held_ == 0 && lock.exclusive_held_ == 0;
  }

  void AcquireHold(LockFp fp, LockMode mode) {
    assert(CanHold(fp, mode));
    IntentLock& lock = map_[fp];
    if (mode == LockMode::kShared) {
      ++lock.shared_held_;
    } else {
      assert(lock.exclusive_held_ == 0);
      ++lock.exclusive_held_;
    }
  }

  void ReleaseHold(LockFp fp, LockMode mode) {
    auto it = map_.find(fp);
    assert(it != map_.end());
    IntentLock& lock = it->second;
    if (mode == LockMode::kShared) {
      assert(lock.shared_held_ > 0);
      --lock.shared_held_;
    } else {
      assert(lock.exclusive_held_ == 1);
      lock.exclusive_held_ = 0;
    }
    // holds ⊆ intents: a held entry always has intents, so IsFree can only
    // trigger from ReleaseIntent.
  }

  std::size_t size() const noexcept { return map_.size(); }

 private:
  struct IntentLock {
    std::uint32_t shared_intent_ = 0;
    std::uint32_t exclusive_intent_ = 0;
    std::uint32_t shared_held_ = 0;
    std::uint32_t exclusive_held_ = 0;

    bool IsFree() const noexcept {
      return shared_intent_ == 0 && exclusive_intent_ == 0 &&
             shared_held_ == 0 && exclusive_held_ == 0;
    }
  };

  absl::flat_hash_map<LockFp, IntentLock, LockFpIdentityHash> map_;
};

}  // namespace keylane::tx
