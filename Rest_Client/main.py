#
# Copyright 2017 the original author or authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
import logging
import logstash
import sys
from rest_client.rest_client import RestClient
if __name__ == '__main__':

    if len(sys.argv) < 2:
        print "Usage:", sys.argv[0], "<vOLT IP>"
        sys.exit(1)

    host = 'localhost'
    logger = logging.getLogger('python-logstash-logger')
    logger.setLevel(logging.DEBUG)
    logger.addHandler(logstash.TCPLogstashHandler(host, 5000, version=1))


    IP= sys.argv[1] + ":8888"
    client = RestClient(IP , None, logger , None)
    client.start_health_monitoring()
