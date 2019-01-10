#!/bin/bash

PLATFORM=`cat /etc/onl/platform`
KO_PATH=/usr/bin/opennsl-accton/examples

if [ -d /usr/bin/opennsl-accton/examples ];then
if [ "$PLATFORM" = "x86-64-accton-as5812-54t-r0" -o  "$PLATFORM" = "x86-64-accton-as5812-54x-r0" ];then

    PIDOSERVICE=`pidof netserve`
                /usr/bin/opennsl_setup
   
    if [ "$PIDOSERVICE" = "" ];then
        cd $KO_PATH 
       	/usr/bin/netserve -d 2001 example_drivshell > /dev/null 2>&1
        acc_sw ps > /dev/null 2>&1 
       	# acc_sw "mod  egr_vlan 1 1  EN_EFILTER=0" > /dev/null 2>&1
        # Start Apply uci config settings #
        port_init.sh start        
        static_mac_init.sh start
        trunk_init.sh start
        acl_init.sh start
    		fi
    fi
fi
