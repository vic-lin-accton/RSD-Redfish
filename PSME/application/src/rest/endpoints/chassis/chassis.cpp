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

#include "agent-framework/module/constants/chassis.hpp"
#include "agent-framework/module/requests/common.hpp"
#include "agent-framework/module/responses/common.hpp"
#include "psme/rest/endpoints/chassis/chassis.hpp"
#include "psme/rest/utils/status_helpers.hpp"
#include "psme/rest/validators/json_validator.hpp"
#include "psme/rest/validators/schemas/chassis.hpp"
#include "psme/rest/server/error/error_factory.hpp"

#include "acc_onlp_helper/acc_onlp_helper.hpp"
using namespace acc_onlp_helper;
using namespace psme::rest;
using namespace psme::rest::constants;
using namespace psme::rest::validators;

namespace
{
json::Value make_prototype()
{
    json::Value r(json::Value::Type::OBJECT);

    r[Common::ODATA_CONTEXT] = "/redfish/v1/$metadata#Chassis.Chassis"; //For pass DMTF conference check //
    r[Common::ODATA_ID] = json::Value::Type::NIL;
    r[Common::ODATA_TYPE] = "#Chassis.v1_3_0.Chassis";
    r[Common::ID] = json::Value::Type::NIL;
    r[Chassis::CHASSIS_TYPE] = json::Value::Type::NIL;
    r[Common::NAME] = "Chassis";
    r[Common::DESCRIPTION] = json::Value::Type::NIL;
    r[Chassis::POWER_STATE] = "On";
    r[Common::MANUFACTURER] = json::Value::Type::NIL;
    r[Common::MODEL] = json::Value::Type::NIL;
    r[Chassis::SKU] = json::Value::Type::NIL;
    r[Common::SERIAL] = json::Value::Type::NIL;
    r[Common::PART_NUMBER] = json::Value::Type::NIL;
    r[Common::ASSET_TAG] = json::Value::Type::NIL;
    r[Chassis::INDICATOR_LED] = json::Value::Type::NIL;
    r[Common::STATUS][Common::STATE] = json::Value::Type::NIL;
    r[Common::STATUS][Common::HEALTH] = json::Value::Type::NIL;
    r[Common::STATUS][Common::HEALTH_ROLLUP] = json::Value::Type::NIL;

    json::Value rs;
    rs[Common::ODATA_TYPE] = "#Intel.Oem.Chassis";
    rs[Common::LOCATION][Common::ID] = json::Value::Type::NIL;
    rs[Common::LOCATION][Chassis::PARENT_ID] = json::Value::Type::NIL;
    r[Common::OEM][Common::RACKSCALE] = std::move(rs);

    r[Common::LINKS][Common::ODATA_TYPE] = "#Chassis.v1_2_0.Links";
    r[Common::LINKS][Chassis::CONTAINS] = json::Value::Type::ARRAY;
    //r[Common::LINKS][Common::CONTAINED_BY] = json::Value::Type::NIL;
    r[Common::LINKS][Chassis::COMPUTER_SYSTEMS] = json::Value::Type::ARRAY;
    r[Common::LINKS][Common::MANAGED_BY] = json::Value::Type::ARRAY;
    //r[Common::LINKS][Chassis::MANAGERS_IN_CHASSIS] = json::Value::Type::ARRAY;
    r[Common::LINKS][Chassis::DRIVES] = json::Value::Type::ARRAY;
    r[Common::LINKS][Chassis::STORAGE] = json::Value::Type::ARRAY;
    r[Common::LINKS][Common::OEM][Common::RACKSCALE][Common::ODATA_TYPE] = "#Intel.Oem.ChassisLinks";
    r[Common::LINKS][Common::OEM][Common::RACKSCALE][Chassis::SWITCHES] = json::Value::Type::ARRAY;

    return r;
}

void fill_containing_links(const agent_framework::model::Chassis &chassis, json::Value &r)
{
    auto is_drawer = [](const agent_framework::model::Chassis &ch)
        -> bool { return ch.get_type() == agent_framework::model::enums::ChassisType::Drawer; };

    auto is_enclosure = [](const agent_framework::model::Chassis &ch)
        -> bool { return ch.get_type() == agent_framework::model::enums::ChassisType::Enclosure; };

    // find manager of the chassis
    if (is_drawer(chassis) || is_enclosure(chassis))
    {

        for (const auto &chassis_id : agent_framework::module::CommonComponents::get_instance()
                                          ->get_chassis_manager()
                                          .get_ids())
        {
            if (chassis_id != chassis.get_id())
            {
                json::Value link;
                link[Common::ODATA_ID] = "/redfish/v1/Chassis/" + std::to_string(chassis_id);
                r[Common::LINKS][Chassis::CONTAINS].push_back(std::move(link));
            }
        }
    }
    else
    {
        try
        {
            for (const auto &chass : agent_framework::module::CommonComponents::get_instance()->get_chassis_manager().get_keys(is_drawer))
            {
                auto ch = agent_framework::module::CommonComponents::get_instance()->get_chassis_manager().get_entry_reference(chass);
                json::Value link;
                link[Common::ODATA_ID] = endpoint::PathBuilder(PathParam::BASE_URL)
                                             .append(constants::Common::CHASSIS)
                                             .append(ch->get_id())
                                             .build();
                r[Common::LINKS][Common::CONTAINED_BY] = std::move(link);
                r[Common::OEM][Common::RACKSCALE][Common::LOCATION][constants::Chassis::PARENT_ID] =
                    std::to_string(ch->get_location_offset());
            }
        }
        catch (const std::exception &e)
        {
            log_error(GET_LOGGER("app"), "Exception caught during filling Chassis"
                                         "links Contains and ContainedBy: "
                                             << e.what());
        }
    }
}

void fill_links(const agent_framework::model::Chassis &chassis, json::Value &r)
{
    // find manager of this chassis
    try
    {

        json::Value allowable;
        allowable.push_back(Chassis::FORCE_OFF);
        allowable.push_back(Chassis::GRACEFUL_SHUTDOWN);
        allowable.push_back(Chassis::GRACEFUL_RESTART);
        allowable.push_back(Chassis::FORCE_RESTART);

        json::Value actions;
        std::string hash_chassis_reset(constants::Common::HASH);
        hash_chassis_reset.append(constants::Chassis::CHASSIS_RESET);
        actions[hash_chassis_reset][Chassis::TARGET] = endpoint::PathBuilder(constants::PathParam::BASE_URL)
                                                           .append(Common::CHASSIS)
                                                           .append(chassis.get_id())
                                                           .append(constants::Common::ACTIONS)
                                                           .append(constants::Chassis::CHASSIS_RESET)
                                                           .build();
        actions[hash_chassis_reset][Chassis::ALLOWABLE_RESET_TYPES] = std::move(allowable);
        r[Common::ACTIONS] = std::move(actions);

        json::Value managed_by;
        managed_by[Common::ODATA_ID] = psme::rest::endpoint::utils::get_component_url(
            agent_framework::model::enums::Component::Manager, chassis.get_parent_uuid());
        r[Common::LINKS][Common::MANAGED_BY].push_back(managed_by);
    }
    catch (const agent_framework::exceptions::InvalidUuid &)
    {
    }

    // systems and storage subsystems in chassis
    auto &system_manager = agent_framework::module::CommonComponents::get_instance()->get_system_manager();
    auto &storage_manager = agent_framework::module::get_manager<agent_framework::model::StorageSubsystem>();
    auto system_uuids = system_manager.get_keys();
    for (const auto &system_uuid : system_uuids)
    {
        auto system = system_manager.get_entry(system_uuid);
        if (system.get_chassis() == chassis.get_uuid())
        {
            json::Value link;
            link[Common::ODATA_ID] = endpoint::PathBuilder(PathParam::BASE_URL)
                                         .append(constants::Root::SYSTEMS)
                                         .append(system.get_id())
                                         .build();
#ifdef CSYSTEMS
            r[Common::LINKS][constants::Chassis::COMPUTER_SYSTEMS].push_back(std::move(link));
#endif
            for (const auto storage_id : storage_manager.get_ids(system.get_uuid()))
            {
                json::Value storage_link;
                storage_link[Common::ODATA_ID] = endpoint::PathBuilder(PathParam::BASE_URL)
                                                     .append(constants::Root::SYSTEMS)
                                                     .append(system.get_id())
                                                     .append(constants::System::STORAGE)
                                                     .append(storage_id)
                                                     .build();
#ifdef CSTORAGE
                r[Common::LINKS][constants::Chassis::STORAGE].push_back(std::move(storage_link));
#endif
            }
        }
    }

    // switches in chassis
    auto &switch_manager = agent_framework::module::NetworkComponents::get_instance()->get_switch_manager();
    auto switch_uuids = switch_manager.get_keys();
    for (const auto &switch_uuid : switch_uuids)
    {
        auto switch_ = switch_manager.get_entry_reference(switch_uuid);
        if (switch_->get_chassis() == chassis.get_uuid())
        {
            json::Value link;
            link[Common::ODATA_ID] = endpoint::PathBuilder(PathParam::BASE_URL)
                                         .append(constants::Root::ETHERNET_SWITCHES)
                                         .append(switch_->get_id())
                                         .build();
            r[Common::LINKS][Common::OEM][Common::RACKSCALE][constants::Chassis::SWITCHES].push_back(std::move(link));
        }
    }

/*Nick Added Begin: */
#ifdef THERMAL_PSU
    // Thermal contains Fans and Temperatures Sensors //

    json::Value link; //  /Redfish/v1/Chassis/X/Thermal/
    link[Common::ODATA_ID] = endpoint::PathBuilder(PathParam::BASE_URL).append(Common::CHASSIS).append(chassis.get_id()).append(constants::Root::THERMAL).build();
    r[constants::Chassis::THERMAL] = link;

    // PSUs  in chassis

    //  /Redfish/v1/Chassis/X/Power/
    link[Common::ODATA_ID] = endpoint::PathBuilder(PathParam::BASE_URL).append(Common::CHASSIS).append(chassis.get_id()).append(constants::Root::PSU).build();
    r[constants::Chassis::PSU] = link;

#endif
    /*Nick Added End  : */

    // drives in chassis
    auto drive_ids = agent_framework::module::get_manager<agent_framework::model::Drive>().get_ids(chassis.get_uuid());
    for (const auto &drive_id : drive_ids)
    {
        json::Value link_elem(json::Value::Type::OBJECT);
        link_elem[Common::ODATA_ID] = endpoint::PathBuilder(constants::PathParam::BASE_URL)
                                          .append(Common::CHASSIS)
                                          .append(chassis.get_id())
                                          .append(constants::Chassis::DRIVES)
                                          .append(drive_id)
                                          .build();
        r[Common::LINKS][Chassis::DRIVES].push_back(std::move(link_elem));
    }
}

static const std::map<std::string, std::string> gami_to_rest_attributes = {
    {agent_framework::model::literals::Chassis::ASSET_TAG, constants::Common::ASSET_TAG}};

} // namespace

