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

#include "psme/rest/endpoints/ethernet/ethernet_switch_port_collection.hpp"
#include "psme/rest/constants/constants.hpp"
#include "psme/rest/utils/mapper.hpp"
#include "psme/rest/validators/json_validator.hpp"
#include "psme/rest/validators/schemas/ethernet_switch_port_collection.hpp"
#include "psme/rest/utils/lag_utils.hpp"
#include "psme/rest/server/error/error_factory.hpp"
#include "psme/rest/model/handlers/handler_manager.hpp"
#include "psme/rest/model/handlers/generic_handler_deps.hpp"
#include "psme/rest/model/handlers/generic_handler.hpp"

#include "agent-framework/module/model/attributes/model_attributes.hpp"
#include "agent-framework/module/requests/network.hpp"
#include "agent-framework/module/responses/network.hpp"

#include "psme/core/agent/agent_manager.hpp"
#include <regex>

#ifdef ONLP
#include "acc_onlp_helper/acc_onlp_helper.hpp"
using namespace acc_onlp_helper;
#endif

using namespace psme::rest;
using namespace psme::rest::constants;
using namespace psme::rest::endpoint;
using namespace psme::rest::error;
using namespace psme::rest::utils;
using namespace psme::rest::validators;
using namespace agent_framework::model;

namespace {
json::Value make_prototype() {
    json::Value r(json::Value::Type::OBJECT);

    r[Common::ODATA_CONTEXT] = "/redfish/v1/$metadata#EthernetSwitchPortCollection.EthernetSwitchPortCollection";
    r[Common::ODATA_ID] = json::Value::Type::NIL;
    r[Common::ODATA_TYPE] = "#EthernetSwitchPortCollection.EthernetSwitchPortCollection";
    r[Common::NAME] = "Ethernet Switch Port Collection";
    r[Common::DESCRIPTION] = "Collection of Ethernet Switch Ports";
    r[Collection::ODATA_COUNT] = json::Value::Type::NIL;
    r[Collection::MEMBERS] = json::Value::Type::ARRAY;

    return r;
}

}


EthernetSwitchPortCollection::EthernetSwitchPortCollection(const std::string& path) : EndpointBase(path) {}


EthernetSwitchPortCollection::~EthernetSwitchPortCollection() {}


void EthernetSwitchPortCollection::get(const server::Request& req, server::Response& res) {

    //For normal port 
    // Port status //
    char command[256] = {0};
    char resultA[256] = {0};	
    int    max_port = 0;		
	
    auto json = ::make_prototype();

    json[Common::ODATA_ID] = PathBuilder(req).build();

    json[Common::ODATA_CONTEXT] = std::regex_replace(json[Common::ODATA_CONTEXT].as_string(),
                                                     std::regex("__SWITCH_ID__"),
                                                     req.params[PathParam::ETHERNET_SWITCH_ID]);

#ifdef ONLP
    auto& sonlp = acc_onlp_helper::Switch::get_instance();
    //Set PSU related info. //
    max_port = sonlp.get_port_num();
    json[Collection::ODATA_COUNT] = max_port;
    
    for(int i = 1; i <= max_port; i++)
    {	
        json::Value link_elem(json::Value::Type::OBJECT);
        link_elem[Common::ODATA_ID] =  PathBuilder(req).append(i).build();
        json[Collection::MEMBERS].push_back(std::move(link_elem));
    }

#else

    sprintf(command, "psme.sh get max_port_num");
    memset(resultA,0x0, sizeof(resultA));
    exec_shell(command, resultA);
    
    if(strlen(resultA) != 0)
    {  
        int count = atoi(resultA);
        max_port = count;

        json[Collection::ODATA_COUNT] = max_port;
		
        for(int i = 1; i <= count; i++)
        {	
                json::Value link_elem(json::Value::Type::OBJECT);
                link_elem[Common::ODATA_ID] =  PathBuilder(req).append(i).build();
                json[Collection::MEMBERS].push_back(std::move(link_elem));
        }
    }
#endif
    //For LAG port 
    // Port status //
	
    sprintf(command, "trunk.sh get num");
    memset(resultA,0x0, sizeof(resultA));
    exec_shell(command, resultA);
    
    if(strlen(resultA) != 0)
    {  
        int count = atoi(resultA);

        json[Collection::ODATA_COUNT] = max_port + count;

        for(int i = 1; i <= count; i++)
        {	
            sprintf(command, "trunk.sh get num_index %d", i);
            memset(resultA,0x0, sizeof(resultA));
            exec_shell(command, resultA);
            if(strlen(resultA) != 0)
            {           
                json::Value link_elem(json::Value::Type::OBJECT);
                link_elem[Common::ODATA_ID] =  PathBuilder(req).append(max_port + atoi(resultA)).build();
                json[Collection::MEMBERS].push_back(std::move(link_elem));
      	     }
        }
    }
    set_response(res, json);
}


