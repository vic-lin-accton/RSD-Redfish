#!/bin/bash

help_p()
{
    echo "get ParentID ifname"
    echo "get PortID ifname" 
    echo "get MgmtIP   ifname" 
}

PATH=/sbin:/usr/sbin:/bin:/usr/bin:/usr/local/bin:/usr/local/sbin


DATE=`date '+%Y-%m-%d %H:%M:%S'`
echo [$DATE] [lldp.sh $1 $2 ] >> /tmp/SDK_CMD_LOG

PIDOSERVICE=`pidof lldpd`

if [ "$PIDOSERVICE" = "" ];then
    echo [$DATE] [lldpd not exist ] >> /tmp/SDK_CMD_LOG
    exit
fi 


case "${1}" in

"get")
        case "${2}" in

        "ParentID")
        ifname=$3
        PID=`lldpcli show ne | grep "Interface:    $ifname" -A 14  | grep ChassisID | awk -F'ChassisID:' '{print $2}'| awk -F'local' '{print $2}' | sed -e 's/^ *//g'`
        #echo "[$content_all]"
        echo $PID
        ;;

        "PortID")
        PortID=`lldpcli show ne | grep "Interface:    $ifname" -A 14  | grep "PortID:" | awk -F'ifname' '{print $2}' | sed -e 's/^ *//g'`
        echo $PortID
        ;;

        "MgmtIP")
        MgmtIP=`lldpcli show ne | grep "Interface:    $ifname" -A 14  | grep "MgmtIP:" -m1 | awk -F'MgmtIP:' '{print $2}' | sed -e 's/^ *//g'`
        echo $MgmtIP
        ;;

        esac
        ;;

"set")
        case "${2}" in
        "static_mac")
        ;;


        esac
        ;;
*)
help_p
;;

esac