endpoint::Chassis::Chassis(const std::string& path) : EndpointBase(path) {}


endpoint::Chassis::~Chassis() {}


void endpoint::Chassis::get(const server::Request& req, server::Response& res) {
    auto r = make_prototype();
    r[Common::ODATA_ID] = PathBuilder(req).build();
    r[Common::ID] = req.params[PathParam::CHASSIS_ID];

    auto chassis = psme::rest::model::Find<agent_framework::model::Chassis>(req.params[PathParam::CHASSIS_ID]).get();

    psme::rest::endpoint::status_to_json(chassis, r);
    // for now, we assume that a chassis has no children
    r[Common::STATUS][Common::HEALTH_ROLLUP] = chassis.get_status().get_health();
    fill_links(chassis, r);
    fill_containing_links(chassis, r);

#ifndef COMCAST
    char command[256] = {0};
    char resultA[256] = {0};
    char IfName[256] = "ma1";	
    
    sprintf(command, "psme.sh get mgmt_port_name");	
    exec_shell(command, resultA);
    
    if(strlen(resultA) !=0 )	
       {
		sprintf(IfName, "%s", resultA);
    }
    
    sprintf(command, "lldp.sh get PortID %s", IfName);	
    exec_shell(command, resultA);
    
    if(strlen(resultA) !=0 )	
    {
        resultA[strcspn(resultA, "\r\n")]=0;
        r[Common::OEM][Common::RACKSCALE][Common::LOCATION][Common::ID] =  resultA;
    }	
	
    sprintf(command, "lldp.sh get ParentID %s", IfName);	
    exec_shell(command, resultA);
    
    if(strlen(resultA) !=0 )	
    {
        resultA[strcspn(resultA, "\r\n")]=0;
        r[Common::OEM][Common::RACKSCALE][Common::LOCATION][constants::Chassis::PARENT_ID] = resultA;
    }
#endif

    auto& sonlp = acc_onlp_helper::Switch::get_instance();
    r[Common::DESCRIPTION]                  = sonlp.get_product_name().c_str();
    r[Common::MANUFACTURER]                 = sonlp.get_manufacturer().c_str();
    r[Common::SERIAL]                       = sonlp.get_serial_number().c_str();
    r[Common::PART_NUMBER]                  = sonlp.get_part_number().c_str();
    r[Common::MODEL]                        = sonlp.get_platform_name().c_str(); 
    r[Common::OEM][Common::RACKSCALE][Common::ACT_OEM][Common::SERVICE_TAG]     
                                            = sonlp.get_service_tag().c_str();
    r[Common::ASSET_TAG]                    = chassis.get_asset_tag();
    r[constants::Chassis::SKU]              = chassis.get_sku();
    r[constants::Chassis::CHASSIS_TYPE]     = chassis.get_type().to_string();
    r[constants::Chassis::INDICATOR_LED]    = "Lit"; //Todo //

    set_response(res, r);
}

