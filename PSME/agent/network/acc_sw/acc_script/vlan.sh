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
echo "vlan.sh <get> <vlan_port_untag_value/vlan_port_count> <[1~4095]> <[1~$MAX_PORT]>"
echo "vlan.sh <set> <vlan_port_value/vlan_value_destroy> <[1~$MAX_PORT]> <1/0> <1/0>"
echo ""
echo "NOTE:"
echo ""
echo "get vlan_port_count       [port: 1~$MAX_PORT]"
echo "get vlan_port_untag_value [VLAN ID:1~4095] [port: 1~$MAX_PORT]"
echo "set vlan_port_value       [VLAN ID:1~4095] [port: 1~$MAX_PORT] [UNTAG Enable/Disable: 1/0] [Enable/Disable:1/0]"
echo "set vlan_value_destroy    [VLAN ID:1~4095]"
}

DATE=`date '+%Y-%m-%d %H:%M:%S'`
echo [$DATE] [vlan.sh $1 $2 $3 $4 $5 $6] >> /var/log/SDK_CMD_LOG 

if [ $# -eq 0 ];then
        help_p
        exit 99
fi


case "${1}" in

"get")

	NO_MAP_PORT=$3

	if [ $NO_MAP_PORT -gt $MAX_PORT ];then
	        echo ""
		exit 99
	fi

	PORT=${opennsl_pm[$NO_MAP_PORT]}

        case "${2}" in
##acc_sw get vlan_total port 
# To get VLAN count that port is its member #
        "vlan_port_count")

        rm /tmp/PORTVLAN 2>/dev/null 
        CC=0
        VLAN_A=($(uci show NETWORK | grep NETWORK.vlan | grep '\.'port | awk -F'NETWORK.vlan' '{print $2}' | awk -F '.port' '{print $1}'))
        TOTAL_VLAN_COUNT=${#VLAN_A[@]}

	for (( c=0; c < "${TOTAL_VLAN_COUNT}"; c++ ))
  	do 
	    VLAN_ID=${VLAN_A[$c]}

	    MEMBER=`uci get NETWORK.vlan$VLAN_ID.port`
            ISIN=$(is_port_in_pbm $NO_MAP_PORT $MEMBER)

            if [ "$ISIN" = 1 ];then
	    	MARRAY[$CC]="${VLAN_ID}"
               	CC=$(($CC+1))
                echo "${VLAN_ID}" >> /tmp/PORTVLAN
            fi
	done

	echo ${#MARRAY[@]}

        ;;
##acc_sw get vlan_port_value array_index 
##
        "vlan_port_value")
        if [ -f  /tmp/PORTVLAN ];then
            index=${3}
	    VLAN=($(cat /tmp/PORTVLAN))
	    echo ${VLAN[$index]}
        fi	
	;;

##acc_sw get vlan_port_un_tag_value vlanid port 
##
        "vlan_port_untag_value")
        VLAN_ID="${3}"

        MEMBER=`uci get NETWORK.vlan"$VLAN_ID".port`

        if [ "$MEMBER" = "" ];then
            echo "none"
            exit
        fi

        ISIN=$(is_port_in_pbm $NO_MAP_PORT $MEMBER)

        if [ ${ISIN} != 0 ];then

            UNTAG_MEMBER=`uci get NETWORK.vlan"$VLAN_ID".untag_port`

            if [ "$UNTAG_MEMBER" = "" ];then
                echo "none"
                exit
            fi

       	    ISIN=$(is_port_in_pbm $NO_MAP_PORT $UNTAG_MEMBER)

            if [ ${ISIN} != 0 ];then
                echo "untag"
            else
                echo "tag"
            fi

        else
            echo "none"
            exit
        fi

	;;
        esac
	;;
"set")

        case "${2}" in
        "vlan_value_destroy")

        VLANID_IN=${3}

        if [ "$VLANID_IN" = "1" ];then
           echo "ERROR"
           exit
        fi

        RESULT=`acc_sw "vlan destroy $VLANID_IN"`
        `uci delete NETWORK.vlan"$VLANID_IN"; uci commit`
	;;


## acc_sw set vlan_port_value vlanid port untag enable
        "vlan_port_value")

	NO_MAP_PORT=$4

	if [ $NO_MAP_PORT -gt $MAX_PORT ];then
	        echo ""
		exit 99
	fi

	PORT=${opennsl_pm[$NO_MAP_PORT]}

