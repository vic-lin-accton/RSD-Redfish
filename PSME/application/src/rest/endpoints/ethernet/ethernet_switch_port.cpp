/*!
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
 * */

#include "psme/rest/model/handlers/handler_manager.hpp"
#include "psme/rest/model/handlers/generic_handler_deps.hpp"
#include "psme/rest/model/handlers/generic_handler.hpp"

#include "psme/rest/utils/mapper.hpp"
#include "psme/rest/utils/lag_utils.hpp"
#include "psme/rest/utils/status_helpers.hpp"
#include "psme/rest/validators/json_validator.hpp"
#include "psme/rest/validators/schemas/ethernet_switch_port.hpp"

#include "psme/rest/server/error/error_factory.hpp"
#include "psme/rest/endpoints/endpoints.hpp"

#include "agent-framework/module/requests/network.hpp"
#include "agent-framework/module/responses/network.hpp"
#include "agent-framework/module/responses/common.hpp"
#include "configuration/configuration.hpp"
#include <arpa/inet.h>

#include "psme/rest/endpoints/manager_network_interface.hpp"
#include "psme/rest/constants/constants.hpp"
#include "psme/rest/utils/network_interface_info.hpp"

#include <stdlib.h>
#include "acc_onlp_helper/acc_onlp_helper.hpp"
using namespace acc_onlp_helper;

#if defined BAL31 || defined BAL32 || defined BAL34
#include "acc_bal3_api_dist_helper/acc_bal3_api_dist_helper.hpp"
using namespace acc_bal3_api_dist_helper;
#endif    

using namespace psme::rest;
using namespace psme::rest::constants;
using namespace psme::rest::error;
using namespace psme::rest::utils;
using namespace psme::rest::validators;
using namespace agent_framework::module;
using namespace agent_framework::model;
using namespace psme::rest::constants;

using PatchMembersRequest = std::tuple<requests::AddEthernetSwitchPortMembers, requests::DeleteEthernetSwitchPortMembers>;