void endpoint::Chassis::patch(const server::Request &request, server::Response &response)
{
    auto chassis = model::Find<agent_framework::model::Chassis>(request.params[PathParam::CHASSIS_ID]).get();

    const auto json = JsonValidator::validate_request_body<schema::ChassisPatchSchema>(request);

    agent_framework::model::attribute::Attributes attributes{};

    if (json.is_member(constants::Common::ASSET_TAG))
    {
        const auto &asset_tag = json[constants::Common::ASSET_TAG].as_string();
        attributes.set_value(agent_framework::model::literals::Chassis::ASSET_TAG, asset_tag);
        /*Nick Added Begin: */
        char command[256] = {0};
        char resultA[256] = {0};
        sprintf(command, "echo %s > /etc/psme/ASSET_TAG", asset_tag.c_str());
        exec_shell(command, resultA);
        /*Nick Added End  : */
    }

    if (!attributes.empty())
    {
        agent_framework::model::requests::SetComponentAttributes
            set_component_attributes_request{chassis.get_uuid(), attributes};

        const auto &gami_agent = psme::core::agent::AgentManager::get_instance()->get_agent(chassis.get_agent_id());

        auto set_chassis_attributes = [&, gami_agent] {
            // Call set component attribute method
            const auto &set_component_attributes_response =
                gami_agent->execute<agent_framework::model::responses::SetComponentAttributes>(
                    set_component_attributes_request);

            const auto &result_statuses = set_component_attributes_response.get_statuses();
            if (!result_statuses.empty())
            {
                const auto &error = error::ErrorFactory::create_error_from_set_component_attributes_results(
                    result_statuses, gami_to_rest_attributes);
                throw error::ServerException(error);
            }

            psme::rest::model::handler::HandlerManager::get_instance()->get_handler(
                                                                          agent_framework::model::enums::Component::Chassis)
                ->load(gami_agent,
                       chassis.get_parent_uuid(),
                       agent_framework::model::enums::Component::Manager,
                       chassis.get_uuid(),
                       false);
        };

        gami_agent->execute_in_transaction(set_chassis_attributes);
    }
    get(request, response);
}
