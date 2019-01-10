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

#include "psme/rest/endpoints/manager_log_services.hpp"
#include "psme/rest/constants/constants.hpp"
#include "psme/rest/validators/json_validator.hpp"
#include <arpa/inet.h>

/*To get terminal info. Begin:*/
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
/*To get terminal info. End  :*/
#include "acc_net_helper/acc_net_helper.hpp"
using namespace acc_net_helper;

using namespace psme::rest;
using namespace psme::rest::constants;
using namespace agent_framework::model::enums;
using namespace psme::rest::validators;

namespace {
json::Value make_prototype() {
    json::Value r(json::Value::Type::OBJECT);

    r[Common::ODATA_CONTEXT] = "/redfish/v1/$metadata#LogService.LogService";
    r[Common::ODATA_ID] = json::Value::Type::NIL;
    r[Common::ODATA_TYPE] = "#LogService.v1_0_0.LogService";
    r[Common::NAME] = "System Log Service";
    r[Common::DESCRIPTION] = "ONL Peripheral Log";
    r[Manager::DATE_TIME] =json::Value::Type::NIL;
    r[Manager::DATE_TIME_LOCAL_OFFSET] =json::Value::Type::NIL;
    r[Manager::ENTRIES] =json::Value::Type::NIL;
    r[Manager::MAX_NUM_RECS] =json::Value::Type::NIL;
    r[Manager::OVERWRITE_POLICY] = json::Value::Type::NIL;
    r[Manager::SERVICE_ENABLED]  = json::Value::Type::NIL;
	
    return r;
}
}

endpoint::ManagerLogServices::ManagerLogServices(const std::string& path) : EndpointBase(path) {}

endpoint::ManagerLogServices::~ManagerLogServices() {}

void endpoint::ManagerLogServices::get(const server::Request& req, server::Response& res) {

    auto r = ::make_prototype();
    r[Common::ODATA_ID] = PathBuilder(req).build();
    r[Common::ID] = req.params[PathParam::LOGSER_ID];


    json::Value actions;
    std::string log_serviec_reset(constants::Common::HASH);
    log_serviec_reset.append("LogService.ClearLog");
    actions[log_serviec_reset][System::TARGET] = PathBuilder(req).append(constants::Common::ACTIONS).append(constants::Management::LOG_SERVICE_RESET).build();
    r[Common::ACTIONS] = std::move(actions);

    json::Value entry;
    entry[Common::ODATA_ID] = PathBuilder(req).append("Entries").build();
    r[Common::ENTRIES] = entry;
	
    try 
    {
        RFLogEntry Entry;
		
        r[Manager::DATE_TIME] =Entry.get_current_time();
		
        r[Manager::DATE_TIME_LOCAL_OFFSET] = Entry.get_zone_offset();
        
        r[Manager::MAX_NUM_RECS] = Entry.get_max_entries_num();
        
        r[Manager::OVERWRITE_POLICY] = "WrapsWhenFull";

        if(Entry.get_log_status())                
            r[Manager::SERVICE_ENABLED]  = true;
        else
            r[Manager::SERVICE_ENABLED]  = false;
			
        r[Common::STATUS][Common::STATE] = "Enabled";
		
        r[Common::STATUS][Common::HEALTH] ="OK";
    }
    catch (const std::exception& ex)
    {
        log_warning(GET_LOGGER("rest"),"Read manage error!!");
    
    }
    set_response(res, r);
}

json::Value validate_post_request(const server::Request& request);

json::Value validate_post_request(const server::Request& request) {
    json::Value schema({JsonValidator::mandatory("ServiceEnabled",JsonValidator::has_type(JsonValidator::BOOL_TYPE))});
    return  JsonValidator::validate_request_body(request, schema);
}

void endpoint::ManagerLogServices::patch(const server::Request& request, server::Response& response) {

    const auto json = validate_post_request(request);

    int status = json[Manager::SERVICE_ENABLED].as_bool();;
    RFLogEntry Entry;
	
    if(status == 1)
        Entry.set_log_status(true);    
    else if (status == 0)
        Entry.set_log_status(false);    
    response.set_status(server::status_2XX::NO_CONTENT);
}

