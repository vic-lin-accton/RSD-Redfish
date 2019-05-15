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

#include "psme/rest/endpoints/session_collection.hpp"
#include "psme/rest/utils/status_helpers.hpp"

#include "agent-framework/module/managers/utils/manager_utils.hpp"

#include "psme/rest/validators/json_validator.hpp"
#include "psme/rest/validators/schemas/sessions.hpp"
#include "psme/rest/session/manager/session_manager.hpp"
#include "psme/rest/validators/schemas/session_collection.hpp"
#include "psme/rest/account/manager/account_manager.hpp"
#include "psme/rest/account/model/accountservice.hpp" 

#include <iostream>
#include <string>
#include <stdlib.h>

using namespace psme::rest::validators;

using namespace psme::rest;
using namespace psme::rest::session::manager;
using namespace psme::rest::constants;
using namespace agent_framework::module;
using namespace agent_framework::model::enums;
using namespace psme::rest::account::manager;

namespace {

json::Value make_session_prototype() {
    json::Value r(json::Value::Type::OBJECT);

    r[Common::ODATA_CONTEXT] = "/redfish/v1/$metadata#SessionCollection.SessionCollection";
    r[Common::ODATA_ID] = json::Value::Type::NIL;
    r[Common::ODATA_TYPE] = "#SessionCollection.SessionCollection";
    r[Common::NAME] = "Session Collection";
    r[Common::DESCRIPTION] = "Session Collection";
    r[Collection::ODATA_COUNT] = json::Value::Type::NIL;
    r[Collection::MEMBERS] = json::Value::Type::ARRAY;

    return r;
}

}

