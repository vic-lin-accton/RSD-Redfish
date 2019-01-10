/*!
 * @brief Unit tests for generation of UUIDv5
 *
 * @copyright
 * Copyright (c) 2015-2017 Intel Corporation
 *
 * @copyright
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * @copyright
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * @copyright
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @file chassis_tree_stabilizer_test.cpp
 * */

#include "gtest/gtest.h"
#include <acc_bal_api_dist_helper/acc_bal_api_dist_helper.hpp>
using namespace acc_bal_api_dist_helper;
using namespace std;
#include "json/json.hpp"
#include <string>

class TestClass1 : public ::testing::Test {
public:
    virtual ~TestClass1();

    virtual void SetUp();

    virtual void TearDown();
};

TestClass1::~TestClass1() 
{ 
    printf("TestClass1-deconstructor\r\n");

}

void TestClass1::SetUp() 
{
    printf("acc_bal_api_dist_helper TESTING BEGIN /////////////////////\r\n");
    json::Value rpon(json::Value::Type::OBJECT);
    json::Value rnni(json::Value::Type::OBJECT);

    while (1)
    {
        auto& pOLT = Olt_Device::Olt_Device::get_instance();
        if(pOLT.is_bal_init())
        {
        printf("get port 1 status !\r\n");
        rpon = pOLT.get_port_statistic(1);
            usleep(1000);
            printf("get port 19 status !\r\n");
        rpon = pOLT.get_port_statistic(19);
            usleep(1000);
    }
        else
        {
            printf("get_instance error\r\n");
    pOLT.cleanup();
        }
    }
    printf("acc_bal_api_dist_helper TESTING END   /////////////////////\r\n");
}


void TestClass1::TearDown() 
{
    printf("TestClass1-TearDown\r\n");

}


TEST_F(TestClass1, Test_Memo_1) 
{
    // TEST 1 Content here //
    printf("TestClass1-content\r\n");
}
