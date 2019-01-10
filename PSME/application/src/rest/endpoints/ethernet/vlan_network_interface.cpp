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

#include "psme/core/agent/agent_manager.hpp"
#include "psme/rest/constants/constants.hpp"
#include "psme/rest/endpoints/ethernet/vlan_network_interface.hpp"
#include "psme/rest/model/handlers/handler_manager.hpp"
#include "psme/rest/model/handlers/generic_handler_deps.hpp"
#include "psme/rest/model/handlers/generic_handler.hpp"
#include "psme/rest/utils/status_helpers.hpp"

#include "agent-framework/module/requests/network.hpp"
#include "agent-framework/module/responses/network.hpp"
#include "agent-framework/module/network_components.hpp"

#include <json/json.hpp>



using namespace psme::rest;
using namespace psme::rest::constants;
using namespace agent_framework::model::attribute;

namespace {
json::Value make_prototype() {
    json::Value r(json::Value::Type::OBJECT);

    r[Common::ODATA_CONTEXT] = "/redfish/v1/$metadata#VLanNetworkInterface.VLanNetworkInterface";
    r[Common::ODATA_ID] = json::Value::Type::NIL;
    r[Common::ODATA_TYPE] = "#VLanNetworkInterface.v1_0_1.VLanNetworkInterface";
    r[Common::ID] = json::Value::Type::NIL;
    r[Common::NAME] = "VLAN Network Interface";
    r[Common::DESCRIPTION] = "VLAN Network Interface description";
    r[Vlan::VLAN_ENABLE] = json::Value::Type::NIL;
    r[Vlan::VLAN_ID] = json::Value::Type::NIL;

    json::Value rs;
    rs[Common::ODATA_TYPE] = "#Intel.Oem.VLanNetworkInterface";
    rs[::Vlan::TAGGED] = json::Value::Type::NIL;
    rs[Common::STATUS][Common::STATE] = json::Value::Type::NIL;
    rs[Common::STATUS][Common::HEALTH] = json::Value::Type::NIL;
    rs[Common::STATUS][Common::HEALTH_ROLLUP] = json::Value::Type::NIL;
    r[Common::OEM][Common::RACKSCALE] = std::move(rs);

    return r;
}

}


endpoint::VlanNetworkInterface::VlanNetworkInterface(const std::string& path) : EndpointBase(path) {}


endpoint::VlanNetworkInterface::~VlanNetworkInterface() {}


void endpoint::VlanNetworkInterface::get(const server::Request& req, server::Response& res) {
    auto r = make_prototype();

    r[Common::ODATA_ID] = PathBuilder(req).build();
    r[Common::ID] = req.params[PathParam::VLAN_ID];
    r[Common::NAME] = Vlan::VLAN + req.params[PathParam::VLAN_ID];
#if 0
    auto vlan = psme::rest::model::Find
        <agent_framework::model::EthernetSwitchPortVlan>(req.params[PathParam::VLAN_ID]).via
        <agent_framework::model::EthernetSwitch>(req.params[PathParam::ETHERNET_SWITCH_ID]).via
        <agent_framework::model::EthernetSwitchPort>(req.params[PathParam::SWITCH_PORT_ID]).get();

    endpoint::status_to_json(vlan, r[Common::OEM][Common::RACKSCALE]);
    r[Vlan::VLAN_ENABLE] = vlan.get_vlan_enable();
    r[Vlan::VLAN_ID] = vlan.get_vlan_id();
    r[Common::OEM][Common::RACKSCALE][::Vlan::TAGGED] = vlan.get_tagged();
#else
    char resultA[256] = {0};	
    char command[256] = {0};
    int iPort = std::stoi(req.params[PathParam::SWITCH_PORT_ID]);

    r[Vlan::VLAN_ID] =std::stoi(req.params[PathParam::VLAN_ID]); 
    r[Vlan::VLAN_ENABLE] = true;  // Todo ??

    r[Common::OEM][Common::RACKSCALE][Common::STATUS][Common::STATE] = "Enabled";
    r[Common::OEM][Common::RACKSCALE][Common::STATUS][Common::HEALTH] = "OK";
    r[Common::OEM][Common::RACKSCALE][Common::STATUS][Common::HEALTH_ROLLUP] ="OK";

    sprintf(command, "vlan.sh get vlan_port_untag_value %d %d" , std::stoi(req.params[PathParam::VLAN_ID]), iPort);
    memset(resultA,0x0, sizeof(resultA));
    exec_shell(command, resultA);

    if(strlen(resultA) != 0)
    {
        if(!strncmp("untag", resultA, 5))
            r[Common::OEM][Common::RACKSCALE][::Vlan::TAGGED] = false;		
        else if(!strncmp("none", resultA, 3))
            r[Common::OEM][Common::RACKSCALE][::Vlan::TAGGED] = true;	
        else
        {
            r[Vlan::VLAN_ENABLE] = false;  // Todo ??
            r[Common::OEM][Common::RACKSCALE][Common::STATUS][Common::STATE] = json::Value::Type::NIL;
            r[Common::OEM][Common::RACKSCALE][Common::STATUS][Common::HEALTH] = json::Value::Type::NIL;
            r[Common::OEM][Common::RACKSCALE][Common::STATUS][Common::HEALTH_ROLLUP] = json::Value::Type::NIL; 

            res.set_status(server::status_2XX::NO_CONTENT);
			
	 }
    }
#endif

    set_response(res, r);
}


