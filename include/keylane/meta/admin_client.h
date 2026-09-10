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

// Raft-free synchronous transport for Meta's one-line administrative
// protocol. Direct administration and cluster discovery share endpoint
// validation, partial I/O, TLS identity checks, response limits, and deadlines.

#include <chrono>
#include <string>
#include <string_view>

#include "absl/status/statusor.h"

namespace keylane::meta {

using MetaAdminDeadline = std::chrono::steady_clock::time_point;

struct MetaAdminTlsOptions {
  std::string ca_file_;
  std::string certificate_file_;
  std::string private_key_file_;
  // Empty verifies the numeric endpoint's IP SAN. This override exists only
  // for direct keylane-ctl commands; cluster discovery deliberately
  // leaves it empty so learned addresses cannot change certificate identity.
  std::string server_name_;
};

struct MetaAdminTarget {
  enum class Transport {
    kUnix,
    kTcpPlaintext,
    kTcpMtls,
  };

  Transport transport_ = Transport::kUnix;
  // Unix socket path for kUnix; canonical numeric IP:port otherwise.
  std::string endpoint_;
  MetaAdminTlsOptions tls_;
};

class MetaAdminClient {
 public:
  // Sends one command and returns the reply without its line terminator.
  // The deadline is absolute and is shared by connect, TLS, partial writes,
  // and response reads. Commands containing CR/LF or exceeding 64 KiB and
  // replies exceeding 256 MiB are rejected.
  absl::StatusOr<std::string> RoundTrip(const MetaAdminTarget& target,
                                        std::string_view command,
                                        MetaAdminDeadline deadline) const;
};

}  // namespace keylane::meta
