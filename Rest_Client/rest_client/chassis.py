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
chassis
"""
import json
from twisted.internet import reactor, task
from constants import UriConst, Defaults


class Chassis(object):
    def __init__(self, rest_client):
        self.rest_client = rest_client
        # Get PSME version #
        m_uri = UriConst.base_uri + UriConst.managers_uri
        response = self.rest_client.http_get(UriConst.base_uri + UriConst.managers_uri)
        psme_version = json.loads(response.text)['FirmwareVersion']

        if psme_version == "2.1.3.59.1" :
            print("calsoft")
            self.rest_client.psme_version="calsoft"
        else :
            print("accton")
            self.rest_client.psme_version="accton"

        self.chassis_list = []
        self.chassis_get()
        self.log = rest_client.log

    def chassis_get(self, uri=UriConst.chassis_uri):
        """
        Method to parse Uri for fans and psu.
        """
        uri = UriConst.base_uri + uri
        try:
            response = self.rest_client.http_get(uri)
            for member in json.loads(response.text)['Members']:
                response = self.rest_client.http_get(member['@odata.id'])
                chassis_dict = json.loads(response.text)
                if (chassis_dict['Status']['State'] == 'Enabled'):
                    self.chassis_list.append(chassis_dict)
        except Exception as e:
            print("Exception-occurred-:", str(e))

    def fans_get_(self):
        """
        Method to get fan health status.
        """

        for chassis in self.chassis_list:
            thermal_uri = chassis['Thermal']['@odata.id']
            try:
                response = self.rest_client.http_get(thermal_uri)
                fans_list = json.loads(response.text)['Fans']
                print "///////////////////////////////////////////////////////////////////"
                for fan_dict in fans_list:
                    print "Name   : [" + str(fan_dict['Name']) + "]"
                    print "Fan    : [" + str(fan_dict['MemberId']) + "]"
                    print "RPM    : [" + str(fan_dict['Reading']) + "]"
                    print "State  : [" + str(fan_dict['Status']['State']) + "]"
                    print "Health : [" + str(fan_dict['Status']['Health']) + "]"

                    if  str(fan_dict['Reading']) == "None":
                        LOGMSG = "{\"MemberId\":" + str(fan_dict['MemberId']) +"," + \
                        "\"Reading\":" + "0" + "," + \
                        "\"State\":\"" + str(fan_dict['Status']['State']) + "\"}"
                    else :
                        LOGMSG = "{\"MemberId\":" + str(fan_dict['MemberId']) +"," + \
                        "\"Reading\":" + str(fan_dict['Reading']) + "," + \
                        "\"State\":\"" + str(fan_dict['Status']['State']) + "\"}"

                    self.log.info(LOGMSG)

                    if (fan_dict['Status']['Health'] != 'OK'):
                        # Raise Alarm
                        print "ALARM !!!!! HEALTH !!! NOT !!!! OK"
                        LOGMSG = "{\"FAN MemberId\":" + str(fan_dict['MemberId']) +"," + \
                        "\"Health\":\"" + "Alarm" + "\"}"
                        self.log.info(LOGMSG)

                    print ""
            except Exception as e:
                print("Exception-occurred-:", str(e))

    def temp_get_(self):
        """
        Method to get temp health status.
        """

        for chassis in self.chassis_list:
            thermal_uri = chassis['Thermal']['@odata.id']
            try:
                response = self.rest_client.http_get(thermal_uri)
                temps_list = json.loads(response.text)['Temperatures']
                print "///////////////////////////////////////////////////////////////////"
                for temp_dict in temps_list:
                    print "Name     : [" + str(temp_dict['Name']) + "]"
                    print "SensorID : [" + str(temp_dict['MemberId']) + "]"
                    print "Temp.    : [" + str(temp_dict['ReadingCelsius']) + "]"
                    print "Sate     : [" + str(temp_dict['Status']['State']) + "]"
                    print "Health   : [" + str(temp_dict['Status']['Health']) + "]"

                    if str(temp_dict['ReadingCelsius']) == "None":
                        LOGMSG = "{\"MemberId\":" + str(temp_dict['MemberId']) +"," + \
                        "\"ReadingCelsius\":" + "0" + "," + \
                        "\"State\":\"" + str(temp_dict['Status']['State']) + "\"}"
                    else:
                        LOGMSG = "{\"MemberId\":" + str(temp_dict['MemberId']) +"," + \
                        "\"ReadingCelsius\":" + str(temp_dict['ReadingCelsius']) + "," + \
                        "\"State\":\"" + str(temp_dict['Status']['State']) + "\"}"

                    self.log.info(LOGMSG)

                    if (temp_dict['Status']['Health'] != 'OK'):
                        # Raise Alarm
                        print "ALARM !!!!! HEALTH !!! NOT !!!! OK"
                        LOGMSG = "{\"Temp MemberId\":" + str(temp_dict['MemberId']) +"," + \
                        "\"Health\":\"" + "Alarm" + "\"}"
                        self.log.info(LOGMSG)


                    print ""
            except Exception as e:
                print("Exception-occurred-:", str(e))

    def fans_get(self):
        """
        Method to get fan health status.
        """

        for chassis in self.chassis_list:
            thermal_uri = chassis['Links']['Thermal'][0]['@odata.id']
            try:
                response = self.rest_client.http_get(thermal_uri)
                fans_list = json.loads(response.text)['Fans']
                print "///////////////////////////////////////////////////////////////////"
                for fan in fans_list:
                    response = self.rest_client.http_get(fan['@odata.id'])
                    fan_dict = json.loads(response.text)['Fan']

                    print "Fan    : [" + str(fan_dict['Id']) + "]"
                    print "RPM    : [" + str(fan_dict['Reading']) + "]"
                    print "State  : [" + str(fan_dict['Status']['State']) + "]"
                    print "Health : [" + str(fan_dict['Status']['Health']) + "]"

                    if  str(fan_dict['Reading']) == "None":
                        LOGMSG = "{\"MemberId\":" + str(fan_dict['Id']) +"," + \
                        "\"Reading\":" + "0" + "," + \
                        "\"State\":\"" + str(fan_dict['Status']['State']) + "\"}"
                    else :
                        LOGMSG = "{\"MemberId\":" + str(fan_dict['Id']) +"," + \
                        "\"Reading\":" + str(fan_dict['Reading']) + "," + \
                        "\"State\":\"" + str(fan_dict['Status']['State']) + "\"}"

                    self.log.info(LOGMSG)

                    if (fan_dict['Status']['Health'] != 'OK'):
                        # Raise Alarm
                        print "ALARM !!!!! HEALTH !!! NOT !!!! OK"
                        LOGMSG = "{\"Fan MemberId\":" + str(fan_dict['Id']) +"," + \
                        "\"Health\":\"" + "Alarm" + "\"}"
                        self.log.info(LOGMSG)

                    print ""

            except Exception as e:
                print("Exception-occurred-:", str(e))

    def temp_get(self):
        """
        Method to get temp status.
                        """

        for chassis in self.chassis_list:
            thermal_uri = chassis['Links']['Thermal'][0]['@odata.id']
            try:
                response = self.rest_client.http_get(thermal_uri)
                temps_list = json.loads(response.text)['Temperatures']
                print "///////////////////////////////////////////////////////////////////"
                for temp in temps_list:
                    response = self.rest_client.http_get(temp['@odata.id'])
                    fan_dict = json.loads(response.text)['Temperature']

                    print "SensorID : [" + str(fan_dict['MemberId']) + "]"
                    print "Temp.    : [" + str(fan_dict['ReadingCelsius']) + "]"
                    print "Sate     : [" + str(fan_dict['Status']['State']) + "]"
                    print "Health   : [" + str(fan_dict['Status']['Health']) + "]"

                    if str(fan_dict['ReadingCelsius']) == "None":
                        LOGMSG = "{\"MemberId\":" + str(fan_dict['MemberId']) +"," + \
                        "\"ReadingCelsius\":" + "0" + "," + \
                        "\"State\":\"" + str(fan_dict['Status']['State']) + "\"}"
                    else:
                        LOGMSG = "{\"MemberId\":" + str(fan_dict['MemberId']) +"," + \
                        "\"ReadingCelsius\":" + str(fan_dict['ReadingCelsius']) + "," + \
                        "\"State\":\"" + str(fan_dict['Status']['State']) + "\"}"


                    self.log.info(LOGMSG)

                    if (fan_dict['Status']['Health'] != 'OK'):
                        # Raise Alarm
                        print "ALARM !!!!! HEALTH !!! NOT !!!! OK"
                        LOGMSG = "{\"Temp MemberId\":" + str(fan_dict['MemberId']) +"," + \
                        "\"Health\":\"" + "Alarm" + "\"}"
                        self.log.info(LOGMSG)



                    print ""
            except Exception as e:
                print("Exception-occurred-:", str(e))


    def psu_get_(self):
        """
        Method to get psu health status.
        """
        for chassis in self.chassis_list:
            power_uri = chassis['Power']['@odata.id']
            try:
                response = self.rest_client.http_get(power_uri)
                psu_list = json.loads(response.text)['PowerControl']

                print "///////////////////////////////////////////////////////////////////"
                for psu_dict in psu_list:
                    print ""
                    print "Power-Supply        : [" + str(psu_dict['MemberId']) + "]"
                    print "PowerConsumedWatts  : [" + str(psu_dict['PowerConsumedWatts']) + "] Watts"
                    print "State               : [" + str(psu_dict['Status']['State']) + "]"
                    print "Health              : [" + str(psu_dict['Status']['Health']) + "]"
                    print ""

                    LOGMSG = "{\"MemberId\":" + str(psu_dict['MemberId']) +"," + \
                    "\"PowerConsumedWatts\":" + str(psu_dict['PowerConsumedWatts']) + "," + \
                    "\"State\":\"" + str(psu_dict['Status']['State']) + "\"}"

                    self.log.info(LOGMSG)
                    if (psu_dict['Status']['Health'] != 'OK'):
                        # Raise Alarm
                        print "Power-Supply: [" + str(psu_dict['MemberId']) + "]  ALARM !!!!! HEALTH !!! NOT !!!! OK"
                        LOGMSG = "{\"Power MemberId\":" + str(psu_dict['MemberId']) +"," + \
                        "\"Health\":\"" + "Alarm" + "\"}"
                        self.log.info(LOGMSG)

                        print ""
                print ""
            except Exception as e:
                print("Exception-occurred-:", str(e))

    def psu_get(self):
        """
        Method to get psu health status.
        """
        for chassis in self.chassis_list:
            power_uri = chassis['Links']['Power'][0]['@odata.id']
            try:
                response = self.rest_client.http_get(power_uri)
                psu_list = json.loads(response.text)['Power']

                print "///////////////////////////////////////////////////////////////////"
                for psu in psu_list:
                    response = self.rest_client.http_get(psu['@odata.id'])
                    psu_dict = json.loads(response.text)['PowerSupply']
                    print ""
                    print "Power-Supply : [" + str(psu_dict['MemberId']) + "]"
                    print "Power  : [" + str(psu_dict['PowerConsumedWatts']) + "] Watts"
                    print "FanRpm : [" + str(psu_dict['PsuFanRpm']) + "]"
                    print "State  : [" + str(psu_dict['Status']['State']) + "]"
                    print "Health : [" + str(psu_dict['Status']['Health']) + "]"
                    print ""

                    LOGMSG = "{\"MemberId\":" + str(psu_dict['MemberId']) +"," + \
                    "\"PowerConsumedWatts\":" + str(psu_dict['PowerConsumedWatts']) + "," + \
                    "\"State\":\"" + str(psu_dict['Status']['State']) + "\"}"

                    self.log.info(LOGMSG)

                    if (psu_dict['Status']['Health'] != 'OK'):
                        # Raise Alarm
                        print "Power-Supply: [" + str(psu_dict['MemberId']) + "]  ALARM !!!!! HEALTH !!! NOT !!!! OK"
                        LOGMSG = "{\"Power MemberId\":" + str(psu_dict['MemberId']) +"," + \
                        "\"Health\":\"" + "Alarm" + "\"}"
                        self.log.info(LOGMSG)
                        print ""
                print ""
            except Exception as e:
                print("Exception-occurred-:", str(e))

    def get_chassis_health(self):
        if self.rest_client.psme_version =="calsoft" :
            self.fans_get()
            self.psu_get()
            self.temp_get()
        else :
            self.fans_get_()
            self.psu_get_()
            self.temp_get_()

        self.health_thread = reactor.callLater(
            Defaults.health_check_interval,
            self.get_chassis_health)

    def stop_chassis_monitoring(self):
        try:
            h, self.health_thread = self.health_thread, None
            if(h is not None):
                h.cancel()
        except Exception as e:
            print("Exception-occured-:", str(e))
