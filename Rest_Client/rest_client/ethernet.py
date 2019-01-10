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

"""
ethernet
"""
import json
from twisted.internet import reactor, task
from constants import UriConst, Defaults


class Ethernet(object):
    def __init__(self, rest_client):
        self.rest_client = rest_client
        self.ethernet_list = []

        if rest_client.psme_version == "calsoft" :
            self.psme_version = "calsoft"
            self.ethernet_get()
        else :
            self.psme_version = "accton"
            self.ethernet_get_()

        self.log = rest_client.log

    def ethernet_get(self, uri=UriConst.ethernet_uri):
        """
        Method to parse Uri for ethernet.
        """
        uri = UriConst.base_uri + uri
        try:
            response = self.rest_client.http_get(uri)
            port_members = json.loads(response.text)['Members']
            response = self.rest_client.http_get(port_members[1]['@odata.id'])
            ethernet_dict = json.loads(response.text)
            self.ethernet_list.append(ethernet_dict['Ports'])

        except Exception as e:
            print("Exception-occurred-:", str(e))

    def ethernet_get_(self, uri=UriConst.ethernet_uri_):
        """
        Method to parse Uri for ethernet.
        """
        uri = UriConst.base_uri + uri
        try:
            response = self.rest_client.http_get(uri)
            for port_members in json.loads(response.text)['Members']:
                response = self.rest_client.http_get(port_members['@odata.id'])
                ethernet_dict = json.loads(response.text)['@odata.id']
                self.ethernet_list.append(ethernet_dict)

        except Exception as e:
            print("Exception-occurred-:", str(e))

    def ports_get(self):
        """
        Method to get port health status.
        """
        for member in self.ethernet_list:
            print "///////////////////////////////////////////////////////////////////"
            try:
                response = self.rest_client.http_get(member['@odata.id'])
                for port in json.loads(response.text)['Members']:
                    response = self.rest_client.http_get(port['@odata.id'])
                    port_dict = json.loads(response.text)
                    if (port_dict['Status']['State'] == 'Enabled'):
                        print "Port         : [" + str(port_dict['Id']) + "] Enabled"
                        print "Bias Current : [" + str(port_dict['BiasCurrent']) + "]"
                        print "Health       : [" + str(port_dict['Status']['Health']) + "]"
                        print ""

                        LOGMSG = "Port         : [" + str(port_dict['Id']) + "] Enabled" + \
                        "Bias Current : [" + str(port_dict['BiasCurrent']) + "]" + \
                        "Health       : [" + str(port_dict['Status']['Health']) + "]"
                        self.log.info(LOGMSG)

                        """
                        self.log.info("Enabled-Port", PortId=port_dict['Id'],
                                      BiasCurrent=port_dict['BiasCurrent'],
                                      Health=port_dict['Status']['Health'])
                        """
                        if (port_dict['Status']['Health'] != 'OK'):
                            """
                            self.rest_client.generate_alarm(
                                status=True,
                                alarm={'Health-check-failed-for-Port':
                                       str(port_dict['Id'])},
                                alarm_severity=port_dict['Status']
                                ['Health'])
                            self.log.info("Raising-alarm-for-Port",
                                          port_dict['Id'])
                            """
                            print("Raising-ARALM-PORT!!!!!")

                            LOGMSG = "Raising-ARALM-PORT!!!!!"
                            self.log.info(LOGMSG)
                    else:
                        """self.log.info("Disabled-Port", PortId=port_dict['Id'])"""
                        print "Port         : [" + str(port_dict['Id']) + "] Disabled"
                        print ""

                        LOGMSG = "Port         : [" + str(port_dict['Id']) + "] Disabled" 
                        self.log.info(LOGMSG)

                print "///////////////////////////////////////////////////////////////////"

            except Exception as e:
                print("Exception-occurred-:", str(e))

    def ports_get_(self):
        """
        Method to get port health status.
        """
        print "///////////////////////////////////////////////////////////////////"
        for member in self.ethernet_list:
            try:
                response = self.rest_client.http_get(member)
                #print("response[%s]"%response.text)
                port_dict = json.loads(response.text)
                if (port_dict['Status']['State'] == 'Enabled'):
                    print "Port         : [" + str(port_dict['Id']) + "] Enabled"
                    print ""

                    LOGMSG = "Port         : [" + str(port_dict['Id']) + "] Enabled" + \
                    "Bias Current : [" + str(port_dict['BiasCurrent']) + "]" + \
                    "Health       : [" + str(port_dict['Status']['Health']) + "]"
                    self.log.info(LOGMSG)

                    self.log.info("Enabled-Port", PortId=port_dict['Id'],
                                  BiasCurrent=port_dict['BiasCurrent'],
                                  Health=port_dict['Status']['Health'])
                    if (port_dict['Status']['Health'] != 'OK'):
                        print("Raising-ARALM-PORT!!!!!")

                        LOGMSG = "Raising-ARALM-PORT!!!!!"
                        self.log.info(LOGMSG)
                else:
                    print "Port         : [" + str(port_dict['Id']) + "] Disabled"
                    print ""

                    #LOGMSG = "Port         : [" + str(port_dict['Id']) + "] Disabled" 
                    #self.log.info(LOGMSG)

            except Exception as e:
                print("Exception-occurred-:", str(e))
        print "///////////////////////////////////////////////////////////////////"

    def get_ethernet_health(self):

        if self.psme_version == "calsoft" :
            self.ports_get()
        else :
            self.ports_get_()

            self.health_thread = reactor.callLater(
            Defaults.health_check_interval,
            self.get_ethernet_health)

    def stop_ether_monitoring(self):
        try:
            h, self.health_thread = self.health_thread, None
            if(h is not None):
                h.cancel()
        except Exception as e:
            print("Exception-occured-:", str(e))
