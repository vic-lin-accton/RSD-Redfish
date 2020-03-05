#!/bin/bash
# To get/set HW device node info.
#
show_help()
{
    echo "psme.sh get max_fan_num                  : get current max fan num                 "
    echo "psme.sh get thermal_sensor_num           : get current thermal sensor number       "
    echo "psme.sh get max_psu_num                  : get current max psu  num                "
    echo "psme.sh get max_port_num                 : get current max port num                "
    echo "psme.sh set force_off                    : set device force off                    "
    echo "psme.sh set shutdown                     : set device shutdown gracefully          "
    echo "psme.sh set force_restart                : set device force restart                "
    echo "psme.sh set restart                      : set device restart gracefully           "
    echo "psme.sh get mgmt_port_name               : get management port name                "
}

if [ $# -lt 1 ]; then
   show_help 
fi

PlatformPath="/etc/onl/platform"

if [ -f "${PlatformPath}" ];then
    #Real Platform
    HW_type=`cat  ${PlatformPath}`
    ONLPDUMP="/lib/platform-config/${HW_type}/onl/bin/onlpdump"
else
    HW_type=`/usr/local/bin/sonic-cfggen -v platform`
    ONLPDUMP="/usr/bin/decode-syseeprom"
fi

# Common Used #

update_sw_volt()
{
    echo "NOT support."
}

set_forceoff()
{
    echo "NOT support."
}

set_forcerestart()
{
    echo "NOT support."
}

get_max_fan_num()
{
    echo "${CURRENT_FAN_NUM}"
}

if [ -f "${PlatformPath}" ];then
    HW_type=`cat  ${PlatformPath}`
else
    HW_type=`/usr/local/bin/sonic-cfggen -v platform`
fi

# Include HW_NODE_INFO

if [ "$HW_type" != "arm-accton-as4610-54-r0" ];then
    Product_name=`dmidecode -s system-product-name | grep SAU`
fi

. /etc/psme/HW_TYPE

DBG=0

cmd_method=$1
cmd_node=$2

if [ "${DBG}" = 1 ];then
    echo "Platform[${HW_type}]"
fi

CURRENT_FAN_NUM=${#fans_type[@]}
CURRENT_THERMAL_NUM=${#thermals_type[@]}
CURRENT_PSU_NUM=${#psus_type[@]}
CURRENT_PORT_NUM=${#mapping[@]}

get_fan_number()
{
	echo "${CURRENT_FAN_NUM}"
}

get_thermal_sensor_num()
{
    echo "${CURRENT_THERMAL_NUM}"
}

get_max_psu_num()
{
	echo "${CURRENT_PSU_NUM}"
}

get_max_port_num()
{
	echo "${CURRENT_PORT_NUM}"
}

get_inf_vlans_count()
{
	echo `ip -d link show | grep "${1}\." | wc -l` 
}

get_inf_vlans_value()
{
	res=`ip -d link show | grep "$1\." | awk '{print $2}'| sed -e s/$1.//g | awk -F'@' '{print $1}'`
	ARR=($res)
	id=$(($2-1))
	echo ${ARR[$id]}
}

##################################################
# ONL/SONiC has differnet management port name 
# ONL system use onlpdump to get eeprom info. 
# SONiC system use decode-syseeprom to get inf. 
##################################################
get_mgmt_port_name()
{
	PlatformPath="/etc/onl/platform"
	if [ -f "${PlatformPath}" ];then
	    #ONL system
		echo "ma1"
	elif [ -f "/usr/bin/decode-syseeprom" ];then
	    #SONiC system
		echo "eth0"
	else
		echo "ma1"
	fi
}

##################################################
# Set command 
##################################################

set_shutdown()
{
    sync;sync; shutdown -h now;
}



set_restart()
{
    sync;sync; shutdown -r now;
}

case "${1}" in
"get")  
	case "${2}" in
	"thermal_sensor")
	get_thermal_sensor
	;;
	"thermal_sensor_num")
	get_thermal_sensor_num
	;;
	"max_fan_num")
	get_max_fan_num
        ;;
	"max_port_num")
	get_max_port_num
        ;;
	"max_psu_num")
	get_max_psu_num
        ;;
	"inf_vlans_count")
	get_inf_vlans_count "${3}"
	;;
	"inf_vlans_value")
	get_inf_vlans_value "${3}" "${4}"
	;;
	"mgmt_port_name")
	get_mgmt_port_name 
	;;
	esac
	;;
"set")  
	case "${2}" in
	"force_off")
        set_forceoff	
	;;
	"shutdown")
        set_shutdown	
	;;
	"force_restart")
        set_forcerestart	
	;;
	"restart")
        set_restart
	;;
	esac
	;;
"help")  
   	show_help 
	;;
esac
