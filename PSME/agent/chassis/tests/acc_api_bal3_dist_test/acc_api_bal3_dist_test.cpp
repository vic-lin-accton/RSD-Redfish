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
#include <acc_bal3_api_dist_helper/acc_bal3_api_dist_helper.hpp>
using namespace acc_bal3_api_dist_helper ;
using namespace std;
#include <json/json.h>
#include <string>
#include <ostream>
#include <iostream>
#include <fstream>
#include <istream>
#include "omci_exam.hpp"

typedef enum
{
    BCMOLT_ACTION_CMD_ID_NONE = 0,
    BCMOLT_ACTION_CMD_ID_ADD_OUTER_TAG = 0x0001,
    BCMOLT_ACTION_CMD_ID_REMOVE_OUTER_TAG = 0x0002,
    BCMOLT_ACTION_CMD_ID_XLATE_OUTER_TAG = 0x0004,
    BCMOLT_ACTION_CMD_ID_ADD_INNER_TAG = 0x0010,
    BCMOLT_ACTION_CMD_ID_REMOVE_INNER_TAG = 0x0020,
    BCMOLT_ACTION_CMD_ID_XLATE_INNER_TAG = 0x0040,
    BCMOLT_ACTION_CMD_ID_REMARK_OUTER_PBITS = 0x0100,
    BCMOLT_ACTION_CMD_ID_REMARK_INNER_PBITS = 0x0200,
} bcmolt_action_cmd_id;

typedef enum
{
    BCMOLT_ACTION_ID__BEGIN = 0,
    BCMOLT_ACTION_ID_CMDS_BITMASK = 0,
    BCMOLT_ACTION_ID_O_VID = 1,
    BCMOLT_ACTION_ID_O_PBITS = 2,
    BCMOLT_ACTION_ID_I_VID = 3,
    BCMOLT_ACTION_ID_I_PBITS = 4,
    BCMOLT_ACTION_ID__NUM_OF,

} bcmolt_action_id;

typedef enum
{
    BCMOLT_CLASSIFIER_ID__BEGIN = 0,
    BCMOLT_CLASSIFIER_ID_O_VID = 0,
    BCMOLT_CLASSIFIER_ID_I_VID = 1,
    BCMOLT_CLASSIFIER_ID_O_PBITS = 2,
    BCMOLT_CLASSIFIER_ID_I_PBITS = 3,
    BCMOLT_CLASSIFIER_ID_ETHER_TYPE = 4,
    BCMOLT_CLASSIFIER_ID_DST_MAC = 5,
    BCMOLT_CLASSIFIER_ID_SRC_MAC = 6,
    BCMOLT_CLASSIFIER_ID_IP_PROTO = 7,
    BCMOLT_CLASSIFIER_ID_DST_IP = 8,
    BCMOLT_CLASSIFIER_ID_SRC_IP = 9,
    BCMOLT_CLASSIFIER_ID_SRC_PORT = 10,
    BCMOLT_CLASSIFIER_ID_DST_PORT = 11,
    BCMOLT_CLASSIFIER_ID_PKT_TAG_TYPE = 12,
    BCMOLT_CLASSIFIER_ID_CLASSIFIER_BITMAP = 13,
    BCMOLT_CLASSIFIER_ID__NUM_OF,
} bcmolt_classifier_id;

struct sFLOW
{
    int interface_id;
    int onu_id; 
    int flow_id;
    char * flow_type;
    char * packet_tag_type;	
    int gemport_id;	
    int network_interface_id;

    bcmolt_action_cmd_id acton_cmd;
    bcmolt_action_id         action; 
    action_val                  action_val_a_val;

    bcmolt_classifier_id classifier;
    class_val class_val_c_val;	

};

