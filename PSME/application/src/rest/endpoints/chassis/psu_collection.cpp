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
#include "agent-framework/module/chassis_components.hpp"
#include "psme/rest/endpoints/chassis/psu_collection.hpp"
#include "psme/rest/constants/constants.hpp"
#include "psme/rest/endpoints/utils.hpp"

#ifdef ONLP
#include "acc_onlp_helper/acc_onlp_helper.hpp"
using namespace acc_onlp_helper;
#endif

using namespace psme::rest;
using namespace psme::rest::endpoint;
using namespace psme::rest::constants;
using agent_framework::module::ChassisComponents;

namespace
{
json::Value make_prototype()
{
    json::Value r(json::Value::Type::OBJECT);

    r[Common::ODATA_CONTEXT] = "/redfish/v1/$metadata#Power.Power";
    r[Common::ODATA_ID] = json::Value::Type::NIL;
    r[Common::ID] = "Power";
    r[Common::ODATA_TYPE] = "#Power.v1_1_0.Power";
    r[Common::NAME] = "Power Collection";
    r[Common::DESCRIPTION] = "Collection of Power";

    return r;
}
} // namespace

PsuCollection::PsuCollection(const std::string &path) : EndpointBase(path) {}

PsuCollection::~PsuCollection() {}

void PsuCollection::get(const server::Request &req, server::Response &res)
{

#ifdef ONLP
    // Use ONLP library to to get ONLP info.

    //Set PSU related info. //
    try
    {
        auto json = ::make_prototype();

        json[Common::ODATA_ID] = PathBuilder(req).build();
        json[Common::ID] = "Power";
        //For PSU
        auto chassis_ids = ChassisComponents::get_instance()->get_thermal_zone_manager().get_ids();
        auto chassis = psme::rest::model::Find<agent_framework::model::Chassis>(req.params[PathParam::CHASSIS_ID]).get();

        auto &psu_manager = agent_framework::module::ChassisComponents::get_instance()->get_psu_manager();
        auto psu_uuids = psu_manager.get_keys();

        for (const auto &psu_uuid : psu_uuids)
        {
            auto psu_ = psu_manager.get_entry(psu_uuid); //Get Fan object by fan_uuid//

            json::Value jsonctrl;
            {
                jsonctrl[Common::ODATA_ID] = endpoint::PathBuilder(PathParam::BASE_URL).append(Common::CHASSIS).append(chassis.get_id()).append("Power").build();

                jsonctrl[Common::MEMBER_ID] = std::to_string(psu_.get_psu_id());
                jsonctrl[Common::NAME] = "System Power Control";

                jsonctrl[PowerZone::POWER_CONSUMED] = (psu_.get_power_output() * 0.001);
                jsonctrl["Status"]["Health"] = psu_.get_status_health();
                jsonctrl["Status"]["State"] = psu_.get_status_state();

                jsonctrl["PowerRequestedWatts"] = json::Value::Type::NIL;
                jsonctrl["PowerAvailableWatts"] = json::Value::Type::NIL;
                jsonctrl["PowerCapacityWatts"] = 0;
                jsonctrl["PowerAllocatedWatts"] = json::Value::Type::NIL;

                json::Value jsonctrl_Metrics;

                jsonctrl_Metrics["PowerMetrics"]["IntervalInMin"] = json::Value::Type::NIL;
                jsonctrl_Metrics["PowerMetrics"]["MinConsumedWatts"] = json::Value::Type::NIL;
                jsonctrl_Metrics["PowerMetrics"]["MaxConsumedWatts"] = json::Value::Type::NIL;
                jsonctrl_Metrics["PowerMetrics"]["AverageConsumedWatts"] = json::Value::Type::NIL;
                jsonctrl.push_back(std::move(jsonctrl_Metrics));

                jsonctrl["PowerLimit"]["LimitInWatts"] = json::Value::Type::NIL;
                jsonctrl["PowerLimit"]["LimitException"] = json::Value::Type::NIL;
                jsonctrl["PowerLimit"]["CorrectionInMs"] = json::Value::Type::NIL;

                json::Value lrejsontmp;
                lrejsontmp[Common::ODATA_ID] = endpoint::PathBuilder(PathParam::BASE_URL).append(Common::CHASSIS).append(chassis.get_id()).build();
                jsonctrl["RelatedItem"].push_back(std::move(lrejsontmp));

                jsonctrl["Oem"] = json::Value::Type::OBJECT;

                json["PowerControl"].push_back(std::move(jsonctrl));
            }

            json::Value jsonVoltages;
            {
                int id = psu_.get_psu_id(); 
                jsonVoltages[Common::ODATA_ID] = endpoint::PathBuilder(PathParam::BASE_URL).append(Common::CHASSIS).append(chassis.get_id()).append("Power").build();
                jsonVoltages[Common::MEMBER_ID] = std::to_string(psu_.get_psu_id());
                jsonVoltages[Common::NAME] = "Voltage";
                jsonVoltages["SensorNumber"] = id; 
                jsonVoltages["ReadingVolts"] = (psu_.get_voltage_output() * 0.001); 
                jsonVoltages["UpperThresholdNonCritical"] = json::Value::Type::NIL;
                jsonVoltages["UpperThresholdCritical"] = json::Value::Type::NIL;
                jsonVoltages["UpperThresholdFatal"] = json::Value::Type::NIL;
                jsonVoltages["LowerThresholdNonCritical"] = json::Value::Type::NIL;
                jsonVoltages["LowerThresholdCritical"] = json::Value::Type::NIL;
                jsonVoltages["LowerThresholdFatal"] = json::Value::Type::NIL;
                jsonVoltages["PhysicalContext"] = "VoltageRegulator";
                jsonVoltages["Status"]["State"] = "Enabled";
                jsonVoltages["Status"]["Health"] = "OK";

                json::Value lrejsontmp;
                lrejsontmp[Common::ODATA_ID] = endpoint::PathBuilder(PathParam::BASE_URL).append(Common::CHASSIS).append(chassis.get_id()).build();
                jsonVoltages["RelatedItem"].push_back(std::move(lrejsontmp));
            }

            json["Voltages"].push_back(std::move(jsonVoltages));

            json::Value json_PowerSupplies;
            {
                json_PowerSupplies[Common::ODATA_ID] = endpoint::PathBuilder(PathParam::BASE_URL).append(Common::CHASSIS).append(chassis.get_id()).append("Power").build();

                json_PowerSupplies[Common::MEMBER_ID] = std::to_string(psu_.get_psu_id());
                json_PowerSupplies[Common::NAME] = "Power Supplies Unit";
                json_PowerSupplies["Oem"] = json::Value::Type::OBJECT;

                switch (psu_.get_psu_type())
                {
                case 0:
                    json_PowerSupplies["PowerSupplyType"] = "AC";
                    break;
                case 1:
                    json_PowerSupplies["PowerSupplyType"] = "DC";
                    break;
                case 2:
                    json_PowerSupplies["PowerSupplyType"] = "DC";
                    break;
                default:
                    json_PowerSupplies["PowerSupplyType"] = "Unknown";
                }

                json_PowerSupplies["LineInputVoltageType"] = json::Value::Type::NIL;
                json_PowerSupplies["LineInputVoltage"] = json::Value::Type::NIL;
                json_PowerSupplies["PowerCapacityWatts"] = json::Value::Type::NIL;
                json_PowerSupplies["LastPowerOutputWatts"] = (psu_.get_power_output() * 0.001);
                json_PowerSupplies["Model"] = psu_.get_psu_module();
                json_PowerSupplies["Manufacturer"] = json::Value::Type::NIL;
                json_PowerSupplies["FirmwareVersion"] = json::Value::Type::NIL;
                json_PowerSupplies["SerialNumber"] = psu_.get_psu_sn();
                json_PowerSupplies["PartNumber"] = json::Value::Type::NIL;
                json_PowerSupplies["SparePartNumber"] = json::Value::Type::NIL;
                json_PowerSupplies["Status"]["State"] = psu_.get_status_state();
                json_PowerSupplies["Status"]["Health"] = psu_.get_status_health();

                json::Value json_input_range;

                json_input_range["InputType"] = json::Value::Type::NIL;
                json_input_range["MinimumVoltage"] = json::Value::Type::NIL;
                json_input_range["MaximumVoltage"] = json::Value::Type::NIL;
                json_input_range["OutputWattage"] = json::Value::Type::NIL;
                //json_PowerSupplies["InputRanges"].push_back(std::move(json_input_range));

                json::Value lrejsontmp;
                lrejsontmp[Common::ODATA_ID] = endpoint::PathBuilder(PathParam::BASE_URL).append(Common::CHASSIS).append(chassis.get_id()).build();
                json_PowerSupplies["RelatedItem"].push_back(std::move(lrejsontmp));
            }

            json["PowerSupplies"].push_back(std::move(json_PowerSupplies));
        }
        json["Oem"] = json::Value::Type::OBJECT;
        set_response(res, json);
    }
    catch (const std::exception &e)
    {
        log_debug(LOGUSR, "PsuCollection get - exception : " << e.what());
    }
#endif
}
