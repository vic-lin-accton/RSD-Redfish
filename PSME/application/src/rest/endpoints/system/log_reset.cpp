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

#include "agent-framework/module/requests/common.hpp"
#include "agent-framework/module/responses/common.hpp"
#include "agent-framework/module/constants/compute.hpp"

#include "psme/core/agent/agent_manager.hpp"
#include "psme/rest/constants/constants.hpp"
#include "psme/rest/validators/json_validator.hpp"
#include "psme/rest/validators/schemas/reset.hpp"
#include "psme/rest/endpoints/system/log_reset.hpp"
#include "psme/rest/server/error/error_factory.hpp"
#include "psme/rest/model/handlers/generic_handler_deps.hpp"
#include "psme/rest/model/handlers/generic_handler.hpp"
#include "acc_net_helper/acc_net_helper.hpp"


using namespace psme::rest;
using namespace psme::rest::constants;
using namespace psme::rest::validators;
using namespace acc_net_helper;

endpoint::LogReset::LogReset(const std::string& path) : EndpointBase(path) {}


endpoint::LogReset::~LogReset() {}


void endpoint::LogReset::post(const server::Request& request, server::Response& response) 
{
    RFLogEntry Entry;
    Entry.clean_log();
/*
    char command[256] = {0};
    char resultA[256] = {0};

    sprintf(command, "%s" ,"logsrv.sh set Reset");	
    exec_shell(command, resultA);
*/
    response.set_status(server::status_2XX::OK);
}