namespace
{
json::Value make_prototype()
{
    json::Value r(json::Value::Type::OBJECT);

    r[Common::ODATA_CONTEXT] = "/redfish/v1/$metadata#EthernetSwitchPort.EthernetSwitchPort";
    r[Common::ODATA_ID] = json::Value::Type::NIL;
    r[Common::ODATA_TYPE] = "#EthernetSwitchPort.v1_0_0.EthernetSwitchPort";
    r[Common::ID] = json::Value::Type::NIL;
    r[Common::NAME] = "Ethernet Switch Port";
    r[Common::DESCRIPTION] = "Ethernet Switch Port description";
    r[constants::EthernetSwitchPort::PORT_ID] = json::Value::Type::NIL;
    r[Common::STATUS][Common::STATE] = json::Value::Type::NIL;
    r[Common::STATUS][Common::HEALTH] = json::Value::Type::NIL;
    r[Common::STATUS][Common::HEALTH_ROLLUP] = json::Value::Type::NIL;

    r[constants::EthernetSwitchPort::LINK_TYPE] = json::Value::Type::NIL;
    r[constants::EthernetSwitchPort::OPERATIONAL_STATE] = json::Value::Type::NIL;
    r[constants::EthernetSwitchPort::ADMINISTRATIVE_STATE] = json::Value::Type::NIL;
    r[constants::EthernetSwitchPort::LINK_SPEED] = json::Value::Type::NIL;

    r[constants::EthernetSwitchPort::NEIGHBOR_INFO]
    [constants::EthernetSwitchPort::SWITCH_ID] = json::Value::Type::NIL;
    r[constants::EthernetSwitchPort::NEIGHBOR_INFO]
    [constants::EthernetSwitchPort::PORT_ID] = json::Value::Type::NIL;
    r[constants::EthernetSwitchPort::NEIGHBOR_INFO]
    [constants::EthernetSwitchPort::CABLE_ID] = json::Value::Type::NIL;
    r[constants::EthernetSwitchPort::NEIGHBOR_MAC] = json::Value::Type::NIL;
    r[constants::EthernetSwitchPort::FRAME_SIZE] = json::Value::Type::NIL;
    r[constants::EthernetSwitchPort::AUTOSENSE] = json::Value::Type::NIL;
    r[constants::EthernetSwitchPort::FULL_DUPLEX] = json::Value::Type::NIL;
    r[constants::Common::MAC_ADDRESS] = json::Value::Type::NIL;
    r[constants::EthernetSwitchPort::PORT_CLASS] = json::Value::Type::NIL;
    r[constants::EthernetSwitchPort::PORT_MODE] = json::Value::Type::NIL;
    r[constants::EthernetSwitchPort::PORT_TYPE] = json::Value::Type::NIL;
    r[Common::OEM] = json::Value::Type::OBJECT;

    r[constants::EthernetSwitchPort::IPv4_ADDRESSES] =
        json::Value::Type::ARRAY;

    r[constants::EthernetSwitchPort::IPv6_ADDRESSES] =
        json::Value::Type::ARRAY;

    r[constants::EthernetSwitchPort::VLANS] =
        json::Value::Type::OBJECT;

    r[constants::EthernetSwitchPort::STATIC_MACS] =
        json::Value::Type::OBJECT;

    json::Value links;
    links[constants::EthernetSwitchPort::PRIMARY_VLAN] =
        json::Value::Type::NIL;
    links[constants::EthernetSwitchPort::SWITCH] =
        json::Value::Type::NIL;
    links[constants::EthernetSwitchPort::MEMBER_OF_PORT] =
        json::Value::Type::NIL;
    links[constants::EthernetSwitchPort::PORT_MEMBERS] =
        json::Value::Type::ARRAY;
    links[constants::EthernetSwitchPort::ACTIVE_ACLS] =
        json::Value::Type::ARRAY;
    links[Common::OEM] = json::Value::Type::OBJECT;

    links[Common::OEM][Common::RACKSCALE]
        [Common::ODATA_TYPE] = "#Intel.Oem.EthernetSwitchPort";

#ifndef COCP
    links[Common::OEM][Common::RACKSCALE][constants::EthernetSwitchPort::NEIGHBOR_INTERFACE][Common::ODATA_ID] =json::Value::Type::NIL;
#endif

    r[Common::LINKS] = std::move(links);
    return r;
}

std::string get_switch(const server::Request &req)
{
    return endpoint::PathBuilder(PathParam::BASE_URL)
        .append(Root::ETHERNET_SWITCHES)
        .append(req.params[PathParam::ETHERNET_SWITCH_ID])
        .build();
}

void add_primary_vlan_link(json::Value &json, const std::string &vlan, const std::string &url)
{
        json[Common::LINKS][constants::EthernetSwitchPort::PRIMARY_VLAN][Common::ODATA_ID] =
    endpoint::PathBuilder(url).append(constants::EthernetSwitchPort::VLANS).append(vlan).build();
}

static const std::map<std::string, std::string> gami_to_rest_attributes = {
    {agent_framework::model::literals::EthernetSwitchPort::LINK_SPEED_MBPS,      constants::EthernetSwitchPort::LINK_SPEED},
    {agent_framework::model::literals::EthernetSwitchPort::ADMINISTRATIVE_STATE, constants::EthernetSwitchPort::ADMINISTRATIVE_STATE},
    {agent_framework::model::literals::EthernetSwitchPort::OPERATIONAL_STATE, constants::EthernetSwitchPort::OPERATIONAL_STATE},
    {agent_framework::model::literals::EthernetSwitchPort::FRAME_SIZE,           constants::EthernetSwitchPort::FRAME_SIZE},
    {agent_framework::model::literals::EthernetSwitchPort::AUTO_SENSE,           constants::EthernetSwitchPort::AUTOSENSE},
    {agent_framework::model::literals::EthernetSwitchPort::DEFAULT_VLAN, constants::EthernetSwitchPort::PRIMARY_VLAN}};
} // namespace

