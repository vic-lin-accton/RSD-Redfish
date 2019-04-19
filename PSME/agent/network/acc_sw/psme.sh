#!/bin/bash
# To get/set HW device node info.
#
show_help()
{
    echo "psme.sh get max_fan_num                  : get current max fan num                 "
    echo "psme.sh get fan_presence                 : get currnet device fans presence status "
    echo "psme.sh get fan_rear_speed_rpm           : get current fan rear speed              "
    echo "psme.sh get thermal_sensor               : get current thermal sensor              "
    echo "psme.sh get thermal_sensor_num           : get current thermal sensor number       "
    echo "psme.sh get psu_power_out                : get currnet psu power out               "
    echo "psme.sh get psu_power_out_sum            : get currnet sum of psu power out        "
    echo "psme.sh get psu_presence                 : get currnet device psu presence status  "
    echo "psme.sh get max_psu_num                  : get current max psu  num                "
    echo "psme.sh get max_port_num                 : get current max port num                "
    echo "psme.sh get sfp_presence                 : get currnet sfp presence status         "
    echo "psme.sh get sfp_port_status PORT         : get current PORT status                 "
    echo "psme.sh set force_off                    : set device force off                    "
    echo "psme.sh set shutdown                     : set device shutdown gracefully          "
    echo "psme.sh set force_restart                : set device force restart                "
    echo "psme.sh set restart                      : set device restart gracefully           "
    echo "psme.sh get pwd_check USER PWD           : get device USER name PWD password match "
    echo "psme.sh get update_sw URI TYPE           : get URI software TYPE [VOLT/PSME/ONL]   "
    echo "psme.sh get mgmt_port_name               : get management port name                "
    echo "psme.sh get upper_sys_th_thermal_temp    : get upper non-critical/critical/fatal temp."
    echo "psme.sh get upper_cpu_th_thermal_temp    : get upper non-critical/critical/fatal temp."
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

##################################################
#  Get upper cpu thermal threshold non-critical/critical/fatal temperature. 
##################################################
get_upper_cpu_th_thermal_temp()
{
    echo "82000 104000 104000"
}

get_max_fan_num()
{
        #if there doesn't have the 'fan1_present' file , it will receive double fan number.
        if [ -f "${CURRENT_FAN_NODE_PATH}fan1_present" ];then
            plugable_fan=$(get_fan_number)
        else
            if [ "${HW_type}" != "arm-accton-as4610-54-r0" ];then
            plugable_fan=` expr $(get_fan_number) / 2`
            else
                plugable_fan=` expr $(get_fan_number)`
            fi
        fi
	
	psu_fan=2
	count=`expr ${plugable_fan} + ${psu_fan}`
	echo "${count}"
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

#[ ! -f ${PlatformPath} ] && echo "WARNING!! UNKNOW HW TYPE !! Under VM env." 


if [ "${DBG}" = 1 ];then
    echo "Platform[${HW_type}]"
fi

get_fan_number()
{
    #if there doesn't have the 'fan1_present' file , use the other judgment mode.
    if [ -f "${CURRENT_FAN_NODE_PATH}fan1_present" ];then
        local count=`ls ${CURRENT_FAN_NODE_PATH} | grep "_present" | wc -l`
        echo "${count}"
    elif [ -f "${CURRENT_FAN_NODE_PATH}fan1_speed_rpm" ]; then
        local count=`ls ${CURRENT_FAN_NODE_PATH} | grep "_speed_rpm" | wc -l`
        echo "${count}"   
    else
        echo 0
    fi

}

get_psu_fan_number()
{
    local count=0
    local rpm=`cat ${CURRENT_PSU1_FAN_NODE_PATH}/psu_fan1_speed_rpm`
    local rpm2=`cat ${CURRENT_PSU2_FAN_NODE_PATH}/psu_fan1_speed_rpm`
    
    if [ "${rpm}" != "" ];then
	count=`expr $count + 1`
    fi

    if [ "${rpm2}" != "" ];then
	count=`expr $count + 1`
    fi
    echo "${count}"
}

get_psu_power_out_number()
{
    local count=0
    local rpm=`cat ${CURRENT_PSU1_FAN_NODE_PATH}`
    local rpm2=`cat ${CURRENT_PSU2_FAN_NODE_PATH}`
    
    if [ "${rpm}" != "" ];then
	count=`expr $count + 1`
    fi

    if [ "${rpm2}" != "" ];then
	count=`expr $count + 1`
    fi
    echo "${count}"
}

##################################################
# return value 
# -1  : not present
# >=0 : output power 
##################################################

get_psu_power_out()
{
    
    if [ -f "${CURRENT_PSU1_FAN_NODE_PATH}psu_p_out" ];then
        local pout1=`cat ${CURRENT_PSU1_FAN_NODE_PATH}\psu_p_out`
    if [ "${pout1}" = "" ];then
	pout1="0"
    fi
    else
        local pout1=-1
    fi

    if [ -f "${CURRENT_PSU2_FAN_NODE_PATH}psu_p_out" ];then
        local pout2=`cat ${CURRENT_PSU2_FAN_NODE_PATH}\psu_p_out`
    if [ "${pout2}" = "" ];then
	pout2="0"
    fi
    else
        local pout2=-1
    fi

    if [ "$1" = "sum" ];then
	if [ "$pout1" = "-1" ];then
	    pout1=0;
	fi

	if [ "$pout2" = "-1" ];then
	    pout2=0;
	fi
	sum=$((${pout1} + ${pout2}))
	B3=$((${sum}/1000/100))
	CB3=$(printf "%x" "'${B3}")
	B2=$(((${sum}/1000 - ${B3}*100)/10))
	CB2=$(printf "%x" "'${B2}")
	B1=$(((${sum}/1000 - ${B3}*100 - ${B2}*10)))
	CB1=$(printf "%x" "'${B1}")
	echo "${CB3},${CB2},${CB1}"
    else
        echo "${pout1} ${pout2}"
    fi
}

	

get_fan_presence()
{
	i=1
	local presence_value=0

        #if there doesn't have the 'fan1_present' file , it will receive double fan number.
        if [ -f "${CURRENT_FAN_NODE_PATH}fan1_present" ];then
            local count=$(get_fan_number)
        else
            if [ "${HW_type}" != "arm-accton-as4610-54-r0" ];then
            local count=`expr $(get_fan_number) / 2`
            else
                local count=` expr $(get_fan_number)`
            fi
        fi

	while [ $count -gt 0 ];do
	    
            if [ -f "${CURRENT_FAN_NODE_PATH}fan1_present" ];then
                eval fan_name=fan"$i"_present
            else
                eval fan_name=fan"$i"_speed_rpm
            fi

	    status=`cat ${CURRENT_FAN_NODE_PATH}/${fan_name}`
	    if [ "$status" = "0" ]; then
               tmp=$((0 << $i - 1))
               presence_value=$(($tmp | $presence_value))
   	    else
	       tmp=$((1 << $i - 1))
               presence_value=$(($tmp | $presence_value))
	    fi 
		
            i=`expr $i + 1`
            count=`expr $count - 1`
#            echo "i[$i]"
	done
# PSU FAN 
	status=`cat ${CURRENT_PSU1_FAN_NODE_PATH}/psu_fan1_speed_rpm`
#	echo "status[${status}]"

        #There are two possible outputs.
	if [ "$status" = "" ] || [ "$status" = "0" ] ; then
	    tmp=$((0 << $i - 1))
	    presence_value=$(($tmp | $presence_value))
	else
            tmp=$((1 << $i - 1))
            presence_value=$(($tmp | $presence_value))
	fi 

	i=`expr $i + 1`
#	echo "i[$i]"

        status2=`cat ${CURRENT_PSU2_FAN_NODE_PATH}/psu_fan1_speed_rpm`
#        echo "status2[${status2}]"	  
        if [ "$status2" = "" ] || [ "$status2" = "0" ] ; then
	    tmp=$((0 << $i - 1))
	    presence_value=$(($tmp | $presence_value))
	else
	    tmp=$((1 << $i - 1))
	    presence_value=$(($tmp | $presence_value))
	fi 		
	echo "$presence_value"
}


get_psu_presence()
{
    PSU1=0
    PSU2=0
    PSU1_P_STATUS=`$ONLPDUMP | grep 'psu @ 1' -m 1 -A 5 | grep Status: | awk -F' ' '{print $2}'`
    PSU2_P_STATUS=`$ONLPDUMP | grep 'psu @ 2' -m 1 -A 5 | grep Status: | awk -F' ' '{print $2}'`

    if [ "$PSU1_P_STATUS" = "0x00000001" -o "$PSU1_P_STATUS" = "0x00000003" -o "$PSU1_P_STATUS" = "0x00000005" ];then
        PSU1=1
    fi

    if [ "$PSU2_P_STATUS" = "0x00000001" -o "$PSU2_P_STATUS" = "0x00000003" -o "$PSU2_P_STATUS" = "0x00000005" ];then
        PSU2=1
    fi

    S_PSU2=$(($PSU2 << 1))
    FR=$(($S_PSU2 | $PSU1))

    echo "$FR"
}

##################################################
# return value 
# -1  : not present
# >=0 : front speed rpm 
##################################################

get_fan_front_speed_rpm()
{
    i=1

    if [ -f "${CURRENT_FAN_NODE_PATH}fan1_present" ];then
        local count=$(get_fan_number)
    else
        if [ "${HW_type}" != "arm-accton-as4610-54-r0" ];then
        local count=`expr $(get_fan_number) / 2`
        else
            local count=` expr $(get_fan_number)`
        fi
    fi

    while [ $i -le $count ];do

        if [ -f "${CURRENT_FAN_NODE_PATH}fan1_present" ];then
            eval fan_name_p=fan"$i"_present
            status_p=`cat ${CURRENT_FAN_NODE_PATH}/${fan_name_p}`
        else
            eval fan_name_p=fan"$i"_speed_rpm
            status_p=`cat ${CURRENT_FAN_NODE_PATH}/${fan_name_p}`
        fi

	if [ "${status_p}" = 1 ];then
	    eval fan_name=fan"$i"_front_speed_rpm
	    status=`cat ${CURRENT_FAN_NODE_PATH}/${fan_name}`
        elif [ "${status_p}" != 0  ];then
            eval fan_name=fan"$i"_speed_rpm
            status=`cat ${CURRENT_FAN_NODE_PATH}/${fan_name}`
	else
	    status=-1
	fi
	f_status="${f_status} ${status} "

    i=`expr $i + 1`
    done
    echo "${CURRENT_FAN_NODE_PATH}/${fan_name}"
    echo "${f_status}"
}

##################################################
# return value 
# -1  : not present
# >=0 : rear speed rpm 
#
##################################################

get_fan_rear_speed_rpm()
{
    i=1

    if [ -f "${CURRENT_FAN_NODE_PATH}fan1_present" ];then
        local count=$(get_fan_number)
    else
        if [ "${HW_type}" != "arm-accton-as4610-54-r0" ];then
        local count=`expr $(get_fan_number) / 2`
        else
            local count=` expr $(get_fan_number)`
        fi
    fi

    while [ $i -le $count ];do

        if [ -f "${CURRENT_FAN_NODE_PATH}fan1_present" ];then
            eval fan_name_p=fan"$i"_present
            status_p=`cat ${CURRENT_FAN_NODE_PATH}/${fan_name_p}`
        else
        
            if [ "${HW_type}" != "arm-accton-as4610-54-r0" ];then
            eval fan_name_p=fanr"$i"_speed_rpm
            else
	            eval fan_name_p=fan"$i"_speed_rpm            
            fi
            
            status_p=`cat ${CURRENT_FAN_NODE_PATH}/${fan_name_p}`
        fi


	if [ "${status_p}" = 1 ];then
	    eval fan_name=fan"$i"_rear_speed_rpm
	    status=`cat ${CURRENT_FAN_NODE_PATH}/${fan_name}`
        elif [ "${status_p}" != 0  ];then
        
            if [ "${HW_type}" != "arm-accton-as4610-54-r0" ];then
            eval fan_name=fanr"$i"_speed_rpm
            else
	            eval fan_name=fan"$i"_speed_rpm            
            fi        
        
            status=`cat ${CURRENT_FAN_NODE_PATH}/${fan_name}`
	else
	    status=-1
	fi
	f_status="${f_status} ${status} "
    i=`expr $i + 1`
    done

# Add psu FAN unit #
    local status1=`cat ${CURRENT_PSU1_FAN_NODE_PATH}/psu_fan1_speed_rpm`
    if [ "$status1" = "" ] || [ "$status1" = "0" ] ; then
	f_status="${f_status} -1 "
    else
	f_status="${f_status} ${status1} "
    fi	

    local status2=`cat ${CURRENT_PSU2_FAN_NODE_PATH}/psu_fan1_speed_rpm`
    if [ "$status2" = "" ] || [ "$status2" = "0" ] ; then
	f_status="${f_status} -1 "
    else
	f_status="${f_status} ${status2} "
    fi	

    echo "${f_status}"
}


get_thermal_sensor_num()
{
    echo "${CURRENT_THERMAL_NUM}"
}

##################################################
# Case number must == get_thermal_sensor_num     
# Current MAX sensor numver is 7                 
# Node 1 is CPU core temp.                       
# Node 2~5 are board thermal sensors            
# Node 6,7 are PSU thermal sensors               
# -1  : not present 
# >=0 : thermal sensor detect value 
##################################################

get_thermal_sensor()
{
    i=1
    local count=$(get_thermal_sensor_num)

    while [ $i -le $count ];do
		case "${i}" in
		1)  
		      files=(${CURRENT_THERMAL_NODE1_PATH})
		      if [ -e "${files[0]}" ];then
		      	status=`cat ${CURRENT_THERMAL_NODE1_PATH}`
		      else
		      	status=-1
		      fi
		    ;;
		2)  
		      files=(${CURRENT_THERMAL_NODE2_PATH})
		      if [ -e "${files[0]}" ];then
		      	status=`cat ${CURRENT_THERMAL_NODE2_PATH}`
		      else
		      	status=-1
		      fi
		    ;;
		3)  
		      files=(${CURRENT_THERMAL_NODE3_PATH})
		      if [ -e "${files[0]}" ];then
		      	status=`cat ${CURRENT_THERMAL_NODE3_PATH}`
		      else
		      	status=-1
		      fi
		   ;;
		4)  
		      files=(${CURRENT_THERMAL_NODE4_PATH})
		      if [ -e "${files[0]}" ];then
		      	status=`cat ${CURRENT_THERMAL_NODE4_PATH}`
		      else
		      	status=-1
		      fi
		   ;;
		5)  
		      files=(${CURRENT_THERMAL_NODE5_PATH})
		      if [ -e "${files[0]}" ];then
		      	status=`cat ${CURRENT_THERMAL_NODE5_PATH}`
		      else
		      	status=-1
		      fi
		   ;;	
		6)  
		      files=(${CURRENT_THERMAL_NODE6_PATH})
		      if [ -e "${files[0]}" ];then
		      	status=`cat ${CURRENT_THERMAL_NODE6_PATH}`
		      else
		      	status=-1
		      fi
		   ;;
		7)  
		      files=(${CURRENT_THERMAL_NODE7_PATH})
		      if [ -e "${files[0]}" ];then
		      	status=`cat ${CURRENT_THERMAL_NODE7_PATH}`
		      else
		      	status=-1
		      fi
		   ;;			   	    
		8)  
		      files=(${CURRENT_THERMAL_NODE8_PATH})
		      if [ -e "${files[0]}" ];then
		      	status=`cat ${CURRENT_THERMAL_NODE8_PATH}`
		      else
		      	status=-1
		      fi
		   ;;
		9)  
		      files=(${CURRENT_THERMAL_NODE9_PATH})
		      if [ -e "${files[0]}" ];then
		      	status=`cat ${CURRENT_THERMAL_NODE9_PATH}`
		      else
		      	status=-1
		      fi
		   ;;		   				   	    
		esac

		if [ "${status}" = "" ];then
			status="0"
		fi
		
		f_status="${f_status} ${status} "
   	i=`expr $i + 1`
    done
    echo "${f_status}"
}

