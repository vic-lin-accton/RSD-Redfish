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
 *
 * @file onlp/watcher/onlp_sensor_task.hpp
 * @brief Onlp Sensor task.
 * */

#pragma once
#include <onlp/watcher/task.hpp>

/*! Agent namspace */
namespace agent {
/*! Chassis namspace */
namespace chassis {
/*! onlp namspace */
namespace onlp {
/*! Watcher namspace */
namespace watcher {

/*! Task class for reading onlp sensor data from onlp api */
class OnlpSensorTask final : public Task {
public:
    ~OnlpSensorTask();
    /*! Executes task */
    void execute() override;
    bool m_port_detect_thread = false;
	
};


}
}
}
}


