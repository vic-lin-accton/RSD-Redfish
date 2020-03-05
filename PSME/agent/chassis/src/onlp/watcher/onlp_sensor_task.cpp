/*!
 * @section LICENSE
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
 * @section DESCRIPTION
 *
 * @file certificate.cpp
 * @brief Certificate
 * */
#include "agent-framework/module/network_components.hpp"
#include "agent-framework/module/chassis_components.hpp"
#include "agent-framework/module/common_components.hpp"
#include "agent-framework/command-ref/network_commands.hpp"
#include "agent-framework/eventing/event_data.hpp"
#include "agent-framework/eventing/events_queue.hpp"
#include <onlp/watcher/onlp_sensor_task.hpp>
#include <thread>

#include <acc_net_helper/acc_net_helper.hpp>
using namespace acc_net_helper;
using namespace agent_framework::model::attribute;

#ifdef ONLP
#include "acc_onlp_helper/acc_onlp_helper.hpp"
using namespace acc_onlp_helper;
#endif

using namespace std;
using namespace agent_framework::model;
using namespace agent_framework::module;
using namespace agent::chassis;
using namespace agent::chassis::onlp;
using namespace agent::chassis::onlp::watcher;
using namespace agent_framework::command_ref;
using namespace agent_framework::model::enums;
using namespace agent_framework::model::attribute;

using agent_framework::module::ChassisComponents;
using agent_framework::module::CommonComponents;
using agent_framework::module::NetworkComponents;
OnlpSensorTask::~OnlpSensorTask() {}

/*! Drawer onlp sensor processing*/
class GetOnlpInfo
{
public:
    /*!
     * Executes Drawer thermal sensor processing
     * @param[in] manager_keys Blades' manager keys
     */
    void execute()
    {
        get_onlp_info();
    }
    void exec_shell(const char *cmd, char *result_a);
    void get_onlp_port_info();
    void get_onlp_port_oom_info();
    void get_onlp_port_static_info();

private:
    void get_onlp_info();
    // Access transceiver oom content will block other PSU/Theremal detection .
    // remove to get_onlp_port_info get_onlp_port_oom_info thread //
};

void OnlpSensorTask::execute()
{
    try
    {
        GetOnlpInfo ps{};
        ps.execute();

        if (!m_port_detect_thread)
        {
            std::thread mThread_port(&GetOnlpInfo::get_onlp_port_info, &ps);
            mThread_port.detach();

            std::thread mThread_port_oom(&GetOnlpInfo::get_onlp_port_oom_info, &ps);
            mThread_port_oom.detach();

            m_port_detect_thread = true;
        }
    }
    catch (const std::exception &e)
    {
        log_debug(LOGUSR, "GetOnlpInfo - exception : " << e.what());
    }
}

void GetOnlpInfo::exec_shell(const char *cmd, char *result_a)
{
    char buffer[128];
    std::string result = "";
    FILE *pipe = popen(cmd, "r");
    if (!pipe)
        throw std::runtime_error("popen() failed!");
    try
    {
        while (!feof(pipe))
        {
            if (fgets(buffer, 128, pipe) != NULL)
                result += buffer;
        }
    }
    catch (...)
    {
        pclose(pipe);
        throw;
    }
    pclose(pipe);
    sprintf(result_a, "%s", result.c_str());

    return;
}