static struct sFLOW a_Flow[] =
{
    {
        .interface_id         = 0,
        .onu_id               = 1,
        .flow_id              = 16,
        .flow_type            = "upstream",
        .packet_tag_type      = "single_tag",
//voltha1.4        .gemport_id           = 1032,
        .gemport_id           = 1024,
        .network_interface_id = 0,
        .acton_cmd            = BCMOLT_ACTION_CMD_ID_ADD_OUTER_TAG,    
        .action               = BCMOLT_ACTION_ID_O_VID, 
        {
            .o_vid      = 10,
            .o_pbits    = 0,
            .o_tpid     = 0,
            .i_vid      = 0,
            .i_pbits    = 0,
            .i_tpid     = 0,
            .ether_type = 0,
            .ip_proto   = 0,
            .src_port   = 0,
            .dst_port   = 0,
        },
        .classifier= BCMOLT_CLASSIFIER_ID_O_VID,
        {
            .o_vid      = 20,
            .o_pbits    = 0,
            .o_tpid     = 0,
            .i_vid      = 0,
            .i_pbits    = 0,
            .i_tpid     = 0,
            .ether_type = 0,
            .ip_proto   = 0,
            .src_port   = 0,
            .dst_port   = 0,
        }
    }
    ,
        {
            .interface_id         = 0,
            .onu_id               = 1,
            .flow_id              = 16,
            .flow_type            = "downstream",
            .packet_tag_type      = "double_tag",
////voltha1.4               .gemport_id           = 1032,
            .gemport_id           = 1024,
            .network_interface_id = 0,
            .acton_cmd            = BCMOLT_ACTION_CMD_ID_REMOVE_OUTER_TAG,    
            .action               = BCMOLT_ACTION_ID_O_VID, 
            {
                .o_vid      = 10,
                .o_pbits    = 0,
                .o_tpid     = 0,
                .i_vid      = 0,
                .i_pbits    = 0,
                .i_tpid     = 0,
                .ether_type = 0,
                .ip_proto   = 0,
                .src_port   = 0,
                .dst_port   = 0,
            },
            .classifier= BCMOLT_CLASSIFIER_ID_O_VID,
            {
                .o_vid      = 10,
                .o_pbits    = 0,
                .o_tpid     = 0,
                .i_vid      = 20,
                .i_pbits    = 0,
                .i_tpid     = 0,
                .ether_type = 0,
                .ip_proto   = 0,
                .src_port   = 0,
                .dst_port   = 0,
            }
        }	
};


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
    printf("////////////acc_bal3_api_dist_helper TESTING BEGIN \n");

    static constexpr char ONU_CFG_NAME[] = "onu_cfg";
    ifstream    m_source_files= {};
    std::string t_path = ONU_CFG_NAME;
    int Onu_count = 0; 
    int SleepSec  = 0; 
    Json::Value onus;	  
    Json::Value onu_cfg;	  
    std::string TestMode = "SM"; 
    //SM : (Simple Mode) 
    //RM : (Rich Mode)
    m_source_files.open(t_path);

    if(m_source_files.good())
    {
        Json::Reader reader;
        bool isJsonOK = (reader.parse(m_source_files, onu_cfg));

        if(isJsonOK)
        {
            onus = onu_cfg["ONUs"];
            TestMode = onu_cfg["Mode"].asString();
            SleepSec = onu_cfg["Sleep"].asInt();
            Onu_count = onus.size(); 
            printf("////////////num[%d]\r\n", Onu_count);
        }
        else
        {
            printf("////////////Get onu_cfg NG, Check JSON format !!!\r\n");
            return;			  
        }
    }
    else
    {
        printf("////////////Open onu_cfg NG\r\n" );
        return;		  
    }

    //Step 1. Connect to managemnet deamon //
    auto& OLT = Olt_Device::Olt_Device::get_instance();

    int pon_if_max = OLT.get_max_pon_num();
    int i = 0,count=0;

    printf("//////////// PON interface num[%d]  !!////////////\r\n", pon_if_max);

    //Wait BAL Ready !! 
    while(!OLT.get_bal_status())
    {
        sleep(1);
        printf("////////////Wait BAL Ready[%d] seconds\r\n",count);
        count++;
    }

    //Wait OLT Ready !! 
    while(!OLT.get_olt_status())
    {
        sleep(1);
        printf("////////////Wait OLT Ready[%d] seconds\r\n",count);
        count++;
    }

    //Step 2. enable pon port//
    for(i = 0; i < pon_if_max ; i++)
    {
        OLT.enable_pon_if(i);
        usleep(300000);
    }

    //Step 3. enable nni port//
    int nni_if_max = OLT.get_max_nni_num();
    //nni_if_max = 1; 
    printf("//////////// NNI interface num[%d]  !!////////////\r\n", nni_if_max);
    for(i = 0; i < nni_if_max ; i++)
    {
        OLT.enable_nni_if(i);
        usleep(300000);
    }

    //Step 4. Enable ONU//
    int jj=0;
    int gem_id = 0;
    int flow_id = 0;

    for(jj = 0 ; jj< Onu_count; jj++)
    { 
        printf("////////////onu_cfg onu_id                 [%d]\r\n", onus[jj]["onu_id"].asInt());
        printf("////////////onu_cfg olt pon interface_id   [%d]\r\n", onus[jj]["interface_id"].asInt());
        printf("////////////onu_cfg onu vendor_id          [%s]\r\n", onus[jj]["vendor_id"].asString().c_str());
        printf("////////////onu_cfg onu vendor_specific id [%s]\r\n", onus[jj]["vendor_specific"].asString().c_str());			  

        int onu_id = onus[jj]["onu_id"].asInt(); 	
        int pon_id = onus[jj]["interface_id"].asInt();
        int nni_id = onus[jj]["nni_id"].asInt();
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

        OLT.activate_onu(pon_id, onu_id, s_vendor_id.c_str(), cs_vendor_spec);

        { 
            int aFlow_size = (sizeof (a_Flow) / sizeof (a_Flow[0]));
            int ii = 0;
            for (ii = 0 ; ii < aFlow_size ; ii++)
            {
                if( TestMode == "RM" )
                {
                    //RM mode
                    //gem_id = a_Flow[ii].gemport_id + jj;
                    gem_id = onu_id + MAG_BASE_VAL; 
                    flow_id = a_Flow[ii].flow_id + jj;
                }
                else
                {   //SM mode
                    //gem_id = a_Flow[ii].gemport_id;
                    gem_id = onu_id + MAG_BASE_VAL; 
                    flow_id = a_Flow[ii].flow_id;
                }
                
                std::string sft(a_Flow[ii].flow_type); //Flow type //
                std::string sptt(a_Flow[ii].packet_tag_type); //pack tag type //

                OLT.flow_add(
                        onu_id, flow_id , sft  , sptt, pon_id , 
                        nni_id, gem_id, a_Flow[ii].classifier, 
                        a_Flow[ii].action , a_Flow[ii].acton_cmd , a_Flow[ii].action_val_a_val, a_Flow[ii].class_val_c_val);                    
            }
        }
        sleep(5);

        int ij = 0;
#if 0
        //VOLTAH 1.4
        int aOMCI_size = (sizeof (a_OMCI) / sizeof (a_OMCI[0]));
        for (ij = 0 ; ij < aOMCI_size ; ij++)
        {
            printf("a[%d] ", ij);
            OLT.omci_msg_out(pon_id, onu_id, a_OMCI[ij].raw_omci);
            usleep(300000);
        }

#else
//VOLTAH 1.7
//Step 1 for EPOA packet
        int bOMCI_size = (sizeof (b_OMCI_1) / sizeof (b_OMCI_1[0]));
        ij = 0;
        for (ij = 0 ; ij < bOMCI_size ; ij++)
        {
            printf("b1[%d] ", ij);
                if ((ij == 31) || (ij == 32))
                {
                    char m_raw_omci[256] = {0x0};
                    char ID[8] = {0x0};
                    sprintf(ID, "%04X", gem_id);
                    memcpy(m_raw_omci, b_OMCI_1[ij].raw_omci, 90);
                    if (ij == 31)
                    {
                        printf("Repalce Alloc ID = 0x%04X\r\n", gem_id);
                        memcpy(&m_raw_omci[20], ID, 4);
                    }
                    if (ij == 32)
                    {
                        printf("Repalce GEM ID = 0x%04X\r\n", gem_id);
                        memcpy(&m_raw_omci[16], ID, 4);
                    }
                    OLT.omci_msg_out(pon_id, onu_id, m_raw_omci);
            }
            else
            OLT.omci_msg_out(pon_id, onu_id, b_OMCI_1[ij].raw_omci);
            usleep(300000);
        }
//Step 2 for vlan action

        bOMCI_size = (sizeof (b_OMCI_2) / sizeof (b_OMCI_2[0]));
        ij = 0;
        for (ij = 0 ; ij < bOMCI_size ; ij++)
        {
            printf("b2[%d] ", ij);
            OLT.omci_msg_out(pon_id, onu_id, b_OMCI_2[ij].raw_omci);
            usleep(300000);
        }
#endif
        if(Onu_count > 1 )
        {
            if( TestMode == "SM" )
            {
            do 
            {
                printf("PRESS ANY KEY to NEXT ONU TEST\r\n");
            } while (cin.get() != '\n');
            printf("Go...\r\n");

            int Flow_size = (sizeof (a_Flow) / sizeof (a_Flow[0]));
            printf("Flow size[%d]\r\n", Flow_size);
            int j = 0;

            for (j = 0 ; j< Flow_size ; j++)
            {
                std::string sft(a_Flow[j].flow_type); //Flow type //
                printf("Remove flow %d id %s\r\n", a_Flow[j].flow_id , sft.c_str());
                OLT.flow_remove(a_Flow[j].flow_id, sft);
                usleep(300000);
            }
        }
            else
            {
                //If RM mode we will add all flows at one time //
                printf("Wait %d seconds\r\n",SleepSec);
                usleep(SleepSec*1000000);
                printf("Ranging....Next......\r\n");
            }
        }
    }
    //char cs_vendor_id[4] = {0x49,0x53,0x4B,0x54};//"ISKT"
    //char cs_vendor_spec[4] = {0x71,0xE8,0x01,0x10};
    OLT.enable_cli();
    OLT.enter_cmd_shell();
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
