#!/bin/bash


help_p()
{
echo "get vlan_port_count mgmt_name"
echo "get vlan_port_value mgmt_name index"
}

case "${1}" in

"get")
    case "${2}" in
    "vlan_port_count")
        port=${3}
        IFS=$'\n'
        VLAN_A=($(ifconfig -a | grep "$port"\\.))
        TOTAL_VLAN_COUNT=${#VLAN_A[@]}
        echo $TOTAL_VLAN_COUNT
    ;;

    "vlan_port_value")
        port=${3} index=${4}
        IFS=$'\n'
        VLAN_A=($(ifconfig -a | grep "$port"\\.))
        TOTAL_VLAN_COUNT=${#VLAN_A[@]}
        VLAN=`echo ${VLAN_A[$index]} | awk -F' ' '{print $1}' | sed -e "s/$port.//g"` 
        echo $VLAN
    ;;
    esac
;;

*)
    help_p
;;
esac
