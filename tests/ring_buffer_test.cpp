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

#include "../src/storage/ring_buffer.h"

#include <gtest/gtest.h>

#include <memory>

namespace {

struct MoveOnlyValue {
  MoveOnlyValue() = default;
  explicit MoveOnlyValue(int value) : value_(std::make_unique<int>(value)) {}
  MoveOnlyValue(MoveOnlyValue&&) noexcept = default;
  MoveOnlyValue& operator=(MoveOnlyValue&&) noexcept = default;
  MoveOnlyValue(const MoveOnlyValue&) = delete;
  MoveOnlyValue& operator=(const MoveOnlyValue&) = delete;

  std::unique_ptr<int> value_;
};

TEST(RingBufferTest, PreservesFifoAcrossWrapAndResize) {
  keylane::storage::RingBuffer<MoveOnlyValue> buffer;
  EXPECT_EQ(buffer.growth_bytes_if_push(), 64 * sizeof(MoveOnlyValue));

  for (int value = 0; value < 64; ++value) {
    buffer.push_back(MoveOnlyValue(value));
  }
  EXPECT_EQ(buffer.capacity(), 64);
  EXPECT_EQ(buffer.growth_bytes_if_push(), 128 * sizeof(MoveOnlyValue));
  for (int value = 0; value < 48; ++value) {
    ASSERT_NE(buffer.front().value_, nullptr);
    EXPECT_EQ(*buffer.front().value_, value);
    buffer.pop_front();
  }

  // Wrap through slot zero, then exceed the old capacity. Resize must move
  // the wrapped logical sequence into the replacement buffer in FIFO order.
  for (int value = 64; value < 129; ++value) {
    buffer.push_back(MoveOnlyValue(value));
  }
  EXPECT_EQ(buffer.capacity(), 128);
  ASSERT_EQ(buffer.size(), 81);
  for (int value = 48; value < 129; ++value) {
    ASSERT_NE(buffer.front().value_, nullptr);
    EXPECT_EQ(*buffer.front().value_, value);
    buffer.pop_front();
  }
  EXPECT_TRUE(buffer.empty());
  EXPECT_EQ(buffer.capacity(), 128);
}

TEST(RingBufferTest, ClearReleasesContentsAndRetainsCapacity) {
  keylane::storage::RingBuffer<MoveOnlyValue> buffer;
  for (int value = 0; value < 65; ++value) {
    buffer.push_back(MoveOnlyValue(value));
  }
  ASSERT_EQ(buffer.capacity(), 128);
  buffer.clear();
  EXPECT_TRUE(buffer.empty());
  EXPECT_EQ(buffer.capacity(), 128);

  buffer.push_back(MoveOnlyValue(7));
  ASSERT_NE(buffer.front().value_, nullptr);
  EXPECT_EQ(*buffer.front().value_, 7);
}

TEST(RingBufferTest, PreparedPushUsesReservedSlotWithoutGrowth) {
  keylane::storage::RingBuffer<MoveOnlyValue> buffer;
  buffer.PrepareCapacity(65);
  ASSERT_EQ(buffer.capacity(), 128);
  const std::size_t prepared_capacity = buffer.capacity();

  for (int value = 0; value < 65; ++value) {
    buffer.push_back_prepared(MoveOnlyValue(value));
  }

  EXPECT_EQ(buffer.size(), 65);
  EXPECT_EQ(buffer.capacity(), prepared_capacity);
  for (int value = 0; value < 65; ++value) {
    ASSERT_NE(buffer.front().value_, nullptr);
    EXPECT_EQ(*buffer.front().value_, value);
    buffer.pop_front();
  }
}

}  // namespace
