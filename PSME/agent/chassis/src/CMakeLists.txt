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
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(onlp)
add_subdirectory(command)
add_subdirectory(loader)
add_subdirectory(tree_stability)

add_library(chassis-commands STATIC
    $<TARGET_OBJECTS:chassis-command-acc>
)

add_library(chassis-utils
    $<TARGET_OBJECTS:chassis-loader>
    $<TARGET_OBJECTS:chassis-tree-stability>
)

add_library(chassis-onlp
    $<TARGET_OBJECTS:onlp-service>
)

add_executable(psme-chassis
    main.cpp
)

TARGET_LINK_LIBRARIES(psme-chassis
    ${AGENT_CHASSIS_LIBRARIES}
    chassis-utils
    chassis-onlp
    ${AGENT_FRAMEWORK_LIBRARIES}
    ${UUID_LIBRARIES}
    ${LOGGER_LIBRARIES}
    ${SAFESTRING_LIBRARIES}
    ${CONFIGURATION_LIBRARIES}
    ${JSONCXX_LIBRARIES}
    ${BASE64_LIBRARIES}
    pthread
    jsonrpccpp-server
    jsonrpccpp-client
    jsonrpccpp-common
    jsoncpp
    ${MICROHTTPD_LIBRARIES}
    ${CURL_LIBRARIES}
    ${SAFESTRING_LIBRARIES}
    ${ACC_NET_HELPER_LIBRARIES}
    ${ACC_ONLP_HELPER_LIBRARIES}
    ${LIBONLP}
    ${LIBONLP_PF}
    ${LIBONLP_PF_DF}
    libm.so
    md5
    )


IF("${CUSE_ACC_BAL3_DISTLIB}" STREQUAL "TRUE" )
    target_link_libraries(psme-chassis
	${ACC_BAL3_API_DIST_HELPER_LIBRARIES}
	#        ${LIBBAL3_API_DIST}
        dl	
	)
ENDIF()
