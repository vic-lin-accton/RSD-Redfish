#!/bin/bash

NETSERVE_PID=`pidof netserve`

if [ "$NETSERVE_PID" = "" ];then
    exit
fi

. /usr/local/bin/psme.sh

MAX_PORT=`psme.sh get max_port_num`

help_p()
{
reset
echo ""
echo "////////////////////////////////////////////////////////////////////////////////////////////////"
echo "l2.sh <get> <static_mac_count/static_mac/static_mac_vlan/first_entry_mac> <[1~54]> <0~>"
echo "l2.sh <set> <static_mac/static_mac_del_idx/static_mac_patch> <[1~54]> <xx:xx:xx:xx:xx:xx/[0~]> <MAC/xx:xx:xx:xx:xx:xx/1~4095> </1~4095>"
echo ""
echo "NOTE:"
echo ""
echo "get static_mac_count   [port: 1~$MAX_PORT]"
echo "get static_mac         [port: 1~$MAX_PORT] [Index:0~]"
echo "get static_mac_vlan    [port: 1~$MAX_PORT] [Index:0~]"
echo "get first_entry_mac    [port: 1~$MAX_PORT]"
echo "set static_mac         [port: 1~$MAX_PORT] [MAC:xx:xx:xx:xx:xx:xx] [VLAN ID:1~4095]"
echo "set static_mac_del_idx [port: 1~$MAX_PORT] [Index:0~]"
echo "set static_mac_patch   [port: 1~$MAX_PORT] [Index:0~] [New MAC:xx:xx:xx:xx:xx:xx] [New VLAN ID:1~4095]"
}

DATE=`date '+%Y-%m-%d %H:%M:%S'`
echo [$DATE] [l2.sh $1 $2 $3 $4 $5 $6] >> /var/log/SDK_CMD_LOG

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
        "static_mac_count")

        if [ -f /tmp/L2MAC ];then
            rm /tmp/L2MAC
        fi

        if [ -f /tmp/L2MAC_VLAN ];then
            rm /tmp/L2MAC_VLAN
        fi

        PT=${NO_MAP_PORT}

        IFS=$'\n'

        L2=($(uci show STATICMAC | grep "STATICMAC.static_mac_$PT.entry_mac_" | awk -F'=' '{print $1}' | sed -e "s/STATICMAC.static_mac_"$PT".entry_mac_//g")) 
        TOTAL_L2_COUNT=${#L2[@]}

        for (( c=1; c <= "${TOTAL_L2_COUNT}"; c++ ))
            do
            content=`uci get STATICMAC.static_mac_$PT.entry_mac_${L2[$(($c-1))]}`
            echo "${content}" >> /tmp/L2MAC
        done

        LV2=($(uci show STATICMAC | grep "STATICMAC.static_mac_$PT.entry_vlan_")) 
        TOTAL_LV2_COUNT=${#LV2[@]}

        for (( c=1; c <= "${TOTAL_LV2_COUNT}"; c++ ))
            do
            content=`uci get STATICMAC.static_mac_$PT.entry_vlan_${L2[$(($c-1))]}`
            echo "${content}" >> /tmp/L2MAC_VLAN
        done
 
        echo $TOTAL_L2_COUNT
        ;;

        "static_mac")
        PT=${NO_MAP_PORT}
        INDEX=${4}

        IFS=$'\n'
        if [ -f /tmp/L2MAC ];then
            L2STATICMAC=($(cat /tmp/L2MAC))
            L2STATICMACV=($(echo ${L2STATICMAC[$(($INDEX-1))]}))
        fi
	echo ${L2STATICMACV} 
        ;;

        "static_mac_vlan")
        PT=${NO_MAP_PORT}
        INDEX=${4}
        IFS=$'\n'

        if [ -f /tmp/L2MAC_VLAN ];then
            L2STATICMAC=($(cat /tmp/L2MAC_VLAN))
            L2STATICMACV=($(echo ${L2STATICMAC[$(($INDEX-1))]}))
        fi
	echo ${L2STATICMACV} 
        ;;

        "first_entry_mac")
        PT=${PORT}

        L2MAC=`acc_sw 'l2 show' | grep -m 1 "port=$PT"| awk -F'mac=' '{print $2}' | awk -F'vlan=' '{print $1}'`
	echo ${L2MAC} 
        ;;

 

        esac
        ;;

