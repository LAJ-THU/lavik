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

#include "keylane/storage/tx_cleaner.h"

#include "gtest/gtest.h"

namespace keylane::storage::internal {
namespace {

TEST(TxCleanerTest, ReclaimsOnlyACompletelySettledGeneration) {
  TxGenerationReadiness ready{.sealed_and_durable_ = true};
  EXPECT_TRUE(CanReclaimTxGeneration(ready));

  ready.active_transactions_ = 1;
  EXPECT_FALSE(CanReclaimTxGeneration(ready));
  ready.active_transactions_ = 0;
  ready.live_tagged_bytes_ = 1;
  EXPECT_FALSE(CanReclaimTxGeneration(ready));
  ready.live_tagged_bytes_ = 0;
  ready.dependency_pins_ = 1;
  EXPECT_FALSE(CanReclaimTxGeneration(ready));
  ready.dependency_pins_ = 0;
  ready.sealed_and_durable_ = false;
  EXPECT_FALSE(CanReclaimTxGeneration(ready));
}

}  // namespace
}  // namespace keylane::storage::internal
