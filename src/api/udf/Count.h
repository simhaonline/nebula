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

#pragma once

#include "CommonUDAF.h"

/**
 * Define expressions used in the nebula DSL.
 */
namespace nebula {
namespace api {
namespace udf {

// UDAF - count
template <nebula::type::Kind KIND>
class Count : public CommonUDAF<KIND> {
  using NativeType = typename nebula::type::TypeTraits<KIND>::CppType;
  static constexpr NativeType S_VALUE = NativeType();

public:
  // for count, we don't need evaluate inner expr
  // unless it's distinct a column, so we can safely replace it with a const expression with value 0
  Count(const std::string& name, std::unique_ptr<nebula::execution::eval::ValueEval>)
    : CommonUDAF<KIND>(name,
                       nebula::execution::eval::constant(1),
                       // block aggregate - count each item
                       [](NativeType ov, NativeType) {
                         return ov + 1;
                       },
                       // partial aggregate - sum each count results
                       [](NativeType ov, NativeType nv) {
                         return ov + nv;
                       }) {}
  virtual ~Count() = default;

  // Override run method to not evaluate the field value since we don't need the value itself
  inline virtual NativeType run(nebula::execution::eval::EvalContext&, bool&) const override {
    return S_VALUE;
  }
};

template <>
Count<nebula::type::Kind::VARCHAR>::Count(const std::string& name, std::unique_ptr<nebula::execution::eval::ValueEval> expr);

} // namespace udf
} // namespace api
} // namespace nebula