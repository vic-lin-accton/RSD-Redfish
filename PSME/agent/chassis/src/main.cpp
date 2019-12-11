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

#include "agent-framework/registration/amc_connection_manager.hpp"
#include "agent-framework/signal.hpp"
#include "agent-framework/version.hpp"
#include "agent-framework/state_machine/state_machine_thread.hpp"
#include "agent-framework/module/compute_components.hpp"
#include "agent-framework/module/common_components.hpp"
#include "agent-framework/module/chassis_components.hpp"
#include "agent-framework/eventing/event_data.hpp"
#include "agent-framework/eventing/events_queue.hpp"
#include "agent-framework/command-ref/command_server.hpp"

#include "loader/chassis_loader.hpp"

#include "configuration/configuration.hpp"
#include "configuration/configuration_validator.hpp"
#include "default_configuration.hpp"
#include "acc_net_helper/acc_net_helper.hpp"

#include "onlp/watcher/onlp_sensor_task.hpp"
#include "onlp/watcher/watcher.hpp"

#include <jsonrpccpp/server/connectors/httpserver.h>
#include <csignal>
#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef ONLP
#include "acc_onlp_helper/acc_onlp_helper.hpp"
using namespace acc_onlp_helper;
#endif

#ifdef VOLT
#include "acc_bal_api_dist_helper/acc_bal_api_dist_helper.hpp"
using namespace acc_bal_api_dist_helper;
#endif

using namespace std;
using namespace agent_framework;
using namespace agent_framework::generic;
using namespace agent_framework::state_machine;
using namespace logger_cpp;
using namespace configuration;
using namespace agent::chassis;
using namespace agent::chassis::onlp;
using namespace agent_framework::eventing;

using agent::generic::DEFAULT_CONFIGURATION;
using agent::generic::DEFAULT_ENV_FILE;
using agent::generic::DEFAULT_FILE;
using agent::generic::DEFAULT_VALIDATOR_JSON;

using agent_framework::module::ChassisComponents;
using agent_framework::module::CommonComponents;

static constexpr unsigned int DEFAULT_SERVER_PORT = 7780;

const json::Value &init_configuration(int argc, const char **argv);
bool check_configuration(const json::Value &json);

void seg_fault_handler(int sig);

void seg_fault_handler(int sig)
{
    void *array[10];
    int size;

    // get void*'s for all entries on the stack
    size = backtrace(array, 10);

    // print out all the frames to stderr
    fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, (int)STDERR_FILENO);
    return;
}

/*!
 * @brief Generic Agent main method.
 * */
int main(int argc, const char *argv[])
{

    unsigned int server_port = DEFAULT_SERVER_PORT;

    /* Initialize configuration */
    const json::Value &configuration = ::init_configuration(argc, argv);
    if (!::check_configuration(configuration))
    {
        return -1;
    }

    /* Initialize logger */
    LoggerLoader loader(configuration);
    LoggerFactory::instance().set_loggers(loader.load());
    LoggerFactory::set_main_logger_name("agent");

    log_info(GET_LOGGER("agent"), "Running PSME Chassis in Accton SW...\n");

    ::agent::chassis::loader::ChassisLoader module_loader{};
    if (!module_loader.load(configuration))
    {
        std::cerr << "Invalid modules configuration" << std::endl;
        return -2;
    }

    EventDispatcher event_dispatcher;
    event_dispatcher.start();

    AmcConnectionManager amc_connection(event_dispatcher);
    amc_connection.start();

    /* Restart ma1 to let dhcpv6 normal */
    InterfaceConfig intf_ip;
    intf_ip.Restart();

    /* Initialize command server */
    signal(SIGSEGV, seg_fault_handler); // install our handler

    //int *foo = (int*)-1; // make a bad pointer
    //printf("%d\n", *foo);       // causes segfault

    jsonrpc::HttpServer http_server((int(server_port)));
    agent_framework::command_ref::CommandServer server(http_server);
    server.add(command_ref::Registry::get_instance()->get_commands());
    server.start();

    watcher::Watcher onlp_watcher{};
    onlp_watcher.add_task(std::make_shared<watcher::OnlpSensorTask>());
    onlp_watcher.start();

    ::agent::chassis::loader::ChassisLoader::wait_for_complete();

    /* Stop the program and wait for interrupt */
    wait_for_interrupt();

    /* Cleanup */
    onlp_watcher.stop();

    server.stop();
    amc_connection.stop();
    event_dispatcher.stop();
    Configuration::cleanup();
    LoggerFactory::cleanup();
#ifdef ONLP
    Switch::cleanup();
#endif

#ifdef VOLT
    Olt_Device::cleanup();
#endif
    return 0;
}

const json::Value &init_configuration(int argc, const char **argv)
{
    log_info(GET_LOGGER("agent"),
             agent_framework::generic::Version::build_info());
    auto &basic_config = Configuration::get_instance();
    basic_config.set_default_configuration(DEFAULT_CONFIGURATION);
    basic_config.set_default_file(DEFAULT_FILE);
    basic_config.set_default_env_file(DEFAULT_ENV_FILE);
    /* @TODO This should be replaced with nice arguments parsing class */
    while (argc > 1)
    {
        basic_config.add_file(argv[argc - 1]);
        --argc;
    }
    basic_config.load_key_file();
    return basic_config.to_json();
}

bool check_configuration(const json::Value &json)
{
    json::Value json_schema;
    if (configuration::string_to_json(DEFAULT_VALIDATOR_JSON, json_schema))
    {
        log_info(GET_LOGGER("agent"), "Loading configuration schema!");

        configuration::SchemaErrors errors;
        configuration::SchemaValidator validator;
        configuration::SchemaReader reader;
        reader.load_schema(json_schema, validator);

        validator.validate(json, errors);
        if (!errors.is_valid())
        {
            std::cerr << "Configuration invalid: " << errors.to_string() << std::endl;
            return false;
        }
    }
    return true;
}
