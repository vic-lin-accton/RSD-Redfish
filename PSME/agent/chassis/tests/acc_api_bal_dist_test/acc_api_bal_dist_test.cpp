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
#include <json/json.h>
#include <string>
#include <ostream>
#include <iostream>
#include <fstream>
#include <istream>



enum bcmbal_action_id
{
    BCMBAL_ACTION_ID_NONE           = 0,
    BCMBAL_ACTION_ID_CMDS_BITMASK   = 0x0001,                       /**< Commands bitmask. */
    BCMBAL_ACTION_ID_O_VID          = 0x0002,                       /**< Outer vid. */
    BCMBAL_ACTION_ID_O_PBITS        = 0x0004,                       /**< Outer pbits. */
    BCMBAL_ACTION_ID_O_TPID         = 0x0008,                       /**< Outer tpid. */
    BCMBAL_ACTION_ID_I_VID          = 0x0010,                       /**< Inner vid. */
    BCMBAL_ACTION_ID_I_PBITS        = 0x0020,                       /**< Inner pbits. */
    BCMBAL_ACTION_ID_I_TPID         = 0x0040,                       /**< Inner tpid. */
    BCMBAL_ACTION_ID_ALL            = 0x007F                        /**< All fields */
} ;

enum bcmbal_action_cmd_id
{
    BCMBAL_ACTION_CMD_ID_NONE                           = 0,
    BCMBAL_ACTION_CMD_ID_XLATE_OUTER_TAG                = 0x0001,   /**< Translate outer tag. */
    BCMBAL_ACTION_CMD_ID_ADD_OUTER_TAG                  = 0x0002,   /**< Add outer tag. */
    BCMBAL_ACTION_CMD_ID_REMOVE_OUTER_TAG               = 0x0004,   /**< Remove outer tag. */
    BCMBAL_ACTION_CMD_ID_XLATE_INNER_TAG                = 0x0010,   /**< Translate inner tag. */
    BCMBAL_ACTION_CMD_ID_ADD_INNER_TAG                  = 0x0020,   /**< Add inner tag. */
    BCMBAL_ACTION_CMD_ID_REMOVE_INNER_TAG               = 0x0040,   /**< Remove inner tag. */
    BCMBAL_ACTION_CMD_ID_REMARK_OUTER_PBITS             = 0x0100,   /**< Set the outer tag pbits */
    BCMBAL_ACTION_CMD_ID_COPY_INNER_PBITS_TO_OUTER_PBITS= 0x0200,   /**< Copy the inner pbits to outer pbits */
    BCMBAL_ACTION_CMD_ID_REMARK_INNER_PBITS             = 0x1000,   /**< Set the outer tag pbits */
    BCMBAL_ACTION_CMD_ID_COPY_OUTER_PBITS_TO_INNER_PBITS= 0x2000,   /**< Copy the outer pbits to inner pbits */
    BCMBAL_ACTION_CMD_ID_DISCARD_DS_BCAST               = 0x00010000UL, /**< drop downstream broadcast packets. */
    BCMBAL_ACTION_CMD_ID_TRAP_TO_HOST                   = 0x00020000UL  /**< Not a valid action for a group object member */
};


enum bcmbal_classifier_id
{
    BCMBAL_CLASSIFIER_ID_NONE                                           = 0,
    BCMBAL_CLASSIFIER_ID_O_TPID                                         = 0x0001,   /**< Outer TPID of the packet to be classified */
    BCMBAL_CLASSIFIER_ID_O_VID                                          = 0x0002,   /**< Outer VID of the packet to be classified */
    BCMBAL_CLASSIFIER_ID_I_TPID                                         = 0x0004,   /**< Inner TPID of the packet to be classified */
    BCMBAL_CLASSIFIER_ID_I_VID                                          = 0x0008,   /**< Inner VID of the packet to be classified */
    BCMBAL_CLASSIFIER_ID_O_PBITS                                        = 0x0010,   /**< Outer PBITS of the packet to be classified */
    BCMBAL_CLASSIFIER_ID_I_PBITS                                        = 0x0020,   /**< Inner PBITS of the packet to be classified */
    BCMBAL_CLASSIFIER_ID_ETHER_TYPE                                     = 0x0040,   /**< Ethertype of the packet to be classified */
    BCMBAL_CLASSIFIER_ID_DST_MAC                                        = 0x0080,   /**< Destination MAC address of the packet to be classified */
    BCMBAL_CLASSIFIER_ID_SRC_MAC                                        = 0x0100,   /**< Source MAC address of the packet to be classified */
    BCMBAL_CLASSIFIER_ID_IP_PROTO                                       = 0x0200,   /**< IP protocol of the packet to be classified */
    BCMBAL_CLASSIFIER_ID_DST_IP                                         = 0x0400,   /**< Destination IP address of the packet to be classified */
    BCMBAL_CLASSIFIER_ID_SRC_IP                                         = 0x0800,   /**< Source IP address of the packet to be classified */
    BCMBAL_CLASSIFIER_ID_SRC_PORT                                       = 0x1000,   /**< Source port of the packet to be classified */
    BCMBAL_CLASSIFIER_ID_DST_PORT                                       = 0x2000,   /**< Destination port of the packet to be classified */
    BCMBAL_CLASSIFIER_ID_PKT_TAG_TYPE                                   = 0x4000,   /**< The tag type of the ingress packets */
    BCMBAL_CLASSIFIER_ID_ALL                                            = 0x7FFF    /**< All fields */
} ;

