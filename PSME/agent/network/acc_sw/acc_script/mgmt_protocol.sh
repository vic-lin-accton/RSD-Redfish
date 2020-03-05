#!/bin/bash

PlatformPath="/etc/onl/platform"



help_p()
{
echo "get ssh_status"
echo "get ssh_port"
echo "get snmp_status"
echo "get snmp_port"
}

case "${1}" in

"get")
    case "${2}" in
    "ssh_status")

    if [ -f "${PlatformPath}" ];then
    #For ONL
        STATUS=`service ssh status | grep running`
        if [ "$STATUS" = "sshd is running." ];then
            echo Enable 
        else
            echo Disable 
        fi
    else
    #For SONiC 
        SSH_PID=`pidof sshd`
        if [ "$SSH_PID" = "" ];then
            echo Disable 
        else
            echo Enable 
        fi
    fi
    ;;

    "ssh_port")

    if [ -f "${PlatformPath}" ];then
    #For ONL
        PORT=`cat /etc/ssh/sshd_config  | grep Port | awk '{print $2}'`
    else
    #For SONiC 
        PORT=`netstat -lnutp  | grep sshd | awk -F' ' '{print $4}'  | awk -F':' '{print $2}'`
    fi

    echo $PORT

    ;;

    "snmp_status")

    if [ -f "${PlatformPath}" ];then
    #For ONL
        STATUS=`service snmpd status | grep running`
        if [ "$STATUS" = "snmpd is running." ];then
            echo Enable 
        else
            echo Disable 
        fi
    else
    #For SONiC 
        SNMP_PID=`pidof snmpd`
        if [ "$SNMP_PID" = "" ];then
            echo Disable 
        else
            echo Enable 
        fi
    fi
    ;;

    "snmp_port")

    if [ -f "${PlatformPath}" ];then
    #For ONL
        PORT=`cat /etc/snmp/snmpd.conf  | grep 'agentAddress udp' | awk -F',' '{print $1}' | awk -F' ' '{print $2}' | sed -s 's/udp://g'`
    else
    #For SONiC 
        PORT=`netstat -lnutp  | grep snmpd | awk -F' ' '{print $4}'  | awk -F':' '{print $2}'`
    fi

    echo $PORT
    ;;
 
    esac
;;

*)
    help_p
;;
esac