endpoint::EthernetSwitchPort::EthernetSwitchPort(const std::string& path) : EndpointBase(path) {}

endpoint::EthernetSwitchPort::~EthernetSwitchPort() {}

bool isValidIpAddress(char *ipAddress);

bool endpoint::EthernetSwitchPort::trigger_rssi(int pon_id)
{
    //Send RSSI trigger command
    //Get ONUs list
    Json::Reader onu_list_j_reader = {};
    Json::Value j_return_value;
    std::string onu_list_file_path = "/tmp/pon_" + std::to_string(pon_id - 1) + "_onu_list";
    printf("EthernetSwitchPort get_onus_list() onu_list_file_path[%s]\r\n", onu_list_file_path.c_str());

    std::ifstream if_onu_list_files(onu_list_file_path);

    bool isJson = (onu_list_j_reader.parse(if_onu_list_files, j_return_value));

    if (isJson)
    {
        int count = j_return_value["total_count"].asInt();
        if (count != 0)
        {
            //Always get onu id 1 RSSI value//
            int id = j_return_value["onu_1_id"].asInt();
#if defined BAL31 || defined BAL32 || defined BAL34
            auto &pOLT = Olt_Device::Olt_Device::get_instance();
            if (pOLT.is_bal_lib_init() == true)
            {
                return pOLT.rssi_measurement(id, pon_id - 1);
            }
#else
            char command[512] = {0};
            char resultA[2048] = {0};
            sprintf(command, "echo \"/a/o object=onu sub=rssi_measurement pon_ni=%d onu_id=%d\" \
         > /broadcom/rssi_trigger_cmd;/broadcom/example_user_appl <  /broadcom/rssi_trigger_cmd",
                    pon_id - 1, id);
            memset(resultA, 0x0, sizeof(resultA));
            exec_shell(command, resultA);
            return true;
#endif
        }
    }
    return false;
}

