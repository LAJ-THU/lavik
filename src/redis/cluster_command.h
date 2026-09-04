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

// CLUSTER command handlers for cluster mode. Called from
// ExecuteCluster (src/redis/command.cpp) when cluster mode is enabled;
// standalone mode keeps the legacy replication-derived NODES/SLOTS shim.
//
// Supported subcommands: SLOTS, NODES, MYID, INFO, KEYSLOT. Anything else
// gets Redis's exact unknown-subcommand error. ASK/ASKING and the gossip
// management subcommands are deliberately out of scope for v1.

#include "keylane/command.h"

namespace keylane {

Task<CommandReply> ExecuteClusterModeCommand(const CommandRequest& request,
                                             ReplyBuilder& reply_builder);

}  // namespace keylane
