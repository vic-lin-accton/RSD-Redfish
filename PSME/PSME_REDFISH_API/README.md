This is an overview of major Restful API services and describe how to receive Redfish 
API responses directly from a PSME Redfish service.

HTTP Request Methods used in PSME service.
  
  - Read Requests (GET):  
  The GET method is used to request resource. The representation can be either a collection or a single resource. 
  
  - Update (PATCH):  
  The PATCH method is used to apply partial modifications to a resource.
  
  - Actions (POST):
  The POST method is used to create a new resource or take a action in some Action items.
    
  - Delete (DELETE):  
  The DELETE method is used to remove a resource.  
   

## Using RESTful APIs  

  You can access PSME Redfish service without authentication or enable Session Management .
  
  ## Authentication   
   
   Enable Session Management          
   Redfish Service uses session management to implement authentication. 
   Enable session management and change session timeout to 30 seconds(Default is 600 seconds). 
   
      - URL     :  /redfish/v1/SessionService/
      - Method  :  POST
      - Payload :   
     
      {
        "ServiceEnabled": true,
        "SessionTimeout": 30
      }
   
      - Response : 200 OK
     
   Create a new session (Login) by using default UserName "admin" and Password "redfish".
   You need enable session management first.
   
      - URL     :  /redfish/v1/SessionService/Sessions
      - Method  :  POST
      - Payload :  
     
	  {  
        "UserName": "admin",  
        "Password": "redfish"  
	  }          

      - Response : 201 CREATED
     
   X-Auth Token header displays Location and session ID 
   Ex: Location: /redfish/v1/SessionService/Sessions/1 
	     
   Delete a session (Logout) already created.  
   You have enable session management first and created a session as ID "1".
   
      - URL     :  /redfish/v1/SessionService/Sessions/1
      - Method  :  Delete
      - Payload :  
     
	    {}

      - Response : 204 NO_CONTENT
	
  ## Account Service 
  
   You can create another account to access PSME Redfish service while enable authentication service.
    
   Get default account
    
      - URL     :  /redfish/v1/AccountService/Accounts
      - Method  :  GET
      - Response Payload :
         
        {
        "@odata.context": "/redfish/v1/$metadata#AccountService.AccountService",
        "@odata.id": "/redfish/v1/AccountService/Accounts",
        "@odata.type": "#ManagerAccountCollection.ManagerAccountCollection",
        "Name": "Accounts Collection",
        "Members@odata.count": 1,
        "Members": [
        {
          "@odata.id": "/redfish/v1/AccountService/Accounts/admin"
        }
        ]
        }
      
      - Response : 200 OK
        
   Change password of admin

      - URL     :  /redfish/v1/AccountService/Accounts/admin
      - Method  :  PATCH
      - Payload :
      
      {
      "UserName": "admin",
      "Password": "redfish_new_password",
      "Locked": false,
      "Enabled": true,
      "RoleId": "Administrator"
      }
      
      - Response : 200 OK
	
   Create new account
      
      - URL     :  /redfish/v1/AccountService/Accounts
      - Method  :  POST
      - Payload :  
     
        { 
        "UserName":"User_Name_1", 
        "Password":"User_Password_1", 
        "RoleId":"Administrator",         // Administrator, Operator, ReadOnlyUser 
        "Enabled":true ,
        "Locked":false 
        }           

      - Response : 201 CREATED  
  
   Remove new account
      
      - URL     :  /redfish/v1/AccountService/Accounts/User_Name_1
      - Method  :  Delete
      - Payload :  
     
        {}           

      - Response : 200 OK


  ## Event Service   

   The event service is a mechanism for Redfish. Events will be sent out through https to the web 
   server that is subscribed to by the users.  
		
   First, user needs to add a subscription to inform Redfish who will receive this event.  
   After user adds subscriptions, following event will be send out.
		  
		+------------------------+-----------------+-------------------+--------------+
		|    Events Types        | Resource Added  | Resource Removed  |   Alert      |
		+-----------------------------------------------------------------------------+
		|   PSU Module Plug In   |        O        |                   |              |
		+-----------------------------------------------------------------------------+
		|   PSU Module Plug Out  |                 |        O          |              |
		+-----------------------------------------------------------------------------+
		|   FAN Module Plug In   |        O        |                   |              |
		+-----------------------------------------------------------------------------+
		|   FAN Module Plug Out  |                 |        O          |              |
		+-----------------------------------------------------------------------------+
		|   FAN Module No Spin   |                 |                   |      O       |
		+-----------------------------------------------------------------------------+
		| CPU/Main Board Thermal |                 |                   |              |
		| Sensor over critical   |                 |                   |      O       |
		| temperature            |                 |                   |              |
		+-----------------------------------------------------------------------------+
		| CPU/Main Board Thermal |                 |                   |              |
		| Sensor over fatal      |                 |                   |      O       |
		| temperature            |                 |                   |              |
		+-----------------------------------------------------------------------------+
		| QSFP or XFP Transceiver|                 |                   |              |
		| Plug-Out               |                 |        O          |              |
		+-----------------------------------------------------------------------------+
		| QSFP or XFP Transceiver|                 |                   |              |
		| Plug-In                |        O        |                   |              |
		+------------------------+-----------------+-------------------+--------------+
				  
   Add a subscription about ResourceAdded

      - URL     :  /redfish/v1/EventService/Subscriptions
      - Method  :  POST
      - Payload :  
     
        {
        "Name": "EventSubscription ResourceAdded TEST",
        "Destination": "https://172.17.10.60:8889",
        "EventTypes": [
        "ResourceAdded"
        ],
        "Context": "////////THIS IS A TEST EVENT SEND TO DEST ResourceAdded////////",
        "Protocol": "Redfish"
        }         

        - Response : 201 CREATED 	  
	   
   Add a subscription about ResourceRemoved

      - URL     :  /redfish/v1/EventService/Subscriptions
      - Method  :  POST
      - Payload :  
     
        {
        "Name": "EventSubscription ResourceRemoved TEST",
        "Destination": "https://172.17.10.60:8889",
        "EventTypes": [
        "ResourceRemoved"
        ],
        "Context": "////////THIS IS A TEST EVENT SEND TO DEST ResourceRemoved////////",
        "Protocol": "Redfish"
        }         

        - Response : 201 CREATED

   Add a subscription about Alert

      - URL     :  /redfish/v1/EventService/Subscriptions
      - Method  :  POST
      - Payload :  
     
        {
        "Name": "EventSubscription Alert TEST",
        "Destination": "https://172.17.10.60:8889",
        "EventTypes": [
        "Alert"
        ],
        "Context": "////////THIS IS A TEST EVENT SEND TO DEST Alert////////",
        "Protocol": "Redfish"
        }         

      - Response : 201 CREATED
	       	   
   Get subscription events
	  
      - URL     :  /redfish/v1/EventService/Subscriptions
      - Method  :  POST
      - Payload :	   	  

        {
        "@odata.context": "/redfish/v1/$metadata#EventService.EventService",
        "@odata.id": "/redfish/v1/EventService/Subscriptions",
        "@odata.type": "#EventDestinationCollection.EventDestinationCollection",
        "Name": "Event Subscriptions Collection",
        "Description": "Collection of Event Subscriptions",
        "Members@odata.count": 3,
        "Members": [
        {
        "@odata.id": "/redfish/v1/EventService/Subscriptions/1"
        },
        {
        "@odata.id": "/redfish/v1/EventService/Subscriptions/2"
        },
        {
        "@odata.id": "/redfish/v1/EventService/Subscriptions/3"
        }
        ]
        }	
        
      - Response : 200 OK	      
	       	     
   Above 3 events will be remain there even device reboot until you delete them.
		 
   Event will be sent out by followiing JSON payload to Event Lister
		 
    {
    "@odata.context": "/rest/v1/$metadata#EventService/Members/Events/1",
    "@odata.id": "/rest/v1/EventService/Events/1",
    "@odata.type": "#EventService.1.0.0.Event",
    "Id": "1",
    "Name": "Event Array",
    "Description": "Events",
    "Events": [
    {
    "EventType": "ResourceAdded",  < = This can be ¡§ResourceRemoved¡¨  or  ¡§Alert¡¨ 
    "EventId": "12f03ec2-714f-11e9-ba1f-ef16238bd344",
    "EventTimestamp": "2019-05-08T05:06:44+00:00",
    "Severity": "OK",
    "Message": "Port 1 Plug In.",  <= This message content depend on OLT¡¦s peripheral situation. Like port plugin/out ..etc.
    "MessageId": "Base.1.0.0.Success",
    "MessageArgs": [],
    "OriginOfCondition": "",
    "Context": "THIS IS TEST FROM LISTENER SUBS"
    }
    ]
    }
		  
   Above event payload will send to event listener https server and lister server need response 
	  
	  - Response : 200 OK			
	 
   If there is no respose from this event listener 3 times then PSME Redfish Service will un-subscribe this event from PSME event service.
	 
   Delete subscription events 1
	  
    - URL     :  /redfish/v1/EventService/Subscriptions/1
    - Method  :  Delete
    - Payload :  

    {}           

    - Response : 204 NO_CONTENT
    
   You can use this test API to trigger 3 type or events.
   
    wget --no-check-certificate  -qO- https://x.x.x.x:8888/redfish/v1/EventService/TestEventSubscription  | python -m json.tool

  ## Querying ONL Status
  
  Here we will take Edgecore XGSPON device x86-64-accton-asxvolt16-r0 for example and 
  get related devices and ONL status from PSME Redfish Service.
  
  ##  Chassis information  
    
   Can get "Model", "SerialNumber","PartNumber" information. 
    
      - URL     :  /redfish/v1/Chassis/1/
      - Method  :  GET
      - Payload :    

        {
        "@odata.context": "/redfish/v1/$metadata#Chassis.Chassis",
        "@odata.id": "/redfish/v1/Chassis/1",
        "@odata.type": "#Chassis.v1_3_0.Chassis",
        "Id": "1",
        "ChassisType": "Drawer",
        "Name": "Chassis",
        "Description": "Chassis of switch devices.",
        "PowerState": "On",
        "Manufacturer": "Accton",
        "Model": "FN1EC0816400Z",
        "SKU": null,
        "SerialNumber": "EC1840000200",
        "PartNumber": "FN1EC0816400Z",
        "AssetTag": "N/A",
        "IndicatorLED": null,
        "Status":{"State": "Enabled", "Health": "OK", "HealthRollup": "OK"},
        "Oem":{"Intel_RackScale":{"@odata.type": "#Intel.Oem.Chassis", "Location":{"Id": null, "ParentId": null¡K},
        "Links":{"@odata.type": "#Chassis.v1_2_0.Links", "Contains":[], "ComputerSystems":[{"@odata.id": "/redfish/v1/Systems/1"¡K},
        "Actions":{"#Chassis.Reset":{"target": "/redfish/v1/Chassis/1/Actions/Chassis.Reset", "ResetType@Redfish.AllowableValues":["ForceOff",¡K},
        "Thermal":{"@odata.id": "/redfish/v1/Chassis/1/Thermal"},
        "Power":{"@odata.id": "/redfish/v1/Chassis/1/Power"
        }
        
	 - Response : 200 OK


   Get current PSME Redfish Service application version. 
   Value of "FirmwareVersion" in payload like "2.1.3.59.20".
      
      - URL     :  /redfish/v1/Managers/1
      - Method  :  GET
      - Payload :   

        {
        "@odata.context": "/redfish/v1/$metadata#Manager.Manager",
        "@odata.id": "/redfish/v1/Managers/1",
        "@odata.type": "#Manager.v1_0_0.Manager",
        "Id": "1",
        "Name": "Manager",
        "Description": "Manager description",
        "ManagerType": "EnclosureManager",
        "Model": null,
        "UUID": "5f471f72-146c-11ff-aeff-dfc55209c612",
        "DateTime": "2038-10-06T09:28:14+00:00",
        "DateTimeLocalOffset": "+00:00",
        "Status":{"State": "Enabled", "Health": "OK", "HealthRollup": "OK"},
        "EthernetInterfaces":{"@odata.id": "/redfish/v1/Managers/1/EthernetInterfaces"},
        "SerialInterfaces":{"@odata.id": "/redfish/v1/Managers/1/SerialInterfaces"},
        "FirmwareVersion": "2.1.3.59.20",
        "Oem":{},
        "Links":{"@odata.type": "#Manager.v1_0_0.Links", "ManagerForChassis":[{"@odata.id": "/redfish/v1/Chassis/1"¡K},
        "NetworkProtocol":{"@odata.id": "/redfish/v1/Managers/1/NetworkProtocol"},
        "ServiceEntryPointUUID": "5f471f72-146c-11ff-aeff-dfc55209c612",
        "SerialConsole":{"ServiceEnabled": true, "MaxConcurrentSessions": 0, "ConnectTypesSupported":[]},
        "LogServices":{"@odata.id": "/redfish/v1/Managers/1/LogServices"}
        }
        
	  - Response : 200 OK

   ## PSU information
    
   PSU FAN speed :
   Can be read from "Reading" property in payload of Fans object 7,8.
    	  
      - URL     :  /redfish/v1/Chassis/1/Thermal
      - Method  :  GET
      - Payload :  
  
        {
        "@odata.context": "/redfish/v1/$metadata#Thermal.Thermal",
        "@odata.id": "/redfish/v1/Chassis/1/Thermal",
        "@odata.type": "#Thermal.v1_1_0.Thermal",
        "Name": "Thermal",
        "Id": "Thermal",
        "Description": "Collection of Thermal sensors",
        "Redundancy":[],
        "Temperatures":[
        {"@odata.id": "/redfish/v1/Chassis/1/Thermal", "MemberId": "1", "Name": "System CPU Thermal Sensor Temperature",¡K},
        {"@odata.id": "/redfish/v1/Chassis/1/Thermal", "MemberId": "2", "Name": "Chassis Thermal Sensor Temperature",¡K},
        {"@odata.id": "/redfish/v1/Chassis/1/Thermal", "MemberId": "3", "Name": "Chassis Thermal Sensor Temperature",¡K},
        {"@odata.id": "/redfish/v1/Chassis/1/Thermal", "MemberId": "4", "Name": "Chassis Thermal Sensor Temperature",¡K},
        {"@odata.id": "/redfish/v1/Chassis/1/Thermal", "MemberId": "5", "Name": "Chassis Thermal Sensor Temperature",¡K},
        {"@odata.id": "/redfish/v1/Chassis/1/Thermal", "MemberId": "6", "Name": "PSU Thermal Sensor Temperature",¡K},
        {"@odata.id": "/redfish/v1/Chassis/1/Thermal", "MemberId": "7", "Name": "PSU Thermal Sensor Temperature",¡K}
        ],
        "Fans":[
        {"@odata.id": "/redfish/v1/Chassis/1/Thermal", "MemberId": "1", "Name": "System Fan",¡K},
        {"@odata.id": "/redfish/v1/Chassis/1/Thermal", "MemberId": "2", "Name": "System Fan",¡K},
        {"@odata.id": "/redfish/v1/Chassis/1/Thermal", "MemberId": "3", "Name": "System Fan",¡K},
        {"@odata.id": "/redfish/v1/Chassis/1/Thermal", "MemberId": "4", "Name": "System Fan",¡K},
        {"@odata.id": "/redfish/v1/Chassis/1/Thermal", "MemberId": "5", "Name": "System Fan",¡K},
        {"@odata.id": "/redfish/v1/Chassis/1/Thermal", "MemberId": "6", "Name": "System Fan",¡K},
        {
            "@odata.id": "/redfish/v1/Chassis/1/Thermal",
            "MemberId": "7",
            "Name": "PSU Fan",
            "PhysicalContext": "PowerSupply",
            "Reading": 6600,   <==PSU FAN speed
            "Status":{"HealthRollup": null, "Health": "OK", "State": "Enabled"},
            "ReadingUnits": "RPM",
            "UpperThresholdNonCritical": null,
            "UpperThresholdCritical": null,
            "UpperThresholdFatal": null,
            "LowerThresholdNonCritical": null,
            "LowerThresholdCritical": null,
            "LowerThresholdFatal": null,
            "RelatedItem":[{"@odata.id": "/redfish/v1/Chassis/1" }]
        },
        {
            "@odata.id": "/redfish/v1/Chassis/1/Thermal",
            "MemberId": "8",
            "Name": "PSU Fan",
            "PhysicalContext": "PowerSupply",
            "Reading": 6496,   <==PSU FAN speed
            "Status":{"HealthRollup": null, "Health": "OK", "State": "Enabled"},
            "ReadingUnits": "RPM",
            "UpperThresholdNonCritical": null,
            "UpperThresholdCritical": null,
            "UpperThresholdFatal": null,
            "LowerThresholdNonCritical": null,
            "LowerThresholdCritical": null,
            "LowerThresholdFatal": null,
            "RelatedItem":[{"@odata.id": "/redfish/v1/Chassis/1" }]
        }
        ]
        }
		  - Response : 200 OK
		  
		        
   PSU Power consumption :  
   Can be read from "PowerConsumedWatts" property in payload of PowerControl object 1,2.

      - URL     :  /redfish/v1/Chassis/1/Power
      - Method  :  GET
      - Payload : 

        {
        "@odata.context": "/redfish/v1/$metadata#Power.Power",
        "@odata.id": "/redfish/v1/Chassis/1/Power",
        "Id": "Power",
        "@odata.type": "#Power.v1_1_0.Power",
        "Name": "Power Collection",
        "Description": "Collection of Power",
        "PowerControl": [
        {
        "@odata.id": "/redfish/v1/Chassis/1/Power",
        "MemberId": "1",
        "Name": "System Power Control",
        "PowerConsumedWatts": 53,    <=== Power Consumed Watts
        "Status": {
        "Health": "OK",
        "State": "Enabled"
        },
        "PowerRequestedWatts": null,
        "PowerAvailableWatts": null,
        "PowerCapacityWatts": 0,
        "PowerAllocatedWatts": null,
        "PowerLimit": {
        "LimitInWatts": null,
        "LimitException": null,
        "CorrectionInMs": null
        },
        "RelatedItem": [
        {
        "@odata.id": "/redfish/v1/Chassis/1"
        }
        ],
        "Oem": {}
        },
        {
        "@odata.id": "/redfish/v1/Chassis/1/Power",
        "MemberId": "2",
        "Name": "System Power Control",
        "PowerConsumedWatts": 43,    <=== Power Consumed Watts
        "Status": {
        "Health": "OK",
        "State": "Enabled"
        },
        "PowerRequestedWatts": null,
        "PowerAvailableWatts": null,
        "PowerCapacityWatts": 0,
        "PowerAllocatedWatts": null,
        "PowerLimit": {
        "LimitInWatts": null,
        "LimitException": null,
        "CorrectionInMs": null
        },
        "RelatedItem": [
        {
        "@odata.id": "/redfish/v1/Chassis/1"
        }
        ],
        "Oem": {}
        }
        ],
        "Oem": {}
        }	
        - Response : 200 OK			
			
   PSU Present :

   Can be read from "Statet" in "Status" property in payload of PowerControl object 1,2.

      - URL     :  /redfish/v1/Chassis/1/Power
      - Method  :  GET
      - Payload : 

        {
        "@odata.context": "/redfish/v1/$metadata#Power.Power",
        "@odata.id": "/redfish/v1/Chassis/1/Power",
        "Id": "Power",
        "@odata.type": "#Power.v1_1_0.Power",
        "Name": "Power Collection",
        "Description": "Collection of Power",
        "PowerControl": [
        {
        "@odata.id": "/redfish/v1/Chassis/1/Power",
        "MemberId": "1",
        "Name": "System Power Control",
        "PowerConsumedWatts": 53,
        "Status": {
        "Health": "OK",  <==== OK : Present , Absent: No Present
        "State": "Enabled"
        },
        "PowerRequestedWatts": null,
        "PowerAvailableWatts": null,
        "PowerCapacityWatts": 0,
        "PowerAllocatedWatts": null,
        "PowerLimit": {
        "LimitInWatts": null,
        "LimitException": null,
        "CorrectionInMs": null
        },
        "RelatedItem": [
        {
          "@odata.id": "/redfish/v1/Chassis/1"
        }
        ],
        "Oem": {}
        },
        {
        "@odata.id": "/redfish/v1/Chassis/1/Power",
        "MemberId": "2",
        "Name": "System Power Control",
        "PowerConsumedWatts": 43,
        "Status": {
        "Health": "OK",  <==== OK : Present , Absent: No Present
        "State": "Enabled"
        },
        "PowerRequestedWatts": null,
        "PowerAvailableWatts": null,
        "PowerCapacityWatts": 0,
        "PowerAllocatedWatts": null,
        "PowerLimit": {
        "LimitInWatts": null,
        "LimitException": null,
        "CorrectionInMs": null
        },
        "RelatedItem": [
        {
          "@odata.id": "/redfish/v1/Chassis/1"
        }
        ],
        "Oem": {}
        }
        ],
        "Oem": {}
        }	
        - Response : 200 OK			
			
   PSU Power thermal sensor:
      
   Can be read from "ReadingCelsius" property in payload of Temperatures object 6,7.

      - URL     :  /redfish/v1/Chassis/1/Thermal
      - Method  :  GET
      - Payload : 

        {
        "@odata.context": "/redfish/v1/$metadata#Thermal.Thermal",
        "@odata.id": "/redfish/v1/Chassis/1/Thermal",
        "@odata.type": "#Thermal.v1_1_0.Thermal",
        "Name": "Thermal",
        "Id": "Thermal",
        "Description": "Collection of Thermal sensors",
        "Redundancy":[],
        "Temperatures":[
        {"@odata.id": "/redfish/v1/Chassis/1/Thermal", "MemberId": "1", "Name": "System CPU Thermal Sensor Temperature",¡K},
        {"@odata.id": "/redfish/v1/Chassis/1/Thermal", "MemberId": "2", "Name": "Chassis Thermal Sensor Temperature",¡K},
        {"@odata.id": "/redfish/v1/Chassis/1/Thermal", "MemberId": "3", "Name": "Chassis Thermal Sensor Temperature",¡K},
        {"@odata.id": "/redfish/v1/Chassis/1/Thermal", "MemberId": "4", "Name": "Chassis Thermal Sensor Temperature",¡K},
        {"@odata.id": "/redfish/v1/Chassis/1/Thermal", "MemberId": "5", "Name": "Chassis Thermal Sensor Temperature",¡K},
        {"@odata.id": "/redfish/v1/Chassis/1/Thermal", "MemberId": "6", "Name": "PSU Thermal Sensor Temperature",¡K},
        {
        "@odata.id": "/redfish/v1/Chassis/1/Thermal",
        "MemberId": "7",
        "Name": "PSU Thermal Sensor Temperature",
        "PhysicalContext": "PowerSupply",
        "SensorNumber": 7,
        "Status":{"HealthRollup": null, "Health": "OK", "State": "Enabled"},
        "ReadingCelsius": 39,  <=== Power thermal sensor
        "UpperThresholdNonCritical": 45,
        "UpperThresholdCritical": 55,
        "UpperThresholdFatal": 60,
        "LowerThresholdNonCritical": null,
        "LowerThresholdCritical": null,
        "LowerThresholdFatal": null,
        "RelatedItem":[{"@odata.id": "/redfish/v1/Chassis/1" }]
        }
        ],
        "Fans":[{"@odata.id": "/redfish/v1/Chassis/1/Thermal", "MemberId": "1",¡K]
        }
        - Response : 200 OK			
			
   ## System FAN information :
      
   System FAN speed :    
   Can be read from "Reading" property in payload of Fans object 1 to 6.

   FAN Present :
   Can be read from "State" in "Status" property in payload of Fans object 1 to 6.
    	  
      - URL     :  /redfish/v1/Chassis/1/Thermal
      - Method  :  GET
      - Payload :  
        {
        "@odata.context": "/redfish/v1/$metadata#Thermal.Thermal",
        "@odata.id": "/redfish/v1/Chassis/1/Thermal",
        "@odata.type": "#Thermal.v1_1_0.Thermal",
        "Name": "Thermal",
        "Id": "Thermal",
        "Description": "Collection of Thermal sensors",
        "Redundancy":[],
        "Temperatures":[
        {"@odata.id": "/redfish/v1/Chassis/1/Thermal", "MemberId": "1", "Name": "System CPU Thermal Sensor Temperature",¡K},
        {"@odata.id": "/redfish/v1/Chassis/1/Thermal", "MemberId": "2", "Name": "Chassis Thermal Sensor Temperature",¡K},
        {"@odata.id": "/redfish/v1/Chassis/1/Thermal", "MemberId": "3", "Name": "Chassis Thermal Sensor Temperature",¡K},
        {"@odata.id": "/redfish/v1/Chassis/1/Thermal", "MemberId": "4", "Name": "Chassis Thermal Sensor Temperature",¡K},
        {"@odata.id": "/redfish/v1/Chassis/1/Thermal", "MemberId": "5", "Name": "Chassis Thermal Sensor Temperature",¡K},
        {"@odata.id": "/redfish/v1/Chassis/1/Thermal", "MemberId": "6", "Name": "PSU Thermal Sensor Temperature",¡K},
        {"@odata.id": "/redfish/v1/Chassis/1/Thermal", "MemberId": "7", "Name": "PSU Thermal Sensor Temperature",¡K}
        ],
        "Fans":[
        {
        "@odata.id": "/redfish/v1/Chassis/1/Thermal",
        "MemberId": "1",
        "Name": "System Fan",
        "PhysicalContext": "Back",
        "Reading": 6600,                                                    <==System FAN ID 1 speed
        "Status":{"HealthRollup": OK, "Health": "OK", "State": "Enabled"},  <==== Enabled : Present , Absent: No Present
        "ReadingUnits": "RPM",
        "UpperThresholdNonCritical": null,
        "UpperThresholdCritical": null,
        "UpperThresholdFatal": null,
        "LowerThresholdNonCritical": null,
        "LowerThresholdCritical": null,
        "LowerThresholdFatal": null,
        "RelatedItem":[{"@odata.id": "/redfish/v1/Chassis/1" }]
        },
        {"@odata.id": "/redfish/v1/Chassis/1/Thermal", "MemberId": "2", "Name": "System Fan",¡K},
        {"@odata.id": "/redfish/v1/Chassis/1/Thermal", "MemberId": "3", "Name": "System Fan",¡K},
        {"@odata.id": "/redfish/v1/Chassis/1/Thermal", "MemberId": "4", "Name": "System Fan",¡K},
        {"@odata.id": "/redfish/v1/Chassis/1/Thermal", "MemberId": "5", "Name": "System Fan",¡K},
        {"@odata.id": "/redfish/v1/Chassis/1/Thermal", "MemberId": "6", "Name": "System Fan",¡K},
        {"@odata.id": "/redfish/v1/Chassis/1/Thermal", "MemberId": "7", "Name": "PSU Fan",¡K},
        {"@odata.id": "/redfish/v1/Chassis/1/Thermal", "MemberId": "8", "Name": "PSU Fan",¡K}
        ]
        }	       	  
        - Response : 200 OK

			
   ## System Thermal information :
   
   Can be read from "ReadingCelsius" property in payload of Temperatures object 1 to 5.

      - URL     :  /redfish/v1/Chassis/1/Thermal
      - Method  :  GET
      - Payload : 
      
        {
        "@odata.context": "/redfish/v1/$metadata#Thermal.Thermal",
        "@odata.id": "/redfish/v1/Chassis/1/Thermal",
        "@odata.type": "#Thermal.v1_1_0.Thermal",
        "Name": "Thermal",
        "Id": "Thermal",
        "Description": "Collection of Thermal sensors",
        "Redundancy":[],
        "Temperatures":[
        {
        "@odata.id": "/redfish/v1/Chassis/1/Thermal",
        "MemberId": "1",
        "Name": "System CPU Thermal Sensor Temperature",
        "PhysicalContext": "CPU",
        "SensorNumber": 1,
        "Status":{"HealthRollup": null, "Health": "Warning", "State": "Enabled"},
        "ReadingCelsius": 52,    <=== System Thermal sensor reading value.
        "UpperThresholdNonCritical": 45,
        "UpperThresholdCritical": 55,
        "UpperThresholdFatal": 60,
        "LowerThresholdNonCritical": null,
        "LowerThresholdCritical": null,
        "LowerThresholdFatal": null,
        "RelatedItem":[{"@odata.id": "/redfish/v1/Chassis/1" }]
        },
        {"@odata.id": "/redfish/v1/Chassis/1/Thermal", "MemberId": "2", "Name": "Chassis Thermal Sensor Temperature",¡K},
        {"@odata.id": "/redfish/v1/Chassis/1/Thermal", "MemberId": "3", "Name": "Chassis Thermal Sensor Temperature",¡K},
        {"@odata.id": "/redfish/v1/Chassis/1/Thermal", "MemberId": "4", "Name": "Chassis Thermal Sensor Temperature",¡K},
        {"@odata.id": "/redfish/v1/Chassis/1/Thermal", "MemberId": "5", "Name": "Chassis Thermal Sensor Temperature",¡K},
        {"@odata.id": "/redfish/v1/Chassis/1/Thermal", "MemberId": "6", "Name": "PSU Thermal Sensor Temperature",¡K},
        {"@odata.id": "/redfish/v1/Chassis/1/Thermal", "MemberId": "7", "Name": "PSU Thermal Sensor Temperature",¡K}
        ],
        "Fans":[{"@odata.id": "/redfish/v1/Chassis/1/Thermal", "MemberId": "1",¡K]
        }      
        - Response : 200 OK
		        
   ## QSFP/XFP information :
      
   Collection of port 
   
       - URL     :  /redfish/v1/EthernetSwitches/1/Ports
       - Method  :  GET
       - Payload :       
      
        {
        "@odata.context": "/redfish/v1/$metadata#EthernetSwitchPortCollection.EthernetSwitchPortCollection",
        "@odata.id": "/redfish/v1/EthernetSwitches/1/Ports",
        "@odata.type": "#EthernetSwitchPortCollection.EthernetSwitchPortCollection",
        "Name": "Ethernet Switch Port Collection",
        "Description": "Collection of Ethernet Switch Ports",
        "Members@odata.count": 20,
        "Members":[
        {"@odata.id": "/redfish/v1/EthernetSwitches/1/Ports/1"},
        {"@odata.id": "/redfish/v1/EthernetSwitches/1/Ports/2"},
        {"@odata.id": "/redfish/v1/EthernetSwitches/1/Ports/3"},
        {"@odata.id": "/redfish/v1/EthernetSwitches/1/Ports/4"},
        {"@odata.id": "/redfish/v1/EthernetSwitches/1/Ports/5"},
        {"@odata.id": "/redfish/v1/EthernetSwitches/1/Ports/6"},
        {"@odata.id": "/redfish/v1/EthernetSwitches/1/Ports/7"},
        {"@odata.id": "/redfish/v1/EthernetSwitches/1/Ports/8"},
        {"@odata.id": "/redfish/v1/EthernetSwitches/1/Ports/9"},
        {"@odata.id": "/redfish/v1/EthernetSwitches/1/Ports/10"},
        {"@odata.id": "/redfish/v1/EthernetSwitches/1/Ports/11"},
        {"@odata.id": "/redfish/v1/EthernetSwitches/1/Ports/12"},
        {"@odata.id": "/redfish/v1/EthernetSwitches/1/Ports/13"},
        {"@odata.id": "/redfish/v1/EthernetSwitches/1/Ports/14"},
        {"@odata.id": "/redfish/v1/EthernetSwitches/1/Ports/15"},
        {"@odata.id": "/redfish/v1/EthernetSwitches/1/Ports/16"},
        {"@odata.id": "/redfish/v1/EthernetSwitches/1/Ports/17"},
        {"@odata.id": "/redfish/v1/EthernetSwitches/1/Ports/18"},
        {"@odata.id": "/redfish/v1/EthernetSwitches/1/Ports/19"},
        {"@odata.id": "/redfish/v1/EthernetSwitches/1/Ports/20"}
        ]
        }
        - Response : 200 OK
		    			
   Port 1 Present :
        
      - URL     :  /redfish/v1/EthernetSwitches/1/Ports/1
      - Method  :  GET
      - Payload :  
  
        {
        "@odata.context": "/redfish/v1/$metadata#EthernetSwitchPort.EthernetSwitchPort",
        "@odata.id": "/redfish/v1/EthernetSwitches/1/Ports/1",
        "@odata.type": "#EthernetSwitchPort.v1_0_0.EthernetSwitchPort",
        "Id": "1",
        "Name": "Port1",
        "Description": "Ethernet Switch Port description",
        "PortId": "Port ID",
        "Status":{
        "State": "Enabled",  <==== Enabled : Present , Absent: No Present
        "Health": "OK",
        "HealthRollup": "OK"
        },
        "LinkType": "Ethernet",
        "OperationalState": null,
        "AdministrativeState": null,
        "LinkSpeedMbps": null,
        "NeighborInfo":{"SwitchId": null, "PortId": null, "CableId": null},
        "NeighborMAC": null,
        "FrameSize": null,
        "Autosense": null,
        "FullDuplex": null,
        "MACAddress": null,
        "PortClass": null,
        "PortMode": null,
        "PortType": null,
        "Oem":{},
        "IPv4Addresses":[],
        "IPv6Addresses":[],
        "VLANs":{"@odata.id": "/redfish/v1/EthernetSwitches/1/Ports/1/VLANs"},
        "StaticMACs":{
        "@odata.id": "/redfish/v1/EthernetSwitches/1/Ports/1/StaticMACs"
        },
        "Links":{"PrimaryVLAN": null, "Switch":{"@odata.id": "/redfish/v1/EthernetSwitches/1"¡K},
        "TransceiverStatistics":{"SFP Vendor Name": "SOURCEPHOTONICS", "Part Number": "XPPXG2N1CDFA", "Serial Number": "H6B2010733", "Manufacture Date": "170510",¡K}
        }
        - Response : 200 OK
        
   Port 1 "Transceiver Statistics" (OEM):
      
   If device detect supported transceiver in device, we can get related information like
   "SFP Vendor Name","Part Number","Serial Number", "Manufacture Date" of transceiver.
   and "Temperature","Voltage","BiasCurrent","TxPower" detail data.
    
   If transceiver not plug-in device then no such data appear in URL.
          
      - URL     :  /redfish/v1/EthernetSwitches/1/Ports/1
      - Method  :  GET
      - Payload :  
        {
        "@odata.context": "/redfish/v1/$metadata#EthernetSwitchPort.EthernetSwitchPort",
        "@odata.id": "/redfish/v1/EthernetSwitches/1/Ports/1",
        "@odata.type": "#EthernetSwitchPort.v1_0_0.EthernetSwitchPort",
        "Id": "1",
        "Name": "Port1",
        "Description": "Ethernet Switch Port description",
        "PortId": "Port ID",
        "Status":{"State": "Enabled", "Health": "OK", "HealthRollup": "OK"},
        "LinkType": "Ethernet",
        "OperationalState": null,
        "AdministrativeState": null,
        "LinkSpeedMbps": null,
        "NeighborInfo":{"SwitchId": null, "PortId": null, "CableId": null},
        "NeighborMAC": null,
        "FrameSize": null,
        "Autosense": null,
        "FullDuplex": null,
        "MACAddress": null,
        "PortClass": null,
        "PortMode": null,
        "PortType": null,
        "Oem":{},
        "IPv4Addresses":[],
        "IPv6Addresses":[],
        "VLANs":{"@odata.id": "/redfish/v1/EthernetSwitches/1/Ports/1/VLANs"},
        "StaticMACs":{"@odata.id": "/redfish/v1/EthernetSwitches/1/Ports/1/StaticMACs"},
        "Links":{"PrimaryVLAN": null, "Switch":{"@odata.id": "/redfish/v1/EthernetSwitches/1"¡K},
        "TransceiverStatistics":
        {
        "SFP Vendor Name": "SOURCEPHOTONICS",
        "Part Number": "XPPXG2N1CDFA",
        "Serial Number": "H6B2010733",
        "Manufacture Date": "170510",
        "Temperature":{
        "Reading": 39.527,
        "UpperThresholdFatal": 80,
        "UpperThresholdCritical": 75,
        "LowerThresholdCritical": -5,
        "LowerThresholdFatal": -10,
        "Status":{"State": "Enabled", "Health": "OK"}
        },
        "Voltage":{
        "Reading": 3.347,
        "UpperThresholdFatal": 3.599,
        "UpperThresholdCritical": 3.549,
        "LowerThresholdCritical": 3.049,
        "LowerThresholdFatal": 3,
        "Status":{"State": "Enabled", "Health": "OK"}
        },
        "BiasCurrent":{
        "Reading": 0,
        "UpperThresholdFatal": 130,
        "UpperThresholdCritical": 120,
        "LowerThresholdCritical": 20,
        "LowerThresholdFatal": 10,
        "Status":{"State": "Enabled", "Health": "Critical"}
        },
        "TxPower":{
        "Reading": 0,
        "UpperThresholdFatal": 5.011,
        "UpperThresholdCritical": 3.981,
        "LowerThresholdCritical": 1.412,
        "LowerThresholdFatal": 1.122,
        "Status":{"State": "Enabled", "Health": "Critical"}
        }
        }
        }  
        - Response : 200 OK			


   If device detect un-supported transceiver in device, we can not get related information like
   "SFP Vendor Name","Part Number","Serial Number", "Manufacture Date" of transceiver and all value 
   are "NA" and "Temperature","Voltage","BiasCurrent","TxPower" value are all 0.

        TransceiverStatistics":{
        "SFP Vendor Name": "NA",
        "Part Number": "NA",
        "Serial Number": "NA",
        "Manufacture Date": "NA",
        "Temperature":{"Reading": 0, "UpperThresholdFatal": 0, "UpperThresholdCritical": 0, "LowerThresholdCritical": 0, "LowerThresholdFatal": 0,¿},
        "Voltage":{"Reading": 0, "UpperThresholdFatal": 0, "UpperThresholdCritical": 0, "LowerThresholdCritical": 0, "LowerThresholdFatal": 0,¿},
        "BiasCurrent":{"Reading": 0, "UpperThresholdFatal": 0, "UpperThresholdCritical": 0, "LowerThresholdCritical": 0, "LowerThresholdFatal": 0,¿},
        "TxPower":{"Reading": 0, "UpperThresholdFatal": 0, "UpperThresholdCritical": 0, "LowerThresholdCritical": 0, "LowerThresholdFatal": 0,¿}
        }

  ##  Supported Statistics of Transceivers Hardware Vendor informations 
  
	+========+===============+===============+
	| QSFP28 |               |               |
	+========+===============+===============+
	|        |   Vendor Name |   Part Number |
	+--------+---------------+---------------+
	|      1 |   Precision   |   QSFP28AOC03 |
	+--------+---------------+---------------+

	+==============+====================+================+
	| XFP (XGSPON) |                    |                |
	+==============+====================+================+
	|              |   Vendor Name      |   Part Number  |
	+--------------+--------------------+----------------+
	|            1 |   Source Photonics |   XPPXG2N1CDFA |
	+--------------+--------------------+----------------+
	|            2 |   Hisense          |   LTH7226-PC+  |
	+--------------+--------------------+----------------+

	+===========+===============+================+
	| SFP(GPON) |               |                |
	+===========+===============+================+
	|           |   Vendor Name |   Part Number  |
	+-----------+---------------+----------------+
	|         1 |   Hisense     |   LTE3680M-BH+ |
	+-----------+---------------+----------------+


  ## Reboot device through PSME
  
        - URL     :  /redfish/v1/Systems/1/Actions/ComputerSystem.Reset
        - Method  :  POST
        - Payload :      

        {
        "ResetType":"GracefulRestart"
        }
        - Response : 200 OK	
		  
		  
  ## Log service

   Log service will record all ONL peripheral Add/Remove/Alert event like
   Fan plug-in and out , thermal over fatal critical threshold temperature etc.
	  
   Enbale/Disabe log service.
	   
    - URL     :  /redfish/v1/Managers/1/LogServices/1
    - Method  :  PATCH
    - Payload :      

    {
    "ServiceEnabled": true    <=== false to disalbe it.
    } 
    - Response : 204 NO_CONTENT		  
      
      
   Get log entries of current device.
	  
    - URL     :  /redfish/v1/Managers/1/LogServices/1/Entries
    - Method  :  GET
    - Payload :       

    {
    "@odata.context": "/redfish/v1/$metadata#LogEntryCollection.LogEntryCollection",
    "@odata.id": "/redfish/v1/Managers/1/LogServices/1/Entries",
    "@odata.type": "#LogEntryCollection.LogEntryCollection",
    "Name": "Log Service Collection",
    "Description": "Collection of Logs for this System",
    "Members@odata.count": 3,
    "Members":[
    {"@odata.id": "/redfish/v1/Managers/1/LogServices/1/Entries/0", "Id": "0",¡K},
    {"@odata.id": "/redfish/v1/Managers/1/LogServices/1/Entries/1", "Id": "1",¡K},
    {"@odata.id": "/redfish/v1/Managers/1/LogServices/1/Entries/2", "Id": "2",¡K}]
    }	      	

    - Response : 200 OK	
		  
   Clear log entries of current device.
		  
    - URL     :  /v1/Managers/1/LogServices/1/Actions/LogService.Reset
    - Method  :  POST
    - Payload : 		  
    {}
    - Response : 200 OK	      
	   	  			
