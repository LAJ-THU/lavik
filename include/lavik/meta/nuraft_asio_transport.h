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

// Assembly helpers for NuRaft's native Asio transport. NuRaft owns peer
// sockets, timers, and its Asio worker pool; Lavik optionally supplies mTLS
// contexts and always supplies replicated member verification, listener
// binding, and allocation bounds through its pinned patch hooks. In plaintext
// mode member ids are checked but are not cryptographically authenticated.

#include <cstddef>
#include <cstdint>
#include <string>

#include "absl/status/statusor.h"
#include "libnuraft/asio_service.hxx"
#include "libnuraft/ptr.hxx"

namespace lavik::meta {

class MetaStateMachine;
class NuraftStateMgr;

struct MetaAsioTransportConfig {
  std::string bind_address_;
  std::string tls_ca_cert_file_;
  std::string tls_cert_file_;
  std::string tls_key_file_;
  std::size_t io_threads_ = 2;
  std::uint32_t max_rpc_payload_bytes_ = 64u << 20;

  bool TlsEnabled() const { return !tls_ca_cert_file_.empty(); }
};

// Builds transport callbacks before any NuRaft thread starts. When mTLS is
// enabled it also validates the TLS files; the returned SSL_CTX providers
// transfer one server and one client context to NuRaft, so the options object
// must be consumed by one launcher.
absl::StatusOr<nuraft::asio_service::options> BuildMetaAsioOptions(
    const MetaAsioTransportConfig& config,
    nuraft::ptr<NuraftStateMgr> state_mgr,
    nuraft::ptr<MetaStateMachine> state_machine);

}  // namespace lavik::meta