get_fan_fault()
{
    i=1
    local fault_value=0

    if [ -f "${CURRENT_FAN_NODE_PATH}fan1_present" ];then
        local count=$(get_fan_number)
    else
            if [ "${HW_type}" != "arm-accton-as4610-54-r0" ];then
        local count=`expr $(get_fan_number) / 2`
            else
                local count=` expr $(get_fan_number)`
            fi
    fi

    while [ $count -gt 0 ];do
		eval fan_name=fan"$i"_fault
        status=`cat ${CURRENT_FAN_NODE_PATH}/${fan_name}`
	
        if [ "$status" = "0" ]; then
            tmp=$((0 << $i - 1))
            fault_value=$(($tmp | $fault_value))
		else
            tmp=$((1 << $i - 1))
            fault_value=$(($tmp | $fault_value))
		fi 
    i=`expr $i + 1`
    count=`expr $count - 1`
    done

    echo "$fault_value"
}

get_max_psu_num()
{
	echo "${CURRENT_PSU_NUM}"
}

get_max_port_num()
{
	echo "${CURRENT_PORT_NUM}"
}

get_sfp_port_status()
{
	if [ "${1}" -ge 1 -a "${1}" -le "${CURRENT_PORT_NUM}" ] ; then
		spf_p=$(get_sfp_port_path "${1}")
		#echo "[$spf_p]"

    		if [ -f "${spf_p}/sfp_is_present" ];then
			status=`cat ${spf_p}/sfp_is_present`
                elif [ "$spf_p" = "/sys/bus/i2c/devices/E" ] || [ "$spf_p" = "/sys/devices/1803b000.i2c/i2c-1E" ];then
                        status=2  #Ethernet port
		elif [ -f "${spf_p}/module_present_${1}" ];then
			status=`cat ${spf_p}/module_present_${1}`
		else
		      	status=-1
		fi
	else
		echo "error port number"
		exit
	fi
	echo "${status}"
}