namespace psme {
namespace rest {
namespace endpoint {


SessionCollection::SessionCollection(const std::string& path) : EndpointBase(path) { }

SessionCollection::~SessionCollection() { }

void SessionCollection::get(const server::Request& req, server::Response& res) {
    auto json = ::make_session_prototype();
    json[Common::ODATA_ID] = PathBuilder(req).build();
    json[Collection::ODATA_COUNT] = SessionManager::get_instance()->Session_size();

    for (const auto& item : SessionManager::get_instance()->getSession()) {
        json::Value link_elem(json::Value::Type::OBJECT);
        const auto& session = item.second;
        link_elem[Common::ODATA_ID] = PathBuilder(req).append(session.get_id()).build();
        json[Collection::MEMBERS].push_back(std::move(link_elem));
    }
    set_response(res, json);
}


Session to_model(const json::Value& json);
Session to_model(const json::Value& json) {
    Session s;    
    const auto& username = json[SessionService::USERNAME].as_string();
    const auto& password = json[SessionService::PASSWORD].as_string();
               
    s.set_username(username);
    s.set_password(password);
    
    return s;
}


json::Value make_session_post_prototype();
json::Value make_session_post_prototype() {
    json::Value r(json::Value::Type::OBJECT);

    r[constants::Common::ODATA_CONTEXT] = "/redfish/v1/$metadata#Session.Session";
    r[constants::Common::ODATA_ID] = json::Value::Type::NIL;
    r[constants::Common::ODATA_TYPE] = "#Session.v1_0_0.Session";
    r[constants::Common::NAME] = "User Session";
    r[constants::Common::DESCRIPTION] = "User Session";
    r[constants::Common::USER_NAME] =  json::Value::Type::NIL;
    return r;
}

void endpoint::SessionCollection::del(const server::Request& request, server::Response& response) {
	std::string id =request.params[constants::PathParam::SESSION_ID];
       response.set_status(server::status_2XX::NO_CONTENT);    
	std::cerr << "Session::del" << std::endl;
	return;
}


void endpoint::SessionCollection::post(const server::Request& request, server::Response& response) {

    bool Enabled = SessionManager::get_instance()->GetSessionConfigEnable();
    if(Enabled == false)
    {
        response.set_status(server::status_5XX::SERVICE_UNAVAILABLE); 	
        return;
    }

    uint64_t Session_size = SessionManager::get_instance()->Session_size();

    uint64_t MaxSessions = SessionManager::get_instance()->GetSessionConfigMaxSessions();
	
    if(Session_size >= MaxSessions)
    {
        response.set_status(server::status_5XX::SERVICE_UNAVAILABLE); 	
        return;
    }
	
	
    json::Value r = make_session_post_prototype();
	
    std::string username{};
    std::string password{};
/*
    json::Value schema({
        JsonValidator::mandatory(constants::Common::USER_NAME, JsonValidator::has_type(JsonValidator::STRING_TYPE)),
        JsonValidator::mandatory(constants::Common::PASSWORD, JsonValidator::has_type(JsonValidator::STRING_TYPE))
    });
    const auto json = JsonValidator::validate_request_body(request, schema);
*/
    const auto& json = JsonValidator::validate_request_body<schema::SessionCollectionPostSchema>(request);
    Session session= to_model(json);

    if (json.is_member(constants::Common::USER_NAME) || json.is_member(constants::Common::PASSWORD))
    {
        //Setup new session //
        username = json[constants::Common::USER_NAME].as_string();
        password  = json[constants::Common::PASSWORD].as_string();

        /*Gen Token*/
        srand((unsigned int)time(0L));
        string str = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
        string AuthenToken;
        unsigned long int pos;
		
        while(AuthenToken.size() != 32) 
	{  //Token len 32 bytes //
            pos = ((rand() % (str.size() - 1)));
            AuthenToken += str.substr(pos,1);
        }	

	if(((username.length() <= 256) && (username.length() > 0)) && ((password.length() <= 256) && (password.length() > 0)))
	{
           /*Check if this username locked*/
           const auto  & account  =AccountManager::get_instance()->getAccount(username);	
	    if(account.get_locked() == true)
	    {
               response.set_status(server::status_5XX::SERVICE_UNAVAILABLE); 
               log_error(GET_LOGGER("rest"), "Lock on user [" << username << "]");
               return;
           }
		   
           /*Check if correct username and password*/ 
	    int res  = AccountManager::get_instance()->login(username,password) ;
           if(res == 0)
           {
               /**/
               const auto  & account_enable_chk  =AccountManager::get_instance()->getAccount(username);
               if(account_enable_chk.get_enabled() != true)
               {
                   response.set_status(server::status_4XX::UNAUTHORIZED);    	
                   return;
               }
			   
               const auto& role = AccountManager::get_instance()->getRole(account.get_roleid());  

               if(SessionManager::get_instance()->checkSession_by_name(username))
               {
                   printf("Found exist ID and del first to cerate new one session\r\n");
                   SessionManager::get_instance()->delSession( SessionManager::get_instance()->getSession(username).get_id());				   
               }
               else
                   printf("Not fund ID\r\n");
			   	   
               session.set_authen_token(AuthenToken);
               session.set_userrole(role.get_roleid());  
		   
               uint64_t id = SessionManager::get_instance()->addSession(session, false);
			   
		 r[constants::Common::USER_NAME] =  username;
		 r[constants::Common::ID] =  id;
		 //r[constants::SessionService::AUTHENTOKEN] =  AuthenToken;
		 
	        const std::string odata_path="/redfish/v1/SessionService/Sessions/" + id;	 
		 r[constants::Common::ODATA_ID] = odata_path;

               /*Set Authen Token in header */
               static const char * loginuri="/redfish/v1/SessionService/Sessions";
               #define XAUTH_NAME "X-Auth-Token"
               #define LOC_ID "Location"               
               char cxauthstr[256];	
               char cxlocation[256];	

               snprintf (cxauthstr, sizeof (cxauthstr), "%s", AuthenToken.c_str());
               const std::string Xauth=XAUTH_NAME;
               response.set_header(Xauth, cxauthstr);
               
               snprintf (cxlocation, sizeof (cxlocation), "%s/%d",loginuri, (int)id);
               const std::string XLOCID=LOC_ID;
               response.set_header(XLOCID, cxlocation);

               response.set_status(server::status_2XX::CREATED);    
			   
               set_response(response, r);	
           }
           else  
           {
               //Todo: log 
               if(res > Accountservice::get_instance()->get_aflt())
                   log_error(GET_LOGGER("rest"), "user [" << username << "] over AuthFailureLoggingThreshold login times !!");
			   	
               response.set_status(server::status_4XX::UNAUTHORIZED);    	
               return;
           }
	}
	else
	{
           response.set_status(server::status_4XX::UNAUTHORIZED); 
           return;		   
	}	
    }
    return;
}


}
}
}