struct sOMCI_RAW
{
    int interface_id;
    int onu_id; 
    char *raw_omci;
};


struct sFLOW
{
    int interface_id;
    int onu_id; 
    int flow_id;
    char * flow_type;
    char * packet_tag_type;	
    int gemport_id;	
    int network_interface_id;

    bcmbal_action_cmd_id acton_cmd;
    bcmbal_action_id action; 
    action_val action_val_a_val;

    bcmbal_classifier_id classifier;
    class_val class_val_c_val;	

};

struct sOMCI_RAW a_OMCI[95] =
{
    [0]={
        .interface_id        = 0,
        .onu_id               = 1,		
        .raw_omci = "00B34F0A00020000000000000000000000000000000000000000000000000000000000000000000000000028"
    }
    ,
        [1]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00B44F0A00020000000000000000000000000000000000000000000000000000000000000000000000000028"
        }
    ,
        [2]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00B5440A01360201000201000000000000000000000000000000000000000000000000000000000000000028"
        }
    ,
        [3]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00B6440A01360201000201000000000000000000000000000000000000000000000000000000000000000028"
        }
    ,
        [4]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00B7440A010C01040FA000000200000000000000000000000000000000000000000000000000000000000028"
        }
    ,
        [5]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00B8440A010C01040FA000000200000000000000000000000000000000000000000000000000000000000028"
        }
    ,
        [6]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00B9440A01100001003000000000000000000000000000000000000000000000000000000000000000000028"
        }
    ,
        [7]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00BA440A01100001003000000000000000000000000000000000000000000000000000000000000000000028"
        }
    ,
        [8]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00BB440A010C01040FA000000200000000000000000000000000000000000000000000000000000000000028"
        }
    ,
        [9]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00BC440A010C01040FA000000200000000000000000000000000000000000000000000000000000000000028"
        }
    ,
        [10]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00BD440A01190006010400000100000000000000000000000000000000000000000000000000000000000028"
        }
    ,
        [11]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00BE440A01190006010400000100000000000000000000000000000000000000000000000000000000000028"
        }
    ,
        [12]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00BF440A002D02010001008000140002000F0001000000000000000000000000000000000000000000000028"
        }
    ,
        [13]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00C0440A002D02010001008000140002000F0001000000000000000000000000000000000000000000000028"
        }
    ,
        [14]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00C1440A01190006010400000100000000000000000000000000000000000000000000000000000000000028"
        }
    ,
        [15]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00C2440A01190006010400000100000000000000000000000000000000000000000000000000000000000028"
        }
    ,
        [16]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00C3480A01350201020040000FA00FA000000000E0000000EFFFFFFF00000000000000000000000000000028"
        }
    ,
        [17]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00C4480A01350201020040000FA00FA000000000E0000000EFFFFFFF00000000000000000000000000000028"
        }
    ,
        [18]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00C5440A00828001FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000000000000000028"
        }
    ,
        [19]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00C6440A00828001FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000000000000000028"
        }
    ,
        [20]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00C7480A01350201020040000FA00FA000000000E0000000EFFFFFFF00000000000000000000000000000028"
        }
    ,
        [21]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00C8480A01350201020040000FA00FA000000000E0000000EFFFFFFF00000000000000000000000000000028"
        }
    ,
        [22]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00C9480A013502010001040FFB00000000000000000000000000000000000000000000000000000000000028"
        }
    ,
        [23]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00CA480A013502010001040FFB00000000000000000000000000000000000000000000000000000000000028"
        }
    ,
        [24]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00CB440A002F2102020103038001000000000000000000000000000000000000000000000000000000000028"
        }
    ,
        [25]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00CC440A002F2102020103038001000000000000000000000000000000000000000000000000000000000028"
        }
    ,
        [26]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00CD480A013502010001040FFB00000000000000000000000000000000000000000000000000000000000028"
        }
    ,
        [27]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00CE480A013502010001040FFB00000000000000000000000000000000000000000000000000000000000028"
        }
    ,
        [28]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00CF440A005421020FFB00000000000000000000000000000000000000000000100100000000000000000028"
        }
    ,
        [29]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00D0440A005421020FFB00000000000000000000000000000000000000000000100100000000000000000028"
        }
    ,
        [30]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00D1480A01068001800004000000000000000000000000000000000000000000000000000000000000000028"
        }
    ,
        [31]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00D2440A00AB0201020101000000000000000000000000000000000000000000000000000000000000000028"
        }
    ,
        [32]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00D3480A01068001800004000000000000000000000000000000000000000000000000000000000000000028"
        }
    ,
        [33]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00D4440A00AB0201020101000000000000000000000000000000000000000000000000000000000000000028"
        }
    ,
        [34]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00D5480A00AB0201380081008100000000000000000000000000000000000000000000000000000000000028"
        }
    ,
        [35]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00D6440A010C0408040880010301000000000000000000000000000000000000000000000000000000000028"
        }
    ,
        [36]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00D7440A010C0408040880010301000000000000000000000000000000000000000000000000000000000028"
        }
    ,
        [37]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00D8480A00AB0201380081008100000000000000000000000000000000000000000000000000000000000028"
        }
    ,
        [38]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00D9440A002F0201020101010101000000000000000000000000000000000000000000000000000000000028"
        }
    ,
        [39]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00DA440A010A0408040805800100000001000000000000000000000000000000000000000000000000000028"
        }
    ,
        [40]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00DB440A010A0408040805800100000001000000000000000000000000000000000000000000000000000028"
        }
    ,
        [41]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00DC440A002F0201020101010101000000000000000000000000000000000000000000000000000000000028"
        }
    ,
        [42]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00DD480A00AB02010400F8000000F8000000000F000000007FDC000000000000000000000000000000000028"
        }
    ,
        [43]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00DE480A008280017F8004080408040804080408040804080408000000000000000000000000000000000028"
        }
    ,
        [44]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00DF480A008280017F8004080408040804080408040804080408000000000000000000000000000000000028"
        }
    ,
        [45]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00E0480A00AB02010400F8000000F8000000000F000000007FDC000000000000000000000000000000000028"
        }
    ,
        [46]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00E1440A00AB0202020102000000000000000000000000000000000000000000000000000000000000000028"
        }
    ,
        [47]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00E2460A00542102000000000000000000000000000000000000000000000000000000000000000000000028"
        }
    ,
        [48]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00E3460A00542102000000000000000000000000000000000000000000000000000000000000000000000028"
        }
    ,
        [49]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00E4440A00AB0202020102000000000000000000000000000000000000000000000000000000000000000028"
        }
    ,
        [50]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00E5480A00AB0202380081008100000000000000000000000000000000000000000000000000000000000028"
        }
    ,
        [51]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00E6440A00542102001400000000000000000000000000000000000000000000100100000000000000000028"
        }
    ,
        [52]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00E7440A00542102001400000000000000000000000000000000000000000000100100000000000000000028"
        }
    ,
        [53]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00E8480A00AB0202380081008100000000000000000000000000000000000000000000000000000000000028"
        }
    ,
        [54]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00E9440A002F0202020102010102000000000000000000000000000000000000000000000000000000000028"
        }
    ,
        [55]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00EA480A00AB02010400F8000000F8000000000F0000000000A4000000000000000000000000000000000028"
        }
    ,
        [56]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00EB480A00AB02010400F8000000F8000000000F0000000000A4000000000000000000000000000000000028"
        }
    ,
        [57]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00EC440A002F0202020102010102000000000000000000000000000000000000000000000000000000000028"
        }
    ,
        [58]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00ED480A00AB02020400F8000000F8000000000F000000007FDC000000000000000000000000000000000028"
        }
    ,
        [59]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00EE480A00AB02010400F800000080000000400F0000000800A4000000000000000000000000000000000028"
        }
    ,
        [60]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00EF480A00AB02010400F800000080000000400F0000000800A4000000000000000000000000000000000028"
        }
    ,
        [61]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00F0480A00AB02020400F8000000F8000000000F000000007FDC000000000000000000000000000000000028"
        }
    ,
        [62]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00F1440A00AB0203020103000000000000000000000000000000000000000000000000000000000000000028"
        }
    ,
        [63]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00F2480A00AB02020400F8000000F8000000000F0000000000A4000000000000000000000000000000000028"
        }
    ,
        [64]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00F3480A00AB02020400F8000000F8000000000F0000000000A4000000000000000000000000000000000028"
        }
    ,
        [65]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00F4440A00AB0203020103000000000000000000000000000000000000000000000000000000000000000028"
        }
    ,
        [66]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00F5480A00AB0203380081008100000000000000000000000000000000000000000000000000000000000028"
        }
    ,
        [67]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00F6480A00AB02020400F800000080000000400F0000000800A4000000000000000000000000000000000028"
        }
    ,
        [68]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00F7480A00AB02020400F800000080000000400F0000000800A4000000000000000000000000000000000028"
        }
    ,
        [69]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00F8480A00AB0203380081008100000000000000000000000000000000000000000000000000000000000028"
        }
    ,
        [70]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00F9440A002F0203020103010103000000000000000000000000000000000000000000000000000000000028"
        }
    ,
        [71]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00FA480A00AB02030400F8000000F8000000000F0000000000A4000000000000000000000000000000000028"
        }
    ,
        [72]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00FB480A00AB02030400F8000000F8000000000F0000000000A4000000000000000000000000000000000028"
        }
    ,
        [73]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00FC440A002F0203020103010103000000000000000000000000000000000000000000000000000000000028"
        }
    ,
        [74]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00FD480A00AB02030400F8000000F8000000000F000000007FDC000000000000000000000000000000000028"
        }
    ,
        [75]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00FE480A00AB02030400F800000080000000400F0000000800A4000000000000000000000000000000000028"
        }
    ,
        [76]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "00FF480A00AB02030400F800000080000000400F0000000800A4000000000000000000000000000000000028"
        }
    ,
        [77]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "0100480A00AB02030400F8000000F8000000000F000000007FDC000000000000000000000000000000000028"
        }
    ,
        [78]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "0101440A00AB0204020104000000000000000000000000000000000000000000000000000000000000000028"
        }
    ,
        [79]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "0102480A00AB02040400F8000000F8000000000F0000000000A4000000000000000000000000000000000028"
        }
    ,
        [80]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "0103480A00AB02040400F8000000F8000000000F0000000000A4000000000000000000000000000000000028"
        }
    ,
        [81]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "0104440A00AB0204020104000000000000000000000000000000000000000000000000000000000000000028"
        }
    ,
        [82]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "0105480A00AB0204380081008100000000000000000000000000000000000000000000000000000000000028"
        }
    ,
        [83]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "0106480A00AB02040400F800000080000000400F0000000800A4000000000000000000000000000000000028"
        }
    ,
        [84]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "0107480A00AB02040400F800000080000000400F0000000800A4000000000000000000000000000000000028"
        }
    ,
        [85]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "0108480A00AB0204380081008100000000000000000000000000000000000000000000000000000000000028"
        }
    ,
        [86]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "0109440A002F0204020104010104000000000000000000000000000000000000000000000000000000000028"
        }
    ,
        [87]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "010A480A00AB02050400F8000000F8000000000F0000000000A4000000000000000000000000000000000028"
        }
    ,
        [88]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "010B480A00AB02050400F8000000F8000000000F0000000000A4000000000000000000000000000000000028"
        }
    ,
        [89]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "010C440A002F0204020104010104000000000000000000000000000000000000000000000000000000000028"
        }
    ,
        [90]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "010D480A00AB02040400F8000000F8000000000F000000007FDC000000000000000000000000000000000028"
        }
    ,
        [91]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "010E480A00AB02050400F800000080000000400F0000000800A4000000000000000000000000000000000028"
        }
    ,
        [92]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "010F480A00AB02050400F800000080000000400F0000000800A4000000000000000000000000000000000028"
        }
    ,
        [93]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "0110480A00AB02040400F8000000F8000000000F000000007FDC000000000000000000000000000000000028"
        }
    ,
        [94]={
            .interface_id        = 0,
            .onu_id               = 1,    
            .raw_omci = "0111440A00AB0205020105000000000000000000000000000000000000000000000000000000000000000028"
        }
};

