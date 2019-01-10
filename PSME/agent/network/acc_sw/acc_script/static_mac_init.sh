#!/bin/bash

DBG=

NETSERVE_PID=`pidof netserve`

if [ "$NETSERVE_PID" = "" ];then
    exit
fi

DATE=`date '+%Y-%m-%d %H:%M:%S'`
echo [$DATE] [static_mac_init.sh $1 ] >> /var/log/SDK_CMD_LOG 

. /usr/local/bin/psme.sh

case "${1}" in

"start")
    PORT_MAX=`psme.sh get max_port_num`
    COUNT=`acl.sh get acl_count`

    for (( c=1; c <= "${PORT_MAX}"; c++ ))
    do  
        Static_MAC_Entry=`l2.sh get static_mac_count $c`
        if [ "$Static_MAC_Entry" != 0 ];then

            for (( ic=1; ic <= "${Static_MAC_Entry}"; ic++ ))
            do
                MAC=`l2.sh get static_mac $c $ic`
                VLAN=`l2.sh get static_mac_vlan $c $ic`
                PORT=${opennsl_pm[$c]}
                RESULT=`acc_sw "l2 add Port=$PORT MACaddress=$MAC Vlanid=$VLAN STatic=true"`
            done #loop end
        fi 
    done #loop end
    ;;

"clean")
    ;;
*)
;;

esac

