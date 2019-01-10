#!/bin/bash

DBG=

NETSERVE_PID=`pidof netserve`

if [ "$NETSERVE_PID" = "" ];then
    exit
fi

DATE=`date '+%Y-%m-%d %H:%M:%S'`
echo [$DATE] [port_init.sh $1 ] >> /var/log/SDK_CMD_LOG 

case "${1}" in

"start")
    PORT_MAX=`psme.sh get max_port_num`

    for (( c=1; c <= "${PORT_MAX}"; c++ ))
    do  
        pvid=`uci get NETWORK.sp$c.pvid`
        tag=`uci get NETWORK.sp$c.tag`
        duplex=`uci get NETWORK.sp$c.duplex`
        fsize=`uci get NETWORK.sp$c.framesize`
        speed=`uci get NETWORK.sp$c.speed`
        auto=`uci get NETWORK.sp$c.auto`
        oper=`uci get NETWORK.sp$c.operate`
        RES=`port_status.sh set SFA $c $speed $fsize $auto`
        RES=`port_status.sh set pvid $c $pvid`
        RES=`port_status.sh set enable $c $oper`
    done #loop end

    IFS=$' '
    VLAN_A=($(uci show NETWORK | grep 'switch_vlan' | awk -F'=' '{printf $1}' | sed -e 's/NETWORK.vlan/ /g'))

    TOTAL_VLAN_COUNT=${#VLAN_A[@]}

    for (( c=0; c < "${TOTAL_VLAN_COUNT}"; c++ ))
    do
        VLAN_ID=${VLAN_A[$c]}
        ENABLE_PORT=`uci get NETWORK.vlan"$VLAN_ID".port`
        UNTAG_ENABLE_PORT=`uci get NETWORK.vlan"$VLAN_ID".untag_port`
        if [ "$VLAN_ID" = "1" ];then
            RESULT=`acc_sw "vlan add $VLAN_ID PortBitMap=$ENABLE_PORT UntagBitMap=$UNTAG_ENABLE_PORT"`
        else
        RESULT=`acc_sw "vlan create $VLAN_ID PortBitMap=$ENABLE_PORT UntagBitMap=$UNTAG_ENABLE_PORT"`
        fi
        #RESULT=`acc_sw "mod egr_vlan $VLAN_ID 1  EN_EFILTER=0"`
	#Cause 5812-54X vlan not accessable
    done

    ;;

"clean")
    ;;
*)
;;

esac