void GetOnlpInfo::get_onlp_info()
{
#ifdef ONLP
    try
    {
        auto &sonlp = Switch::Switch::get_instance();

        /*Get/Set  FAN info.*/
        sonlp.get_fan_info();
        unsigned int fan_num = sonlp.get_fan_num();

        unsigned int fanid = 1;

        for (fanid = 1; fanid <= fan_num; fanid++)
        {
            auto &fan_manager = agent_framework::module::ChassisComponents::get_instance()->get_fan_manager();
            auto fan_uuids = fan_manager.get_keys();

            for (const auto &fan_uuid : fan_uuids)
            {
                auto fan_ = fan_manager.get_entry_reference(fan_uuid); //Get Fan object by fan_uuid//

                if (fan_->get_fan_id() == fanid)
                {
                    int current_rpm = sonlp.get_fan_info_by_(fanid, Switch::Fan_Content::RPM);
                    fan_->set_current_speed(current_rpm);

                    int current_fan_type = sonlp.get_fan_info_by_(fanid, Switch::Fan_Content::Type);
                    fan_->set_fan_type(current_fan_type);

                    std::string current_health = sonlp.get_fan_info_by_(fanid, "Status_Health");
                    fan_->set_status_health(current_health);

                    std::string current_state = sonlp.get_fan_info_by_(fanid, "Status_State");
                    fan_->set_status_state(current_state);
                }
            }
        }

        sonlp.update_fan_present_event();

        /*Get/Set  PSU info.*/
        sonlp.get_psu_info();

        unsigned int psu_num = sonlp.get_psu_num();

        unsigned int psuid = 1;

        auto &psu_manager = agent_framework::module::ChassisComponents::get_instance()->get_psu_manager();

        auto psu_uuids = psu_manager.get_keys();

        for (psuid = 1; psuid <= psu_num; psuid++)
        {
            for (const auto &psu_uuid : psu_uuids)
            {
                auto psu_ = psu_manager.get_entry_reference(psu_uuid); //Get Psu object by psu_uuid//

                if (psu_->get_psu_id() == psuid)
                {
                    int p_in = sonlp.get_psu_info_by_(psuid, Switch::Psu_Content::Pin);
                    psu_->set_power_input(p_in);

                    int p_out = sonlp.get_psu_info_by_(psuid, Switch::Psu_Content::Pout);
                    psu_->set_power_output(p_out);

                    std::string current_health = sonlp.get_psu_info_by_(psuid, "Status_Health");
                    psu_->set_status_health(current_health);

                    std::string current_state = sonlp.get_psu_info_by_(psuid, "Status_State");
                    psu_->set_status_state(current_state);

                    int c_in = sonlp.get_psu_info_by_(psuid, Switch::Psu_Content::Iin);
                    psu_->set_current_input(c_in);

                    int c_out = sonlp.get_psu_info_by_(psuid, Switch::Psu_Content::Iout);
                    psu_->set_current_output(c_out);

                    int v_in = sonlp.get_psu_info_by_(psuid, Switch::Psu_Content::Vin);
                    psu_->set_voltage_input(v_in);

                    int v_out = sonlp.get_psu_info_by_(psuid, Switch::Psu_Content::Vout);
                    psu_->set_voltage_output(v_out);

                    int psu_type = sonlp.get_psu_info_by_(psuid, Switch::Psu_Content::Psu_type);
                    psu_->set_psu_type(psu_type);

                    std::string sn = sonlp.get_psu_info_by_(psuid, "SN");
                    psu_->set_psu_sn(sn);

                    std::string model = sonlp.get_psu_info_by_(psuid, "Model");
                    psu_->set_psu_module(model);
                }
            }
        }

        sonlp.update_psu_present_event();

        /*Get/Set  Thernal info.*/
        sonlp.get_thermal_info();

        unsigned int thermal_num = sonlp.get_thermal_num();
        unsigned int thermalid = 1;

        auto &tz_manager = agent_framework::module::ChassisComponents::get_instance()->get_thermal_zone_manager();

        auto tz_uuids = tz_manager.get_keys();

        for (thermalid = 1; thermalid <= thermal_num; thermalid++)
        {
            for (const auto &tz_uuid : tz_uuids)
            {
                auto tz_ = tz_manager.get_entry_reference(tz_uuid); //Get Psu object by psu_uuid//
                if (tz_->get_tz_id() == thermalid)
                {
                    int current_temp = sonlp.get_thermal_info_by_(thermalid, Switch::Thermal_Content::Current_Temperature);
                    tz_->set_temperature(current_temp);

                    int current_thermal_type = sonlp.get_thermal_info_by_(thermalid, Switch::Thermal_Content::Thermal_Type);
                    tz_->set_thermal_type(current_thermal_type);

                    std::string current_health = sonlp.get_thermal_info_by_(thermalid, "Status_Health");
                    tz_->set_status_health(current_health);

                    std::string current_state = sonlp.get_thermal_info_by_(thermalid, "Status_State");
                    tz_->set_status_state(current_state);

                    int warning_tmp = sonlp.get_thermal_info_by_(thermalid, Switch::Thermal_Content::Warning);
                    tz_->set_warning_temp(warning_tmp);

                    int error_tmp = sonlp.get_thermal_info_by_(thermalid, Switch::Thermal_Content::Error);
                    tz_->set_error_temp(error_tmp);

                    int shutdown_tmp = sonlp.get_thermal_info_by_(thermalid, Switch::Thermal_Content::Shutdown);
                    tz_->set_shutdown_temp(shutdown_tmp);
                }
            }
        }
        sonlp.update_thermal_present_event();
        /*Send all events */
        std::vector<std::string> tmp_e_a = sonlp.get_Event_Resouce_Add();

        for (unsigned int i = 0; i < tmp_e_a.size(); i++)
        {
            std::string t_es = tmp_e_a[i];
            agent_framework::eventing::EventData edat;
            edat.set_notification(::agent_framework::eventing::Notification::ResourceAdded);
            edat.set_event_content(t_es);
            agent_framework::eventing::EventsQueue::get_instance()->push_back(edat);
        }

        std::vector<std::string> tmp_e_r = sonlp.get_Event_Resouce_Remove();

        for (unsigned int i = 0; i < tmp_e_r.size(); i++)
        {
            std::string t_er = tmp_e_r[i];
            agent_framework::eventing::EventData edat;
            edat.set_notification(::agent_framework::eventing::Notification::ResourceRemoved);
            edat.set_event_content(t_er);
            agent_framework::eventing::EventsQueue::get_instance()->push_back(edat);
        }

        std::vector<std::string> tmp_e_al = sonlp.get_Event_Resouce_Alert();

        for (unsigned int i = 0; i < tmp_e_al.size(); i++)
        {
            std::string t_ea = tmp_e_al[i];
            agent_framework::eventing::EventData edat;
            edat.set_notification(::agent_framework::eventing::Notification::Alert);
            edat.set_event_content(t_ea);
            agent_framework::eventing::EventsQueue::get_instance()->push_back(edat);
        }
        sonlp.clean_Event_Resource_Event(); //Reset event //
    }
    catch (const std::exception &e)
    {
        log_debug(LOGUSR, "get_onlp_info - exception : " << e.what());
    }
#endif
}

