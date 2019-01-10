#!/bin/bash

NETSERVE_PID=`pidof netserve`

if [ "$NETSERVE_PID" = "" ];then
    exit
fi

. /usr/local/bin/psme.sh
. /usr/local/bin/tools.sh


MAX_PORT=`psme.sh get max_port_num`

help_p()
{
reset
echo ""
echo "////////////////////////////////////////////////////////////////////////////////////////////////"
echo "port_status.sh <get> <link/pvid/link_speed/cfg_speed/duplex/auto/framesize/enable> <[1~54]>"
echo "port_status.sh <set> <[1~54]> <[fsize/auto/pvid/enable/SFA]> <[10/100/1000/10000/40000](MB)/[1~9412]/[1/0]/[1~4095]>"
echo ""
echo "NOTE:"
echo ""
echo "get link       [port: 1~$MAX_PORT]"
echo "get pvid       [port: 1~$MAX_PORT]"
echo "get link_speed [port: 1~$MAX_PORT](BYTES)"
echo "get cfg_speed  [port: 1~$MAX_PORT](KB)"
echo "get duplex     [port: 1~$MAX_PORT]"
echo "get auto       [port: 1~$MAX_PORT]"
echo "get framesize  [port: 1~$MAX_PORT]"
echo "get enable     [port: 1~$MAX_PORT]"
echo "set speed      [port: 1~$MAX_PORT]  [link_speed: 10/100/1000/10000/40000 MB]"
echo "set fsize      [port: 1~$MAX_PORT]  [frame size: 1~9412]"
echo "set auto       [port: 1~$MAX_PORT]  [Enable/Disable: 1/0]"
echo "set pvid       [port: 1~$MAX_PORT]  [vlan id: 1~4095]"
echo "set enable     [port: 1~$MAX_PORT]  [Enable/Disable: 1/0]"
echo "set SFA        [port: 1~$MAX_PORT]  [link_speed: 10/100/1000/10000/40000 MB] [framesize: 1~9412] [auto: 1/0] "
echo "////////////////////////////////////////////////////////////////////////////////////////////////"
}

DATE=`date '+%Y-%m-%d %H:%M:%S'`
echo [$DATE] [port_status.sh $1 $2 $3 $4 $5 $6] >> /var/log/SDK_CMD_LOG 

if [ $# -eq 0 ];then
	help_p
        exit 99
fi


NO_MAP_PORT=$3

if [ $NO_MAP_PORT -gt $MAX_PORT ];then
	echo ""
        exit 99
fi

PORT=${opennsl_pm[$NO_MAP_PORT]}				

case "${1}" in

"get")
        case "${2}" in
        
        "link")

				link=`acc_sw "ps $PORT" | grep ")" | awk -F")" '{print $2}' |  awk -F" " '{print $1}'`

                                if [ "$link" = "" ];then
                                    cat /tmp/PORT"$PORT"LINK
                                else
				    echo "${link}"
				    echo "${link}" > /tmp/PORT"$PORT"LINK
                                fi
        ;;

        "pvid")
				#pvid=`acc_sw "pvlan show $PORT" | grep Port | awk -F'is ' '{print $2}'`
                                #uci command no need do port mappng #
				pvid=`uci get NETWORK.sp"$NO_MAP_PORT".pvid`


				echo "${pvid}"
        ;;

        "link_speed")
				speed=`acc_sw "ps $PORT" | grep ")" | awk -F"up" '{print $2}' |  awk -F" " '{print $1}' | sed -e 's/M/000/g' | sed -e 's/G/000000/g' `
				echo "${speed}"
        ;;

        "cfg_speed")
				u_speed=`uci get NETWORK.sp"$NO_MAP_PORT".speed`
				    echo "${u_speed}"
        ;;



        "vlan_mem")
        VLAN_A=($(acc_sw 'vlan show' | awk -F'ports' '{print $1}' | awk -F'vlan' '{print $2}' | sed -e 's/ //g'))
        echo "${#VLAN_A[@]}"
        ;;

