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

#include "psme/rest/endpoints/manager_log_services_entries.hpp"
#include "psme/rest/constants/constants.hpp"
#include <arpa/inet.h>

/*To get terminal info. Begin:*/
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <fstream>
/*To get terminal info. End  :*/
#include "acc_net_helper/acc_net_helper.hpp"


using namespace psme::rest;
using namespace psme::rest::constants;
using namespace agent_framework::model::enums;
using namespace acc_net_helper;

namespace {
json::Value make_prototype() {
    json::Value r(json::Value::Type::OBJECT);

    r[Common::ODATA_CONTEXT] = "/redfish/v1/$metadata#LogEntry.LogEntry";
    r[Common::ODATA_ID] = json::Value::Type::NIL;
    r[Common::ODATA_TYPE] = "#LogEntry.v1_0_0.LogEntry";
    r[Common::NAME] = "Log Entry";
    r[Manager::ENTRYTYPE] = json::Value::Type::NIL;
    r[Manager::SEVERITY] = json::Value::Type::NIL;
    r[Manager::CREATED] = json::Value::Type::NIL;
    r[Manager::ENTRY_CODE] = json::Value::Type::NIL;
    r[Manager::SENSOR_TYPE] = json::Value::Type::NIL;
    r[Manager::SENSOR_NUMBER] = json::Value::Type::NIL;
    r[Manager::MESSAGE] = json::Value::Type::NIL;
    return r;
}
}

endpoint::ManagerLogServicesEntries::ManagerLogServicesEntries(const std::string& path) : EndpointBase(path) {}

endpoint::ManagerLogServicesEntries::~ManagerLogServicesEntries() {}

void endpoint::ManagerLogServicesEntries::get(const server::Request& req, server::Response& res) {

    int EntryID= atoi(req.params[PathParam::LOGSER_ID].c_str());
	
    auto r = ::make_prototype();
    r[Common::ODATA_ID] = PathBuilder(req).build();
    r[Common::ID] = req.params[PathParam::LOGSER_ID].c_str();

	
    try 
    {       
        Json::Value aroot;
        Json::Reader reader;
        std::ifstream ifile("/var/log/rf_server.log");
        bool isJsonOK = (ifile != NULL && reader.parse(ifile, aroot));  

        if(isJsonOK)
        {              
            Json::Value root = aroot["MEM"][EntryID];        
            const Json::Value defValue; //used for default reference					
            const Json::Value s = root.get("ENTRY", defValue);
            if (s.isObject())
            {
                Json::Value s2 = s.get("EntryType", "");
                r[Manager::ENTRYTYPE] = s2.asString();

                s2 = s.get("Severity", "");
                r[Manager::SEVERITY] = s2.asString();	
				
                s2 = s.get("Created", "");
                r[Manager::CREATED] = s2.asString();

                s2 = s.get("EntryCode", "");
                r[Manager::ENTRY_CODE] = s2.asString();

                s2 = s.get("SensorType", "");
                r[Manager::SENSOR_TYPE] = s2.asString();
/*
                if(!strcmp(s2.asString().c_str(), "Fan") || !strcmp(s2.asString().c_str(), "Temperature") )
                {
                    json::Value link;
                    link[Common::ODATA_ID] = "/redfish/v1/Chassis/1/Thermal";
                    r[Common::LINKS]["OriginOfCondition"].push_back(std::move(link));
                }
                else if(!strcmp(s2.asString().c_str(), "PowerUnit"))
                {
                    json::Value link;
                    link[Common::ODATA_ID] = "/redfish/v1/Chassis/1/Power";
                    r[Common::LINKS]["OriginOfCondition"].push_back(std::move(link));
                }
*/					
                s2 = s.get("SensorNumber", "");
                r[Manager::SENSOR_NUMBER] = s2.asInt();

                s2 = s.get("Message", "");
                r[Manager::MESSAGE] = s2.asString();						
            }               
        }	
    }
    catch (const std::exception& ex)
    {
        log_warning(GET_LOGGER("rest"),"Read manage error!!");
    
    }
    
    set_response(res, r);
}

