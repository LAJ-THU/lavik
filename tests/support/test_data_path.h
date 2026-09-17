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

#include <cstdlib>
#include <filesystem>
#include <string>
#include <string_view>

namespace lavik::test {

// Keeps large test devices and diagnostic logs relocatable without changing
// the host's general-purpose temporary-file policy.
inline std::filesystem::path TestDataDirectory() {
  const char* configured = std::getenv("LAVIK_TEST_DATA_DIR");
  return configured != nullptr && configured[0] != '\0'
             ? std::filesystem::path(configured)
             : std::filesystem::path("/tmp");
}

// Resolves one test artifact beneath TestDataDirectory.
inline std::string TestDataPath(std::string_view filename) {
  return (TestDataDirectory() / filename).string();
}

// Supports suites that derive several sibling artifact names from one prefix.
inline std::string TestDataPathPrefix() {
  std::string prefix = TestDataDirectory().string();
  if (!prefix.ends_with(std::filesystem::path::preferred_separator)) {
    prefix.push_back(std::filesystem::path::preferred_separator);
  }
  return prefix;
}

}  // namespace lavik::test
