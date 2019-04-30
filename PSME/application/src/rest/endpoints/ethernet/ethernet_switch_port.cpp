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
//#define CTS
#ifdef ONLP
#include "acc_onlp_helper/acc_onlp_helper.hpp"
using namespace acc_onlp_helper;
#endif

#ifdef VOLT
#include "acc_bal_api_dist_helper/acc_bal_api_dist_helper.hpp"
using namespace acc_bal_api_dist_helper; 
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

namespace {
json::Value make_prototype() {
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

#ifdef VOLT
    json::Value status(json::Value::Type::OBJECT);
    status["rx_bytes"]         = 0; 
    status["rx_packets"]       = 0; 
    status["rx_ucast_packets"] = 0; 
    status["rx_mcast_packets"] = 0; 
    status["rx_bcast_packets"] = 0;
    status["rx_error_packets"] = 0; 

    status["tx_bytes"]         = 0; 
    status["tx_packets"]       = 0; 
    status["tx_ucast_packets"] = 0; 
    status["tx_mcast_packets"] = 0; 
    status["tx_bcast_packets"] = 0; 
    status["tx_error_packets"] = 0; 

    auto& pOLT = Olt_Device::Olt_Device::get_instance();
    if(pOLT.is_bal_lib_init() == true)
    r["Statistics"] = status; 
#endif

    r[Common::LINKS] = std::move(links);

    return r;
}


std::string get_switch(const server::Request& req) {
    return endpoint::PathBuilder(PathParam::BASE_URL)
        .append(Root::ETHERNET_SWITCHES)
        .append(req.params[PathParam::ETHERNET_SWITCH_ID])
        .build();
}
#if 0

void add_member_of_port_link(json::Value& json, const std::string& port,
                             const std::string& switchID) {

    json[Common::LINKS][constants::EthernetSwitchPort::MEMBER_OF_PORT] =
        json::Value::Type::NIL;

#if 0
    // return if the port is not a member
    const auto& port_members_manager =
        NetworkComponents::get_instance()->get_port_members_manager();
    if (!port_members_manager.child_exists(port)) {
        return;
    }

    // find parent
    const auto& port_manager = NetworkComponents::get_instance()->get_port_manager();
    auto parents = port_members_manager.get_parents(port);

    // safety check, if there is a parent, there should be only one
    if (1 != parents.size()) {
        log_error(GET_LOGGER("rest"), "Model Port/MemberOfPort link error");
        log_error(GET_LOGGER("rest"), "Port " << port <<
                                              " is used by more than one ports!");
        return;
    }

    const auto& parent = parents.front();
    try {
        // convert UUID into ID and fill the link
        const auto& parent_port_id = port_manager.get_entry(parent).get_id();
        json[Common::LINKS]
        [constants::EthernetSwitchPort::MEMBER_OF_PORT]
        [Common::ODATA_ID] = endpoint::PathBuilder(switchID)
            .append(constants::EthernetSwitch::PORTS)
            .append(parent_port_id).build();
    }
    catch (const agent_framework::exceptions::InvalidUuid&) {
        log_error(GET_LOGGER("rest"), "Model Port/MemberOfPort link error");
        log_error(GET_LOGGER("rest"), "Port " << parent <<
                                              " is present in the PortMembers table but it does not exist as a resource");
    }
#else

	

#endif

	
}


void add_port_members_links(json::Value& json, const std::string& port,
                            const std::string& switchID) {

    json[Common::LINKS][constants::EthernetSwitchPort::PORT_MEMBERS] =
        json::Value::Type::ARRAY;

    // return if the port is not a LAG
    const auto& port_members_manager =
        NetworkComponents::get_instance()->get_port_members_manager();
    if (!port_members_manager.parent_exists(port)) {
        return;
    }

    // find children
    const auto& port_manager = NetworkComponents::get_instance()->get_port_manager();
    auto children = port_members_manager.get_children(port);

    // fill links
    for (const auto& child : children) {
        try {
            const auto& child_port = port_manager.get_entry(child);
            json::Value link;
            link[Common::ODATA_ID] = endpoint::PathBuilder(switchID)
                .append(constants::EthernetSwitch::PORTS)
                .append(child_port.get_id()).build();
            json[Common::LINKS][constants::EthernetSwitchPort::PORT_MEMBERS]
                .push_back(link);
        }
        catch (const agent_framework::exceptions::InvalidUuid&) {
            log_error(GET_LOGGER("rest"), "Model Port/PortMembers link error");
            log_error(GET_LOGGER("rest"), "Port " << child <<
                                                  " is present in the PortMembers table but it does not exist"
                                                      " as a resource");
        }
    }
}
#endif
/*
void add_active_acls_links(json::Value& json, const std::string& port) {
    const auto& acls = NetworkComponents::get_instance()->
        get_acl_port_manager().get_parents(port);

    for (const auto& acl : acls) {
        try {
            json::Value link;
            link[Common::ODATA_ID] = endpoint::utils::get_component_url(
                enums::Component::Acl, acl);
            json[Common::LINKS][constants::EthernetSwitchPort::ACTIVE_ACLS].
                push_back(std::move(link));
        }
        catch (const agent_framework::exceptions::InvalidUuid&) {
            log_error(GET_LOGGER("rest"), "Model Port/ActiveACLs link error");
            log_error(GET_LOGGER("rest"), "ACL " << acl <<
                                                 " is present in the ActiveACLs table but it does not exist"
                                                     " as a resource");
        }
    }
}
*/

void add_primary_vlan_link(json::Value& json, const std::string& vlan, const std::string& url) {
        json[Common::LINKS][constants::EthernetSwitchPort::PRIMARY_VLAN][Common::ODATA_ID] =
    endpoint::PathBuilder(url).append(constants::EthernetSwitchPort::VLANS).append(vlan).build();
}

#if 0
PatchMembersRequest prepare_patch_members_requests(const std::string& port_uuid,
                                                   const std::vector<std::string> requested_members) {
    std::vector<std::string> members_to_add;
    std::vector<std::string> members_to_remove;
    psme::rest::endpoint::utils::children_to_add_to_remove(NetworkComponents::get_instance()->
        get_port_members_manager(), port_uuid, requested_members, members_to_add, members_to_remove);

    auto remove_request = requests::DeleteEthernetSwitchPortMembers(
        members_to_remove, port_uuid, attribute::Oem());

    auto add_request = requests::AddEthernetSwitchPortMembers(
        members_to_add, port_uuid, attribute::Oem());

    return PatchMembersRequest(add_request, remove_request);
}


void execute_patch_members(const agent_framework::model::EthernetSwitchPort& port,
                           psme::core::agent::JsonAgentSPtr gami_agent,
                           const PatchMembersRequest& patch_members_requests) {
    using HandlerManager = psme::rest::model::handler::HandlerManager;

    const auto& add_request = std::get<0>(patch_members_requests);
    const auto& remove_request = std::get<1>(patch_members_requests);

    if (add_request.get_members().size() > 0) {
        gami_agent->execute<responses::AddEthernetSwitchPortMembers>(add_request);

        // Update info about the added member ports
        for (const auto& member : add_request.get_members()) {
            HandlerManager::get_instance()->get_handler(enums::Component::EthernetSwitchPort)->
                load(gami_agent, port.get_parent_uuid(), enums::Component::EthernetSwitch, member, false);
            HandlerManager::get_instance()->get_handler(enums::Component::EthernetSwitchPort)->
                load_collection(gami_agent, member,
                                enums::Component::EthernetSwitchPort,
                                enums::CollectionType::EthernetSwitchPortVlans, false);
        }
    }

    if (remove_request.get_members().size() > 0) {
        gami_agent->execute<responses::DeleteEthernetSwitchPortMembers>(remove_request);

        // Update info about the deleted member ports
        for (const auto& member : remove_request.get_members()) {
            HandlerManager::get_instance()->get_handler(enums::Component::EthernetSwitchPort)->
                load(gami_agent,
                     port.get_parent_uuid(),
                     enums::Component::EthernetSwitch,
                     member,
                     false);
            HandlerManager::get_instance()->get_handler(enums::Component::EthernetSwitchPort)->
                load_collection(gami_agent,
                                member,
                                enums::Component::EthernetSwitchPort,
                                enums::CollectionType::EthernetSwitchPortVlans, false);
        }
    }
}
#endif

static const std::map<std::string, std::string> gami_to_rest_attributes = {
    {agent_framework::model::literals::EthernetSwitchPort::LINK_SPEED_MBPS,      constants::EthernetSwitchPort::LINK_SPEED},
    {agent_framework::model::literals::EthernetSwitchPort::ADMINISTRATIVE_STATE, constants::EthernetSwitchPort::ADMINISTRATIVE_STATE},
    {agent_framework::model::literals::EthernetSwitchPort::FRAME_SIZE,           constants::EthernetSwitchPort::FRAME_SIZE},
    {agent_framework::model::literals::EthernetSwitchPort::AUTO_SENSE,           constants::EthernetSwitchPort::AUTOSENSE},
    {agent_framework::model::literals::EthernetSwitchPort::DEFAULT_VLAN,         constants::EthernetSwitchPort::PRIMARY_VLAN}
};

#if 0
attribute::Attributes fill_attributes(json::Value& json) {
    attribute::Attributes attributes{};

    if (json.is_member(constants::EthernetSwitchPort::LINK_SPEED)) {
        long long int value{json[constants::EthernetSwitchPort::LINK_SPEED].as_int64()};
        attributes.set_value(agent_framework::model::literals::EthernetSwitchPort::LINK_SPEED_MBPS,
                             value);
    }
    if (json.is_member(constants::EthernetSwitchPort::ADMINISTRATIVE_STATE)) {
        attributes.set_value(agent_framework::model::literals::EthernetSwitchPort::ADMINISTRATIVE_STATE,
                             json[constants::EthernetSwitchPort::ADMINISTRATIVE_STATE].as_string());
    }
    if (json.is_member(constants::EthernetSwitchPort::FRAME_SIZE)) {
        long long int value{json[constants::EthernetSwitchPort::FRAME_SIZE].as_int64()};
        attributes.set_value(agent_framework::model::literals::EthernetSwitchPort::FRAME_SIZE,
                             value);
    }
    if (json.is_member(constants::EthernetSwitchPort::AUTOSENSE)) {
        attributes.set_value(agent_framework::model::literals::EthernetSwitchPort::AUTO_SENSE,
                             json[constants::EthernetSwitchPort::AUTOSENSE].as_bool());
    }
    if (json[Common::LINKS].is_member(constants::EthernetSwitchPort::PRIMARY_VLAN)) {
        try {
            const auto& primary_vlan_url =
                json[Common::LINKS][constants::EthernetSwitchPort::PRIMARY_VLAN][Common::ODATA_ID].as_string();
            auto params = psme::rest::model::Mapper::get_params(primary_vlan_url,
                                                                constants::Routes::VLAN_NETWORK_INTERFACE_PATH);
            auto pvid =
                psme::rest::model::Find<agent_framework::model::EthernetSwitchPortVlan>(params[PathParam::VLAN_ID])
                    .via<agent_framework::model::EthernetSwitch>(params[PathParam::ETHERNET_SWITCH_ID])
                    .via<agent_framework::model::EthernetSwitchPort>(params[PathParam::SWITCH_PORT_ID])
                    .get_one();

            attributes.set_value(agent_framework::model::literals::EthernetSwitchPort::DEFAULT_VLAN, pvid->get_uuid());
        }
        catch (const agent_framework::exceptions::NotFound& ex) {
            throw agent_framework::exceptions::InvalidValue("Cannot patch default VLAN: " + ex.get_message());
        }
    }

    return attributes;
}
#endif
}


