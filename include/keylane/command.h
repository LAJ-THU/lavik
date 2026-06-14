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

#include <string>
#include <vector>

#include "celer/base/status.h"
#include "celer/runtime/task.h"

namespace keylane {

struct RespCommand;

using celer::StatusOr;
using celer::Task;

enum class CommandKind {
  kPing,
  kDel,
  kExists,
  kGet,
  kIncr,
  kSet,
  kUnknown,
};

struct CommandRequest {
  CommandKind kind = CommandKind::kUnknown;
  std::vector<std::string> args;
};

struct CommandReply {
  std::string encoded;
  bool close_connection = false;
};

StatusOr<CommandRequest> BuildCommandRequest(RespCommand command);

// Create one shard (DbShard) per worker. Call once before the server starts.
void InitShards(unsigned num_shards);

// Route `request` to the shard that owns its key (hash(key) % num_shards),
// running it locally or via cross-core SubmitTo, and return the encoded reply.
Task<CommandReply> ExecuteCommand(const CommandRequest& request);

}  // namespace keylane