get_sfp_presence()
{
	i=1
	local presence_value=0

	local count="${CURRENT_PORT_NUM}"

	while [ $count -gt 0 ];do
	    
	    status=$(get_sfp_port_status "${i}")

		echo "count[$i] status[$status]"

	    if [ "$status" = "0" ] || [ "$status" = "-1" ] ; then
               tmp=$((0 << $i - 1))
               presence_value=$(($tmp | $presence_value))
   	    else
	       tmp=$((1 << $i - 1))
               presence_value=$(($tmp | $presence_value))
	    fi 
		
            i=`expr $i + 1`
            count=`expr $count - 1`
	done

	echo "$presence_value"
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

get_pwd_check()
{
	USER_NAME_I="$1"
	PASSWD_I="$2"
	HA=`getent shadow ${USER_NAME_I} | awk -F"$" '{print $3}'`
	#echo "HA[$HA]"
	HASHTYPE=`getent shadow ${USER_NAME_I} | awk -F"$" '{print $2}'`
	
	case "${HASHTYPE}" in
	1)  #MD5
	    HASHTYPEG="md5"
	    ;;
	5)  #SHA256
	    HASHTYPEG="sha-256"
	    ;;		    
	6)  #SHA512
	    HASHTYPEG="sha-512"
	    ;;	
  esac

  PASSWD=`mkpasswd -m ${HASHTYPEG} ${PASSWD_I} ${HA} | awk -F"$" '{print $4}'`  
  
	#echo "PASSWD[$PASSWD]"
	SHADOWPWD=`getent shadow ${USER_NAME_I} | awk -F"$" '{print $4}'  | awk -F":" '{print $1}' `
	#echo "SHAPASSWD[$SHADOWPWD]"
	if [ "${PASSWD}" = "${SHADOWPWD}" ];then
		echo 1
	else
		echo 0
	fi
}


