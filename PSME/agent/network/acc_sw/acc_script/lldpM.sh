#!/bin/bash

#10M log file size
DSIZE=10000000
OPORTID="N/A"
NEED_CLIENT_CA=`cat /etc/psme/psme.conf  | grep client-cert-requir | awk -F'"client-cert-required" :' '{print $2}' | sed -e 's/,//g'`

if [ "$NEED_CLIENT_CA" = "true" ];then
    if [ -f /etc/psme/certs/ca.crt ];then
        rm /etc/psme/certs/ca.crt
    fi
fi

while [ 1 ];do

DATE=`date '+%Y-%m-%d %H:%M:%S'`

PORTID=` lldp.sh get PortID ma1 `
if [ "$PORTID" != "$OPORTID" ];then
    DID=`echo $PORTID | sed -e "s/Drawer//g"`
    echo "[$DATE] PORT ID[$DID]" >> /var/log/SDK_CMD_LOG

    if [ -f "/usr/local/sbin/lldpcli" ];then
        cmd="/usr/local/sbin/lldpcli configure lldp portidsubtype local Drawer$DID"
        #todo wait for knet ready
        #cmd="/usr/local/sbin/lldpcli configure lldp portidsubtype local  port1"
        echo "[$DATE][$cmd]" >> /var/log/SDK_CMD_LOG
        $cmd
    fi
    OPORTID="$PORTID"

fi

#GET CA before start PSME service

PIDOSERVICE=`pidof psme-rest-server`

if [ "$PIDOSERVICE" = "" ];then

    if [ "$NEED_CLIENT_CA" = "true" ];then

        if [ -f /etc/psme/certs/ca.crt ];then
            service psme start
            continue 
        else
            # get CA from TLV
            echo "[$DATE] Get CA from TLV" >> /var/log/SDK_CMD_LOG
            ca_from_tlv.sh
        fi

        #Legency spec 0.5 method

        MgmtIP=`lldp.sh get MgmtIP ma1`
        echo "RMM Mgmt IP[$MgmtIP]" >> /var/log/SDK_CMD_LOG

        if [ "$MgmtIP" != "" ];then
            cd /etc/psme/certs/
            RESULT=`scp root@"$MgmtIP":/etc/rmm/podm.cert ./ca.crt`

            if [ -f /etc/psme/certs/ca.crt ];then
                echo "Got ca.crt from RMM and start PSME"
                service psme start
            fi
        elif [ -f /etc/psme/certs/podm.cert ];then
            # If do not connect mgmt port to RMM, we can't get ca.crt forever..
            # If you want test client CA enabled without RMM connected, please
            # cp podm.cert to /etc/psme/certs/
            # podm.cert cp from RMM side or PODM's root.crt
            cp /etc/psme/certs/podm.cert /etc/psme/certs/ca.crt
            service psme start
        fi

    else
        echo "Start PSME server without Client CA needed!!"
        service psme start
    fi
 
fi #endof service check


sleep 5

## Check log filesize ##
SIZE=`ls -alF /var/log/SDK_CMD_LOG  | awk -F' ' '{print $5}'`

if [ "$SIZE" -gt "$DSIZE" ];then
   result=`rm /var/log/SDK_CMD_LOG`
   echo "rm /var/log/SDK_CMD_LOG" > "/var/log/SDK_CMD_LOG"
   echo "rm /var/log/SDK_CMD_LOG" 
fi

done
