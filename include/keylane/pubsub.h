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
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "celer/runtime/task.h"

namespace celer {
class TcpStream;
}

namespace keylane {

class PubSubSession;

// Initializes one worker-local subscription registry per runtime worker.
void PreparePubSub(unsigned worker_count);

std::shared_ptr<PubSubSession> RegisterPubSubSession(int fd);
void UnregisterPubSubSession(const std::shared_ptr<PubSubSession>& session);

std::size_t PubSubSubscriptionCount(
    const std::shared_ptr<PubSubSession>& session) noexcept;

// These return one or more complete RESP2 push-style frames.
std::string SubscribeChannels(const std::shared_ptr<PubSubSession>& session,
                              std::span<const std::string> channels);
std::string UnsubscribeChannels(const std::shared_ptr<PubSubSession>& session,
                                std::span<const std::string> channels);
std::string PSubscribePatterns(const std::shared_ptr<PubSubSession>& session,
                               std::span<const std::string> patterns);
std::string PUnsubscribePatterns(const std::shared_ptr<PubSubSession>& session,
                                 std::span<const std::string> patterns);
void ResetPubSubSubscriptions(const std::shared_ptr<PubSubSession>& session);

// PUBSUB introspection reports this node's aggregate worker-local state.
celer::Task<std::vector<std::string>> PubSubChannels(
    std::optional<std::string> pattern);
celer::Task<std::vector<std::uint64_t>> PubSubNumSub(
    std::span<const std::string> channels);
celer::Task<std::uint64_t> PubSubNumPat();

// Delivers to every worker-local registry and returns the number of matching
// live subscriptions on this node. The encoded message body is shared across
// all recipients.
celer::Task<std::uint64_t> PublishChannel(std::string_view channel,
                                          std::string_view payload);

void EnqueuePubSubReply(const std::shared_ptr<PubSubSession>& session,
                        std::string encoded);
void ExitPubSubMode(const std::shared_ptr<PubSubSession>& session);
void ClosePubSubSession(const std::shared_ptr<PubSubSession>& session);
void MarkPubSubReaderStarted(const std::shared_ptr<PubSubSession>& session);
void MarkPubSubReaderDone(const std::shared_ptr<PubSubSession>& session);
celer::Task<absl::Status> WaitPubSubReaderDone(
    const std::shared_ptr<PubSubSession>& session);

celer::Task<absl::Status> StreamPubSubMessages(
    celer::TcpStream& stream, const std::shared_ptr<PubSubSession>& session);

}  // namespace keylane