endpoint::EthernetSwitchPort::EthernetSwitchPort(const std::string& path) : EndpointBase(path) {}


endpoint::EthernetSwitchPort::~EthernetSwitchPort() {}

bool isValidIpAddress(char *ipAddress);

void endpoint::EthernetSwitchPort::get(const server::Request& req, server::Response& res) {

    // Port status //
    char command[256] = {0};
    char resultA[256] = {0};	
    int max_port = 0;	
	
    auto r = ::make_prototype();

    r[Common::ODATA_ID] = PathBuilder(req).build();
    auto switch_id = psme::rest::model::Find<agent_framework::model::EthernetSwitch>(
        req.params[PathParam::ETHERNET_SWITCH_ID]).get_one()->get_id();
    r[Common::ODATA_CONTEXT] = std::regex_replace(r[Common::ODATA_CONTEXT].as_string(),
                                                  std::regex("__SWITCH_ID__"), std::to_string(switch_id));
    r[Common::ID] = req.params[PathParam::SWITCH_PORT_ID];
    r[Common::NAME] = constants::EthernetSwitchPort::PORT + req.params[PathParam::SWITCH_PORT_ID];

    auto port =
        psme::rest::model::Find<agent_framework::model::EthernetSwitchPort>(req.params[PathParam::SWITCH_PORT_ID])
            .via<agent_framework::model::EthernetSwitch>(req.params[PathParam::ETHERNET_SWITCH_ID])
            .get();

    int   IN_PORT_ID= std::stoi(req.params[PathParam::SWITCH_PORT_ID]);

#ifdef VOLT
    auto& pOLT = Olt_Device::Olt_Device::get_instance();
#endif    

#ifdef ONLP
// Use ONLP library to to get ONLP info.
    auto& sonlp = acc_onlp_helper::Switch::get_instance();
    //Set PSU related info. //
    sonlp.get_port_info();
    max_port = sonlp.get_port_num();	
#else
    sprintf(command, "psme.sh get max_port_num");
    memset(resultA,0x0, sizeof(resultA));
    exec_shell(command, resultA);
    
    if(strlen(resultA) != 0)
    {  
        max_port = atoi(resultA);
    }
#endif

    if( IN_PORT_ID  >max_port )	
    {
        //For LAG info //
        int TrunkID =   IN_PORT_ID   - max_port  ;

        // Get total count of TRUNK
        // Check if port in this TRUNK //
        
        sprintf(command, "trunk.sh get mem_count %d", TrunkID);
        memset(resultA,0x0, sizeof(resultA));
        exec_shell(command, resultA);

        for(int i = 1; i <= max_port; i++)
        {
            sprintf(command, "trunk.sh get mem_IN %d %d", TrunkID  ,i);
            memset(resultA,0x0, sizeof(resultA));
            exec_shell(command, resultA);
            
            if(strlen(resultA) != 0 && !strncmp(resultA, "1", 1))
            {
                json::Value link_elem(json::Value::Type::OBJECT);
                link_elem[Common::ODATA_ID] =  endpoint::PathBuilder( get_switch(req)).append(constants::EthernetSwitch::PORTS).append(i).build();
                r[Common::LINKS][constants::EthernetSwitchPort::PORT_MEMBERS].push_back(std::move(link_elem));
            }
        }       
        
        r[constants::EthernetSwitchPort::PORT_MODE] = "LinkAggregationStatic";

        //add_port_members_links(r, port.get_uuid(), get_switch(req));
    }
    else
    {
        // For NORMAL PORT info //
        int iPort = std::stoi(req.params[PathParam::SWITCH_PORT_ID]);
#ifndef VOLT        
        sprintf(command, "port_status.sh get  portname %d" , iPort);
        memset(resultA,0x0, sizeof(resultA));
        exec_shell(command, resultA);
        
        if(strlen(resultA) != 0)
        {
            resultA[strcspn(resultA, "\r\n")]=0;
            r[constants::EthernetSwitchPort::PORT_ID] = resultA;			   
        }
        else
#endif			
            r[constants::EthernetSwitchPort::PORT_ID] = port.get_port_identifier();
        
        endpoint::status_to_json(port, r);
        
        const json::Value config = configuration::Configuration::get_instance().to_json();
        
#ifdef ONLP

        if(sonlp.get_port_info_by_(iPort, acc_onlp_helper::Switch::Port_Present ))
        {
            r[Common::STATUS][Common::STATE]  = "Enabled";
            r[Common::STATUS][Common::HEALTH] = "OK";
            r[Common::STATUS][Common::HEALTH_ROLLUP] = "OK";	
            r[constants::EthernetSwitchPort::LINK_TYPE] = "Ethernet";	
        }
        else
        {
            r[Common::STATUS][Common::STATE]  = "Absent";
            r[Common::STATUS][Common::HEALTH] = "Warning";
            r[Common::STATUS][Common::HEALTH_ROLLUP] = "Warning";	
        }

#ifdef VOLT

        if(pOLT.is_bal_lib_init() == true)
            r["Statistics"] = pOLT.get_port_statistic(iPort); 
#endif    
/*Todo , Add GPON transceiver */		
        r["Transceiver Statistics"] = sonlp.get_port_trans_info_by_(iPort);		
/**/




#else
        sprintf(command, "psme.sh get sfp_port_status %d" , std::stoi(req.params[PathParam::SWITCH_PORT_ID]));
        memset(resultA,0x0, sizeof(resultA));
        exec_shell(command, resultA);
		
        if(!strncmp(resultA, "1", 1) || !strncmp("2", resultA, 1))
        {
            r[Common::STATUS][Common::STATE]  = "Enabled";
            r[Common::STATUS][Common::HEALTH] = "OK";
            r[Common::STATUS][Common::HEALTH_ROLLUP] = "OK";	
            r[constants::EthernetSwitchPort::LINK_TYPE] = "Ethernet";		
        }
        else
        {
            r[Common::STATUS][Common::STATE]  = "Absent";
            r[Common::STATUS][Common::HEALTH] = "Warning";
            r[Common::STATUS][Common::HEALTH_ROLLUP] = "Warning";	
        }

#endif
        r[Common::STATUS][Common::HEALTH_ROLLUP] =
        endpoint::HealthRollup<agent_framework::model::EthernetSwitchPort>().get(port.get_uuid());
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
            exec_shell(command, resultA);
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
                    /*For get link speed*/
                    sprintf(command, "port_status.sh get link_speed %d", iPort );	
                    memset(resultA,0x0, sizeof(resultA));
                    exec_shell(command, resultA);
                    
                    if(strlen(resultA) != 0)
                    {
                        r[constants::EthernetSwitchPort::LINK_SPEED] =(std::uint64_t )strtoull(resultA, NULL, 10)/1000;//Mbps
                    }
                    /*For get full duplex*/
                    sprintf(command, "port_status.sh get duplex %d", iPort );	
                    memset(resultA,0x0, sizeof(resultA));
                    exec_shell(command, resultA);
                    
                    if(strlen(resultA) != 0)
                    {
                       if(!strncmp("FD", resultA, 2))
                           r[constants::EthernetSwitchPort::FULL_DUPLEX] = true;
                       else
                           r[constants::EthernetSwitchPort::FULL_DUPLEX] = false;
                    }
                   
                   /*For get auto nego*/
                   sprintf(command, "port_status.sh get auto %d", iPort );	
                   memset(resultA,0x0, sizeof(resultA));
                   exec_shell(command, resultA);
                   
                   if(strlen(resultA) != 0)
                   {
                       if(!strncmp("1", resultA, 1))
                           r[constants::EthernetSwitchPort::AUTOSENSE] =true;
                       else
                           r[constants::EthernetSwitchPort::AUTOSENSE] =false;
                   }
                   
                   /*For get framesize*/
                   sprintf(command, "port_status.sh get framesize %d", iPort );	
                   memset(resultA,0x0, sizeof(resultA));
                   exec_shell(command, resultA);
                   
                   if(strlen(resultA) != 0)
                   {
                       r[constants::EthernetSwitchPort::FRAME_SIZE] = (std::uint64_t )strtoull(resultA, NULL, 10);
                   }   	

                   /*For get op mode*/
                   sprintf(command, "port_status.sh get enable %d", iPort );	
                   memset(resultA,0x0, sizeof(resultA));
                   exec_shell(command, resultA);

                  if(!strncmp("1", resultA, 1))
                  {
                      r[constants::EthernetSwitchPort::OPERATIONAL_STATE] = "Up";
                      r[constants::EthernetSwitchPort::ADMINISTRATIVE_STATE] = "Up";
                  }
                  else
                  {
                      r[constants::EthernetSwitchPort::OPERATIONAL_STATE] = "Down";
                      r[constants::EthernetSwitchPort::ADMINISTRATIVE_STATE] = "Down";
                  }		   

                   /*For NeighborMAC */
                   sprintf(command, "l2.sh get first_entry_mac %d", iPort );	
                   memset(resultA,0x0, sizeof(resultA));
                   exec_shell(command, resultA);

                   if(strlen(resultA) != 0)
                   {
                       resultA[strcspn(resultA, "\r\n")]=0;                   
                       r[constants::EthernetSwitchPort::NEIGHBOR_MAC] = resultA;
                   } 
				   
                }
                else
                {
                    r[constants::EthernetSwitchPort::OPERATIONAL_STATE] = "Down";
                    r[constants::EthernetSwitchPort::ADMINISTRATIVE_STATE] = "Down";
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
        exec_shell(command, resultA);
        
        if(strlen(resultA) != 0)
        {
            resultA[strcspn(resultA, "\r\n")]=0;
            const std::string vlan = resultA;
            add_primary_vlan_link(r, vlan, PathBuilder(req).build());
        }	
		
        //add_member_of_port_link(r, port.get_uuid(), get_switch(req));
        
        sprintf(command, "trunk.sh get num");
        memset(resultA,0x0, sizeof(resultA));
        exec_shell(command, resultA);
        
        if(strlen(resultA) != 0)
        {
            // Get total count of TRUNK
            // Check if port in this TRUNK //
            int count =  atoi(resultA);
            
            for(int i = 1; i <= count; i++)
            {
                sprintf(command, "trunk.sh get num_index %d", i );
                memset(resultA,0x0, sizeof(resultA));
                exec_shell(command, resultA);      //Get Trunk ID
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
                //r[constants::EthernetSwitchPort::PORT_MODE] = "LinkAggregationStatic";
            }
        }
#endif       
        r[constants::EthernetSwitchPort::STATIC_MACS][Common::ODATA_ID] 
        = PathBuilder(req).append(constants::EthernetSwitchPort::STATIC_MACS).build();

    }
    

	
    set_response(res, r);
}


