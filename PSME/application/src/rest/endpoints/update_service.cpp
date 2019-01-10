/*!
 * @copyright
 * Copyright (c) 2016-2017 Intel Corporation
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
#include "psme/rest/endpoints/update_service.hpp"
#include "psme/rest/utils/status_helpers.hpp"
#include "psme/rest/server/error/error_factory.hpp"
#include "psme/rest/model/handlers/handler_manager.hpp"
#include "psme/rest/validators/json_validator.hpp"

using namespace psme::rest::validators;
using namespace agent_framework::model::enums;
using namespace psme::rest;
using namespace psme::rest::error;

namespace {

json::Value make_update_service_prototype() {
    json::Value r(json::Value::Type::OBJECT);

    r[constants::Common::ODATA_CONTEXT] = "/redfish/v1/$metadata#UpdateService.UpdateService";
    r[constants::Common::ODATA_ID] = json::Value::Type::NIL;
    r[constants::Common::ID] = "1";
    r[constants::Common::ODATA_TYPE] = "#UpdateService.v1_2_0.UpdateService";
    r[constants::Common::NAME] = "Update Service";
    r[constants::Common::DESCRIPTION] = "Update Service";


    json::Value image_path;
    image_path.push_back("");


    json::Value actions;
    std::string update_service("#");
    update_service.append("UpdateService.SimpleUpdate");
    actions[update_service]["target"] = "/redfish/v1/UpdateService";
    actions[update_service]["ImageURI"] = std::move(image_path);
    r["Actions"] = std::move(actions);
	
    return r;

}
}
namespace psme {
namespace rest {
namespace endpoint {
endpoint::UpdateService::UpdateService(const std::string& path) : EndpointBase(path) {}
endpoint::UpdateService::~UpdateService() {}

void endpoint::UpdateService::get(const server::Request& request, server::Response& response) {

    json::Value r = make_update_service_prototype();
    r[constants::Common::ODATA_ID] = PathBuilder(request).build();
    set_response(response, r);
    return;	
}

void endpoint::UpdateService::post(const server::Request& request, server::Response& response) {


    json::Value schema({
        JsonValidator::mandatory("ImageURI", JsonValidator::has_type(JsonValidator::STRING_TYPE))
    });
    const auto json = JsonValidator::validate_request_body(request, schema);
    if (json.is_member("ImageURI"))
    {	   
        std::string ImageURI =json["ImageURI"].as_string();
        printf("ImageURI[%s]\r\n", ImageURI.c_str());
        char command[256] = {0};
        char resultA[256] = {0};
        
        sprintf(command, "psme.sh get update_sw \"%s\" VOLT" ,ImageURI.c_str());	
        exec_shell(command, resultA);	   

        if(!strncmp(resultA, "NOT support.", 12)) //Device not support
            response.set_status(server::status_5XX::NOT_IMPLEMENTED);
        else if(!strncmp(resultA, "OK", 2))        //get file , update OK //
            response.set_status(server::status_2XX::OK);
        else if(!strncmp(resultA, "ERROR", 2))  //file not found //
            response.set_status(server::status_4XX::NOT_FOUND);
        else
            response.set_status(server::status_4XX::BAD_REQUEST);			
        return;	   
    }
    response.set_status(server::status_4XX::NOT_FOUND); //file not found //
    return;
}

}
}
}