##get in_vlan_mem  vlanid portid
        "in_vlan_mem")
	MEMBER=`acc_sw "vlan show" | grep "vlan ${3}" | awk -F"(" '{print $2}' | awk -F")" '{print $1}'`
	shp=$((1 <<  ${4}))
	bitop=$((${MEMBER} & ${shp}))
        if [ ${bitop} != 0 ];then
            echo 1
        else
            echo 0
        fi
	;;

        "duplex")
        duplex=`acc_sw "ps $PORT" | grep ")" | awk -F"up" '{print $2}' |  awk -F" " '{print $2}'`
        u_duplex=`uci get NETWORK.sp$NO_MAP_PORT.duplex`

        if [ "$duplex" = "" ];then
            echo "${u_duplex}"
        else
            echo "${duplex}"
        fi
        ;;
        "auto")
        #autonego=`acc_sw "ps $PORT" | grep ")" | awk -F"up" '{print $2}' |  awk -F" " '{print $4}'`
        autonego=`uci get NETWORK.sp"$NO_MAP_PORT".auto`
        echo "${autonego}"
        ;;
        "framesize")        
        #framesize=`acc_sw "ps $PORT" | grep ")" | awk -F"up" '{print $2}'  |  awk -F" " '{print $11}'`
        framesize=`uci get NETWORK.sp"$NO_MAP_PORT".framesize`
        echo "${framesize}"
        ;;
        "portname")
        #portname=`acc_sw "ps $PORT" | grep ")" | awk -F"(" '{print $1}' | sed -e 's/ //g'`
        portname=`uci get NETWORK.knet"$NO_MAP_PORT".ifname`
        echo "${portname}"
        ;;    

        "enable")
        port_ope=`uci get NETWORK.sp"$NO_MAP_PORT".operate`
        echo "${port_ope}"
        ;;   
        esac
        ;;    
"set")
        case "${2}" in
       "speed")
        #Mbps
        HPORT=$(mapping_hexport add $NO_MAP_PORT 0x000000000000000000)
        SPEED=$4
	result=`acc_sw "port $PORT speed=$SPEED"`
	result=`uci set NETWORK.sp"$NO_MAP_PORT".speed="$SPEED";uci commit`
        ;;
        "fsize")
        HPORT=$(mapping_hexport add $NO_MAP_PORT 0x000000000000000000)
        FSIZE=$4
	result=`acc_sw "port $PORT FrameMax=$FSIZE"`
	uci set NETWORK.sp"$NO_MAP_PORT".framesize="$FSIZE";uci commit
        ;;
        "auto")
        HPORT=$(mapping_hexport add $NO_MAP_PORT 0x000000000000000000)
        AUTO=$4
	result=`acc_sw "port $PORT AutoNeg=$AUTO"`
	result=`uci set NETWORK.sp"$NO_MAP_PORT".auto="$AUTO";uci commit`
        ;;
        "pvid")
        PVID=$4
        HPORT=$(mapping_hexport add $NO_MAP_PORT 0x000000000000000000)
	result=`acc_sw "pvlan set $HPORT $PVID"`
	result=`uci set NETWORK.sp"$NO_MAP_PORT".pvid="$PVID";uci commit`
        ;;
        "enable")
        ENA=$4
	result=`acc_sw "port $PORT enable=$ENA"`
	result=`uci set NETWORK.sp"$NO_MAP_PORT".operate="$ENA";uci commit`
        ;;
        "SFA")
        SPEED=$4
        FSIZE=$5
        AUTO=$6
	result=`acc_sw "port $PORT speed=$SPEED  FrameMax=$FSIZE  AutoNeg=$AUTO"`
	# 1st setting has ERROR return , need set twice....
	result=`acc_sw "port $PORT speed=$SPEED  FrameMax=$FSIZE  AutoNeg=$AUTO"`
	result=`uci set NETWORK.sp"$NO_MAP_PORT".speed="$SPEED"`
	result=`uci set NETWORK.sp"$NO_MAP_PORT".framesize="$FSIZE"`
	result=`uci set NETWORK.sp"$NO_MAP_PORT".auto="$AUTO";uci commit`
        ;;
        esac
        ;;
*)
       help_p
       ;;
esac