"set")
        case "${2}" in
        "static_mac")
        PT=${NO_MAP_PORT}
        MAC=${4}
        VLANID=${5}

        if [ "$VLANID" != "0" ];then
            RESULT=`acc_sw "l2 add Port=$PORT MACaddress=$MAC Vlanid=$VLANID STatic=true"`
        else
            RESULT=`acc_sw "l2 add Port=$PORT MACaddress=$MAC Vlanid=1 STatic=true"`
            VLANID=1
        fi

        # Get index #
        IFS=$'\n'
        L2=($(uci show STATICMAC | grep "STATICMAC.static_mac_$PT.entry_mac_" | awk -F'=' '{print $1}' | sed -e "s/STATICMAC.static_mac_"$PT".entry_mac_//g")) 
        TOTAL_L2_COUNT=${#L2[@]}

        scontent=($(sort <<< "${L2[*]}"))

        for (( c=1; c <= "${TOTAL_L2_COUNT}"; c++ ))
            do
            if [ "${scontent[$(($c-1))]}" != "$c" ];then
                result=`uci set STATICMAC.static_mac_"$PT"="static_mac"`
                result=`uci set STATICMAC.static_mac_"$PT".entry_mac_$c="$MAC"`
                result=`uci set STATICMAC.static_mac_"$PT".entry_vlan_$c="$VLANID";uci commit`
                exit
        fi
        done
 
        result=`uci set STATICMAC.static_mac_"$PT"="static_mac"`
        result=`uci set STATICMAC.static_mac_"$PT".entry_mac_$c="$MAC"`
        result=`uci set STATICMAC.static_mac_"$PT".entry_vlan_$c="$VLANID";uci commit`
        ;;

        "static_mac_del_idx")

        PT=${NO_MAP_PORT}
        DELINDEX=${4}

        IFS=$'\n'
        L2=($(uci show STATICMAC | grep "STATICMAC.static_mac_$PT.entry_mac_" | awk -F'=' '{print $1}' | sed -e "s/STATICMAC.static_mac_"$PT".entry_mac_//g")) 
        TOTAL_L2_COUNT=${#L2[@]}

        IDX=$(($DELINDEX-1))
        OMAC=`uci get STATICMAC.static_mac_$PT.entry_mac_"${L2[$IDX]}"`
        OVLAN=`uci get STATICMAC.static_mac_$PT.entry_vlan_"${L2[$IDX]}"`

            RESULT=`acc_sw "l2 del MACaddress=$OMAC Vlanid=$OVLAN"`
        RESULT=`uci delete STATICMAC.static_mac_"$PT".entry_mac_"${L2[$IDX]}"`
        RESULT=`uci delete STATICMAC.static_mac_"$PT".entry_vlan_"${L2[$IDX]}";uci commit`

        ;;

        "static_mac_patch")

        PT=${NO_MAP_PORT}
        DELINDEX=${4}
        MAC=${5}
        VLANID=${6}
        IFS=$'\n'
        L2=($(uci show STATICMAC | grep "STATICMAC.static_mac_$PT.entry_mac_" | awk -F'=' '{print $1}' | sed -e "s/STATICMAC.static_mac_"$PT".entry_mac_//g")) 
        TOTAL_L2_COUNT=${#L2[@]}
        EntryID=${L2[$(($DELINDEX-1))]}
        OMAC=`uci get STATICMAC.static_mac_$PT.entry_mac_"${EntryID}"`
        OVLAN=`uci get STATICMAC.static_mac_$PT.entry_vlan_"${EntryID}"`

        RESULT=`acc_sw "l2 del MACaddress=$OMAC Vlanid=$OVLAN"`
        RESULT=`acc_sw "l2 add Port=$PORT MACaddress=$MAC Vlanid=$VLANID STatic=true"`
        RESULT=`uci set STATICMAC.static_mac_"$PT".entry_mac_$EntryID="$MAC"`
        RESULT=`uci set STATICMAC.static_mac_"$PT".entry_vlan_$EntryID="$VLANID";uci commit`
        ;;


        esac
        ;;
*)
help_p
;;

esac
