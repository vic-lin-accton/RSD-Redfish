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

add_gtest(acc_api_bal30_dist_test psme-chassis
    acc_api_bal30_dist_test.cpp
    test_runner.cpp
)

include_directories(
    ${ACC_BAL30_API_DIST_HELPER_INCLUDE_DIRS}
    ${JSONCPP_INCLUDE_DIRS}
)

TARGET_LINK_LIBRARIES(${test_target}
    ${ACC_BAL30_API_DIST_HELPER_LIBRARIES}
    ${JSONCXX_LIBRARIES}
    ${LIBBAL30_API_DIST}
    jsoncpp
    dl 
)
