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
#include <optional>
#include <string>
#include <vector>

#include "keylane/replication.h"
#include "keylane/storage/format.h"

namespace keylane {

inline constexpr long kDefaultMimallocPurgeDelayMs = 60'000;

struct ServerOptions {
  std::string config_file_;
  std::string bind_ip_ = "127.0.0.1";
  std::uint16_t port_ = 6379;
  std::uint16_t metrics_port_ = 0;
  unsigned thread_count_ = 1;
  bool pin_workers_ = true;
  int idle_timeout_ms_ = -1;
  unsigned recv_buffer_count_ = 1024;
  unsigned busy_poll_us_ = 20;
  unsigned background_budget_us_ = 10;
  unsigned background_warrant_percent_ = 1;
  unsigned spdk_max_completions_per_poll_ = 8;
  unsigned spdk_foreground_pre_poll_us_ = 5;
  long mimalloc_purge_delay_ms_ = kDefaultMimallocPurgeDelayMs;
  std::size_t registered_buffer_bytes_ = 256ULL * 1024 * 1024;
  std::size_t replication_publish_queue_bytes_ = 8ULL * 1024 * 1024;
  std::uint64_t max_memory_bytes_ = 0;
  std::size_t inline_key_max_bytes_ = storage::kDefaultInlineKeyBytes;
  std::uint32_t flush_max_ms_ = 1000;
  std::size_t flush_size_bytes_ = 128ULL * 1024;
  bool verify_read_crc_ = true;
  std::vector<std::string> data_files_{"keylane.data"};
  std::uint32_t tomb_raider_interval_ms_ = 86'400'000;
  std::uint32_t tomb_raider_sleep_ms_ = 10;
  unsigned defrag_max_active_per_device_ = 8;
  std::uint32_t defrag_sleep_ms_ = 0;
  std::uint32_t defrag_record_sleep_us_ = 0;
  bool defrag_paused_ = false;
  std::optional<ReplicaOfConfig> replicaof_;
  ReplicationOptions replication_options_;
};

int RunServer(ServerOptions options);

}  // namespace keylane
