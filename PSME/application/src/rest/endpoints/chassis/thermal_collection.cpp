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
#include "agent-framework/module/chassis_components.hpp" 
#include "psme/rest/endpoints/chassis/thermal_collection.hpp"
#include "psme/rest/constants/constants.hpp"
#include "psme/rest/endpoints/utils.hpp"
#include "psme/rest/endpoints/chassis/thermal_zone.hpp"
#include "psme/rest/constants/constants.hpp"

#ifdef ONLP
#include "acc_onlp_helper/acc_onlp_helper.hpp"
using namespace acc_onlp_helper;
#endif

using namespace psme::rest;
using namespace psme::rest::endpoint;
using namespace psme::rest::constants;
using agent_framework::module::ChassisComponents; 


#define PSU_NUM 2
unsigned int  UPPER_SYS_THRESHOLD_NON_CRITICAL=0;
unsigned int  UPPER_SYS_THRESHOLD_CRITICAL=0;
unsigned int  UPPER_SYS_THRESHOLD_FATAL=0;

unsigned int  UPPER_CPU_THRESHOLD_NON_CRITICAL=0;
unsigned int  UPPER_CPU_THRESHOLD_CRITICAL=0;
unsigned int  UPPER_CPU_THRESHOLD_FATAL=0;



#define ZERO 0

namespace {
json::Value make_prototype() {
    json::Value r(json::Value::Type::OBJECT);

    r[Common::ODATA_CONTEXT] = "/redfish/v1/$metadata#Thermal.Thermal";
    r[Common::ODATA_ID] = json::Value::Type::NIL;
    r[Common::ODATA_TYPE] = "#Thermal.v1_1_0.Thermal";
    r[Common::NAME] = "Thermal";	
    r[Common::ID] = "Thermal";
    r[Common::DESCRIPTION] = "Collection of Thermal sensors";
    r[constants::Switch::REDUNDANCY] = json::Value::Type::ARRAY;
    return r;
}
}

ThermalCollection::ThermalCollection(const std::string& path) : EndpointBase(path) {}

ThermalCollection::~ThermalCollection() {}