void endpoint::VlanNetworkInterface::del(const server::Request& req, server::Response& res) {
#if 0
    using AgentManager = psme::core::agent::AgentManager;
    using HandlerManager = psme::rest::model::handler::HandlerManager;


    auto vlan_interface = psme::rest::model::Find<agent_framework::model::EthernetSwitchPortVlan>(
        req.params[PathParam::VLAN_ID])
        .via<agent_framework::model::EthernetSwitch>(req.params[PathParam::ETHERNET_SWITCH_ID])
        .via<agent_framework::model::EthernetSwitchPort>(req.params[PathParam::SWITCH_PORT_ID])
        .get();
    auto gami_agent = AgentManager::get_instance()->get_agent(vlan_interface.get_agent_id());

    auto delete_vlan = [&, gami_agent] {
        // Those resources are needed to properly lock the resource tree and avoid deadlocks
        auto parent_switch = psme::rest::model::Find
            <agent_framework::model::EthernetSwitch>(req.params[PathParam::ETHERNET_SWITCH_ID]).get_one();
        auto parent_switch_port = psme::rest::model::Find
            <agent_framework::model::EthernetSwitchPort>(req.params[PathParam::SWITCH_PORT_ID])
            .via<agent_framework::model::EthernetSwitch>(req.params[PathParam::ETHERNET_SWITCH_ID]).get_one();
        auto vlan = psme::rest::model::Find<agent_framework::model::EthernetSwitchPortVlan>(
            req.params[PathParam::VLAN_ID])
            .via<agent_framework::model::EthernetSwitch>(req.params[PathParam::ETHERNET_SWITCH_ID])
            .via<agent_framework::model::EthernetSwitchPort>(req.params[PathParam::SWITCH_PORT_ID])
            .get_one();

        auto port_uuid = vlan->get_parent_uuid();
        auto switch_uuid = parent_switch->get_uuid();

        auto gami_req = agent_framework::model::requests::DeletePortVlan{vlan->get_uuid()};
        gami_agent->execute<agent_framework::model::responses::DeletePortVlan>(gami_req);

        // remove the resource
        HandlerManager::get_instance()->
            get_handler(agent_framework::model::enums::Component::EthernetSwitchPortVlan)->remove(vlan->get_uuid());

        psme::rest::model::handler::HandlerManager::get_instance()->get_handler(
            agent_framework::model::enums::Component::EthernetSwitchPort)->
            load(gami_agent, switch_uuid, agent_framework::model::enums::Component::EthernetSwitch, port_uuid, false);

        res.set_status(server::status_2XX::NO_CONTENT);
    };

    gami_agent->execute_in_transaction(delete_vlan);
#else

    int VLAN_ID = std::stoi(req.params[PathParam::VLAN_ID]);
    char resultA[256] = {0};	
    char command[256] = {0};

    if(VLAN_ID ==1 )
    {
        res.set_status(server::status_5XX::SERVICE_UNAVAILABLE);
        return;
    }

    sprintf(command, "vlan.sh set vlan_value_destroy  %d" , VLAN_ID);
    memset(resultA,0x0, sizeof(resultA));
    exec_shell(command, resultA);

    res.set_status(server::status_2XX::NO_CONTENT);


#endif

}
