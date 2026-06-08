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

#include <cstdint>
#include <string>

#include "keylane/CLI11.hpp"
#include "keylane/server.h"

int main(int argc, char** argv) {
  CLI::App app{"keylane — high-performance Redis-compatible storage"};

  std::string bind_ip = "127.0.0.1";
  std::uint16_t port = 6379;
  unsigned threads = 1;
  int idle_timeout_ms = -1;

  app.add_option("-b,--bind", bind_ip, "Bind address")->capture_default_str();
  app.add_option("-p,--port", port, "Listen port")->capture_default_str();
  app.add_option("-t,--threads", threads, "Worker thread count")
      ->capture_default_str()
      ->check(CLI::PositiveNumber);
  app.add_option("-i,--idle-timeout", idle_timeout_ms, "Idle timeout in ms (-1 = disabled)")
      ->capture_default_str();

  try {
    app.parse(argc, argv);
  } catch (const CLI::ParseError& e) {
    return app.exit(e);
  }

  return keylane::RunServer(bind_ip, port, threads, idle_timeout_ms);
}
