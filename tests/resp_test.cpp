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

#include "keylane/resp.h"

#include <gtest/gtest.h>

#include <string_view>

namespace keylane {
namespace {

TEST(ReplyBuilderTest, EncodesScalarAndCompositeReplies) {
  ReplyBuilder builder;

  EXPECT_EQ(builder.AppendSimpleString("OK"), "+OK\r\n");
  builder.Reset();
  EXPECT_EQ(builder.AppendInteger(-42), ":-42\r\n");
  builder.Reset();
  EXPECT_EQ(builder.AppendError("ERR failed"), "-ERR failed\r\n");
  builder.Reset();
  EXPECT_EQ(builder.AppendError("ERR ", "failed"), "-ERR failed\r\n");
  builder.Reset();

  const char binary[] = {'a', '\0', 'b'};
  EXPECT_EQ(builder.AppendBulkString(std::string_view(binary, sizeof(binary))),
            std::string_view("$3\r\na\0b\r\n", 9));

  builder.Reset();
  builder.AppendArrayHeader(2);
  builder.AppendBulkString("first");
  builder.AppendNullBulkString();
  EXPECT_EQ(builder.View(), "*2\r\n$5\r\nfirst\r\n$-1\r\n");
}

TEST(ReplyBuilderTest, ReusesBoundedCapacityAndReleasesOversizedBuffer) {
  ReplyBuilder builder;
  builder.Reserve(4096);
  const std::size_t retained_capacity = builder.Capacity();
  builder.AppendError("ERR failed");
  builder.Reset();
  EXPECT_EQ(builder.Capacity(), retained_capacity);

  builder.Reserve(128 * 1024);
  ASSERT_GT(builder.Capacity(), 64 * 1024);
  builder.Reset();
  EXPECT_LE(builder.Capacity(), 64 * 1024);
}

}  // namespace
}  // namespace keylane
