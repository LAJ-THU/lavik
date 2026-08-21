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
#include <string>
#include <string_view>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "keylane/server.h"

namespace keylane {

// Parses Redis-style binary memory sizes such as "67108864", "64mb", and
// "1gb". A suffix is optional; unsuffixed values are bytes.
absl::StatusOr<std::size_t> ParseMemorySize(std::string_view text);

// Tokenizes one Redis configuration line. Whitespace separates arguments,
// single and double quotes preserve whitespace, and an unquoted '#' starts a
// comment. An empty/comment-only line returns an empty vector.
absl::StatusOr<std::vector<std::string>> ParseRedisConfigLine(
    std::string_view line);

// Applies one already-tokenized directive to Keylane's startup options.
// Unsupported directives are rejected instead of being silently ignored.
absl::Status ApplyRedisConfigDirective(
    const std::vector<std::string>& directive, ServerOptions* options);

// Loads a Redis-style configuration file. Errors include the file and line
// number so startup failures can be fixed directly.
absl::Status LoadRedisConfigFile(const std::string& path,
                                 ServerOptions* options);

// Validates cross-field startup constraints after config-file and CLI values
// have both been applied.
absl::Status ValidateServerOptions(const ServerOptions& options);

}  // namespace keylane
