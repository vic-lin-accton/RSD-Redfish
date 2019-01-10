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

#include "psme/rest/endpoints/manager_log_services_entries_collection.hpp"
#include "psme/rest/constants/constants.hpp"
#include <arpa/inet.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <fstream>

#include "acc_net_helper/acc_net_helper.hpp"
using namespace acc_net_helper;

using namespace psme::rest;
using namespace psme::rest::constants;
using namespace agent_framework::model::enums;

namespace {
json::Value make_prototype() {
    json::Value r(json::Value::Type::OBJECT);

    r[Common::ODATA_CONTEXT] = "/redfish/v1/$metadata#LogEntryCollection.LogEntryCollection";
    r[Common::ODATA_ID] = json::Value::Type::NIL;
    r[Common::ODATA_TYPE] = "#LogEntryCollection.LogEntryCollection";
    r[Common::NAME] = "Log Service Collection";
    r[Common::DESCRIPTION] = "Collection of Logs for this System";
    r[Collection::ODATA_COUNT] = 0;
    r[Collection::MEMBERS] = json::Value::Type::ARRAY;
	
    return r;
}
}

endpoint::ManagerLogServicesEntriesCollection::ManagerLogServicesEntriesCollection(const std::string& path) : EndpointBase(path) {}

endpoint::ManagerLogServicesEntriesCollection::~ManagerLogServicesEntriesCollection() {}

void endpoint::ManagerLogServicesEntriesCollection::get(const server::Request& req, server::Response& res) {

    auto jsons = ::make_prototype();

    jsons[Common::ODATA_ID] = PathBuilder(req).build();
    int entrynum = 0;

    int ii = 0;
    
    Json::Value aroot;
    RFLogEntry Entry;
    aroot = Entry.get_log_entry_content();
	
    try 
    {
        entrynum = aroot["MEM"].size();
    	
        if(aroot["MEM"].isArray() && entrynum != 0)
            jsons[Collection::ODATA_COUNT] = entrynum >= Entry.get_max_entries_num() ? Entry.get_max_entries_num() : entrynum ;
        else
            jsons[Collection::ODATA_COUNT] = 0;	
    }
    catch (const std::exception& ex)
    {
        log_warning(GET_LOGGER("rest"),"Read manage error!!");
    }	
			
    try 
    {
        for(ii = 0 ; (ii < entrynum && ii <  Entry.get_max_entries_num()) ; ii++)
        {
            Json::Value root = aroot["MEM"][ii];            
            const Json::Value defValue; //used for default reference		
            const Json::Value s = root.get("ENTRY", defValue);
			
            if (s.isObject())
            {
                json::Value entry(json::Value::Type::OBJECT);
                entry[Common::ODATA_ID] = PathBuilder(req).append(ii).build();

                entry[Common::ID] = std::to_string(ii);
	       entry[Common::NAME] = "Log Entry";

                Json::Value s2 = s.get("EntryType", "");
                entry[Manager::ENTRYTYPE] = s2.asString();

                s2 = s.get("Severity", "");
                entry[Manager::SEVERITY] = s2.asString();	
				
                s2 = s.get("Created", "");
                entry[Manager::CREATED] = s2.asString();

                s2 = s.get("EntryCode", "");
                entry[Manager::ENTRY_CODE] = s2.asString();

                s2 = s.get("SensorType", "");
                entry[Manager::SENSOR_TYPE] = s2.asString();
		
                s2 = s.get("SensorNumber", "");
				
                entry[Manager::SENSOR_NUMBER] = s2.asInt();

                s2 = s.get("Message", "");
				
                entry[Manager::MESSAGE] = s2.asString();	
				
                jsons[Collection::MEMBERS].push_back(std::move(entry));
            }               
        }
    }	
    catch (const std::exception& ex)
    {
        log_warning(GET_LOGGER("rest"),"Read manage error!!");
    }

    set_response(res, jsons);
}
