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

#include "../src/storage/engine/device_affinity.h"

#include <gtest/gtest.h>

namespace lavik::storage {
namespace {

TEST(DeviceAffinityTest, RejectsInsufficientQpairCapacity) {
  const std::vector<ControllerAffinityInput> controllers{
      {.id_ = "0000:01:00.0", .foreground_weight_ = 10, .io_qpair_count_ = 2},
      {.id_ = "0000:02:00.0", .foreground_weight_ = 10, .io_qpair_count_ = 1},
  };
  auto plan = PlanControllerAffinity(controllers, 4);
  ASSERT_FALSE(plan.ok());
  EXPECT_EQ(plan.status().code(), absl::StatusCode::kResourceExhausted);
}

TEST(DeviceAffinityTest, SingleControllerCoversEveryWorker) {
  const std::vector<ControllerAffinityInput> controllers{
      {.id_ = "0000:01:00.0", .foreground_weight_ = 10, .io_qpair_count_ = 4},
  };
  auto plan = PlanControllerAffinity(controllers, 4);
  ASSERT_TRUE(plan.ok()) << plan.status();
  EXPECT_EQ(plan->controller_quotas_, (std::vector<unsigned>{4}));
  for (const auto& owned : plan->worker_controllers_) {
    EXPECT_EQ(owned, (std::vector<std::size_t>{0}));
  }
}

TEST(DeviceAffinityTest, AllocatesWorkersByCapacityWeightAndQpairCap) {
  const std::vector<ControllerAffinityInput> controllers{
      {.id_ = "0000:01:00.0", .foreground_weight_ = 1, .io_qpair_count_ = 4},
      {.id_ = "0000:02:00.0", .foreground_weight_ = 3, .io_qpair_count_ = 4},
  };
  auto plan = PlanControllerAffinity(controllers, 4);
  ASSERT_TRUE(plan.ok()) << plan.status();
  EXPECT_EQ(plan->controller_quotas_, (std::vector<unsigned>{1, 3}));
  for (const auto& owned : plan->worker_controllers_) {
    EXPECT_EQ(owned.size(), 1);
  }

  const std::vector<ControllerAffinityInput> capped{
      {.id_ = "0000:01:00.0", .foreground_weight_ = 100, .io_qpair_count_ = 1},
      {.id_ = "0000:02:00.0", .foreground_weight_ = 1, .io_qpair_count_ = 3},
  };
  plan = PlanControllerAffinity(capped, 4);
  ASSERT_TRUE(plan.ok()) << plan.status();
  EXPECT_EQ(plan->controller_quotas_, (std::vector<unsigned>{1, 3}));
}

TEST(DeviceAffinityTest, AssignsEveryControllerWhenControllersExceedWorkers) {
  const std::vector<ControllerAffinityInput> controllers{
      {.id_ = "0000:01:00.0", .foreground_weight_ = 10, .io_qpair_count_ = 1},
      {.id_ = "0000:02:00.0", .foreground_weight_ = 5, .io_qpair_count_ = 1},
      {.id_ = "0000:03:00.0", .foreground_weight_ = 1, .io_qpair_count_ = 1},
  };
  auto plan = PlanControllerAffinity(controllers, 2);
  ASSERT_TRUE(plan.ok()) << plan.status();
  EXPECT_EQ(plan->worker_controllers_[0], (std::vector<std::size_t>{0}));
  EXPECT_EQ(plan->worker_controllers_[1], (std::vector<std::size_t>{1, 2}));
}

}  // namespace
}  // namespace lavik::storage
