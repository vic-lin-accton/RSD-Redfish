# <license_header>
#
# Copyright (c) 2015-2017 Intel Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# </license_header>

cmake_minimum_required(VERSION 2.8)
project(acc_bal3_api_dist_helper CXX)

set(ACC_BAL3_API_DIST_HELPER_LIBRARIES
    acc_bal3_api_dist_helper 
    CACHE PATH "acc_bal3_api_dist_helper libraries"
)

set(ACC_BAL3_API_DIST_HELPER_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    CACHE PATH "acc_bal3_api_dist_helper include directory"
)
#include current include path
include_directories(
    include
)

include_directories(SYSTEM
    ${SAFESTRING_INCLUDE_DIRS}
    ${UUID_INCLUDE_DIRS}
    ${MD5_INCLUDE_DIRS}
    ${JSONCPP_INCLUDE_DIRS}
    ${JSONRPCCPP_INCLUDE_DIRS}
    ${ACC_BAL3_API_DIST_HELPER_INCLUDE_DIRS}
)

#include bal3 api include  path
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../asfvolt16-bal/host_driver/bal/bal_include/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../asfvolt16-bal/host_driver/topology/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../asfvolt16-bal/host_driver/utils/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../asfvolt16-bal/host_driver/api/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../asfvolt16-bal/host_driver/system_types/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../asfvolt16-bal/host_driver/api_conn_mgr/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../asfvolt16-bal/host_driver/conn_mgr/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../asfvolt16-bal/host_driver/api/host/topology/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../asfvolt16-bal/host_driver/transport/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../asfvolt16-bal/host_customized/os_abstraction/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../asfvolt16-bal/host_customized/os_abstraction/posix/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../asfvolt16-bal/host_customized/config/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../asfvolt16-bal/host_reference/cli/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../asfvolt16-bal/host_reference/api_cli/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../asfvolt16-bal/host_reference/dev_log/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../asfvolt16-bal/host_reference/host_api/

    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../asgvolt64-bal/host_driver/bal/bal_include/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../asgvolt64-bal/host_driver/topology/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../asgvolt64-bal/host_driver/utils/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../asgvolt64-bal/host_driver/api/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../asgvolt64-bal/host_driver/system_types/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../asgvolt64-bal/host_driver/api_conn_mgr/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../asgvolt64-bal/host_driver/conn_mgr/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../asgvolt64-bal/host_driver/api/host/topology/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../asgvolt64-bal/host_driver/transport/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../asgvolt64-bal/host_customized/os_abstraction/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../asgvolt64-bal/host_customized/os_abstraction/posix/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../asgvolt64-bal/host_customized/config/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../asgvolt64-bal/host_reference/cli/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../asgvolt64-bal/host_reference/api_cli/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../asgvolt64-bal/host_reference/dev_log/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../asgvolt64-bal/host_reference/host_api/
 
    ${CMAKE_CURRENT_SOURCE_DIR}/../../common/json-cxx/include/
)


add_library(acc_bal3_api_dist_helper STATIC
    src/acc_bal3_api_dist_helper.cpp
)

if (CMAKE_CXX_COMPILER_ID MATCHES GNU|Clang)
  set_source_files_properties(
    src/acc_bal3_api_dist_helper.cpp
    PROPERTIES COMPILE_FLAGS "-Wno-conversion"
    )
endif()

install(TARGETS acc_bal3_api_dist_helper 
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

