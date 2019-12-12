#!/bin/bash
#rfip=172.17.8.122:8888
#curl --insecure -X POST -D headers.txt https://${rfip}/redfish/v1/SessionService/ -d '{"ServiceEnabled":false,"SessionTimeout":600}'
#robot -v OPENBMC_HOST:${rfip} Redfish_Service_Validator.robot 
DATE=`date +"%d-%m-%Y"`
echo $DATE
res=`mdir ~/workspace/RF_Auto_Test/$DATE/dmtf_tools`
