*** Settings ***
Documentation    Test Redfish user account.

Resource         ../../lib/resource.robot
Resource         ../../lib/bmc_redfish_resource.robot
Library          Collections

Test Setup       Test Setup Execution
Test Teardown    Test Teardown Execution

*** Variables ***
${EVENT_ALERT}  { "Name": "This is Alert Subscription event test", "Destination": "https://${LISTENER_HOST}", "Context": "THIS IS TEST FROM AUTO TEST SUBS", "Protocol": "Redfish", "EventTypes": [ "Alert" ] }

${EVENT_RESOURCEADD}  { "Name": "This is ResourceAdded Subscription event test", "Destination": "https://${LISTENER_HOST}", "Context": "THIS IS TEST FROM AUTO TEST SUBS", "Protocol": "Redfish", "EventTypes": [ "ResourceAdded" ] }

${EVENT_RESOURCEREMOVE}  { "Name": "This is ResourceRemoved Subscription event test", "Destination": "https://${LISTENER_HOST}", "Context": "THIS IS TEST FROM AUTO TEST SUBS", "Protocol": "Redfish", "EventTypes": [ "ResourceRemoved" ] }

** Test Cases **

Verify Redfish Add Event
    [Documentation]  Subscribe Event
    [Tags]  Redfish_Add_Event 
    Redfish Add Event 

Verify Redfish Del Event
    [Documentation]  Del Subscribed Event
    [Tags]  Redfish_Del_Event 
    Redfish Del Event 

*** Keywords ***

Test Setup Execution
    [Documentation]  Do test case setup tasks.

    Redfish.Login

Test Teardown Execution
    [Documentation]  Do the post test teardown.

    Redfish.Logout

Redfish Add Event
    [Documentation]  Subscribe Event 

    ${payload}=  Evaluate  json.loads($EVENT_ALERT)    json 
    Redfish.Post  /redfish/v1/EventService/Subscriptions/  body=${payload}
    ...  valid_status_codes=[${HTTP_CREATED}]

    ${payload}=  Evaluate  json.loads($EVENT_RESOURCEADD)    json 
    Redfish.Post  /redfish/v1/EventService/Subscriptions/  body=${payload}
    ...  valid_status_codes=[${HTTP_CREATED}]

    ${payload}=  Evaluate  json.loads($EVENT_RESOURCEREMOVE)    json 
    Redfish.Post  /redfish/v1/EventService/Subscriptions/  body=${payload}
    ...  valid_status_codes=[${HTTP_CREATED}]

Redfish Del Event
    [Documentation]  Del Subscribed Event 

    ${resp_list}=  Redfish_Utils.List Request
    ...  redfish/v1/EventService/Subscriptions/

    Redfish.Delete  ${resp_list[1]} 
    ...  valid_status_codes=[${HTTP_NO_CONTENT}]

    Redfish.Delete  ${resp_list[2]}
    ...  valid_status_codes=[${HTTP_NO_CONTENT}]

    Redfish.Delete  ${resp_list[3]}
    ...  valid_status_codes=[${HTTP_NO_CONTENT}]
