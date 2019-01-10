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

kafka_IP_address="172.17.8.102"
kafka_Port="32769"
TOPIC="voltha.events"


class KafkaAgent(object):

    def __init__(self, msg):
        self._msg = msg
        self._producer = KafkaProducer(bootstrap_servers=kafka_IP_address + ':' + kafka_Port, api_version=(0, 10))
        #value_serializer=lambda m: json.dumps(m).encode('utf-8'), api_version=(0, 10)

    def send(self):
        try:
            #msg = json.dumps(self._msg, ensure_ascii=False).encode('utf-8')
            #producer.send(topic=TOPIC, value=msg).get(timeout=5)
            self._producer.send(topic=TOPIC, value=self._msg).get(timeout=5)
            self._producer.flush()
        except KafkaError as e:
            print(e)

def paser_arguments():

    paser = ArgumentParser('send kafa message')
    paser.add_argument('-m', '--msg', required=True, help='add messages')
    return paser.parse_args()


if __name__ == "__main__":
    args = paser_arguments()
    #msg=[{"ENTRY" : { "Created" : "2001-01-01T02:35:39+00:00", "EntryCode" : "Assert", "EntryType" : "Event", "Message" : "Port plug in.", "OemRecordForm    at" : "NULL", "SensorNumber" : 50, "SensorType" : "Port", "Severity" : "OK" }}]
    kafka_agent = KafkaAgent(args.msg)
    #kafka_agent = KafkaAgent(msg)
    #logging.basicConfig(
    #        format='%(asctime)s.%(msecs)s:%(name)s:%(thread)d:' +
    #        '%(levelname)s:%(process)d:%(message)s',
    #        level=logging.INFO)
    kafka_agent.send()