void endpoint::EthernetSwitchPort::patch(const server::Request& request, server::Response& response) {
 //   using HandlerManager = psme::rest::model::handler::HandlerManager;

    auto json = JsonValidator::validate_request_body<schema::EthernetSwitchPortPatchSchema>(request);

    int iPort              = std::stoi(request.params[PathParam::SWITCH_PORT_ID]);
    unsigned long long  linkspeed      = 0;
    std::string  admin ;
    bool autonego = 0;	
    unsigned long long  fsize = 0;           	

     char resultA[256] = {0};	
     char command[256] = {0};
	
    if (json.is_member(constants::EthernetSwitchPort::LINK_SPEED)) 
    {
        linkspeed = json[constants::EthernetSwitchPort::LINK_SPEED].as_uint64();
    }
    else
    {
        /*For get link speed*/
        sprintf(command, "port_status.sh get cfg_speed %d", iPort );	
        memset(resultA,0x0, sizeof(resultA));
        exec_shell(command, resultA);
        
        if(strlen(resultA) != 0)
            linkspeed= (std::uint64_t )strtoull(resultA, NULL, 10);
    }
	

    if (json.is_member(constants::EthernetSwitchPort::FRAME_SIZE)) 
    {
        fsize = json[constants::EthernetSwitchPort::FRAME_SIZE].as_uint64();
    }
    else
    {

        /*For get framesize*/
        sprintf(command, "port_status.sh get framesize %d", iPort );	
        memset(resultA,0x0, sizeof(resultA));
        exec_shell(command, resultA);
        
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
        /*For get auto nego*/
        sprintf(command, "port_status.sh get auto %d", iPort );	
        memset(resultA,0x0, sizeof(resultA));
        exec_shell(command, resultA);
        
        if(strlen(resultA) != 0)
        {
            if(!strncmp("1", resultA, 1))
                autonego =true;
            else
                autonego =false;
        }    
    }

    if (json.is_member(constants::EthernetSwitchPort::ADMINISTRATIVE_STATE)) 
    {
        admin = json[constants::EthernetSwitchPort::ADMINISTRATIVE_STATE].as_string();
		
        if(!strcmp("Down",admin.c_str()))
            sprintf(command, "port_status.sh set enable %d 0" , iPort);
        else if(!strcmp("Up",admin.c_str()))
            sprintf(command, "port_status.sh set enable %d 1" , iPort);
    		
        memset(resultA,0x0, sizeof(resultA));
        exec_shell(command, resultA);		
    }
	
    sprintf(command, "port_status.sh set SFA %d %llu %llu %d", iPort, linkspeed , fsize, (int)autonego );
    memset(resultA,0x0, sizeof(resultA));
    exec_shell(command, resultA);
 //      const auto reset_type = json[constants::Common::RESET_TYPE].as_string();
 //       const auto reset_type_enum = ResetType::from_string(reset_type);

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
        exec_shell(command, resultA);		   
    }
    

