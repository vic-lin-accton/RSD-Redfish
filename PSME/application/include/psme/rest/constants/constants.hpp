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



#include "psme/rest/constants/chassis.hpp"
#include "psme/rest/constants/ethernet.hpp"
#include "psme/rest/constants/storage.hpp"
#include "psme/rest/constants/system.hpp"
#include "psme/rest/constants/fabric.hpp"
#include "psme/rest/constants/routes.hpp"



namespace psme {
namespace rest {
namespace constants {

/*!
 * @brief Constant literals for Metadata endpoint.
 */
namespace Metadata {
extern const char METADATA_ROOT_FILE[];
}

/*!
 * @brief Constant literals for use with PathBuilder function.
 */
namespace PathParam {
extern const char BASE_ROOT[];
extern const char REDFISH[];
extern const char METADATA_ROOT[];
extern const char METADATA[];
extern const char BASE_URL[];
extern const char METADATA_FILE[];
extern const char SUBSCRIPTION_ID[];
extern const char ACCOUNT_ID[]; //viclin add
extern const char ROLE_ID[]; //viclin add
extern const char CHASSIS_ID[];
extern const char THERMAL_ZONE_ID[];
extern const char POWER_ZONE_ID[];
extern const char SYSTEM_ID[];
extern const char ADAPTER_ID[];
extern const char DEVICE_ID[];
extern const char SERVICE_ID[];
extern const char DRIVE_ID[];
extern const char LOGICAL_DRIVE_ID[];
extern const char REMOTE_TARGET_ID[];
extern const char MANAGER_ID[];
extern const char NIC_ID[];
extern const char NETWORK_INTERFACE_ID[];
extern const char NETWORK_DEVICE_FUNCTION_ID[];
extern const char NODE_ID[];
extern const char ETHERNET_SWITCH_ID[];
extern const char SWITCH_PORT_ID[];
extern const char VLAN_ID[];
extern const char PROCESSOR_ID[];
extern const char SIMPLE_STORAGE_ID[];
extern const char MEMORY_ID[];
extern const char FABRIC_ID[];
extern const char ENDPOINT_ID[];
extern const char SWITCH_ID[];
extern const char FUNCTION_ID[];
extern const char ZONE_ID[];
extern const char PORT_ID[];
extern const char FUNCTIONS[];
extern const char RULE_ID[];
extern const char STORAGE_ID[];
extern const char ACL_ID[];
extern const char STATIC_MAC_ID[];
extern const char TASK_ID[];
extern const char ODATA[];
extern const char MESSAGE_REGISTRY_FILE_ID[];

extern const char PATH_SEP;
extern const char VARIABLE_BEGIN;
extern const char VARIABLE_END;
extern const char REGEX_MARK;
extern const char ID_REGEX[];
extern const char USERNAME_REGEX[];
extern const char XML_REGEX[];
/*Nick Added Begin: */
extern const char FAN_ID[];     
extern const char PSU_ID[];  
extern const char THERMAL_ID[];
extern const char SESSION_ID[];
extern const char SER_ID[];
extern const char LOGSER_ID[];
/*Nick Added End  : */
}

/*!
 * @brief Common constant literals for rest endpoints.
 */
namespace Common {
extern const char INTEL[];
extern const char ODATA_CONTEXT[];
extern const char ODATA_ID[];
extern const char ODATA_TYPE[];
extern const char ID[];
extern const char MEMBER_ID[];
extern const char NAME[];
extern const char LINKS[];
extern const char MANUFACTURER[];
extern const char MODEL[];
extern const char SERIAL[];
extern const char PART_NUMBER[];
extern const char ASSET_TAG[];
extern const char STATUS[];
extern const char STATE[];
extern const char HEALTH[];
extern const char HEALTH_ROLLUP[];
extern const char ENUM_STATUS[];
extern const char OEM[];
extern const char RACKSCALE[];
extern const char API_VERSION[];
extern const char LOCATION[];
extern const char CONTAINED_BY[];
extern const char MANAGED_BY[];
extern const char COOLED_BY[];
extern const char POWERED_BY[];
extern const char DESCRIPTION[];
extern const char VENDOR_ID[];
extern const char DEVICE_ID[];
extern const char FIRMWARE_VERSION[];
extern const char UUID[];
extern const char ACTIONS[];
extern const char HASH[];
extern const char CHASSIS[];
extern const char RESET_TYPE[];
extern const char MAC_ADDRESS[];
extern const char USER_NAME[];
extern const char PASSWORD[];
extern const char ENTRIES[];
}

namespace ErrorMessage {
extern const char ERROR[];
extern const char CODE[];
extern const char MESSAGE[];
extern const char EXTENDED_INFO[];
}

/*!
 * @brief Constant literals for error message objects.
 */
namespace MessageObject {
extern const char JSON_POINTER_PREFIX[];
extern const char MESSAGE_ID[];
extern const char MESSAGE[];
extern const char MESSAGE_ARGS[];
extern const char RELATED_PROPERTIES[];
extern const char SEVERITY[];
extern const char RESOLUTION[];
}

/*!
 * @brief Constant literals for collection endpoints.
 */
namespace Collection {
extern const char ODATA_COUNT[];
extern const char MEMBERS[];
}

/*!
 * @brief Constant literals for /redfish/v1 root endpoint.
 */
namespace Root {
extern const char REDFISH_VERSION[];
extern const char EVENT_SERVICE[];
extern const char TASKS[];
extern const char SYSTEMS[];
extern const char SERVICES[];
extern const char MANAGERS[];
extern const char ETHERNET_SWITCHES[];
extern const char ETHERNET_INTERFACE[];
extern const char REGISTRIES[];
extern const char FABRICS[];
extern const char ACCOUNT_SERVICE[]; //viclin add
/*Nick Added Begin: */
extern const char FANS[]; 
extern const char THERMALZONES[];
extern const char POWERZONES[];
extern const char PSUS[];
extern const char PCTRL[];
extern const char PVOLTAGE[];
extern const char PSU[];
extern const char THERMALS[];
extern const char TEMPS[];
extern const char THERMAL[];
extern const char SESSION_SERVICE[]; 

/*Nick Added End  : */
}

/*!
 * @brief Constant literals for Redfish endpoint.
 */
namespace Redfish {
extern const char V1[];
}

/*!
 * @brief Constant literals for /redfish/v1/odata endpoint.
 */
namespace OdataServiceDocument {
extern const char VALUE[];
extern const char NAME[];
extern const char KIND[];
extern const char URL[];
}

/*!
 * @brief Constant literals for Manager endpoint.
 */
namespace Manager {
extern const char TARGET[];
extern const char MANAGER_TYPE[];
extern const char POWER_STATE[];
extern const char DATE_TIME[];
extern const char DATE_TIME_LOCAL_OFFSET[];
extern const char SERVICE_ENTRY_POINT_UUID[];
extern const char GRAPHICAL_CONSOLE[];
extern const char MAX_CONCURRENT_SESSIONS[];
extern const char CONNECT_TYPES_SUPPORTED[];
extern const char ENABLED[];
extern const char SERVICE_ENABLED[];
extern const char SERIAL_CONSOLE[];
extern const char COMMAND_SHELL[];
extern const char MANAGER_FOR_CHASSIS[];
extern const char MANAGER_FOR_SERVERS[];
extern const char MANAGER_FOR_SWITCHES[];
extern const char MANAGER_IN_CHASSIS[];
extern const char MANAGER_FOR_SERVICES[];
extern const char MANAGER_FOR_REMOTE_TARGETS[];
extern const char MANAGER_FOR_LOGICAL_DRIVES[];
extern const char MANAGER_FOR_PHYSICAL_DRIVES[];
extern const char NETWORK_PROTOCOL[];
extern const char ETHERNET_INTERFACES[];
extern const char SERIAL_INTERFACES[];
extern const char LOG_SERVICES[];
extern const char MAX_NUM_RECS[];
extern const char OVERWRITE_POLICY[];

extern const char ENTRIES[];
extern const char ENTRYTYPE[];
extern const char OEM_RECORD_FORMAT[];
extern const char SEVERITY[];
extern const char CREATED[];
extern const char ENTRY_CODE[];
extern const char SENSOR_TYPE[];
extern const char SENSOR_NUMBER[];
extern const char MESSAGE[];
}

/*!
 * @brief Constant literals for NetworkProtocol endpoint.
 */
namespace NetworkProtocol {
extern const char HOST_NAME[];
extern const char FQDN[];
extern const char HTTP[];
extern const char HTTPS[];
extern const char SNMP[];
extern const char VIRTUAL_MEDIA[];
extern const char TELNET[];
extern const char SSDP[];
extern const char IPMI[];
extern const char SSH[];
extern const char KVMIP[];
extern const char PROTOCOL_ENABLED[];
extern const char PORT[];
extern const char NOTIFY_IPV6_SCOPE[];
extern const char NOTIFY_MULTICAST_INTERVAL[];
extern const char NOTIFY_TTL[];
}

/*!
 * @brief Constant IpAddress literals for rest endpoints.
 */
namespace IpAddress {
extern const char ADDRESS[];
extern const char ADDRESS_ORIGIN[];
extern const char GATEWAY[];
extern const char HOST_NAME[];
extern const char FQDN[];
extern const char SUBNET_MASK[];
extern const char PREFIX_LENGTH[];
extern const char SCOPE[];
extern const char ADDRESS_STATE[];
}

/*!
 * @brief Constant literals for NetworkInterface endpoint.
 */
namespace NetworkInterface {
extern const char MAC_ADDRESS[];
extern const char PERMANENT_MAC_ADDRESS[];
extern const char SPEED[];
extern const char AUTO_NEG[];
extern const char FULL_DUPLEX[];
extern const char MTU_SIZE[];
extern const char HOST_NAME[];
extern const char FQDN[];
extern const char MAX_IPv6_STATIC_ADDRESSES[];
extern const char IPv4_ADDRESSES[];
extern const char IPv6_ADDRESS_POLICY_TABLE[];
extern const char PREFIX[];
extern const char PRECEDENCE[];
extern const char LABEL[];
extern const char IPv6_STATIC_ADDRESSES[];
extern const char IPv6_DHCP[];
extern const char IPv6_DHCP_OPERATINMODE[];
extern const char IPv6_DHCP_USE_DNS_SERVERS[];
extern const char IPv6_DHCP_USE_NTP_SERVERS[];
extern const char IPv6_DHCP_USE_RAPID_COMMIT[];
extern const char IPv4_DHCP[];
extern const char IPv4_DHCP_ENABLED[];
extern const char IPv4_DHCP_USE_DNS_SERVERS[];
extern const char IPv4_DHCP_USE_DOMAIN_NAME[];
extern const char IPv4_DHCP_USE_GATEWAY[];
extern const char IPv4_DHCP_USE_NTP_SERVERS[];
extern const char IPv4_DHCP_USE_STATIC_ROUTES[];
extern const char IPv4_DHCP_CONFIG[];
extern const char STATELESS_ADDRESS_AUTO_CONFIG[];

extern const char STATIC_NAME_SERVERS[];
extern const char IPv4_STACIC_ADDRESSES[];
extern const char IPv6_ADDRESSES[];
extern const char IPv6_DEFAULT_GATEWAY[];
extern const char IPv6_AUTO_CONFIG_ENABLED[];
extern const char IPv6_Static_DEFAULT_GATEWAY[];
extern const char NAME_SERVERS[];
extern const char INTERFACE_ENABLED[];
extern const char UEFI_DEVICE_PATH[];
extern const char VLANS[];
extern const char VLAN[];
extern const char NEIGHBOR_PORT[];
extern const char LINK_STATUS[];
}

/*!
 * @brief Constant literals for EventService endpoint.
 */
namespace EventService {
extern const char SERVICE_ENABLED[];
extern const char DELIVERY_RETRY_ATTEMPTS[];
extern const char DELIVERY_RETRY_INTERVAL_SECONDS[];
extern const char EVENT_TYPES_FOR_SUBSCRIPTION[];
extern const char STATUS_CHANGE[];
extern const char RESOURCE_UPDATED[];
extern const char RESOURCE_ADDED[];
extern const char RESOURCE_REMOVED[];
extern const char ALERT[];
extern const char SUBSCRIPTIONS[];
extern const char ACTIONS[];
extern const char SEND_TEST_EVENT[];
extern const char TARGET[];
extern const char ALLOWABLE_EVENT_TYPES[];
}


//viclin add begin
namespace AccountService {
extern const char SERVICE_ENABLED[];
extern const char AUTHFAILURELOGGINGTHRESHOLD[];
extern const char MINPASSWORDLENGTH[];
extern const char MAXPASSWORDLENGTH[];
extern const char ACCOUNTLOCKOUTTHRESHOLD[];
extern const char ACCOUNTLOCKOUTDURATION[];
extern const char ACCOUNTLOCKOUTCOUNTERRESETAFTER[];
extern const char ACCOUNTS[];
extern const char ROLES[];

}

namespace AccountConst {
extern const char ENABLED[];
extern const char PASSWORD[];
extern const char USERNAME[];
extern const char ROLEID[];
extern const char LOCKED[];
extern const char ROLE[];
}


namespace AccountRole {
extern const char ISPREDEFINED[];
extern const char ROLEID[];
extern const char PRIVILEGE_TYPES[];
	
}

//viclin add end


/*!
 * @brief Constant literals for EventSubscription endpoint.
 */
namespace EventSubscription {
extern const char DESTINATION[];
extern const char CONTEXT[];
extern const char PROTOCOL[];
extern const char EVENT_TYPES[];
extern const char ID[];
}

/*!
 * @brief Constant literals for capabilities.
 */
namespace Capability {
extern const char CHASSIS[];
extern const char COMPUTE[];
extern const char NETWORK[];
extern const char STORAGE[];
}

/*!
 * @brief Constant literals for TaskService endpoint.
 */
namespace TaskService {
extern const char TASK_SERVICE[];
extern const char SERVICE_ENABLED[];
extern const char DATETIME[];
extern const char COMPLETED_TASK_OVERWRITE_POLICY[];
extern const char LIFE_CYCLE_EVENT_ON_TASK_STATE_CHANGE[];
extern const char TASKS[];
}

/*!
 * @brief Constant literals for Task endpoint.
 * */
namespace Task {
extern const char TASK_STATE[];
extern const char START_TIME[];
extern const char END_TIME[];
extern const char TASK_STATUS[];
extern const char MESSAGES[];
}

namespace Monitor {
extern const char MONITOR[];
}

/*!
 * @brief Constant literals for MessageRegistryFile endpoint
 * */
namespace MessageRegistryFile {
extern const char LANGUAGES[];
extern const char REGISTRY[];
extern const char LOCATION[];
extern const char LANGUAGE[];
extern const char URI[];
extern const char ARCHIVE_URI[];
extern const char PUBLICATION_URI[];
extern const char ARCHIVE_FILE[];
}

/*Nick Added Begin: */
/*!
 * @brief Constant literals for SessionService endpoint.
 */
namespace SessionService {
extern const char SESSION_SERVICE[];
extern const char SERVICE_ENABLED[];
extern const char SERVICE_TIMEOUT[];
extern const char SERVICE_MAXSESSIONS[];
extern const char BASICAUTHEN_ENABLED[];
extern const char SERVICES[];
extern const char ENABLED[];
extern const char PASSWORD[];
extern const char USERNAME[];
extern const char AUTHENTOKEN[]; 
extern const char TIMESTAMP[]; 
}


namespace SerialInterface {
extern const char INTERFACE_ENABLED[] ;
extern const char SIGNAL_TYPE[];
extern const char BIT_RATE[];
extern const char PARITY[];
extern const char DATA_BIT[];
extern const char STOP_BITS[];
extern const char FLOW_CONTROL[];
extern const char CONNECTOR_TYPE[];
extern const char PIN_OUT[] ;
}


namespace UpdateService {
extern const char UPDATE_SERVICE[];
}
/*Nick Added End  : */
}
}
}

