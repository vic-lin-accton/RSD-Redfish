#!/usr/bin/python
#
# Copyright 2018 the original author or authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

import os
from argparse import ArgumentParser
from kafka import KafkaProducer
from kafka.errors import KafkaError
import time
import json
import logging
import socket
import requests
import re
import subprocess
from enum import Enum
import json

REDFISH_IP_ADDRESS="172.17.8.118"
REDFISH_PORT=8888
KAFKA_IP_ADDRESS="172.17.8.106"
KAFKA_PORT=32796
TOPIC="voltha.events"
TIME_OUT=5
PERIOD_TIME=1
HTTP_HEAD='https://'

class RedfishApis(Enum):
    THERMAL=1
    PORT=2
    POWER=3

REDFISH_APIS=dict({RedfishApis.THERMAL.value:'/redfish/v1/Chassis/1/Thermal',
                   RedfishApis.PORT.value:'/redfish/v1/EthernetSwitches/1/Ports',
                   RedfishApis.POWER.value:'/redfish/v1/Chassis/1/Power'})
THERMAL_API=HTTP_HEAD + REDFISH_IP_ADDRESS + ':' + str(REDFISH_PORT) + REDFISH_APIS[RedfishApis.THERMAL.value]
PORT_API=HTTP_HEAD + REDFISH_IP_ADDRESS + ':' + str(REDFISH_PORT) + REDFISH_APIS[RedfishApis.PORT.value]
POWER_API=HTTP_HEAD + REDFISH_IP_ADDRESS + ':' + str(REDFISH_PORT) + REDFISH_APIS[RedfishApis.POWER.value]


class KafkaAgent(object):

    def __init__(self, msg=""):
        self._msg = msg
        self._time_out = TIME_OUT
        self._period_time = PERIOD_TIME
        self._producer = KafkaProducer(bootstrap_servers=KAFKA_IP_ADDRESS + ':' + str(KAFKA_PORT),
                api_version=(0, 10))

    def send(self):
        try:
            self._producer.send(topic=TOPIC, value=self._msg).get(timeout=self._time_out)
            self._producer.flush()
        except KafkaError as e:
            print(e)

    def detect_port(self, ip=KAFKA_IP_ADDRESS, port=KAFKA_PORT):
        sock=socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        return sock.connect_ex((ip, port))


def paser_arguments():

    paser = ArgumentParser('send kafa message')
    paser.add_argument('-t', '--period_time', type=int, default=PERIOD_TIME,
            help='update PM data in period time')
    return paser.parse_args()

def is_json(myjson):
    try:
        json_object = json.loads(myjson)
    except ValueError, e:
        return False
    return True


if __name__ == "__main__":
    args = paser_arguments()
    while True:
        kafka_agent = KafkaAgent()
        port_state = kafka_agent.detect_port(KAFKA_IP_ADDRESS, KAFKA_PORT)
        if port_state == 0:
            time.sleep(args.period_time)
            for index in range(0, 7):
                kafka_agent._msg = subprocess.check_output(['curl -sk ' + THERMAL_API + ' | jq -r .Temperatures[' + str(index) + ']'], shell=True)
                kafka_agent.send()
                time.sleep(2)

            for index in range(0, 8): # Rear Fans: 0~5, PSU Fans: 6~7
                kafka_agent._msg = subprocess.check_output(['curl -sk ' + THERMAL_API + ' | jq -r .Fans[' + str(index) + ']'], shell=True)
                kafka_agent.send()
                time.sleep(2)

            for index in range(1, 21): # PON Ports: 1~16, Up-link Port: 17-20
                port_name = subprocess.check_output(['curl -sk ' + PORT_API + '/' + str(index) + ' | jq -r ".Name"'], shell=True)
                port_statistics = subprocess.check_output(['curl -sk ' + PORT_API + '/' + str(index) + ' | jq -r ".Statistics"'], shell=True)
                port_status = subprocess.check_output(['curl -sk ' + PORT_API + '/' + str(index) + ' | jq -r ".Status"'], shell=True)
                kafka_agent._msg = "Name: " + port_name.strip('\n') + ", "  + port_statistics
                kafka_agent._msg += "Name: " + port_name.strip('\n') + ", "  + port_status 
                kafka_agent._msg = "\"" + port_name.strip("\n") + "\" : " + port_statistics + "\"" + port_name.strip("\n") + "\" : " + port_status 
                kafka_agent.send()
                time.sleep(2)

            for index in range(1, 3): # PSU 1~2
                kafka_agent._msg = subprocess.check_output(['curl -sk ' + POWER_API + '/' + str(index) + ' | jq -r .'], shell=True)
                kafka_agent.send()
                time.sleep(2)
        else:
            print("Kafka server is not available")
            break