##################################################
# Update software from URI path 
# URI   :  /x.x.x.x/image_name 
# TYPE  :  VOLT;PSME;ONL
##################################################
get_sw_update()
{
	URI="$1"
	TYPE="$2"
	
	case "${TYPE}" in
	"VOLT") 
            update_sw_volt "${URI}" 
	    ;;
	"PSME") 
	echo "NOT support."
	    ;;		    
	"ONL") 
	echo "NOT support."
	    ;;	
        esac
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
#  Get upper system threshold thermal non-critical/critical/fatal temperature. 
##################################################
get_upper_sys_th_thermal_temp()
{
#Todo Need ONL to provide these info .
    echo "44000 55000 65000"
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
	"fan_presence")
	get_fan_presence
	;;
	"fan_fault")
	get_fan_fault
	;;
	"fan_front_speed_rpm")
	get_fan_front_speed_rpm
	;;
	"fan_rear_speed_rpm")
	get_fan_rear_speed_rpm
	;;
	"thermal_sensor")
	get_thermal_sensor
	;;
	"thermal_sensor_num")
	get_thermal_sensor_num
	;;
	"psu_presence")
	get_psu_presence
	;;
	"psu_fan_num")
	get_psu_fan_number
	;;		
	"psu_power_out")
	get_psu_power_out
	;;		
	"psu_power_out_sum")
	get_psu_power_out sum
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
	"sfp_port_status")
	get_sfp_port_status "${3}"
        ;;
	"sfp_presence")
	get_sfp_presence
	;;
	"inf_vlans_count")
	get_inf_vlans_count "${3}"
	;;
	"inf_vlans_value")
	get_inf_vlans_value "${3}" "${4}"
	;;
	"pwd_check")
	get_pwd_check "${3}" "${4}"
	;;
	"update_sw")
	get_sw_update "${3}" "${4}"
	;;
	"mgmt_port_name")
	get_mgmt_port_name 
	;;
	"upper_sys_th_thermal_temp")
	get_upper_sys_th_thermal_temp
	;;
	"upper_cpu_th_thermal_temp")
	get_upper_cpu_th_thermal_temp
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
