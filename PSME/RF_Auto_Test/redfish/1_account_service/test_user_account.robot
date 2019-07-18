*** Settings ***
Documentation    Test Redfish user account.

Resource         ../../lib/resource.robot
Resource         ../../lib/bmc_redfish_resource.robot

Test Setup       Test Setup Execution
Test Teardown    Test Teardown Execution

** Test Cases **

Verify AccountService Available
    [Documentation]  Verify Redfish account service is available.
    [Tags]  Verify_AccountService_Available

    ${resp} =  Redfish_utils.Get Attribute  /redfish/v1/AccountService  ServiceEnabled
    Should Be Equal As Strings  ${resp}  ${True}

Redfish Create and Verify Users
    [Documentation]  Create Redfish users with various roles.
    [Tags]  Redfish_Create_and_Verify_Users
    [Template]  Redfish Create And Verify User

    # username     password    role_id             locked     enabled
    admin_user     TestPwd123  Administrator       ${False}   ${True}
    admin_user_1   TestPwd123  ReadOnlyUser        ${False}   ${True}
    operator_user_2  TestPwd123  Operator          ${False}   ${True}


Verify Redfish User with Wrong Password
    [Documentation]  Verify Redfish User with Wrong Password.
    [Tags]  Verify_Redfish_User_with_Wrong_Password
    [Template]  Verify Redfish User with Wrong Password

    # username     password    role_id  locked   enabled   wrong_password
    admin_user_3   TestPwd123  Administrator    ${False}   ${True}  aa 
    admin_user_4   TestPwd123  ReadOnlyUser     ${False}   ${True}  bb
    operator_user_5  TestPwd123  Operator       ${False}   ${True}  cc

Verify Login with Deleted Redfish Users
    [Documentation]  Verify login with deleted Redfish Users.
    [Tags]  Verify_Login_with_Deleted_Redfish_Users
    [Template]  Verify Login with Deleted Redfish User

    # username     password    role_id  locked  enabled    wrong_password
    admin_user_6   TestPwd123  Administrator    ${False}   ${True}  aa 
    admin_user_7   TestPwd123  ReadOnlyUser     ${False}   ${True}  dd
    operator_user_8  TestPwd123  Operator       ${False}   ${True}  ee

Verify User Creation Without Enabling it
    [Documentation]  Verify User Creation Without Enabling it.
    [Tags]  Verify_User_Creation_Without_Enabling_it
    [Template]  Redfish Create And Verify User

    # username        password    role_id             locked     enabled
    admin_user_9      TestPwd123  Administrator       ${False}   ${False}
    admin_user_10     TestPwd123  ReadOnlyUser        ${False}   ${False}
    operator_user_11  TestPwd123  Operator            ${False}   ${False}



Verify Redfish User Persistence After Reboot
    [Documentation]  Verify Redfish user persistence after reboot.
    [Tags]  Verify_Redfish_User_Persistence_After_Reboot
    # Create Redfish users.

    Redfish Create User  admin_user_12     TestPwd123  Administrator       ${False}   ${True}
    Redfish Create User  admin_user_13     TestPwd123  ReadOnlyUser        ${False}   ${True}
    Redfish Create User  operator_user_14  TestPwd123  Operator            ${False}   ${True}

    # Reboot Device 
    Redfish GracefulRestart

    # Wait device ready
    Sleep  180s

    # Verify users after reboot.
    Redfish Verify User  admin_user_12     TestPwd123  Administrator  ${False}  ${True}
    Redfish Verify User  admin_user_13     TestPwd123  ReadOnlyUser   ${False}  ${True}
    Redfish Verify User  operator_user_14  TestPwd123  Operator       ${False}  ${True}

    # Delete created users.
    Redfish.Delete  /redfish/v1/AccountService/Accounts/admin_user_12
    Redfish.Delete  /redfish/v1/AccountService/Accounts/admin_user_13
    Redfish.Delete  /redfish/v1/AccountService/Accounts/operator_user_14


Verify User Creation With locking it
    [Documentation]  Verify User Creation With locking it.
    [Tags]  Verify_User_Creation_With_locking_it
    [Template]  Redfish Create And Verify User With Locked

    # username        password    role_id             locked     enabled
    admin_user_15     TestPwd123  Administrator       ${True}   ${True}
    admin_user_16     TestPwd123  ReadOnlyUser        ${True}   ${True}
    operator_user_17  TestPwd123  Operator            ${True}   ${True}


*** Keywords ***

Test Setup Execution
    [Documentation]  Do test case setup tasks.
    Redfish.Login

Test Teardown Execution
    [Documentation]  Do the post test teardown.
    Redfish.Logout


Redfish GracefulRestart 
    [Documentation]  Redfish GracefulRestart 

    # Create specified user.
    ${payload}=  Create Dictionary
    ...  ResetType=GracefulRestart 
    Redfish.Post  /redfish/v1/Systems/1/Actions/ComputerSystem.Reset  body=&{payload}
    Sleep  1s
    ...  valid_status_codes=[${HTTP_OK}]


Redfish Enable SessionService 
    [Documentation]  Redfish enable sesssion service 
    [Arguments]   ${enabled}

    # Create specified user.
    ${payload}=  Create Dictionary
    ...  ServiceEnabled=${enabled}  SessionTimeout=${600}
    Redfish.Post  /redfish/v1/SessionService/  body=&{payload}
    ...  valid_status_codes=[${HTTP_OK}]

