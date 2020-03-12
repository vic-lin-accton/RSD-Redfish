#!/bin/bash

. raw_omci_data

res=`curl --insecure  -v  -X PATCH -d '{"OltOperateState": true}'  https://"$1":8888/redfish/v1/Olt`

sleep 25

res=`curl --insecure  -v  -X PATCH -d '{"AdministrativeState" : "Up"}'  https://"$1":8888/redfish/v1/EthernetSwitches/1/Ports/1/`

sleep 1 

res=`curl --insecure  -v  -X PATCH -d '{"AdministrativeState" : "Up"}'  https://"$1":8888/redfish/v1/EthernetSwitches/1/Ports/17/`

sleep 1 

res=`curl --insecure  -v  -X POST -d '{"onu_id":1,"vendor_id":"ISKT","vendor_specific":"71E80110"}' https://"$1":8888/redfish/v1/EthernetSwitches/1/Ports/1/ONUs `

echo "wait 10 secs"
sleep 10

res=`curl --insecure  -v  -X POST -d '
{
    "onuId": 1,
    "FlowId": 16,
    "PortId": 0,
    "NniId": 0,
    "FlowType": "upstream",
    "PktTagType": "single_tag",
    "GemportId": 1024,
    "Classifier": [
    "BCMOLT_CLASSIFIER_ID_O_VID"
    ],
    "Action": "BCMOLT_ACTION_ID_O_VID",
    "ActionCmd": "BCMOLT_ACTION_CMD_ID_ADD_OUTER_TAG",
    "ActionVal": {
    "OVid": 10,
    "OPbits": 0,
    "OTpid": 0,
    "IVid": 0,
    "IPbits": 0,
    "ITpid": 0,
    "EtherType": 0,
    "IpProto": 0,
    "SrcPort": 0,
    "DstPort": 0
},
"ClassVal": [
{
    "OVid": 20,
    "IVid": 0,      
    "OPbits": 0,
    "IPbits": 0,
    "EtherType": 0,
    "OTpid": 0,
    "IpProto": 0,
    "DstPort": 0,  
    "SrcPort": 0,          
    "ITpid": 0
}
]
}
' https://"$1":8888/redfish/v1/Olt/Flow` 

res=`curl --insecure  -v  -X POST -d '
{
    "onuId": 1,
    "FlowId": 16,
    "PortId": 0,
    "NniId": 0,
    "FlowType": "downstream",
    "PktTagType": "double_tag",
    "GemportId": 1024,
    "Classifier": [
    "BCMOLT_CLASSIFIER_ID_O_VID",
    "BCMOLT_CLASSIFIER_ID_I_VID"
    ],
    "Action": "BCMOLT_ACTION_ID_O_VID",
    "ActionCmd": "BCMOLT_ACTION_CMD_ID_REMOVE_OUTER_TAG",
    "ActionVal": {
    "OVid": 10,
    "OPbits": 0,
    "OTpid": 0,
    "IVid": 0,
    "IPbits": 0,
    "ITpid": 0,
    "EtherType": 0,
    "IpProto": 0,
    "SrcPort": 0,
    "DstPort": 0
},
"ClassVal": [
{
    "OVid": 10,
    "IVid": 20,      
    "OPbits": 0,
    "IPbits": 0,
    "EtherType": 0,
    "OTpid": 0,
    "IpProto": 0,
    "DstPort": 0,  
    "SrcPort": 0,          
    "ITpid": 0
}
]
}
' https://"$1":8888/redfish/v1/Olt/Flow` 

echo "wait 5 seconds then send omci"
sleep 5 

omci_size=${#OMCI_RAW[@]}
echo "omci_size[$omci_size]"

i=0
while [ $i != "$omci_size" ]
do
    echo i=$i
res=`curl --insecure  -v  -X POST -d '
{
"Omci" : "'${OMCI_RAW[$i]}'"
}
' https://"$1":8888/redfish/v1/EthernetSwitches/1/Ports/1/ONUs/1/Omci
`
i=$(($i+1))
sleep 0.3
done


