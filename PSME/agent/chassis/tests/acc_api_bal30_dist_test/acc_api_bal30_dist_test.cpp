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
#include <acc_bal30_api_dist_helper/acc_bal30_api_dist_helper.hpp>
using namespace acc_bal30_api_dist_helper ;
using namespace std;
#include <json/json.h>
#include <string>
#include <ostream>
#include <iostream>
#include <fstream>
#include <istream>

class TestClass1 : public ::testing::Test 
{
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
    printf("acc_bal30_api_dist_helper TESTING BEGIN /////////////////////\r\n");

    static constexpr char ONU_CFG_NAME[] = "onu_cfg";
    ifstream    m_source_files= {};
    std::string t_path = ONU_CFG_NAME;
    int Onu_count = 0; 
    Json::Value onus;	  
    Json::Value onu_cfg;	  

    m_source_files.open(t_path);

    if(m_source_files.good())
    {
        Json::Reader reader;
        bool isJsonOK = (reader.parse(m_source_files, onu_cfg));

        if(isJsonOK)
        {
            onus = onu_cfg["ONUs"];
            Onu_count = onus.size(); 
            printf("Get onu_cfg OK \r\n");

        }
        else
        {
            printf("Get onu_cfg NG, Check JSON format !!!\r\n");
            return;			  
        }
    }
    else
    {
        printf("Open onu_cfg NG\r\n" );
        return;		  
    }

    //Step 1. Connect to managemnet deamon //
    auto& OLT = Olt_Device::Olt_Device::get_instance();

    int pon_if_max = OLT.get_max_pon_num();
    int i = 0,count=0;

    printf("//////////// PON interface num[%d]  !!////////////\r\n", pon_if_max);
    while(!OLT.get_olt_status())
    {
        sleep(1);
        printf("[%d]\r\n",count);
        count++;
    }
    sleep(5);

    //Step 2. enable pon port//
    for(i = 0; i < pon_if_max ; i++)
    {
        printf("////////////Enable PON interface [%d] to UP !!////////////\r\n", i);
        OLT.enable_pon_if_(i);
    }

    int jj=0;

    for(jj = 0 ; jj< Onu_count; jj++)
    { 
        printf("onu_cfg onu_id                        [%d]\r\n", onus[jj]["onu_id"].asInt());
        printf("onu_cfg olt pon interface_id      [%d]\r\n", onus[jj]["interface_id"].asInt());
        printf("onu_cfg onu vendor_id             [%s]\r\n", onus[jj]["vendor_id"].asString().c_str());
        printf("onu_cfg onu vendor_specific id [%s]\r\n", onus[jj]["vendor_specific"].asString().c_str());			  

        int onu_id = onus[jj]["onu_id"].asInt(); 	
        int interface_id = onus[jj]["interface_id"].asInt();
        std::string s_vendor_id = onus[jj]["vendor_id"].asString();
        std::string s_vendor_spec = onus[jj]["vendor_specific"].asString();

        long unsigned int buflen = s_vendor_spec.size();
        char cs_vendor_spec[8] = {0x0};
        uint16_t idx1 = 0;
        uint16_t idx2 = 0;
        char str1[20]= {0x0};;
        char str2[20]= {0x0};;
        memset(&cs_vendor_spec, 0, buflen);

        for (idx1=0,idx2=0; idx1< buflen ; idx1++,idx2++) 
        {
            sprintf(str1,"%c", s_vendor_spec[idx1]);
            sprintf(str2,"%c", s_vendor_spec[++idx1]);
            strcat(str1,str2);
            cs_vendor_spec[idx2] = (char) strtol(str1, NULL, 16);
        }

        printf("////////////Active ONU[%s][0x%02X][0x%02X][0x%02X][0x%02X] !!////////////\r\n", 
        s_vendor_id.c_str(), cs_vendor_spec[0],cs_vendor_spec[1],cs_vendor_spec[2],cs_vendor_spec[3]);

        OLT.activate_onu(interface_id, onu_id, s_vendor_id.c_str(), cs_vendor_spec);
    }
 

    //char cs_vendor_id[4] = {0x49,0x53,0x4B,0x54};//"ISKT"
    //char cs_vendor_spec[4] = {0x71,0xE8,0x01,0x10};
    //sleep(5);

    OLT.enter_cmd_shell();
    return 1;
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
