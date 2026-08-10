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
#include <string>
#include <string_view>
#include <vector>

#include "keylane/replication.h"

namespace keylane {

int RunServer(std::string_view bind_ip, std::uint16_t port,
              std::uint16_t metrics_port, unsigned thread_count,
              int idle_timeout_ms, unsigned recv_buffer_count,
              unsigned busy_poll_us, std::size_t registered_buffer_bytes,
              std::uint64_t max_memory_bytes, std::uint32_t flush_max_ms,
              std::size_t flush_size_bytes, bool verify_read_crc,
              const std::vector<std::string>& data_files,
              std::uint32_t tomb_raider_interval_ms = 600'000,
              std::uint32_t tomb_raider_sleep_ms = 10,
              ReplicationOptions replication_options = {});

}  // namespace keylane
