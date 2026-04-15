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
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

namespace celer::redis {

struct StringValue {
  std::string data;
  std::optional<std::int64_t> expire_at_ms;
};

class DbShard {
 public:
  void Set(std::string key, std::string value);
  const StringValue* Get(std::string_view key) const;
  bool Delete(std::string_view key);
  bool Exists(std::string_view key) const;
  bool Increment(std::string_view key, std::int64_t* value);

 private:
  std::unordered_map<std::string, StringValue> strings_;
};

}  // namespace celer::redis
