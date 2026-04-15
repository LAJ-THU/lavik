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

#include "celer/redis/db.h"

#include <charconv>
#include <limits>
#include <string>

namespace celer::redis {

void DbShard::Set(std::string key, std::string value) {
  strings_.insert_or_assign(std::move(key), StringValue{std::move(value), std::nullopt});
}

const StringValue* DbShard::Get(std::string_view key) const {
  auto it = strings_.find(std::string(key));
  if (it == strings_.end()) {
    return nullptr;
  }
  return &it->second;
}

bool DbShard::Delete(std::string_view key) {
  return strings_.erase(std::string(key)) > 0;
}

bool DbShard::Exists(std::string_view key) const {
  return strings_.find(std::string(key)) != strings_.end();
}

bool DbShard::Increment(std::string_view key, std::int64_t* value) {
  auto [it, inserted] =
      strings_.try_emplace(std::string(key), StringValue{"0", std::nullopt});
  (void)inserted;

  std::int64_t current = 0;
  const std::string_view current_view = it->second.data;
  auto [ptr, ec] = std::from_chars(current_view.data(), current_view.data() + current_view.size(),
                                   current);
  if (ec != std::errc{} || ptr != current_view.data() + current_view.size()) {
    return false;
  }
  if (current == std::numeric_limits<std::int64_t>::max()) {
    return false;
  }

  ++current;
  it->second.data = std::to_string(current);
  if (value != nullptr) {
    *value = current;
  }
  return true;
}

}  // namespace celer::redis