void ThermalCollection::get(const server::Request& req, server::Response& res) {
    auto json = ::make_prototype();

    json[Common::ODATA_ID] = PathBuilder(req).build();

#ifdef ONLP
// Use ONLP library to to get ONLP info.
    auto& tz_manager = agent_framework::module::ChassisComponents::get_instance()->get_thermal_zone_manager();
    auto tz_uuids = tz_manager.get_keys();
    auto chassis = psme::rest::model::Find<agent_framework::model::Chassis>(req.params[PathParam::CHASSIS_ID]).get();

    //Set thermal related info. //
    for (const auto& tz_uuid : tz_uuids) 
    {
        auto tz_ = tz_manager.get_entry(tz_uuid);  //Get TZ object by fan_uuid//
        int thermal_id=tz_.get_tz_id();		
        
        json::Value jsontmp; 
/*        
        jsontmp[Common::ODATA_ID] =  endpoint::PathBuilder(PathParam::BASE_URL).
        append(Common::CHASSIS).
        append(chassis.get_id()).
        append("Thermal#").
        append(constants::Root::TEMPS).				
        append(thermal_id).build();		
*/
        
        jsontmp[Common::ODATA_ID] =  endpoint::PathBuilder(PathParam::BASE_URL).
        append(Common::CHASSIS).
        append(chassis.get_id()).
        append("Thermal").build();		
        

        jsontmp[Common::MEMBER_ID] = std::to_string(thermal_id);

        int  thermal_type = tz_.get_thermal_type();	
		
        if(thermal_type ==  acc_onlp_helper::Thermal_Info::PSU_Sensor) 		
        {
              jsontmp[Common::NAME] = "PSU Thermal Sensor Temperature";
            jsontmp["PhysicalContext"] ="PowerSupply";	  
        }
        else if(thermal_type ==  acc_onlp_helper::Thermal_Info::CPU_Sensor)
        {
            jsontmp[Common::NAME] = "System CPU Thermal Sensor Temperature";
            jsontmp["PhysicalContext"] ="CPU";	  			  
        }
        else if(thermal_type == acc_onlp_helper::Thermal_Info::SYSTEM_Sensor)
        {
            jsontmp[Common::NAME] = "Chassis Thermal Sensor Temperature";			
            jsontmp["PhysicalContext"] ="SystemBoard";	  			  
        }
        else
        {
            jsontmp[Common::NAME] = "unknow";			
            jsontmp["PhysicalContext"] ="unknow";	  			  
        }
        
        jsontmp["SensorNumber"] =  thermal_id;		
        jsontmp["Status"]["HealthRollup"]= json::Value::Type::NIL;
        jsontmp["ReadingCelsius"] = (tz_.get_temperature() * 0.001); 
        
        jsontmp["UpperThresholdNonCritical"]  = (tz_.get_warning_temp()  * 0.001);
        jsontmp["UpperThresholdCritical"]       = (tz_.get_error_temp()  * 0.001);
        jsontmp["UpperThresholdFatal"]          = (tz_.get_shutdown_temp()  * 0.001);

        jsontmp["Status"]["Health"] = tz_.get_status_health();
        jsontmp["Status"]["State"] =  tz_.get_status_state();
        
        jsontmp["LowerThresholdNonCritical"] =json::Value::Type::NIL;
        jsontmp["LowerThresholdCritical"] =json::Value::Type::NIL;
        jsontmp["LowerThresholdFatal"] =json::Value::Type::NIL;
		
        json::Value lrejsontmp;
        lrejsontmp[Common::ODATA_ID] = endpoint::PathBuilder(PathParam::BASE_URL).append(Common::CHASSIS).append(chassis.get_id()).build();	
        jsontmp["RelatedItem"].push_back(std::move(lrejsontmp));
        json[constants::Root::TEMPS].push_back(std::move(jsontmp));

    }


    //Set FAN related info. //
    auto& fan_manager = agent_framework::module::ChassisComponents::get_instance()->get_fan_manager();
    auto fan_uuids = fan_manager.get_keys();


    for (const auto& fan_uuid : fan_uuids) 
    {
        auto fan_ = fan_manager.get_entry(fan_uuid);  //Get Fan object by fan_uuid//
        int  fan_id= (int)fan_.get_id();
        signed int RPM = fan_.get_current_speed();	
        json::Value jsontmp; 

/*        
        jsontmp[Common::ODATA_ID] =  endpoint::PathBuilder(PathParam::BASE_URL).
        		append(Common::CHASSIS).
        		append(chassis.get_id()).
        		append("Thermal#").
        		append(constants::Root::FANS).				
        		append(fan_id).build();
*/    
        jsontmp[Common::ODATA_ID] =  endpoint::PathBuilder(PathParam::BASE_URL).
        		append(Common::CHASSIS).
        		append(chassis.get_id()).
        		append("Thermal").build();

        jsontmp[Common::MEMBER_ID] = std::to_string(fan_id);

        int  fan_type = fan_.get_fan_type();	
		
        if(fan_type ==  acc_onlp_helper::Fan_Info::PSU_Fan) 		
        {
                  jsontmp[Common::NAME] =  "PSU Fan";
        	    jsontmp["PhysicalContext"] ="PowerSupply";				  
        }
        else  if(fan_type ==  acc_onlp_helper::Fan_Info::SYSTEM_Fan) 		
        {
                  jsontmp[Common::NAME] = "System Fan";
        	    jsontmp["PhysicalContext"] ="Back";	 			
        }
        else
        {
                  jsontmp[Common::NAME] = "unknow";
        	    jsontmp["PhysicalContext"] ="unknow";	 			
        }

        if(RPM >= 0)
                  jsontmp["Reading"] = RPM;
        else
                  jsontmp["Reading"] = json::Value::Type::NIL;
        	
        jsontmp["Status"]["HealthRollup"]= json::Value::Type::NIL;

        std::string str_Health = fan_.get_status_health();
        std::string str_State   = fan_.get_status_state();

		
        jsontmp["Status"]["Health"] = str_Health.c_str();
        jsontmp["Status"]["State"]   = str_State.c_str();  	  

        
        jsontmp["ReadingUnits"] = "RPM";	
        jsontmp["UpperThresholdNonCritical"] =json::Value::Type::NIL;	
        jsontmp["UpperThresholdCritical"] =json::Value::Type::NIL;	
        jsontmp["UpperThresholdFatal"] =json::Value::Type::NIL;
        jsontmp["LowerThresholdNonCritical"] =json::Value::Type::NIL;
        jsontmp["LowerThresholdCritical"] =json::Value::Type::NIL;
        jsontmp["LowerThresholdFatal"] =json::Value::Type::NIL;
        
        json::Value lrejsontmp;
        lrejsontmp[Common::ODATA_ID] = endpoint::PathBuilder(PathParam::BASE_URL).append(Common::CHASSIS).append(chassis.get_id()).build();	
        jsontmp["RelatedItem"].push_back(std::move(lrejsontmp));
        json[constants::Root::FANS].push_back(std::move(jsontmp));
    }

#else
//Todo , move all platform using ONLP library
    char command[256] = {0};
    char resultA[256] = {0};	
    int PRESENT= 0;
    int PSU_PRESENT= 0;	
    int MAX_FAN = 0;
    int MAX_THERMAL = 0;



    if(UPPER_SYS_THRESHOLD_NON_CRITICAL == 0)
    {
         sprintf(command, "psme.sh get upper_sys_th_thermal_temp");
         memset(resultA,0x0, sizeof(resultA));
         exec_shell(command, resultA);
         
         if(strlen(resultA) != 0)
         {  
             int tmp1[3] = {0};
             sscanf(resultA, "%d %d %d" ,  &tmp1[0] , &tmp1[1], &tmp1[2]);         
             UPPER_SYS_THRESHOLD_NON_CRITICAL = (tmp1[0]);
             UPPER_SYS_THRESHOLD_CRITICAL = (tmp1[1]);
             UPPER_SYS_THRESHOLD_FATAL = (tmp1[2]);;
         }
    }

    if(UPPER_CPU_THRESHOLD_NON_CRITICAL == 0)
    {
         sprintf(command, "psme.sh get upper_cpu_th_thermal_temp");
         memset(resultA,0x0, sizeof(resultA));
         exec_shell(command, resultA);
         
         if(strlen(resultA) != 0)
         {  
             int tmp1[3] = {0};
             sscanf(resultA, "%d %d %d" ,  &tmp1[0] , &tmp1[1], &tmp1[2]);         
             UPPER_CPU_THRESHOLD_NON_CRITICAL = (tmp1[0]);
             UPPER_CPU_THRESHOLD_CRITICAL = (tmp1[1]);
             UPPER_CPU_THRESHOLD_FATAL = (tmp1[2]);;
         }
    }
	
    sprintf(command, "psme.sh get max_fan_num");
    memset(resultA,0x0, sizeof(resultA));
    exec_shell(command, resultA);
    
    if(strlen(resultA) != 0)
    {  
        MAX_FAN=atoi(resultA);
    }

    sprintf(command, "psme.sh get thermal_sensor_num");
    memset(resultA,0x0, sizeof(resultA));
    exec_shell(command, resultA);
    
    if(strlen(resultA) != 0)
    {  
        MAX_THERMAL=atoi(resultA);
    }

    sprintf(command, "psme.sh get psu_presence");
    memset(resultA,0x0, sizeof(resultA));
    exec_shell(command, resultA);
    
    if(strlen(resultA) != 0)
    {  
        PSU_PRESENT=atoi(resultA);
    }

    PRESENT = (( PSU_PRESENT <<  (MAX_THERMAL-PSU_NUM)) );

    //auto chassis_ids = ChassisComponents::get_instance()->get_thermal_zone_manager().get_ids();	
    auto chassis = psme::rest::model::Find<agent_framework::model::Chassis>(req.params[PathParam::CHASSIS_ID]).get();

    auto& tz_manager = agent_framework::module::ChassisComponents::get_instance()->get_thermal_zone_manager();
    auto tz_uuids = tz_manager.get_keys();
		
    for (const auto& tz_uuid : tz_uuids) {
	       auto tz_ = tz_manager.get_entry(tz_uuid);  //Get TZ object by fan_uuid//
		unsigned long thermal_id=tz_.get_tz_id();		
	       
              json::Value jsontmp; 
			  
		jsontmp[Common::ODATA_ID] =  endpoint::PathBuilder(PathParam::BASE_URL).
				append(Common::CHASSIS).
				append(chassis.get_id()).
				append("Thermal#").
				append(constants::Root::TEMPS).				
				append(thermal_id).build();
	  

		jsontmp[Common::MEMBER_ID] = std::to_string(thermal_id);
		
		if(thermal_id >(unsigned long) (MAX_THERMAL-PSU_NUM)) 		
		{
                  jsontmp[Common::NAME] = "PSU Thermal Sensor Temperature";
 		    jsontmp["PhysicalContext"] ="PowerSupply";	  
		}
		else if( thermal_id == 1)
		{
                  jsontmp[Common::NAME] = "System CPU Thermal Sensor Temperature";
 		    jsontmp["PhysicalContext"] ="CPU";	  			  
		}
		else 
		{
                  jsontmp[Common::NAME] = "Chassis Thermal Sensor Temperature";			
 		    jsontmp["PhysicalContext"] ="SystemBoard";	  			  
		}

              jsontmp["SensorNumber"] =  thermal_id;		
		jsontmp["Status"]["HealthRollup"]= json::Value::Type::NIL;

		jsontmp["ReadingCelsius"] = (tz_.get_temperature() * 0.001); 	

		if(thermal_id == 1) //CPU sensor//
              {
                  jsontmp["UpperThresholdNonCritical"]  = UPPER_CPU_THRESHOLD_NON_CRITICAL/1000;
                  jsontmp["UpperThresholdCritical"]       = UPPER_CPU_THRESHOLD_CRITICAL/1000;
                  jsontmp["UpperThresholdFatal"]          = UPPER_CPU_THRESHOLD_FATAL/1000;
              }
		else
              {
                  jsontmp["UpperThresholdNonCritical"]  = UPPER_SYS_THRESHOLD_NON_CRITICAL/1000;
                  jsontmp["UpperThresholdCritical"]       = UPPER_SYS_THRESHOLD_CRITICAL/1000;
                  jsontmp["UpperThresholdFatal"]          = UPPER_SYS_THRESHOLD_FATAL/1000;
              }
		
              int bpresent=(1 << (thermal_id-1));

		if(thermal_id >(unsigned long) (MAX_THERMAL-PSU_NUM) && (PRESENT & bpresent) && (tz_.get_temperature() == ZERO))
		{
			jsontmp["Status"]["Health"]                 = json::Value::Type::NIL;
			jsontmp["Status"]["State"]                  = "UnavailableOffline";	
			jsontmp["ReadingCelsius"]                  = json::Value::Type::NIL;
                     jsontmp["UpperThresholdNonCritical"]  = json::Value::Type::NIL;
                     jsontmp["UpperThresholdCritical"]        = json::Value::Type::NIL;
                     jsontmp["UpperThresholdFatal"]           = json::Value::Type::NIL;
		}
		else if(thermal_id >(unsigned long) (MAX_THERMAL-PSU_NUM) && !(PRESENT & bpresent))
		{
			jsontmp["Status"]["Health"] = json::Value::Type::NIL;
			jsontmp["Status"]["State"] = "Absent";	
			jsontmp["ReadingCelsius"]     = json::Value::Type::NIL;
                     jsontmp["UpperThresholdNonCritical"]  = json::Value::Type::NIL;
                     jsontmp["UpperThresholdCritical"]        = json::Value::Type::NIL;
                     jsontmp["UpperThresholdFatal"]           = json::Value::Type::NIL;			
		}
		else
		{
                     
                     if(thermal_id == 1)
                     {
                         if((tz_.get_temperature() < ZERO) || ((tz_.get_temperature() < UPPER_CPU_THRESHOLD_CRITICAL) && (tz_.get_temperature() >= UPPER_CPU_THRESHOLD_NON_CRITICAL)))
                         {
                         	jsontmp["Status"]["Health"] = "Warning";
                         	jsontmp["Status"]["State"] = "Enabled";					
                         }
                         else if((tz_.get_temperature() < UPPER_CPU_THRESHOLD_FATAL) && ((tz_.get_temperature() >= UPPER_CPU_THRESHOLD_CRITICAL)))
                         {
                         	jsontmp["Status"]["Health"] = "Warning";
                         	jsontmp["Status"]["State"] = "Enabled";					
                         }
                         else if((tz_.get_temperature() >= UPPER_CPU_THRESHOLD_FATAL))
                         {
                         	jsontmp["Status"]["Health"] = "Critical";
                         	jsontmp["Status"]["State"] = "Enabled";					
                         }
                         else
                         {
                         	jsontmp["Status"]["Health"] = "OK";					
                         	jsontmp["Status"]["State"] = "Enabled";
                         }                     
                     }
                     else
                     {
                         if((tz_.get_temperature() < ZERO) || ((tz_.get_temperature() < UPPER_SYS_THRESHOLD_CRITICAL) && (tz_.get_temperature() >= UPPER_SYS_THRESHOLD_NON_CRITICAL)))
                         {
                         	jsontmp["Status"]["Health"] = "Warning";
                         	jsontmp["Status"]["State"] = "Enabled";					
                         }
                         else if((tz_.get_temperature() < UPPER_SYS_THRESHOLD_FATAL) && ((tz_.get_temperature() >= UPPER_SYS_THRESHOLD_CRITICAL)))
                         {
                         	jsontmp["Status"]["Health"] = "Warning";
                         	jsontmp["Status"]["State"] = "Enabled";					
                         }
                         else if((tz_.get_temperature() >= UPPER_SYS_THRESHOLD_FATAL))
                         {
                         	jsontmp["Status"]["Health"] = "Critical";
                         	jsontmp["Status"]["State"] = "Enabled";					
                         }
                         else
                         {
                         	jsontmp["Status"]["Health"] = "OK";					
                         	jsontmp["Status"]["State"] = "Enabled";
                         }                         
                     }
		}

		jsontmp["LowerThresholdNonCritical"] =json::Value::Type::NIL;
		jsontmp["LowerThresholdCritical"] =json::Value::Type::NIL;
		jsontmp["LowerThresholdFatal"] =json::Value::Type::NIL;
		json::Value lrejsontmp;
		lrejsontmp[Common::ODATA_ID] = endpoint::PathBuilder(PathParam::BASE_URL).append(Common::CHASSIS).append(chassis.get_id()).build();	
              jsontmp["RelatedItem"].push_back(std::move(lrejsontmp));
              json[constants::Root::TEMPS].push_back(std::move(jsontmp));

    }

    //For Fans
    auto& fan_manager = agent_framework::module::ChassisComponents::get_instance()->get_fan_manager();
    auto fan_uuids = fan_manager.get_keys();

	
    sprintf(command, "psme.sh get fan_presence");
    memset(resultA,0x0, sizeof(resultA));
    exec_shell(command, resultA);
    
    if(strlen(resultA) != 0)
    {  
        PRESENT=atoi(resultA);
    }

    PRESENT = (( PSU_PRESENT <<  (MAX_FAN-PSU_NUM)) |  PRESENT);

    for (const auto& fan_uuid : fan_uuids) {

		auto fan_ = fan_manager.get_entry(fan_uuid);  //Get Fan object by fan_uuid//
		unsigned long fan_id= fan_.get_id();
              signed int RPM = fan_.get_current_speed();	
		json::Value jsontmp; 
		
		jsontmp[Common::ODATA_ID] =  endpoint::PathBuilder(PathParam::BASE_URL).
				append(Common::CHASSIS).
				append(chassis.get_id()).
				append("Thermal#").
				append(constants::Root::FANS).				
				append(fan_id).build();

		jsontmp[Common::MEMBER_ID] = std::to_string(fan_id);

		if(fan_id >(unsigned long) (MAX_FAN-PSU_NUM)) 
		{
                  jsontmp[Common::NAME] =  "PSU Fan";
 		    jsontmp["PhysicalContext"] ="PowerSupply";				  
		}
		else
		{
		jsontmp[Common::NAME] = "System Fan";
 		    jsontmp["PhysicalContext"] ="Back";	 			
		}	

		if(RPM >= 0)
                  jsontmp["Reading"] = RPM;
		else
                  jsontmp["Reading"] = json::Value::Type::NIL;
			
		jsontmp["Status"]["HealthRollup"]= json::Value::Type::NIL;
		
                int bpresent=(1 << (fan_id-1));

		if((fan_id >(unsigned long) (MAX_FAN-PSU_NUM)) && ( PRESENT & bpresent) && (RPM == -1))
		{     // PSU plugin without power cord  //
			jsontmp["Status"]["State"] = "UnavailableOffline";  
			jsontmp["Status"] ["Health"] = json::Value::Type::NIL;
			jsontmp["Reading"] = json::Value::Type::NIL;
		}		
		else if(!(PRESENT & bpresent))
		{     // Chassis FAN module do not plugin //
			jsontmp["Status"]["State"] = "Absent";  
			jsontmp["Status"] ["Health"] = json::Value::Type::NIL;
			jsontmp["Reading"] = json::Value::Type::NIL;
		}
		else if((PRESENT & bpresent) && RPM == 0)
		{     // FAN module plugin but RPM ==  0 //
			jsontmp["Status"]["State"] = "Enabled";
			jsontmp["Status"]["Health"] ="Warning";	  	  
		}
		else if((PRESENT & bpresent) && RPM != 0)
		{     // FAN module plugin but RPM !=  0 //
			jsontmp["Status"]["State"] = "Enabled";
			jsontmp["Status"]["Health"] ="OK";	  	  
		}
		else
		{     
			jsontmp["Status"]["State"]  = json::Value::Type::NIL;
			jsontmp["Status"]["Health"] = json::Value::Type::NIL;
		}
		
		jsontmp["ReadingUnits"] = "RPM";	
		jsontmp["UpperThresholdNonCritical"] =json::Value::Type::NIL;	
		jsontmp["UpperThresholdCritical"] =json::Value::Type::NIL;	
		jsontmp["UpperThresholdFatal"] =json::Value::Type::NIL;
		jsontmp["LowerThresholdNonCritical"] =json::Value::Type::NIL;
		jsontmp["LowerThresholdCritical"] =json::Value::Type::NIL;
		jsontmp["LowerThresholdFatal"] =json::Value::Type::NIL;
		
		json::Value lrejsontmp;
		lrejsontmp[Common::ODATA_ID] = endpoint::PathBuilder(PathParam::BASE_URL).append(Common::CHASSIS).append(chassis.get_id()).build();	
              jsontmp["RelatedItem"].push_back(std::move(lrejsontmp));
              json[constants::Root::FANS].push_back(std::move(jsontmp));
	
    }


#endif


    set_response(res, json);
}

