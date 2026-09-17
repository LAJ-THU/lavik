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

#include "lua_eval.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

namespace {

class LuaCatalogReset {
 public:
  ~LuaCatalogReset() {
    lavik::AbortStagedLuaFunctionCatalogLocally();
    const std::vector<std::string> empty;
    if (lavik::StageCompleteLuaFunctionCatalogLocally(empty).ok()) {
      lavik::CommitStagedLuaFunctionCatalogLocally();
    }
  }
};

TEST(LuaEvalTest, CatalogSwapKeepsSuspendedExecutionRuntimeAlive) {
  LuaCatalogReset reset;
  const std::vector<std::string> first_catalog{
      "#!lua name=runtime_owner\n"
      "redis.register_function{function_name='runtime_owner_value', "
      "callback=function(keys, args) return redis.call('PING') end, "
      "flags={'no-writes'}}"};
  ASSERT_TRUE(
      lavik::StageCompleteLuaFunctionCatalogLocally(first_catalog).ok());
  lavik::CommitStagedLuaFunctionCatalogLocally();

  const std::vector<std::string> no_arguments;
  auto old_execution = lavik::LuaExecution::CreateFunction(
      "runtime_owner_value", no_arguments, no_arguments);
  ASSERT_TRUE(old_execution.ok());
  lavik::LuaExecutionStep old_step = (*old_execution)->Start(false);
  ASSERT_TRUE(old_step.call_.has_value());
  EXPECT_EQ(old_step.call_->args_, (std::vector<std::string>{"PING"}));

  const std::vector<std::string> replacement_catalog{
      "#!lua name=runtime_owner\n"
      "redis.register_function{function_name='runtime_owner_value', "
      "callback=function(keys, args) return 'new' end, "
      "flags={'no-writes'}}"};
  ASSERT_TRUE(
      lavik::StageCompleteLuaFunctionCatalogLocally(replacement_catalog).ok());
  lavik::CommitStagedLuaFunctionCatalogLocally();

  old_step = (*old_execution)->Resume("+PONG\r\n");
  EXPECT_FALSE(old_step.call_.has_value());
  EXPECT_EQ(old_step.reply_, "+PONG\r\n");

  auto new_execution = lavik::LuaExecution::CreateFunction(
      "runtime_owner_value", no_arguments, no_arguments);
  ASSERT_TRUE(new_execution.ok());
  const lavik::LuaExecutionStep new_step = (*new_execution)->Start(false);
  EXPECT_FALSE(new_step.call_.has_value());
  EXPECT_EQ(new_step.reply_, "$3\r\nnew\r\n");
}

}  // namespace