void endpoint::EthernetSwitchPort::get(const server::Request &req, server::Response &res)
{
    // Port status //
    char command[256] = {0};
    char resultA[256] = {0};	
    int max_port = 0;	
	
    auto r = ::make_prototype();

    r[Common::ODATA_ID] = PathBuilder(req).build();
    auto switch_id = psme::rest::model::Find<agent_framework::model::EthernetSwitch>( req.params[PathParam::ETHERNET_SWITCH_ID]) .get_one() ->get_id();
    r[Common::ODATA_CONTEXT] = std::regex_replace(r[Common::ODATA_CONTEXT].as_string(), std::regex("__SWITCH_ID__"), std::to_string(switch_id));
    r[Common::ID] = req.params[PathParam::SWITCH_PORT_ID];
    r[Common::NAME] = constants::EthernetSwitchPort::PORT + req.params[PathParam::SWITCH_PORT_ID];

    auto port = psme::rest::model::Find<agent_framework::model::EthernetSwitchPort>(req.params[PathParam::SWITCH_PORT_ID])
            .via<agent_framework::model::EthernetSwitch>(req.params[PathParam::ETHERNET_SWITCH_ID])
            .get();

    int   port_id= std::stoi(req.params[PathParam::SWITCH_PORT_ID]);

            r[constants::EthernetSwitchPort::PORT_ID] = port.get_port_identifier();
        endpoint::status_to_json(port, r);
        const json::Value config = configuration::Configuration::get_instance().to_json();
        
    int iPort = port_id; 
        auto network_components = agent_framework::module::NetworkComponents::get_instance();
        auto &port_manager = network_components->get_instance()->get_port_manager();
        auto port_uuids = port_manager.get_keys();
    auto &Onlp = acc_onlp_helper::Switch::Switch::get_instance();
    
        for (const auto& port_uuid : port_uuids) 
        {
        auto port_ = port_manager.get_entry_reference(port_uuid);
            if (port_->get_port_id() == (unsigned int)port_id) 
            { 
                if(port_->get_status().get_state()== enums::State::Enabled)
                {
                    r[Common::STATUS][Common::STATE] = "Enabled";
                    r[Common::STATUS][Common::HEALTH] = "OK";
                    r[Common::STATUS][Common::HEALTH_ROLLUP] = "OK";
                    r[constants::EthernetSwitchPort::LINK_TYPE] = "Ethernet";
                    r[constants::EthernetSwitchPort::TRANS_STATIC] = port_->get_trans_info_json();
                    //Send RSSI Trigger to get Rx Pwr//

                    if (r[constants::EthernetSwitchPort::PORT_ID] == "PON port")
                    {
                        //Show ONUs links under PON port//
                        if (trigger_rssi(iPort))
                            printf("RSSI M OK\r\n");
                        else
                            printf("RSSI M NG\r\n");
                        r[constants::EthernetSwitchPort::ONUS][Common::ODATA_ID] = PathBuilder(req).append(constants::EthernetSwitchPort::ONUS).build();
                    }
#if defined BAL31 || defined BAL32 || defined BAL34
                    auto &pOLT = Olt_Device::Olt_Device::get_instance();

                    if (pOLT.is_bal_lib_init() == true)
                    {
                        r["Statistics"] = pOLT.get_port_statistic(iPort);
                    }
                    else
                    {
                        printf("bal lib not init !!\r\n");
                    }

                    if (Onlp.get_port_tx_status(port_id) == 1)
                        r[constants::EthernetSwitchPort::OPERATIONAL_STATE] = "Down";
                    else
                        r[constants::EthernetSwitchPort::OPERATIONAL_STATE] = "Up";
#endif
                }
                else
                {
                    r[Common::STATUS][Common::STATE] = "Absent";
                    r[Common::STATUS][Common::HEALTH] = "Warning";
                    r[Common::STATUS][Common::HEALTH_ROLLUP] = "Warning";
                }
            }

            r[Common::STATUS][Common::HEALTH_ROLLUP] = endpoint::HealthRollup<agent_framework::model::EthernetSwitchPort>().get(port.get_uuid());

#ifdef CTS       
       if(1) //for CTS         
#else
       if( !strncmp(resultA, "1", 1) || !strncmp("2", resultA, 1))
#endif
	{    
            r[constants::EthernetSwitchPort::LINK_TYPE] = "Ethernet";		
#ifndef VOLT        
            /*For link status*/
            sprintf(command, "port_status.sh get link %d", iPort );	
            memset(resultA,0x0, sizeof(resultA));
            //exec_shell(command, resultA);
#ifdef CTS  
         if(1) //for CTS                
#else
            if(strlen(resultA) != 0)
#endif
            {
#ifdef CTS              
             if(1) //for CTS        
#else
                if(!strncmp("up", resultA, 2))
#endif
                {
                    sprintf(command, "port_status.sh get link_speed %d", iPort );	
                    memset(resultA,0x0, sizeof(resultA));
                    //exec_shell(command, resultA);
                    
                    if(strlen(resultA) != 0)
                    {
                        r[constants::EthernetSwitchPort::LINK_SPEED] =(std::uint64_t )strtoull(resultA, NULL, 10)/1000;//Mbps
                    }
                    sprintf(command, "port_status.sh get duplex %d", iPort );	
                    memset(resultA,0x0, sizeof(resultA));
                    //exec_shell(command, resultA);
                    
                    if(strlen(resultA) != 0)
                    {
                       if(!strncmp("FD", resultA, 2))
                           r[constants::EthernetSwitchPort::FULL_DUPLEX] = true;
                       else
                           r[constants::EthernetSwitchPort::FULL_DUPLEX] = false;
                    }
                   
                   sprintf(command, "port_status.sh get auto %d", iPort );	
                   memset(resultA,0x0, sizeof(resultA));
                    //exec_shell(command, resultA);
                   
                   if(strlen(resultA) != 0)
                   {
                       if(!strncmp("1", resultA, 1))
                           r[constants::EthernetSwitchPort::AUTOSENSE] =true;
                       else
                           r[constants::EthernetSwitchPort::AUTOSENSE] =false;
                   }
                   
                   sprintf(command, "port_status.sh get framesize %d", iPort );	
                   memset(resultA,0x0, sizeof(resultA));
                    //exec_shell(command, resultA);
                   
                   if(strlen(resultA) != 0)
                   {
                       r[constants::EthernetSwitchPort::FRAME_SIZE] = (std::uint64_t )strtoull(resultA, NULL, 10);
                   }   	

                   sprintf(command, "l2.sh get first_entry_mac %d", iPort );	
                   memset(resultA,0x0, sizeof(resultA));
                    //exec_shell(command, resultA);

                   if(strlen(resultA) != 0)
                   {
                       resultA[strcspn(resultA, "\r\n")]=0;                   
                       r[constants::EthernetSwitchPort::NEIGHBOR_MAC] = resultA;
                   } 
                }
            }
#endif
        }
        
        r[constants::EthernetSwitchPort::VLANS][Common::ODATA_ID] =
        PathBuilder(req).append(constants::EthernetSwitchPort::VLANS).build();
        
        r[Common::LINKS][constants::EthernetSwitchPort::SWITCH][Common::ODATA_ID] =
        get_switch(req);
		
#ifndef VOLT           
        sprintf(command, "port_status.sh get pvid %d" , std::stoi(req.params[PathParam::SWITCH_PORT_ID]));
        memset(resultA,0x0, sizeof(resultA));
        //exec_shell(command, resultA);
        
        if(strlen(resultA) != 0)
        {
            resultA[strcspn(resultA, "\r\n")]=0;
            const std::string vlan = resultA;
            add_primary_vlan_link(r, vlan, PathBuilder(req).build());
        }	
        
        sprintf(command, "trunk.sh get num");
        memset(resultA,0x0, sizeof(resultA));
        //exec_shell(command, resultA);
        
        if(strlen(resultA) != 0)
        {
            int count =  atoi(resultA);
            
            for(int i = 1; i <= count; i++)
            {
                sprintf(command, "trunk.sh get num_index %d", i );
                memset(resultA,0x0, sizeof(resultA));
                //exec_shell(command, resultA); //Get Trunk ID
                int TrunkID = atoi(resultA) ;
                sprintf(command, "trunk.sh get mem_IN %d %d", TrunkID  ,iPort);
                memset(resultA,0x0, sizeof(resultA));
                exec_shell(command, resultA);
                
                if(strlen(resultA) != 0 && !strncmp(resultA, "1", 1))
                {
                    json::Value link_elem(json::Value::Type::OBJECT);
                    link_elem[Common::ODATA_ID] =  endpoint::PathBuilder( get_switch(req)).append(constants::EthernetSwitch::PORTS).append(TrunkID + max_port ).build();
                    r[Common::LINKS][constants::EthernetSwitchPort::MEMBER_OF_PORT].push_back(std::move(link_elem));
                }
            }
        }
#endif       
        r[constants::EthernetSwitchPort::STATIC_MACS][Common::ODATA_ID] = PathBuilder(req).append(constants::EthernetSwitchPort::STATIC_MACS).build();
    }
    
    set_response(res, r);
}