Redfish Create User
    [Documentation]  Redfish create user.
    [Arguments]   ${username}  ${password}  ${role_id}  ${locked}  ${enabled}

    # Description of argument(s):
    # username            The username to be created.
    # password            The password to be assigned.
    # role_id             The role id of the user to be created
    #                     (e.g. "Administrator", "Operator", etc.).
    # locked              should be enabled (${True}, ${False}).
    # enabled             Indicates whether the username being created
    #                     should be enabled (${True}, ${False}).

    # Create specified user.
    ${payload}=  Create Dictionary
    ...  UserName=${username}  Password=${password}  RoleId=${role_id}  Locked=${locked}  Enabled=${enabled}
    Redfish.Post  /redfish/v1/AccountService/Accounts  body=&{payload}
    ...  valid_status_codes=[${HTTP_CREATED}]


Redfish Verify User
    [Documentation]  Redfish user verification.
    [Arguments]   ${username}  ${password}  ${role_id}  ${locked}  ${enabled}

    # Description of argument(s):
    # username            The username to be created.
    # password            The password to be assigned.
    # role_id             The role id of the user to be created
    # locked              should be enabled (${True}, ${False}).
    #                     (e.g. "Administrator", "Operator", etc.).

    # Trying todo a login with created user
    ${is_redfish}=  Run Keyword And Return Status  Redfish.Login  ${username}  ${password}

    # Doing a check of the rerurned status
    Should Be Equal  ${is_redfish}  ${enabled}

    # We do not needed to login with created user (user could be in disabled status)
     Redfish.Login

    # Validate Role Id of created user.
    ${role_config}=  Redfish_Utils.Get Attribute
    ...  /redfish/v1/AccountService/Accounts/${username}  RoleId
    Should Be Equal  ${role_id}  ${role_config}


Redfish Verify User With Locked
    [Documentation]  Redfish user verification with locked.
    [Arguments]   ${username}  ${password}  ${role_id}  ${locked}  ${enabled}

    # Description of argument(s):
    # username            The username to be created.
    # password            The password to be assigned.
    # role_id             The role id of the user to be created
    # locked              should be enabled (${True}, ${False}).
    #                     (e.g. "Administrator", "Operator", etc.).

    # Trying todo a login with created user
    ${is_redfish}=  Run Keyword And Return Status  Redfish.Login  ${username}  ${password}

    # Doing a check of the rerurned status
    Should Be Equal  ${is_redfish}  ${False}


Redfish Create And Verify User
    [Documentation]  Redfish create and verify user.
    [Arguments]   ${username}  ${password}  ${role_id}  ${locked}  ${enabled}

    # Description of argument(s):
    # username            The username to be created.
    # password            The password to be assigned.
    # role_id             The role id of the user to be created
    #                     (e.g. "Administrator", "Operator", etc.).
    # locked              should be enabled (${True}, ${False}).
    # enabled             Indicates whether the username being created
    #                     should be enabled (${True}, ${False}).

    Redfish Create User  ${username}  ${password}  ${role_id}  ${locked}  ${enabled}

    Redfish Verify User  ${username}  ${password}  ${role_id}  ${locked}  ${enabled}

    # Delete Specified User
    Redfish.Delete  /redfish/v1/AccountService/Accounts/${username}


Redfish Create And Verify User With Locked
    [Documentation]  Redfish create and verify user with locked.
    [Arguments]   ${username}  ${password}  ${role_id}  ${locked}  ${enabled}

    # Description of argument(s):
    # username            The username to be created.
    # password            The password to be assigned.
    # role_id             The role id of the user to be created
    #                     (e.g. "Administrator", "Operator", etc.).
    # locked              should be enabled (${True}, ${False}).
    # enabled             Indicates whether the username being created
    #                     should be enabled (${True}, ${False}).

    Redfish Create User  ${username}  ${password}  ${role_id}  ${locked}  ${enabled}

    Redfish Verify User With Locked  ${username}  ${password}  ${role_id}  ${locked}  ${enabled}

    # Delete Specified User
    Redfish.Delete  /redfish/v1/AccountService/Accounts/${username}


Verify Redfish User with Wrong Password
    [Documentation]  Verify Redfish User with Wrong Password.
    [Arguments]   ${username}  ${password}  ${role_id}  ${locked}  ${enabled}  ${wrong_password}

    # Description of argument(s):
    # username            The username to be created.
    # password            The password to be assigned.
    # role_id             The role id of the user to be created
    #                     (e.g. "Administrator", "Operator", etc.).
    # locked              should be enabled (${True}, ${False}).
    # enabled             Indicates whether the username being created
    #                     should be enabled (${True}, ${False}).
    # wrong_password      Any invalid password.

    Redfish Create User  ${username}  ${password}  ${role_id}  ${locked}  ${enabled}

    # Attempt to login with created user with invalid password.
    ${msg}=  Run Keyword And Expect Error  *
    ...  Redfish.Login  ${username}  ${wrong_password}

    #Redfish.Login

    # Delete newly created user.
    Redfish.Delete  /redfish/v1/AccountService/Accounts/${username}


Verify Login with Deleted Redfish User
    [Documentation]  Verify Login with Deleted Redfish User.
    [Arguments]   ${username}  ${password}  ${role_id}  ${locked}  ${enabled}  ${wrong_password}

    # Description of argument(s):
    # username            The username to be created.
    # password            The password to be assigned.
    # role_id             The role id of the user to be created
    #                     (e.g. "Administrator", "Operator", etc.).
    # locked              should be enabled (${True}, ${False}).
    # enabled             Indicates whether the username being created
    #                     should be enabled (${True}, ${False}).

    Redfish Create User  ${username}  ${password}  ${role_id}  ${locked}  ${enabled}

    # Delete newly created user.
    Redfish.Delete  /redfish/v1/AccountService/Accounts/${username}

    ${is_redfish}=  Run Keyword And Return Status  Redfish.Login  ${username}  ${password}

    # Doing a check of the rerurned status
    Should Be Equal  ${is_redfish}  ${False}

    Redfish.Login
