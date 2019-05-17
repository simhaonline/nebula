/*
 * Copyright 2017-present Shawn Cao
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <fmt/format.h>
#include <glog/logging.h>
#include <gtest/gtest.h>
#include "execution/io/trends/Pins.h"

namespace nebula {
namespace execution {
namespace test {
TEST(ExecutionTest, TestOperator) {
  LOG(INFO) << "Execution provides physical executuin units at any stage";
}

TEST(ExecutionTest, TestLoadPins) {
  nebula::execution::io::trends::PinsTable pins;
  pins.load(1);
}
} // namespace test
} // namespace execution
} // namespace nebula