void endpoint::EthernetSwitchPort::patch(const server::Request &request, server::Response &response)
{
 //   using HandlerManager = psme::rest::model::handler::HandlerManager;

    auto json = JsonValidator::validate_request_body<schema::EthernetSwitchPortPatchSchema>(request);

    int iPort              = std::stoi(request.params[PathParam::SWITCH_PORT_ID]);
    unsigned long long  linkspeed      = 0;
    std::string  admin ;
    std::string operate;
    bool autonego = 0;	
    unsigned long long  fsize = 0;           	

     char resultA[256] = {0};	
     char command[256] = {0};
	
    auto &Onlp = acc_onlp_helper::Switch::Switch::get_instance();

    if (json.is_member(constants::EthernetSwitchPort::LINK_SPEED)) 
    {
        linkspeed = json[constants::EthernetSwitchPort::LINK_SPEED].as_uint64();
    }
    else
    {
        sprintf(command, "port_status.sh get cfg_speed %d", iPort );	
        memset(resultA,0x0, sizeof(resultA));
        //exec_shell(command, resultA);
        
        if(strlen(resultA) != 0)
            linkspeed= (std::uint64_t )strtoull(resultA, NULL, 10);
    }
	
    if (json.is_member(constants::EthernetSwitchPort::FRAME_SIZE)) 
    {
        fsize = json[constants::EthernetSwitchPort::FRAME_SIZE].as_uint64();
    }
    else
    {
        sprintf(command, "port_status.sh get framesize %d", iPort );	
        memset(resultA,0x0, sizeof(resultA));
        //exec_shell(command, resultA);
        
        if(strlen(resultA) != 0)
        {
            fsize = (std::uint64_t )strtoull(resultA, NULL, 10);
        }  
    }

    if (json.is_member(constants::EthernetSwitchPort::AUTOSENSE)) 
    {
        autonego = json[constants::EthernetSwitchPort::AUTOSENSE].as_bool();
    }
    else
    {
        sprintf(command, "port_status.sh get auto %d", iPort );	
        memset(resultA,0x0, sizeof(resultA));
        //exec_shell(command, resultA);
        
        if(strlen(resultA) != 0)
        {
            if(!strncmp("1", resultA, 1))
                autonego =true;
            else
                autonego =false;
        }    
    }

    if (json.is_member(constants::EthernetSwitchPort::OPERATIONAL_STATE))
    {
        operate = json[constants::EthernetSwitchPort::OPERATIONAL_STATE].as_string();
        if (!strcmp("Down", operate.c_str()))
            Onlp.set_port_tx_status(iPort, false);
        else if (!strcmp("Up", operate.c_str()))
            Onlp.set_port_tx_status(iPort, true);
    }
	
    sprintf(command, "port_status.sh set SFA %d %llu %llu %d", iPort, linkspeed , fsize, (int)autonego );
    memset(resultA,0x0, sizeof(resultA));
    //exec_shell(command, resultA);
	
    if (json[Common::LINKS].is_member(constants::EthernetSwitchPort::PRIMARY_VLAN)) 
    {
        const auto& primary_vlan_url =
        json[Common::LINKS][constants::EthernetSwitchPort::PRIMARY_VLAN][Common::ODATA_ID].as_string();        
        std::string pvid = "";
        
        size_t pos = primary_vlan_url.find_last_of("VLANs/");
        
        if (pos != string::npos)
            pvid = primary_vlan_url.substr(pos+1);
        
        sprintf(command, "port_status.sh set pvid %d %s" , iPort , pvid.c_str());
        memset(resultA,0x0, sizeof(resultA));
        //exec_shell(command, resultA);		   
    }
    
    get(request, response);
}

void endpoint::EthernetSwitchPort::del(const server::Request &req, server::Response &res)
{

    const auto&  LAG_ID=req.params[PathParam::SWITCH_PORT_ID];

    char command[256] = {0};
    char resultA[256] = {0};	
    int    max_port = 0;	
    sprintf(command, "psme.sh get max_port_num");
    memset(resultA,0x0, sizeof(resultA));
    //exec_shell(command, resultA);
    
    if(strlen(resultA) != 0)
    {  
       max_port = atoi(resultA);
    }

    sprintf(command, "trunk.sh set ID_del %d" , atoi(LAG_ID.c_str())-max_port );
    memset(resultA,0x0, sizeof(resultA));
    //exec_shell(command, resultA);

    res.set_status(server::status_2XX::NO_CONTENT);
}
