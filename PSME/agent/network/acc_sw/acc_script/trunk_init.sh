#!/bin/bash

DBG=

NETSERVE_PID=`pidof netserve`

if [ "$NETSERVE_PID" = "" ];then
    exit
fi

DATE=`date '+%Y-%m-%d %H:%M:%S'`
echo [$DATE] [trunk_init.sh $1 ] >> /var/log/SDK_CMD_LOG 

case "${1}" in

"start")
    PORT_MAX=`psme.sh get max_port_num`

    COUNT=`trunk.sh get num`

    for (( c=1; c <= "${COUNT}"; c++ ))
    do  
        Trunk_Index=`trunk.sh get num_index  $c`
        Trunk_Port_Mem=`uci get NETWORK.Trunk_$Trunk_Index.port`
        iTrunk_Port_Mem=`printf "%d" $Trunk_Port_Mem`

        for (( ic=1; ic <= "${PORT_MAX}"; ic++ ))
        do
            shp=$((1 <<  ${ic}))
            if [ $(($shp & $iTrunk_Port_Mem)) != 0 ];then
                RES=`trunk.sh set mem $c $ic`
            fi
        done #loop end

    done #loop end
    ;;

"clean")
    RES=`acc_sw 'trunk deinit'`
    RES=`acc_sw 'trunk init'`
    ;;
*)
;;

esac

