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

#include <atomic>
#include <cstdint>
#include <memory>
#include <string>

#include "celer/rpc/rpc.h"
#include "celer/runtime/task.h"

namespace celer {
class Service;
class Worker;
}  // namespace celer

namespace keylane::storage {
class StorageEngine;
}  // namespace keylane::storage

namespace keylane {

struct ReplicationOptions {
  std::uint16_t listen_port_ = 0;
  std::string target_ip_;
  std::uint16_t target_port_ = 0;
  bool replica_read_only_ = false;
};

class ReplicationManager {
 public:
  ReplicationManager(storage::StorageEngine* storage,
                     ReplicationOptions options);
  ReplicationManager(const ReplicationManager&) = delete;
  ReplicationManager& operator=(const ReplicationManager&) = delete;
  ~ReplicationManager();

  celer::Service* service() noexcept;
  void StorageReady(celer::Worker& worker);
  bool replica_read_only() const noexcept {
    return options_.replica_read_only_;
  }

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
  ReplicationOptions options_;
};

}  // namespace keylane
