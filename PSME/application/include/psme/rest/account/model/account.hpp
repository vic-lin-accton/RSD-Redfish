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

#pragma once
#include "agent-framework/module/enum/enum_builder.hpp"
#include <string>
#include <vector>
#include <chrono> 
#include <iostream>


#include "psme/rest/account/model/accountservice.hpp"


namespace json {
    class Value;
}



namespace psme {
namespace rest {
namespace account {
namespace model {

//using model::Accountservice;



/*!
 * @brief Subscription representation
 */
class Account final {
public:
    /*!
     * @brief Set account id
     *
     * @param id account id
     */
    void set_id(uint64_t id) {
        m_id = id;
    }

    /*!
     * @brief Get account id
     *
     * @return account id
     */
    uint64_t get_id() const {
        return m_id;
    }

    /*!
     * @brief Set account name
     *
     * @param name account name
     */
    void set_name(const std::string& name) {
        m_name = name;
    }

    /*!
     * @brief Get account name
     *
     * @return account name
     */
    const std::string& get_name() const {
        return m_name;
    }

    /*!
     * @brief Set account password
     *
     * @param  account password
     */
    void set_password(const std::string& password) {
        m_password = password;
    }
    
  int checkpw(const std::string& password) 
  {
    	if (m_password == password)
    	{
    	    m_loginf=0;
    	    
        }
        else
        {
           m_loginf++;
           if ( m_loginf > Accountservice::get_instance()->get_alt() )
           {
               m_locked = true;
            }   
            m_loginftime=std::chrono::steady_clock::now();  //no matter locked or not  record login fail time to wait reset loginf counter
        }
        
        return m_loginf;
    }
    
    void timekick(const std::chrono::steady_clock::time_point& now)
    {
    
#if 0   
        std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(now - m_lastkick);	
        std::cout << "In Account.hpp got kick in " << m_username << " " << time_span.count() << "seconds " << std::endl;       
        std::chrono::duration<double> loginf_time_span = std::chrono::duration_cast<std::chrono::duration<double>>(now- m_loginftime);
        std::cout << "In Account.hpp login fail time span " << m_username << " " << loginf_time_span.count() << "seconds " << std::endl;  
        std::cout << "In Account.hpp Account Lock duration is " << Accountservice::get_instance()->get_ald() << " seconds " <<std::endl;
        std::cout << "In Account.hpp Account Lock counter reset is " << Accountservice::get_instance()->get_alcra() << " seconds " <<std::endl;
       
        m_lastkick = now;
#endif        
      
    	if (m_loginf)
    	{
    	   auto elapse = std::chrono::duration_cast<std::chrono::seconds> (now - m_loginftime).count();
    	   
    	   if (m_locked)  // account is locked
    	   {
    	   	if ( elapse > Accountservice::get_instance()->get_ald())  //check AccountLockoutDuration
    	   	{
    	   	   m_loginf=0;
    	   	   m_locked=false;
    	   	}
    	   	    
    	   }
    	   else  // account is not locked but ever login fail 
    	   {
    	        if ( elapse > Accountservice::get_instance()->get_alcra()) // check AccountLockoutCounterResetAfter
    	           m_loginf=0;
    	   }
    	}
    	   
    }

    /*!
     * @brief Get account password
     *
     * @return account password
     */
    const std::string& get_password() const {
        return m_password;
    }

    /*!
     * @brief Set account username
     *
     * @param account username
     */
    void set_username(const std::string& username) {
        m_username = username;
    }

    /*!
     * @brief Get account username
     *
     * @return account username
     */
    const std::string& get_username() const {
        return m_username;
    }

    /*!
     * @brief Set account enabled
     *
     * @param account enabled
     */
    void set_enabled(const bool& enabled) {
        m_enabled= enabled;
    }

    /*!
     * @brief Get account enabled
     *
     * @return account enabled
     */
    const bool& get_enabled() const {
        return m_enabled;
    }

    /*!
     * @brief Set account locked
     *
     * @param account locked
     */
    void set_locked(const bool& locked) {
        m_locked= locked;
    }

    /*!
     * @brief Get account locked
     *
     * @return account locked
     */
    const bool& get_locked() const {
        return m_locked;
    }

    /*!
     * @brief Set account roleid
     *
     * @param account roleid
     */
    void set_roleid(const std::string& roleid) {
        m_roleid = roleid;
    }

    /*!
     * @brief Get account roleid
     *
     * @return account roleid
     */
    const std::string& get_roleid() const {
        return m_roleid;
    }

    /*!
     * @brief Creates json representation of account
     *
     * @return JSON representation of subscription
     */
    json::Value to_json() const;

    /*!
     * @brief Creates model representation from account JSON
     *
     * @param json JSON representation of account
     * @return Model representation of account
     */
    static Account from_json(const json::Value& json);

private:
    uint64_t m_id{};
    std::string m_name{};
    std::string m_username{};
    std::string m_roleid{};
    std::string m_password{};
    bool m_enabled{true};
    bool m_locked{false};
    int m_loginf{0};
    std::chrono::steady_clock::time_point m_loginftime{};
    std::chrono::steady_clock::time_point m_lastkick{};

};


}
}
}
}
