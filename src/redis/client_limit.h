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

#include "absl/status/status.h"

namespace keylane {

// Runtime boundary used by CONFIG without coupling command dispatch to the
// concrete RedisService owned by the server lifecycle.
class ClientLimit {
 public:
  virtual ~ClientLimit() = default;

  virtual std::uint64_t max_clients() const noexcept = 0;
  // Raises the process file-descriptor limit when needed. The update fails
  // without changing the live limit if the requested value cannot preserve
  // Keylane's non-client descriptor reserve.
  virtual absl::Status SetMaxClients(std::uint64_t value) = 0;

  virtual std::size_t client_query_buffer_limit() const noexcept = 0;
  // Updates the per-connection parser guard. Existing connections observe the
  // new value before their next socket-read parsing round.
  virtual absl::Status SetClientQueryBufferLimit(std::size_t value) = 0;
};

// The server installs its live limit before listeners open and clears it only
// after every service coroutine has stopped.
void InitClientLimit(ClientLimit* limit) noexcept;

}  // namespace keylane