#if 0
    auto attributes = fill_attributes(json);

    // Holding reference to parent object ensures that locks are acquired in the same order in each thread
    auto parent_switch = psme::rest::model::Find<agent_framework::model::EthernetSwitch>(
        request.params[PathParam::ETHERNET_SWITCH_ID]).get();
    auto port = psme::rest::model::Find<agent_framework::model::EthernetSwitchPort>(
        request.params[PathParam::SWITCH_PORT_ID])
        .via<agent_framework::model::EthernetSwitch>(request.params[PathParam::ETHERNET_SWITCH_ID])
        .get();

    auto gami_agent = psme::core::agent::AgentManager::get_instance()->get_agent(port.get_agent_id());
    std::vector<std::string> port_members{};

    if (json[Common::LINKS].is_member(constants::EthernetSwitchPort::PORT_MEMBERS)) {
        LagUtils::validate_is_logical(port.get_port_class().value());
        port_members = LagUtils::get_port_members(json);
        LagUtils::validate_port_members(port_members, port.get_uuid());
    }

    auto patch_port = [&, gami_agent] {
        bool updated{false};

        if (!port_members.empty()) {
            const auto& patch_members_request = prepare_patch_members_requests(port.get_uuid(), port_members);
            execute_patch_members(port, gami_agent, patch_members_request);
            updated = true;
        }

        if (!attributes.empty()) {
            const auto& set_component_attributes_request = requests::SetComponentAttributes{port.get_uuid(),
                                                                                            attributes};
            const auto& set_component_attributes_response =
                gami_agent->execute<responses::SetComponentAttributes>(set_component_attributes_request);

            const auto& result_statuses = set_component_attributes_response.get_statuses();
            if (!result_statuses.empty()) {
                const auto& error = ErrorFactory::create_error_from_set_component_attributes_results(
                    result_statuses, gami_to_rest_attributes);
                throw ServerException(error);
            }

            updated = true;
        }

        if (updated) {
            HandlerManager::get_instance()->get_handler(enums::Component::EthernetSwitchPort)->
                load(gami_agent,
                     parent_switch.get_uuid(), agent_framework::model::enums::Component::EthernetSwitch,
                     port.get_uuid(), true);
        }
    };

    gami_agent->execute_in_transaction(patch_port);

