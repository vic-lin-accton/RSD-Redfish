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
#include <acc_onlp_helper/acc_onlp_helper.hpp>
using namespace acc_onlp_helper;
using namespace std;

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
    printf("TestClass1-SetUp-Begin InterfaceConfig TESTING/////////////////////\r\n");

    InterfaceConfig intf_ip;
    intf_ip.Restart();

    printf("TestClass1-SetUp-Begin RFLogEntry TESTING/////////////////////\r\n");	

    RFLogEntry Entry;	
	
    Entry.clean_log();	
    Entry.get_current_time();	
    Entry.get_zone_offset();
    Entry.get_max_entries_num();
    Entry.get_log_status();      
    Entry.set_log_status(true);
    Entry.set_log_status(false);

    printf("TestClass1-SetUp-End   RFLogEntry TESTING/////////////////////\r\n");

    printf("TestClass1-SetUp-Begin   Switch TESTING/////////////////////\r\n");
	
    //Switch TESTs;
    auto& sonlp = Switch::Switch::get_instance();
    int ii = 1;
    while(ii !=0)
    {
        sonlp.get_port_info();
        int port_max = sonlp.get_port_num();
    
        if(port_max == 0)
        {
            printf("You need check you have done HW_NODE_ASXXX Poring first!!!!\r\n");
            return ;
        }
        printf("Port MAX [%d]\r\n",port_max);	
    	
        for(int i = 1; i <= port_max; i++)	
        {
            printf("Port present %d [%d] \r\n", i, sonlp.get_port_info_by_(i, acc_onlp_helper::Switch::Port_Present ));
#ifdef VOLT			
            sonlp.get_port_trans_info_by_(i);
#endif
        }
		
        sonlp.get_psu_info();
        int psu_max = sonlp.get_psu_num();
    
        for(int i = 1; i <= psu_max; i++)	
        {
            printf("PSU present %d [%d] \r\n", i, sonlp.get_psu_info_by_(i, acc_onlp_helper::Switch::Psu_Present));    
            printf("PSU health %d [%s] \r\n", i, sonlp.get_psu_info_by_(i, "Status_Health").c_str());
            printf("PSU state %d [%s] \r\n", i, sonlp.get_psu_info_by_(i, "Status_State").c_str());
        }	
             
        auto& sonlp2 = acc_onlp_helper::Switch::get_instance();
        sonlp2.get_thermal_info();
    
        int thermal_max = sonlp2.get_thermal_num();
    
    
        for(int i = 1; i <= thermal_max; i++)	
        {
            printf("Thermal type %d [%d] \r\n", i, sonlp2.get_thermal_info_by_(i, acc_onlp_helper::Switch::Thermal_Type));    
            printf("Thermal health %d [%s] \r\n", i, sonlp2.get_thermal_info_by_(i, "Status_Health").c_str());
            printf("Thermal state %d [%s] \r\n", i, sonlp2.get_thermal_info_by_(i, "Status_State").c_str());
        }		
    
        auto& sonlp3 = acc_onlp_helper::Switch::get_instance();
        sonlp3.get_fan_info();
        int fan_max = sonlp2.get_fan_num();
    	
        for(int i = 1; i <= fan_max; i++)	
        {
            printf("fan type %d [%d] \r\n", i, sonlp3.get_fan_info_by_(i, acc_onlp_helper::Switch::Type));    
            printf("fan health %d [%s] \r\n", i, sonlp3.get_fan_info_by_(i, "Status_Health").c_str());
            printf("fan state %d [%s] \r\n", i, sonlp3.get_fan_info_by_(i, "Status_State").c_str());
        }	
	ii--;
    }
    printf("TestClass1-SetUp-End      Switch TESTING/////////////////////\r\n");
	
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
