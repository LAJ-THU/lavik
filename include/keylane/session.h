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
#include <string>
#include <vector>

#include "keylane/command.h"
#include "keylane/resp.h"
#include "keylane/storage/format.h"
#include "keylane/tx/fingerprint.h"

namespace keylane {

// Per-connection state, owned by the connection's Serve coroutine frame.
// Everything here must be cleaned up through the single cleanup point at the
// end of RedisService::Serve.
struct ConnectionContext {
  std::uint8_t selected_db_ = 0;
  bool counted_as_client_ = true;
  ReplyBuilder reply_builder_;

  // MULTI/EXEC queueing. `multi_db` tracks SELECTs issued while queueing so
  // every queued command records the database it will execute against;
  // `multi_dirty` marks queue-time errors that turn EXEC into EXECABORT.
  bool in_multi_ = false;
  bool multi_dirty_ = false;
  std::uint8_t multi_db_ = 0;
  std::vector<CommandRequest> queued_;

  // WATCH registrations: enough to check and unregister on the owning
  // shards. Deduplicated by (db, key) — never by fingerprint, which can
  // collide across distinct keys — and the first registration's liveness
  // snapshot is authoritative (sticky, like Redis).
  struct WatchedKey {
    std::string key_;
    storage::Digest digest_;
    tx::LockFp fp_ = 0;
    std::uint16_t owner_ = 0;
    std::uint8_t db_ = 0;
    // Liveness observed on the owning shard at WATCH time; EXEC compares it
    // against the key's own current liveness, so fingerprint collisions can
    // only ever cause false aborts, not missed ones.
    bool live_ = false;
  };
  std::uint64_t conn_id_ = 0;
  std::vector<WatchedKey> watched_;

  void ResetMulti() {
    in_multi_ = false;
    multi_dirty_ = false;
    queued_.clear();
  }
};

}  // namespace keylane
