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

#define UNUSED(x) (void)(x)

#include "psme/rest/endpoints/olt.hpp"
#include "psme/rest/endpoints/utils.hpp"
#include "psme/rest/validators/json_validator.hpp"
#include "psme/rest/validators/schemas/olt.hpp"
#include "psme/rest/constants/constants.hpp"
#include "json/json.hpp"
#include "psme/rest/server/error/error_factory.hpp"
#include "psme/rest/server/error/server_exception.hpp"
#include "psme/rest/server/parameters.hpp"
#ifdef BAL34
#include "acc_bal3_api_dist_helper/acc_bal3_api_dist_helper.hpp"
using namespace acc_bal3_api_dist_helper;
#else
#define UNUSED(x) (void)(x)
#endif

using namespace psme::rest;
using namespace psme::rest::server;
using namespace psme::rest::endpoint::utils;
using namespace psme::rest::validators;
using namespace psme::rest::constants;

namespace
{
json::Value make_prototype()
{
    json::Value r(json::Value::Type::OBJECT);
    r[Olt::BAL_STATE] = json::Value::Type::NIL;
    r[Olt::OLT_OPTR_STATE] = json::Value::Type::NIL;
    return r;
}
} // namespace

endpoint::Olt::Olt(const std::string &path) : EndpointBase(path) {}

endpoint::Olt::~Olt() {}

void endpoint::Olt::get(const server::Request &request, server::Response &response)
{
    auto r = make_prototype();
#ifdef BAL34
    auto &OLT = Olt_Device::Olt_Device::get_instance();
    OLT.get_board_basic_info();
    r[constants::Olt::BAL_STATE] = OLT.get_bal_oper_state();
    r[constants::Olt::OLT_OPTR_STATE] = OLT.get_olt_status();

    UNUSED(request);
#else
    UNUSED(request);
    set_response(response, r);
#endif
}

void endpoint::Olt::del(const server::Request &request, server::Response &response)
{
    UNUSED(request);
    response.set_status(server::status_2XX::OK);
}

void endpoint::Olt::patch(const server::Request &request, server::Response &response)
{
    using namespace psme::rest::error;
#ifdef BAL34
    try
    {
        const auto json = JsonValidator::validate_request_body<schema::OltPatchSchema>(request);
        if (json.is_member(constants::Olt::OLT_OPTR_STATE))
        {
            const auto &olt_optr_state = json[constants::Olt::OLT_OPTR_STATE];
            if (olt_optr_state == true)
            {
                auto &OLT = Olt_Device::Olt_Device::get_instance();
                OLT.connect_bal(false);
            }
        }
    }
    catch (const agent_framework::exceptions::NotFound &ex)
    {
        return;
    }

    server::Request get_request{request};
    get(get_request, response);
#else
    UNUSED(response);
    UNUSED(request);
#endif
}