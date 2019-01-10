/*!
 * @section LICENSE
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
 * @section Declaration of SubscriptionConfig class
 * @file role_config.hpp
 * */

#pragma once
#include "agent-framework/generic/singleton.hpp"
#include "psme/rest/account/model/role.hpp"
#include "psme/rest/account/manager/account_manager.hpp"
#include <mutex>
#include <map>

namespace psme {
namespace rest {
namespace account {
namespace config {

using namespace psme::rest::account;
using namespace psme::rest::account::manager;
using namespace psme::rest::account::model;
/*!
 * RoleConfig implementation
 */
class RoleConfig : public agent_framework::generic::Singleton<RoleConfig> {
public:
    /*!
     * @brief Default roles file path
     */
    static constexpr const char DEFAULT_ROLE_FILE_PATH[] = "/etc/psme/roles";

    /*!
     * @brief Set roles configuration file
     * @param config_file_path roles configuration file path
     */
    void set_config_file(const std::string& config_file_path);

    /*!
     * @brief Save roles to file
     */
    void saveRoles();

    /*!
     * @brief Load roles from file
     */
    void loadRoles();

    /*!
     * @brief Load predefined roles from file/hard coding ?
     */
    void loadPreRoles();


    /*!
     * @brief Destructor
     */
    virtual ~RoleConfig();
private:
    json::Value get_roles_json(const RoleMap& roles) const;
    json::Value get_roles_json(const std::string& json_content) const;
    std::mutex m_mutex{};
    std::string m_config_file_path{DEFAULT_ROLE_FILE_PATH};
};


}
}
}
}