struct sFLOW a_Flow[] =
{
    /*
       {
       .interface_id        = 0,
       .onu_id               = 1,
       .flow_id               = 16,
       .flow_type           = "upstream",
       .packet_tag_type = "single_tag",
       .gemport_id = 1032,
       .network_interface_id = 0,
       .acton_cmd = BCMBAL_ACTION_CMD_ID_ADD_OUTER_TAG,
       .action        = BCMBAL_ACTION_ID_O_VID, 
       {
       .o_vid = 10,
       .o_pbits = 0,
       .o_tpid = 0,
       .i_vid = 0,
       .i_pbits = 0,
       .i_tpid = 0,
       .ether_type = 0,
       .ip_proto = 0,
       .src_port = 0,
       .dst_port = 0,
       },
       .classifier= BCMBAL_CLASSIFIER_ID_O_VID,
       {
       .o_vid = 20,
       .o_pbits = 0,
       .o_tpid = 0,
       .i_vid = 0,
       .i_pbits = 0,
       .i_tpid = 0,
       .ether_type = 0,
       .ip_proto = 0,
       .src_port = 0,
       .dst_port = 0,
       }
       }
       ,
       */
    {
        .interface_id        = 0,
            .onu_id               = 1,
    .flow_id               = 16,
            .flow_type           = "upstream",
            .packet_tag_type = "single_tag",
            .gemport_id = 1032,
            .network_interface_id = 0,
            .acton_cmd = BCMBAL_ACTION_CMD_ID_ADD_OUTER_TAG,    
            .action        = BCMBAL_ACTION_ID_O_VID, 
            {
                .o_vid = 10,
                .o_pbits = 0,
                .o_tpid = 0,
                .i_vid = 0,
                .i_pbits = 0,
                .i_tpid = 0,
                .ether_type = 0,
                .ip_proto = 0,
                .src_port = 0,
                .dst_port = 0,
            },
            .classifier= BCMBAL_CLASSIFIER_ID_O_VID,
            {
                .o_vid = 20,
                .o_pbits = 0,
                .o_tpid = 0,
                .i_vid = 0,
                .i_pbits = 0,
                .i_tpid = 0,
                .ether_type = 0,
                .ip_proto = 0,
                .src_port = 0,
                .dst_port = 0,
            }
    }
    ,
        {
            .interface_id        = 0,
            .onu_id               = 1,
    .flow_id               = 16,
            .flow_type           = "downstream",
            .packet_tag_type = "double_tag",
            .gemport_id = 1032,
            .network_interface_id = 0,
            .acton_cmd = BCMBAL_ACTION_CMD_ID_REMOVE_OUTER_TAG,    
            .action= BCMBAL_ACTION_ID_O_VID, 
            {
                .o_vid = 10,
                .o_pbits = 0,
                .o_tpid = 0,
                .i_vid = 0,
                .i_pbits = 0,
                .i_tpid = 0,
                .ether_type = 0,
                .ip_proto = 0,
                .src_port = 0,
                .dst_port = 0,
            },
            .classifier= BCMBAL_CLASSIFIER_ID_O_VID,
            {
                .o_vid = 20,
                .o_pbits = 0,
                .o_tpid = 0,
                .i_vid = 10,
                .i_pbits = 0,
                .i_tpid = 0,
                .ether_type = 0,
                .ip_proto = 0,
                .src_port = 0,
                .dst_port = 0,
            }
        }
	
