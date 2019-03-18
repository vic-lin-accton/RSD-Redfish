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

if (NOT GTEST_FOUND)
    return()
endif()

add_gtest(onlp psme-chassis
    onlp_test.cpp
    test_runner.cpp
)

include_directories(
    ${ACC_NET_HELPER_INCLUDE_DIRS}
    ${ACC_ONLP_HELPER_INCLUDE_DIRS}
)

TARGET_LINK_LIBRARIES(${test_target}
    ${ACC_ONLP_HELPER_LIBRARIES}
    ${ACC_NET_HELPER_LIBRARIES}
    ${ACC_BAL_API_DIST_HELPER_LIBRARIES}
    ${LIBONLP}
    ${LIBONLP_PF}
    ${LIBONLP_PF_DF}
    libm.so
    jsoncpp
    json-cxx
    )