##check VLAN exist or not, if not create it and disable Egress Filter ## 
        CC=0
        VLAN_A=($(acc_sw 'vlan show' | awk -F'ports' '{print $1}' | awk -F'vlan' '{print $2}' | sed -e 's/ //g'))
        TOTAL_VLAN_COUNT=${#VLAN_A[@]}
        VLANID_IN=${3}
        UNTAG=${5}
        ENABLE=${6}

        #echo "VLANID_IN[$VLANID_IN]"

	for (( c=0; c < "${TOTAL_VLAN_COUNT}"; c++ ))
  	    do 
	    VLAN_ID=${VLAN_A[$c]}

            if [ "$VLAN_ID" = "$VLANID_IN" ];then
               FOUND=1 
               break
            else
               FOUND=0 
            fi
	done

        if [ "$FOUND" = "1" ];then
##VLAN exist
##Get this vlan PORT member 
	    MEMBER=`acc_sw "vlan show" | grep "vlan ${VLANID_IN}" -m1 | awk -F"(" '{print $2}' | awk -F")" '{print $1}'`
            UNTAG_MEMBER=`acc_sw "vlan show" | grep "vlan ${VLANID_IN}" -m1 | awk -F"untagged" '{print $2}' | awk -F"(" '{print $2}' | awk -F")" '{print $1}'`
	    if [ $VLANID_IN != 1 ];then

	            if [ "${ENABLE}" = "1" ];then
       	         	#Enable PORT BIT
			#add/remove port orgvalue
			HMEMBER=$(mapping_hexport add $NO_MAP_PORT $MEMBER)
              	    else
                	#Disalbe PORT BIT
			#add/remove port orgvalue
			HMEMBER=$(mapping_hexport remove $NO_MAP_PORT $MEMBER)
            	    fi

	            if [ "${UNTAG}" = "1" ];then
       		        #Enable PORT BIT
			#add/remove port orgvalue
			HUNTAG_MEMBER=$(mapping_hexport add $NO_MAP_PORT $UNTAG_MEMBER)
       		    else
               		 #Disalbe PORT BIT
			 HUNTAG_MEMBER=$(mapping_hexport remove $NO_MAP_PORT $UNTAG_MEMBER)
           	    fi

	            RESULT=`acc_sw "vlan destroy $VLANID_IN"`
            	    RESULT=`acc_sw "vlan create $VLANID_IN PortBitMap=$HMEMBER UntagBitMap=$HUNTAG_MEMBER"`
                    RESULT=`acc_sw "mod  egr_vlan $VLANID_IN 1  EN_EFILTER=0"` 
            	    RESULT=`uci set NETWORK.vlan"$VLANID_IN".port="$HMEMBER"`
                    RESULT=`uci set NETWORK.vlan"$VLANID_IN".untag_port="$HUNTAG_MEMBER";uci commit`

	    else

		    #VLAN CANNOT DIABLE/DISTROY ON PORT#
	            if [ "${UNTAG}" = "1" ];then
       		        #Enable PORT BIT
			#add/remove port orgvalue
			HUNTAG_MEMBER=$(mapping_hexport add $NO_MAP_PORT $UNTAG_MEMBER)
       		    else
               		 #Disalbe PORT BIT
			 HUNTAG_MEMBER=$(mapping_hexport remove $NO_MAP_PORT $UNTAG_MEMBER)
           	    fi

		    HMEMBER=$(mapping_hexport add $NO_MAP_PORT $MEMBER)
            	    RESULT=`acc_sw "vlan add 1 PortBitMap=$HMEMBER UntagBitMap=$HUNTAG_MEMBER"`
            	    RESULT=`uci set NETWORK.vlan"$VLANID_IN".port="$HMEMBER"`
                    RESULT=`uci set NETWORK.vlan"$VLANID_IN".untag_port="$HUNTAG_MEMBER";uci commit`



	    fi

        else

##VLAN NOT exist
##Create VLAN 

            if [ "${ENABLE}" = "1" ];then
		HMEMBER=$(mapping_hexport add $NO_MAP_PORT 0x000000000000000000)
            fi

            if [ "${UNTAG}" = "1" ];then
  	        #Enable PORT BIT
		#add/remove port orgvalue
		HUNMEMBER="0x222223fffffffffffe"
       	    else
                #Need TAG Disalbe PORT BIT
	        HUNMEMBER=$(mapping_hexport remove $NO_MAP_PORT "0x222223fffffffffffe")
            fi

            # echo "UP UNTAG MEMBER[$HUNMEMBER]"
            `uci set NETWORK.vlan"$VLANID_IN"=switch_vlan`
            `uci set NETWORK.vlan"$VLANID_IN".port="$HMEMBER"`
            `uci set NETWORK.vlan"$VLANID_IN".untag_port="$HUNMEMBER";uci commit`

            RESULT=`acc_sw "vlan create $VLANID_IN PortBitMap=$HMEMBER UntagBitMap=$HUNMEMBER"`
            #Need let Engress Filter diable, or traffic will not forward to other port
            RESULT=`acc_sw "mod  egr_vlan $VLANID_IN 1  EN_EFILTER=0"` 
        fi
        ;;
        esac
        ;;
*)
help_p
;;

esac