    /*
       ,
       {
       .interface_id        = 0,
       .onu_id               = 1,
       .flow_id               = 16,
       .flow_type           = "upstream",
       .packet_tag_type = "single_tag",
       .gemport_id = 1032,
       .network_interface_id = 0,
       .acton_cmd = BCMBAL_ACTION_CMD_ID_ADD_OUTER_TAG,    
       .action= BCMBAL_ACTION_ID_O_VID, 
       {
       .o_vid = 10,
       .o_pbits = 0,
       .o_tpid = 0,
       .i_vid = 0,
       .i_pbits = 0,
       .i_tpid = 0,
       .ether_type = 0,
       .ip_proto = 0,
       .src_port = 0,
       .dst_port = 0,
       },
       .classifier= BCMBAL_CLASSIFIER_ID_O_VID,
       {
       .o_vid = 20,
       .o_pbits = 0,
       .o_tpid = 0,
       .i_vid = 0,
       .i_pbits = 0,
       .i_tpid = 0,
       .ether_type = 0,
       .ip_proto = 0,
       .src_port = 0,
       .dst_port = 0,
       }
       }

       ,
       {
       .interface_id        = 0,
       .onu_id               = 1,
       .flow_id               = 16,
       .flow_type           = "upstream",
       .packet_tag_type = "single_tag",
       .gemport_id = 1032,
       .network_interface_id = 0,
       .acton_cmd = BCMBAL_ACTION_CMD_ID_ADD_OUTER_TAG,    
       .action= BCMBAL_ACTION_ID_O_VID, 
       {
       .o_vid = 10,
       .o_pbits = 0,
       .o_tpid = 0,
       .i_vid = 0,
       .i_pbits = 0,
       .i_tpid = 0,
       .ether_type = 0,
       .ip_proto = 0,
       .src_port = 0,
       .dst_port = 0,
       },
       .classifier= BCMBAL_CLASSIFIER_ID_O_VID,
       {
       .o_vid = 20,
       .o_pbits = 0,
       .o_tpid = 0,
       .i_vid = 0,
       .i_pbits = 0,
       .i_tpid = 0,
       .ether_type = 0,
    .ip_proto = 0,
        .src_port = 0,
        .dst_port = 0,
}
}
,
{
    .interface_id        = 0,
    .onu_id               = 1,
    .flow_id               = 18,
    .flow_type           = "upstream",
    .packet_tag_type = "single_tag",
    .gemport_id = 1032,
    .network_interface_id = 0,
    .acton_cmd = BCMBAL_ACTION_CMD_ID_TRAP_TO_HOST,    
    .action= BCMBAL_ACTION_ID_O_VID, 
    {
        .o_vid = 0,
        .o_pbits = 0,
        .o_tpid = 0,
        .i_vid = 0,
        .i_pbits = 0,
        .i_tpid = 0,
        .ether_type = 0,
        .ip_proto = 0,
        .src_port = 0,
        .dst_port = 0,
    },
    .classifier= BCMBAL_CLASSIFIER_ID_O_VID,
    {
        .o_vid = 4091,
        .o_pbits = 0,
        .o_tpid = 0,
        .i_vid = 0,
        .i_pbits = 0,
        .i_tpid = 0,
        .ether_type = 34958,
        .ip_proto = 0,
        .src_port = 0,
        .dst_port = 0,
    }
}
,
{
    .interface_id        = 0,
    .onu_id               = 1,
    .flow_id               = 19,
    .flow_type           = "downstream",
    .packet_tag_type = "single_tag",
    .gemport_id = 1032,
    .network_interface_id = 0,
    .acton_cmd = BCMBAL_ACTION_CMD_ID_ADD_OUTER_TAG,    
    .action= BCMBAL_ACTION_ID_O_VID, 
    {
        .o_vid = 4091,
        .o_pbits = 0,
        .o_tpid = 0,
        .i_vid = 0,
        .i_pbits = 0,
        .i_tpid = 0,
        .ether_type = 0,
        .ip_proto = 0,
        .src_port = 0,
        .dst_port = 0,
    },
    .classifier= BCMBAL_CLASSIFIER_ID_O_VID,
    {
        .o_vid = 4091,
        .o_pbits = 0,
        .o_tpid = 0,
        .i_vid = 0,
        .i_pbits = 0,
        .i_tpid = 0,
        .ether_type = 34958,
        .ip_proto = 0,
        .src_port = 0,
        .dst_port = 0,
    }
}
*/
/*
   ,
   {
   .interface_id        = 0,
   .onu_id               = 1,
   .flow_id               = 16,
   .flow_type           = "upstream",
   .packet_tag_type = "single_tag",
   .gemport_id = 1032,
   .network_interface_id = 0,
   .acton_cmd = BCMBAL_ACTION_CMD_ID_ADD_OUTER_TAG,    
   .action= BCMBAL_ACTION_ID_O_VID, 
   {
   .o_vid = 10,
   .o_pbits = 0,
   .o_tpid = 0,
   .i_vid = 0,
   .i_pbits = 0,
   .i_tpid = 0,
   .ether_type = 0,
   .ip_proto = 0,
   .src_port = 0,
   .dst_port = 0,
   },
   .classifier= BCMBAL_CLASSIFIER_ID_O_VID,
   {
   .o_vid = 20,
   .o_pbits = 0,
   .o_tpid = 0,
   .i_vid = 0,
   .i_pbits = 0,
   .i_tpid = 0,
   .ether_type = 0,
   .ip_proto = 0,
   .src_port = 0,
   .dst_port = 0,
   }
   }

   ,
   {
   .interface_id        = 0,
   .onu_id               = 1,
   .flow_id               = 18,
   .flow_type           = "upstream",
   .packet_tag_type = "single_tag",
   .gemport_id = 1032,
   .network_interface_id = 0,
   .acton_cmd = BCMBAL_ACTION_CMD_ID_TRAP_TO_HOST,    
   .action= BCMBAL_ACTION_ID_NONE, 
   {
   .o_vid = 0,
   .o_pbits = 0,
   .o_tpid = 0,
   .i_vid = 0,
   .i_pbits = 0,
   .i_tpid = 0,
   .ether_type = 0,
   .ip_proto = 0,
   .src_port = 0,
   .dst_port = 0,
   },
   .classifier= BCMBAL_CLASSIFIER_ID_O_VID,
   {
   .o_vid = 4091,
   .o_pbits = 0,
   .o_tpid = 0,
   .i_vid = 0,
   .i_pbits = 0,
   .i_tpid = 0,
   .ether_type = 34958,
.ip_proto = 0,
    .src_port = 0,
    .dst_port = 0,
    }
}

,
{
    .interface_id        = 0,
    .onu_id               = 1,
    .flow_id               = 16,
    .flow_type           = "upstream",
    .packet_tag_type = "single_tag",
    .gemport_id = 1032,
    .network_interface_id = 0,
    .acton_cmd = BCMBAL_ACTION_CMD_ID_ADD_OUTER_TAG,    
    .action= BCMBAL_ACTION_ID_NONE, 
    {
        .o_vid = 10,
        .o_pbits = 0,
        .o_tpid = 0,
        .i_vid = 0,
        .i_pbits = 0,
        .i_tpid = 0,
        .ether_type = 0,
        .ip_proto = 0,
        .src_port = 0,
        .dst_port = 0,
    },
    .classifier= BCMBAL_CLASSIFIER_ID_O_VID,
    {
        .o_vid = 20,
        .o_pbits = 0,
        .o_tpid = 0,
        .i_vid = 0,
        .i_pbits = 0,
        .i_tpid = 0,
        .ether_type = 0,
        .ip_proto = 0,
        .src_port = 0,
        .dst_port = 0,
    }
}

,
{
    .interface_id        = 0,
    .onu_id               = 1,
    .flow_id               = 18,
    .flow_type           = "upstream",
    .packet_tag_type = "single_tag",
    .gemport_id = 1032,
    .network_interface_id = 0,
    .acton_cmd = BCMBAL_ACTION_CMD_ID_TRAP_TO_HOST,    
    .action= BCMBAL_ACTION_ID_NONE, 
    {
        .o_vid = 10,
        .o_pbits = 0,
        .o_tpid = 0,
        .i_vid = 0,
        .i_pbits = 0,
        .i_tpid = 0,
        .ether_type = 0,
        .ip_proto = 0,
        .src_port = 0,
        .dst_port = 0,
    },
    .classifier= BCMBAL_CLASSIFIER_ID_O_VID,
    {
        .o_vid = 4091,
        .o_pbits = 0,
        .o_tpid = 0,
        .i_vid = 0,
        .i_pbits = 0,
        .i_tpid = 0,
        .ether_type = 34958,
        .ip_proto = 0,
        .src_port = 0,
        .dst_port = 0,
    }
}
*/
};


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
#if 0
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
#else
      static constexpr char ONU_CFG_NAME[] = "onu_cfg";
      ifstream    m_source_files= {};
      std::string t_path = ONU_CFG_NAME;
      Json::Value onu_cfg;	  

      m_source_files.open(t_path);

      if(m_source_files.good())
      {
          Json::Reader reader;
          bool isJsonOK = (reader.parse(m_source_files, onu_cfg));

          if(isJsonOK)
          {
              printf("Get onu_cfg OK \r\n");
              printf("onu_cfg onu_id                        [%d]\r\n", onu_cfg["onu_id"].asInt());
              printf("onu_cfg olt pon interface_id      [%d]\r\n", onu_cfg["interface_id"].asInt());
              printf("onu_cfg onu vendor_id             [%s]\r\n", onu_cfg["vendor_id"].asString().c_str());
              printf("onu_cfg onu vendor_specific id [%s]\r\n", onu_cfg["vendor_specific"].asString().c_str());			  
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

    auto& pOLT = Olt_Device::Olt_Device::get_instance();

    //Step 1. enable bal //
    if(!pOLT.enable_bal())
    {
        printf("////////////Fail connect to bal server////////////\r\n");
        return;
    }

    while(!pOLT.get_olt_status())
    {
        static int count = 0;
        printf("////////////Wait bal init !! count[%d]////////////\r\n", count++);
        usleep(1000000);
    }


    //Step 2. enable pon port//
    int pon_if_max = pOLT.get_max_pon_num();
    int i = 0;
	
    printf("//////////// PON interface num[%d]  !!////////////\r\n", pon_if_max);
	
    for(i = 0; i < pon_if_max ; i++)
    {
    
        printf("////////////Enable PON interface [%d] UP !!////////////\r\n", i);
        pOLT.enable_pon_if_(i);
    }

    int onu_id = onu_cfg["onu_id"].asInt(); 	
    int interface_id = onu_cfg["interface_id"].asInt();
    std::string s_vendor_id = onu_cfg["vendor_id"].asString();
    std::string s_vendor_spec = onu_cfg["vendor_specific"].asString();
	
    int buflen = s_vendor_spec.size();
    char cs_vendor_spec[8] = {0x71,0xe8,0x01,0x10};//default value //
    
    uint16_t idx1 = 0;
    uint16_t idx2 = 0;
    char str1[20];
    char str2[20];
    memset(&cs_vendor_spec, 0, buflen);
        
    for (idx1=0,idx2=0; idx1< buflen ; idx1++,idx2++) 
    {
        sprintf(str1,"%c", s_vendor_spec[idx1]);
        sprintf(str2,"%c", s_vendor_spec[++idx1]);
        strcat(str1,str2);
        cs_vendor_spec[idx2] = strtol(str1, NULL, 16);
    }

    printf("////////////Active ONU[%s][0x%02X][0x%02X][0x%02X][0x%02X] !!////////////\r\n", 
    s_vendor_id.c_str(), cs_vendor_spec[0],cs_vendor_spec[1],cs_vendor_spec[2],cs_vendor_spec[3]);

    pOLT.activate_onu(interface_id, onu_id, s_vendor_id.c_str(), cs_vendor_spec);
    

    // usleep(1000000*10); //
#if 0    
    {
        int intf_id = 0, onu_id = 1, agg_port_id= 1024;		
        pOLT.sched_add(intf_id, onu_id, agg_port_id) ;
    }
#endif	

#if 1
    { 
        int aFlow_size = (sizeof (a_Flow) / sizeof (a_Flow[0]));
        int i = 0;
        for (int i = 0 ; i < aFlow_size ; i++)
        {
            printf("apply flow [%i] settings\r\n", i);
            std::string sft(a_Flow[i].flow_type); //Flow type //
            std::string sptt(a_Flow[i].packet_tag_type); //pack tag type //
           
            pOLT.flow_add(
                    onu_id, a_Flow[i].flow_id, sft  , sptt, interface_id , 
                    a_Flow[i].network_interface_id, a_Flow[i].gemport_id, a_Flow[i].classifier, 
                    a_Flow[i].action , a_Flow[i].acton_cmd , a_Flow[i].action_val_a_val, a_Flow[i].class_val_c_val);                    
        }
    }
    usleep(1000000*10); //

    {
        int aOMCI_size = (sizeof (a_OMCI) / sizeof (a_OMCI[0]));
        int i = 0;
        for (int i = 0 ; i < aOMCI_size ; i++)
        {
            printf("apply comi [%i] settings\r\n", i);
            pOLT.omci_msg_out(interface_id, onu_id, a_OMCI[i].raw_omci);
            usleep(200000);
        }
    }


#endif

    printf("////////////Init PON port done !!////////////\r\n");

    pOLT.enter_cmd_shell();
#endif
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