[[noreturn]] void GetOnlpInfo::get_onlp_port_info() {
#ifdef ONLP
    get_onlp_port_static_info();

    auto &sonlp = Switch::Switch::get_instance();

    while (true)
    {
        try
        {
            /*Get/Set  Port info.*/
            sonlp.get_port_present_info();
            sonlp.update_port_present_event();
            /*Send all events */
            std::vector<std::string> tmp_e_a = sonlp.get_Event_Port_Resouce_Add();

            for (unsigned int i = 0; i < tmp_e_a.size(); i++)
            {
                std::string t_es = tmp_e_a[i];
                agent_framework::eventing::EventData edat;
                edat.set_notification(::agent_framework::eventing::Notification::ResourceAdded);
                edat.set_event_content(t_es);
                agent_framework::eventing::EventsQueue::get_instance()->push_back(edat);
            }

            std::vector<std::string> tmp_e_r = sonlp.get_Event_Port_Resouce_Remove();

            for (unsigned int i = 0; i < tmp_e_r.size(); i++)
            {
                std::string t_er = tmp_e_r[i];
                agent_framework::eventing::EventData edat;
                edat.set_notification(::agent_framework::eventing::Notification::ResourceRemoved);
                edat.set_event_content(t_er);
                agent_framework::eventing::EventsQueue::get_instance()->push_back(edat);
            }

            std::vector<std::string> tmp_e_al = sonlp.get_Event_Port_Resouce_Alert();

            for (unsigned int i = 0; i < tmp_e_al.size(); i++)
            {
                std::string t_ea = tmp_e_al[i];
                agent_framework::eventing::EventData edat;
                edat.set_notification(::agent_framework::eventing::Notification::Alert);
                edat.set_event_content(t_ea);
                agent_framework::eventing::EventsQueue::get_instance()->push_back(edat);
            }
            sonlp.clean_Event_Port_Resource_Event(); //Reset event //
        }
        catch (const std::exception &e)
        {
            log_debug(LOGUSR, "get_onlp_port_info - exception : " << e.what());
        }
        sleep(2);
    }
#endif
}

    [[noreturn]] void GetOnlpInfo::get_onlp_port_oom_info()
{
#ifdef ONLP
    auto &sonlp = Switch::Switch::get_instance();

    while (true)
    {
        try
        {
            /*Get/Set  Port info.*/
            sonlp.get_port_oom_info();

            unsigned int port_num = sonlp.get_port_num();
            unsigned int portid = 1;

            auto network_components = agent_framework::module::NetworkComponents::get_instance();
            auto &port_manager = network_components->get_instance()->get_port_manager();
            auto port_uuids = port_manager.get_keys();

            for (portid = 1; portid <= port_num; portid++)
            {
                for (const auto &port_uuid : port_uuids)
                {
                    auto port_ = port_manager.get_entry_reference(port_uuid); //Get Port object by psu_uuid//

                    if (port_->get_port_id() == portid)
                    {
                        bool current_present = sonlp.get_port_info_by_(portid, Switch::Port_Content::Port_Present);

                        if (current_present)
                        {
                            port_->set_status(attribute::Status(
                                agent_framework::model::enums::State::Enabled,
                                agent_framework::model::enums::Health::OK));
                        }
                        else
                        {
                            port_->set_status(attribute::Status(
                                agent_framework::model::enums::State::Absent,
                                agent_framework::model::enums::Health::OK));
                        }

                        //Collect  trans info
                        json::Value r(json::Value::Type::OBJECT);
                        attribute::TransInfo tTransInfo;

                        r = sonlp.get_port_trans_info_by_(portid);
                        if (r != json::Value::Type::NIL)
                        {
                            tTransInfo.set_spf_vendor_name(r["SFP Vendor Name"]);
                            tTransInfo.set_part_number(r["Part Number"]);
                            tTransInfo.set_serial_number(r["Serial Number"]);
                            tTransInfo.set_manufacture_date(r["Manufacture Date"]);

                            tTransInfo.set_temp_reading(r["Temperature"]["Reading"]);
                            tTransInfo.set_temp_upper_th_fatal(r["Temperature"]["UpperThresholdFatal"]);
                            tTransInfo.set_temp_upper_th_critical(r["Temperature"]["UpperThresholdCritical"]);
                            tTransInfo.set_temp_lower_th_critical(r["Temperature"]["LowerThresholdCritical"]);
                            tTransInfo.set_temp_lower_th_fatal(r["Temperature"]["LowerThresholdFatal"]);
                            tTransInfo.set_temp_status_state(r["Temperature"]["Status"]["State"]);
                            tTransInfo.set_temp_status_health(r["Temperature"]["Status"]["Health"]);

                            tTransInfo.set_voltage_reading(r["Voltage"]["Reading"]);
                            tTransInfo.set_voltage_upper_th_fatal(r["Voltage"]["UpperThresholdFatal"]);
                            tTransInfo.set_voltage_upper_th_critical(r["Voltage"]["UpperThresholdCritical"]);
                            tTransInfo.set_voltage_lower_th_critical(r["Voltage"]["LowerThresholdCritical"]);
                            tTransInfo.set_voltage_lower_th_fatal(r["Voltage"]["LowerThresholdFatal"]);
                            tTransInfo.set_voltage_status_state(r["Voltage"]["Status"]["State"]);
                            tTransInfo.set_voltage_status_health(r["Voltage"]["Status"]["Health"]);

                            tTransInfo.set_bias_current_reading(r["BiasCurrent"]["Reading"]);
                            tTransInfo.set_bias_current_upper_th_fatal(r["BiasCurrent"]["UpperThresholdFatal"]);
                            tTransInfo.set_bias_current_upper_th_critical(r["BiasCurrent"]["UpperThresholdCritical"]);
                            tTransInfo.set_bias_current_lower_th_critical(r["BiasCurrent"]["LowerThresholdCritical"]);
                            tTransInfo.set_bias_current_lower_th_fatal(r["BiasCurrent"]["LowerThresholdFatal"]);
                            tTransInfo.set_bias_current_status_state(r["BiasCurrent"]["Status"]["State"]);
                            tTransInfo.set_bias_current_status_health(r["BiasCurrent"]["Status"]["Health"]);

                            tTransInfo.set_tx_power_reading(r["TxPower"]["Reading"]);
                            tTransInfo.set_tx_power_upper_th_fatal(r["TxPower"]["UpperThresholdFatal"]);
                            tTransInfo.set_tx_power_upper_th_critical(r["TxPower"]["UpperThresholdCritical"]);
                            tTransInfo.set_tx_power_lower_th_critical(r["TxPower"]["LowerThresholdCritical"]);
                            tTransInfo.set_tx_power_lower_th_fatal(r["TxPower"]["LowerThresholdFatal"]);
                            tTransInfo.set_tx_power_status_state(r["TxPower"]["Status"]["State"]);
                            tTransInfo.set_tx_power_status_health(r["TxPower"]["Status"]["Health"]);

                            tTransInfo.set_rx_power_reading(r["RxPower"]["Reading"]);
                            tTransInfo.set_rx_power_upper_th_fatal(r["RxPower"]["UpperThresholdFatal"]);
                            tTransInfo.set_rx_power_upper_th_critical(r["RxPower"]["UpperThresholdCritical"]);
                            tTransInfo.set_rx_power_lower_th_critical(r["RxPower"]["LowerThresholdCritical"]);
                            tTransInfo.set_rx_power_lower_th_fatal(r["RxPower"]["LowerThresholdFatal"]);
                            tTransInfo.set_rx_power_status_state(r["RxPower"]["Status"]["State"]);
                            tTransInfo.set_rx_power_status_health(r["RxPower"]["Status"]["Health"]);

                            port_->set_trans_info(tTransInfo);

                            attribute::Onu_Trans_Rx_Pwr_Info tOnu_Trans_Rx_Pwr_Info;

                            tOnu_Trans_Rx_Pwr_Info.set_rx_power_reading(r["RxPower"]["Reading"]);
                            tOnu_Trans_Rx_Pwr_Info.set_rx_power_upper_th_fatal(r["RxPower"]["UpperThresholdFatal"]);
                            tOnu_Trans_Rx_Pwr_Info.set_rx_power_upper_th_critical(r["RxPower"]["UpperThresholdCritical"]);
                            tOnu_Trans_Rx_Pwr_Info.set_rx_power_lower_th_critical(r["RxPower"]["LowerThresholdCritical"]);
                            tOnu_Trans_Rx_Pwr_Info.set_rx_power_lower_th_fatal(r["RxPower"]["LowerThresholdFatal"]);
                            tOnu_Trans_Rx_Pwr_Info.set_rx_power_status_state(r["RxPower"]["Status"]["State"]);
                            tOnu_Trans_Rx_Pwr_Info.set_rx_power_status_health(r["RxPower"]["Status"]["Health"]);

                            port_->set_trans_rx_pwr_info(tOnu_Trans_Rx_Pwr_Info);
                        }
                    }
                }
            }
        }
        catch (const std::exception &e)
        {
            log_debug(LOGUSR, "get_onlp_port_oom_info - exception : " << e.what());
        }
        sleep(2);
    }
#endif
}

