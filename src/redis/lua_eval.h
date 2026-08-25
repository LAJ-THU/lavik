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

#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "absl/status/statusor.h"

namespace keylane {

struct LuaRedisCall {
  bool protected_call_ = false;
  std::vector<std::string> args_;
};

struct LuaExecutionStep {
  std::optional<LuaRedisCall> call_;
  std::string reply_;
};

class LuaExecution {
 public:
  static absl::StatusOr<std::unique_ptr<LuaExecution>> Create(
      std::string_view script, std::span<const std::string> keys,
      std::span<const std::string> argv);
  static absl::StatusOr<std::unique_ptr<LuaExecution>> CreateCached(
      std::string_view sha, std::span<const std::string> keys,
      std::span<const std::string> argv);

  LuaExecution(const LuaExecution&) = delete;
  LuaExecution& operator=(const LuaExecution&) = delete;
  ~LuaExecution();

  // lua_dump output for the user chunk. It can be loaded into another
  // lua_State created by this binary without parsing the source again.
  std::string_view bytecode() const;
  LuaExecutionStep Start();
  LuaExecutionStep Resume(std::string_view command_reply);

 private:
  struct Impl;
  explicit LuaExecution(std::unique_ptr<Impl> impl);
  std::unique_ptr<Impl> impl_;
};

std::string LuaScriptSha1(std::string_view script);
// Stores one stable process-wide copy of the compiled chunk and returns a view
// that is valid until the script cache is flushed or the process exits.
std::string_view StoreLuaScript(std::string_view sha,
                                std::string_view bytecode);

// Each worker keeps a persistent Lua VM and one registry function per SHA.
// The bytecode view refers to StoreLuaScript-owned immutable storage.
bool CacheLuaScriptLocally(std::string_view sha, std::string_view bytecode);
std::optional<std::string_view> FindCachedLuaScript(std::string_view sha);
void ClearLocalLuaScriptCache();
void ClearStoredLuaScripts();

}  // namespace keylane
