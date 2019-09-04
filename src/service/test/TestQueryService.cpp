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
#include "execution/core/NodeConnector.h"
#include "execution/core/ServerExecutor.h"
#include "execution/io/trends/Pins.h"
#include "execution/io/trends/Trends.h"
#include "service/base/NebulaService.h"
#include "service/server/QueryHandler.h"
#include "surface/DataSurface.h"
#include "surface/MockSurface.h"
#include "type/Serde.h"

namespace nebula {
namespace service {
namespace test {

using namespace nebula::api::dsl;
using nebula::common::Cursor;
using nebula::common::Evidence;
using nebula::execution::BlockManager;
using nebula::execution::core::NodeConnector;
using nebula::execution::core::ServerExecutor;
using nebula::execution::io::trends::PinsTable;
using nebula::execution::io::trends::TrendsTable;
using nebula::service::base::ErrorCode;
using nebula::service::base::ServiceProperties;
using nebula::service::server::QueryHandler;
using nebula::surface::RowData;
using nebula::type::Schema;
using nebula::type::TypeSerializer;

TEST(ServiceTest, TestServiceEndpoint) {
  TrendsTable trends;
  // load test data to run this query
  trends.load(10);

  //    .where(col("_time_") > 1554076800 && col("_time_") < 1556582400 && like(col("query"), "apple%"))
  //    .select(
  //      col("query"),
  //      count(col("count")).as("total"))
  //    .groupby({ 1 })
  //    .sortby({ 2 }, SortType::DESC)
  //    .limit(10);
  nebula::common::Evidence::Duration tick;
  QueryRequest request;
  request.set_table(trends.name());
  request.set_start(1554076800);
  request.set_end(1556582400);
  auto pa = request.mutable_filtera();
  {
    auto expr = pa->add_expression();
    expr->set_column("query");
    expr->set_op(Operation::LIKE);
    expr->add_value("apple%");
  }

  // set dimensions and metrics
  request.add_dimension("query");
  auto metric = request.add_metric();
  metric->set_column("count");
  metric->set_method(Rollup::COUNT);

  // set sorting and limit schema
  auto order = request.mutable_order();
  order->set_column("count");
  order->set_type(OrderType::DESC);
  request.set_top(10);

  // execute the service code
  QueryHandler handler;
  ErrorCode error = ErrorCode::NONE;
  // compile the query into a plan
  auto query = handler.build(trends, request, error);
  auto plan = handler.compile(query, { request.start(), request.end() }, error);

  // set time range constraints in execution plan directly since it should always present
  plan->display();
  EXPECT_EQ(error, ErrorCode::NONE);

  auto connector = std::make_shared<NodeConnector>();
  nebula::surface::RowCursorPtr result = handler.query(*plan, connector, error);
  EXPECT_EQ(error, ErrorCode::NONE);

  LOG(INFO) << "JSON BLOB:";
  LOG(INFO) << ServiceProperties::jsonify(result, plan->getOutputSchema());
}
TEST(ServiceTest, TestPinsData) {
  PinsTable pins;
  // load test data to run this query
  pins.load(1);
  nebula::common::Evidence::Duration tick;
  QueryRequest request;
  request.set_table(pins.name());
  request.set_start(1);
  request.set_end(2556582400);
  auto pa = request.mutable_filtera();
  {
    auto expr = pa->add_expression();
    expr->set_column("id");
    expr->set_op(Operation::EQ);
    expr->add_value("726838827343444700");
  }

  // set dimensions and metrics
  request.add_dimension("id");
  request.add_dimension("user_id");
  request.set_display(DisplayType::SAMPLES);
  request.set_top(20);

  // execute the service code
  QueryHandler handler;
  ErrorCode error = ErrorCode::NONE;
  // compile the query into a plan
  auto query = handler.build(pins, request, error);
  auto plan = handler.compile(query, { request.start(), request.end() }, error);

  // set time range constraints in execution plan directly since it should always present
  EXPECT_EQ(error, ErrorCode::NONE);
  plan->display();

  auto connector = std::make_shared<NodeConnector>();
  nebula::surface::RowCursorPtr result = handler.query(*plan, connector, error);
  EXPECT_EQ(error, ErrorCode::NONE);

  LOG(INFO) << "JSON BLOB:";
  LOG(INFO) << ServiceProperties::jsonify(result, plan->getOutputSchema());
}

} // namespace test
} // namespace service
} // namespace nebula
