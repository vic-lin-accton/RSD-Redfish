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
#include "psme/rest/endpoints/ethernet/static_mac.hpp"
#include "psme/rest/server/error/error_factory.hpp"
#include "psme/rest/validators/json_validator.hpp"
#include "psme/rest/validators/schemas/static_mac.hpp"
#include "psme/rest/model/handlers/handler_manager.hpp"
#include "psme/rest/model/handlers/generic_handler_deps.hpp"
#include "psme/rest/model/handlers/generic_handler.hpp"

#include "agent-framework/module/requests/common.hpp"
#include "agent-framework/module/responses/common.hpp"
#include "agent-framework/module/requests/network.hpp"
#include "agent-framework/module/responses/network.hpp"

using namespace psme::rest;
using namespace psme::rest::validators;
using namespace psme::rest::endpoint;
using namespace psme::rest::model::handler;
using namespace psme::core::agent;
using namespace psme::rest::constants;
using namespace agent_framework::module;
using namespace agent_framework::model;
using NetworkComponents = agent_framework::module::NetworkComponents;

namespace {
json::Value make_prototype() {
    json::Value r(json::Value::Type::OBJECT);

    r[Common::ODATA_CONTEXT] = "/redfish/v1/$metadata#EthernetSwitchStaticMAC.EthernetSwitchStaticMAC";
    r[Common::ODATA_ID] = json::Value::Type::NIL;
    r[Common::ODATA_TYPE] = "#EthernetSwitchStaticMAC.v1_0_0.EthernetSwitchStaticMAC";
    r[Common::ID] = json::Value::Type::NIL;
    r[Common::NAME] = "Static MAC";
    r[Common::DESCRIPTION] = "Static MAC address rule";

    r[constants::Common::MAC_ADDRESS] = json::Value::Type::NIL;
    r[constants::StaticMac::VLAN_ID] = json::Value::Type::NIL;
    r[Common::OEM] = json::Value::Type::OBJECT;

    return r;
}


static const std::map<std::string, std::string> gami_to_rest_attributes = {
    {agent_framework::model::literals::StaticMac::ADDRESS,      constants::Common::MAC_ADDRESS},
    {agent_framework::model::literals::StaticMac::VLAN_ID, constants::StaticMac::VLAN_ID},
};


}

endpoint::StaticMac::StaticMac(const std::string& path) : EndpointBase(path) {}
endpoint::StaticMac::~StaticMac() {}

void endpoint::StaticMac::get(const server::Request& req, server::Response& res) {
    auto json = ::make_prototype();

    json[Common::ODATA_CONTEXT] = std::regex_replace(
            json[Common::ODATA_CONTEXT].as_string(),
            std::regex("__SWITCH_ID__"),
            req.params[PathParam::ETHERNET_SWITCH_ID]);

    json[Common::ODATA_CONTEXT] = std::regex_replace(
            json[Common::ODATA_CONTEXT].as_string(),
            std::regex("__PORT_ID__"),
            req.params[PathParam::SWITCH_PORT_ID]);

    json[Common::ODATA_ID] = PathBuilder(req).build();
    json[Common::ID] = req.params[PathParam::STATIC_MAC_ID];
    json[Common::NAME] = constants::StaticMac::STATIC_MAC + req.params[PathParam::STATIC_MAC_ID];

    int index = std::stoi(req.params[PathParam::STATIC_MAC_ID]);
    int iPort = std::stoi(req.params[PathParam::SWITCH_PORT_ID]);
    char command[256] = {0};
    char resultA[256] = {0};	

    sprintf(command, "l2.sh get static_mac_count %d" , iPort);
    memset(resultA,0x0, sizeof(resultA));
    exec_shell(command, resultA);
    if(strlen(resultA) != 0)
    {
        sprintf(command, "l2.sh get static_mac %d %d" , iPort, index);
        memset(resultA,0x0, sizeof(resultA));
        exec_shell(command, resultA);
        if(strlen(resultA) != 0)
        {
            resultA[strcspn(resultA, "\r\n")]=0;
            json[constants::Common::MAC_ADDRESS] = resultA;
        }
        sprintf(command, "l2.sh get static_mac_vlan %d %d" , iPort, index);
        memset(resultA,0x0, sizeof(resultA));
        exec_shell(command, resultA);
        if(strlen(resultA) != 0)
        {
            json[constants::StaticMac::VLAN_ID] = atoi(resultA);
        }		
    }
#if 0
    const auto static_mac =
        psme::rest::model::Find<agent_framework::model::StaticMac>(req.params[PathParam::STATIC_MAC_ID])
            .via<agent_framework::model::EthernetSwitch>(req.params[PathParam::ETHERNET_SWITCH_ID])
            .via<agent_framework::model::EthernetSwitchPort>(req.params[PathParam::SWITCH_PORT_ID])
            .get();

    json[constants::Common::MAC_ADDRESS] = static_mac.get_address();
    json[constants::StaticMac::VLAN_ID] = static_mac.get_vlan_id();
#else


#endif
    set_response(res, json);
}

