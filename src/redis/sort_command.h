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

#include <span>

#include "lavik/command.h"

namespace lavik {

class ReplyBuilder;

struct SortExecKey {
  storage::Digest digest_;
  std::uint16_t owner_ = 0;
  std::uint16_t arg_ = 0;
};

void InitSortCommandStorage(storage::StorageEngine* engine);

Task<CommandReply> ExecuteSortCommand(const CommandRequest& request,
                                      ReplyBuilder& reply_builder);

// EXEC already owns all statically discoverable SORT keys. Pattern-derived
// BY/GET keys cannot be added after EXEC has acquired its global lock set and
// are rejected unless they alias one of those keys. Script callers request
// deterministic ordering for Set input with a constant BY pattern, matching
// Valkey's replication-safe Lua behavior.
Task<std::string> ExecuteSortCommandLocked(
    const CommandRequest& request, std::span<const SortExecKey> keys,
    std::vector<storage::TxShardWrites>& tx_writes,
    bool deterministic_set_order);

}  // namespace lavik