void EthernetSwitchPortCollection::post(const server::Request& req, server::Response& res) {
//    using HandlerManager = psme::rest::model::handler::HandlerManager;

    auto json = JsonValidator::validate_request_body<schema::EthernetSwitchPortCollectionPostSchema>(req);

    auto port_members = LagUtils::get_port_members(json);
    int size=(int)port_members.size();

#if 0	
    LagUtils::validate_port_members(port_members);

    auto parent = psme::rest::model::Find<agent_framework::model::EthernetSwitch>(
        req.params[PathParam::ETHERNET_SWITCH_ID]).get();
    auto switch_uuid = parent.get_uuid();

    requests::AddEthernetSwitchPort add_port_request{
        switch_uuid,
        json[constants::EthernetSwitchPort::PORT_ID].as_string(),
        agent_framework::model::enums::PortMode::from_string(
            json[constants::EthernetSwitchPort::PORT_MODE].as_string()),
        port_members,
        agent_framework::model::attribute::Oem()
    };

    const auto& gami_agent = psme::core::agent::AgentManager::get_instance()->get_agent(parent.get_agent_id());

    auto add_port = [&, gami_agent] {
        auto add_port_response =
            gami_agent->execute<responses::AddEthernetSwitchPort>(add_port_request);

        HandlerManager::get_instance()->
            get_handler(enums::Component::EthernetSwitchPort)->
            load(gami_agent, switch_uuid, enums::Component::EthernetSwitch, add_port_response.get_port(), true);

        auto created_port = agent_framework::module::NetworkComponents::get_instance()->
            get_port_manager().get_entry_reference(add_port_response.get_port());

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

        ::psme::rest::endpoint::utils::set_location_header(
            res, PathBuilder(req).append(created_port->get_id()).build());

        res.set_status(server::status_2XX::CREATED);
    };
    gami_agent->execute_in_transaction(add_port);

#else

        std::string port_id =  json[constants::EthernetSwitchPort::PORT_ID].as_string();
        char resultA[256] = {0};	
        char command[256] = {0};
        
        int max_port = 0;	
        int iTrunkID;
        std::string fport = std::regex_replace(   port_id.c_str(),    std::regex("LAG"), "");
        iTrunkID = atoi(fport.c_str());

				
        sprintf(command, "psme.sh get max_port_num");
        memset(resultA,0x0, sizeof(resultA));
        exec_shell(command, resultA);
        
        if(strlen(resultA) != 0)
        {
            max_port=atoi(resultA);
        }
//Delete Trunk ID
        sprintf(command, "trunk.sh set ID_del %d" , iTrunkID );  //LAG PORT ID == TRUNK ID //
        memset(resultA,0x0, sizeof(resultA));
        exec_shell(command, resultA);
		
//Add Trunk ID with port
        for(int i = 0; i < size; i++)
        {
            std::string temp_mem = std::regex_replace(   port_members.at(i).c_str(),    std::regex("/redfish/v1/EthernetSwitches/1/Ports/"), "");
            sprintf(command, "trunk.sh  set mem %d %s" , iTrunkID , temp_mem.c_str() );
            memset(resultA,0x0, sizeof(resultA));
            exec_shell(command, resultA);
        }

        ::psme::rest::endpoint::utils::set_location_header(
            res, PathBuilder(req).append(max_port + iTrunkID).build());
		
        res.set_status(server::status_2XX::CREATED);

#endif
}
