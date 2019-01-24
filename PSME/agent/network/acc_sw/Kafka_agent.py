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

KAFKA_IP_ADDRESS="172.17.8.106"
KAFKA_PORT=32796
TOPIC="voltha.events"


class KafkaAgent(object):

    def __init__(self, msg):
        self._msg = msg
        self._producer = KafkaProducer(bootstrap_servers=KAFKA_IP_ADDRESS + ':' + str(KAFKA_PORT),
                api_version=(0, 10))

    def send(self):
        try:
            self._producer.send(topic=TOPIC, value=self._msg).get(timeout=5)
            self._producer.flush()
        except KafkaError as e:
            print(e)

    def detect_port(self, ip=KAFKA_IP_ADDRESS, port=KAFKA_PORT):
        sock=socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        return sock.connect_ex((ip, port))

def paser_arguments():

    paser = ArgumentParser('send kafa message')
    paser.add_argument('-m', '--msg', required=True, help='add messages')
    return paser.parse_args()


if __name__ == "__main__":
    args = paser_arguments()
    #msg=[{"ENTRY" : { "Created" : "2001-01-01T02:35:39+00:00", "EntryCode" : "Assert", "EntryType" : "Event", "Message" : "Port plug in.", "OemRecordForm    at" : "NULL", "SensorNumber" : 50, "SensorType" : "Port", "Severity" : "OK" }}]
    kafka_agent = KafkaAgent(args.msg)
    port_state = kafka_agent.detect_port(KAFKA_IP_ADDRESS, KAFKA_PORT)
    if port_state == 0:
    #kafka_agent = KafkaAgent(msg)
    #logging.basicConfig(
    #        format='%(asctime)s.%(msecs)s:%(name)s:%(thread)d:' +
    #        '%(levelname)s:%(process)d:%(message)s',
    #        level=logging.INFO)
    kafka_agent.send()
    else:
        print("Kafka server is not available")