void endpoint::StaticMac::del(const server::Request& req, server::Response& res) {

#if 0	
    auto static_mac = psme::rest::model::Find<agent_framework::model::StaticMac>(req.params[PathParam::STATIC_MAC_ID])
        .via<agent_framework::model::EthernetSwitch>(req.params[PathParam::ETHERNET_SWITCH_ID])
        .via<agent_framework::model::EthernetSwitchPort>(req.params[PathParam::SWITCH_PORT_ID])
        .get_one();

    auto delete_port_static_mac_request = requests::DeletePortStaticMac(static_mac->get_uuid());

    // try removing Static MAC from agent's model
    AgentManager::get_instance()->call_method<responses::DeletePortStaticMac>(static_mac->get_agent_id(),
                                                                              delete_port_static_mac_request);

    // remove the resource from application's model
    HandlerManager::get_instance()->get_handler(enums::Component::StaticMac)->remove(static_mac->get_uuid());
#else

    int index = std::stoi(req.params[PathParam::STATIC_MAC_ID]);
    int iPort = std::stoi(req.params[PathParam::SWITCH_PORT_ID]);

    char command[256] = {0};
    char resultA[256] = {0};	

    sprintf(command, "l2.sh  set static_mac_del_idx %d  %d" , iPort,  index);
    memset(resultA,0x0, sizeof(resultA));
    exec_shell(command, resultA);
#endif
    res.set_status(server::status_2XX::NO_CONTENT);
}

void endpoint::StaticMac::patch(const server::Request& request, server::Response& response) {

    const auto json = JsonValidator::validate_request_body<schema::StaticMacPatchSchema>(request);

#if 0
    const auto static_mac =
        psme::rest::model::Find<agent_framework::model::StaticMac>(request.params[PathParam::STATIC_MAC_ID])
            .via<agent_framework::model::EthernetSwitch>(request.params[PathParam::ETHERNET_SWITCH_ID])
            .via<agent_framework::model::EthernetSwitchPort>(request.params[PathParam::SWITCH_PORT_ID])
            .get();

    attribute::Attributes attributes{};
    if (json.is_member(constants::Common::MAC_ADDRESS)) {
        // Workaround for GAMI - REST spec mismatch:
        THROW(agent_framework::exceptions::NotImplemented, "rest",
              "Setting 'MACAddress' is not implemented in the network agent.");
    }

    if (json.is_member(constants::StaticMac::VLAN_ID)) {
        if(json[constants::StaticMac::VLAN_ID].is_null()) {
            attributes.set_value(literals::StaticMac::VLAN_ID, Json::Value::null);
        }
        else {
            attributes.set_value(literals::StaticMac::VLAN_ID, json[constants::StaticMac::VLAN_ID].as_uint());
        }
    }

    if (!attributes.empty()) {
        requests::SetComponentAttributes set_component_attributes_request{static_mac.get_uuid(), attributes};

        const auto& gami_agent = psme::core::agent::AgentManager::get_instance()->get_agent(static_mac.get_agent_id());

        auto set_static_mac_params = [&, gami_agent] {
            auto set_component_attributes_response = gami_agent
                ->execute<responses::SetComponentAttributes>(set_component_attributes_request);

            const auto& result_statuses = set_component_attributes_response.get_statuses();
            if (!result_statuses.empty()) {
                const auto& error = error::ErrorFactory::create_error_from_set_component_attributes_results(
                    result_statuses, gami_to_rest_attributes);
                throw error::ServerException(error);
            }

            HandlerManager::get_instance()->get_handler(enums::Component::StaticMac)->
                load(gami_agent,
                     static_mac.get_parent_uuid(),
                     enums::Component::EthernetSwitchPort,
                     static_mac.get_uuid(),
                     true);
        };
        gami_agent->execute_in_transaction(set_static_mac_params);
    }
#else
    auto mac = json[constants::Common::MAC_ADDRESS].as_string();
    int iPort = std::stoi(request.params[PathParam::SWITCH_PORT_ID]);
    int index = std::stoi(request.params[PathParam::STATIC_MAC_ID]);
    auto vlan_id = json[constants::StaticMac::VLAN_ID].as_uint();

    char command[256] = {0};
    char resultA[256] = {0};	

    sprintf(command, "l2.sh set static_mac_patch %d %d %s %d" , iPort, index, mac.c_str(), vlan_id);
    memset(resultA,0x0, sizeof(resultA));
    exec_shell(command, resultA);
#endif
    get(request, response);
}