#endif

    get(request, response);
}


void endpoint::EthernetSwitchPort::del(const server::Request& req, server::Response& res) {

#if 0	
    using HandlerManager = psme::rest::model::handler::HandlerManager;

    auto port = psme::rest::model::Find
        <agent_framework::model::EthernetSwitchPort>(req.params[PathParam::SWITCH_PORT_ID]).via
        <agent_framework::model::EthernetSwitch>(req.params[PathParam::ETHERNET_SWITCH_ID]).get();

    auto gami_req = requests::DeleteEthernetSwitchPort(port.get_uuid());

    const auto port_members = NetworkComponents::get_instance()->get_port_members_manager().get_children(
        port.get_uuid());
    const auto switch_uuid = port.get_parent_uuid();

    const auto& gami_agent = psme::core::agent::AgentManager::get_instance()->get_agent(port.get_agent_id());

    auto remove_port = [&, gami_agent] {
        // try removing port from agent's model
        gami_agent->execute<responses::DeleteEthernetSwitchPort>(gami_req);

        // remove the resource
        HandlerManager::get_instance()->get_handler(enums::Component::EthernetSwitchPort)->remove(port.get_uuid());

        // Update our information about member ports and their PortVlans
        for (const auto& member : port_members) {
            HandlerManager::get_instance()->get_handler(enums::Component::EthernetSwitchPort)->
                load(gami_agent,
                     switch_uuid,
                     enums::Component::EthernetSwitch,
                     member,
                     false);
            HandlerManager::get_instance()->get_handler(enums::Component::EthernetSwitchPort)->
                load_collection(gami_agent,
                                member,
                                enums::Component::EthernetSwitchPort,
                                enums::CollectionType::EthernetSwitchPortVlans,
                                false);
        }
    };

    gami_agent->execute_in_transaction(remove_port);
#endif

    const auto&  LAG_ID=req.params[PathParam::SWITCH_PORT_ID];

    char command[256] = {0};
    char resultA[256] = {0};	
    int    max_port = 0;	
    sprintf(command, "psme.sh get max_port_num");
    memset(resultA,0x0, sizeof(resultA));
    exec_shell(command, resultA);
    
    if(strlen(resultA) != 0)
    {  
       max_port = atoi(resultA);
    }

    sprintf(command, "trunk.sh set ID_del %d" , atoi(LAG_ID.c_str())-max_port );
    memset(resultA,0x0, sizeof(resultA));
    exec_shell(command, resultA);

    res.set_status(server::status_2XX::NO_CONTENT);
}