void GetOnlpInfo::get_onlp_port_static_info()
{
#ifdef ONLP
    auto &sonlp = Switch::Switch::get_instance();
    try
    {
        /*Get/Set Port info.*/
        unsigned int port_num = sonlp.get_port_num();
        unsigned int portid = 1;

        auto network_components = agent_framework::module::NetworkComponents::get_instance();
        auto &port_manager = network_components->get_instance()->get_port_manager();
        auto port_uuids = port_manager.get_keys();

        for (portid = 1; portid <= port_num; portid++)
        {
            for (const auto &port_uuid : port_uuids)
            {
                auto port_ = port_manager.get_entry_reference(port_uuid); //Get Port object by psu_uuid//

                if (port_->get_port_id() == portid)
                {
                    if (sonlp.get_port_info_by_(portid, Switch::Port_Content::Port_Type) == acc_onlp_helper::Port_Info::Port_Type::PON_Port)
                    {
                        port_->set_port_type(PortType::Downstream);
                        port_->set_port_identifier("PON port");
                    }
                    else if (sonlp.get_port_info_by_(portid, Switch::Port_Content::Port_Type) == acc_onlp_helper::Port_Info::Port_Type::SFP_Port)
                    {
                        port_->set_port_type(PortType::MeshPort);
                        port_->set_port_identifier("SFP port");
                    }
                    else if (sonlp.get_port_info_by_(portid, Switch::Port_Content::Port_Type) == acc_onlp_helper::Port_Info::Port_Type::QSFP_Port)
                    {
                        port_->set_port_type(PortType::MeshPort);
                        port_->set_port_identifier("QSFP port");
                    }
                    else
                    {
                        port_->set_port_type(PortType::MeshPort);
                        port_->set_port_identifier("RJ45 port");
                    }
                }
            }
        }
    }
    catch (const std::exception &e)
    {
        log_debug(LOGUSR, "get_onlp_port_static_info - exception : " << e.what());
    }
#endif
}
