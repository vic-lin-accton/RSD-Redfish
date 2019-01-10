#!/bin/bash
IFS=$'\n'
TLV_C=($(/usr/local/sbin/lldpcli show ne | grep "OUI: 00,00,E8" | awk -F'Len: ' '{print $2}'))

LE1=`echo ${TLV_C[0]}|awk -F' ' '{print $1}'`
LE2=`echo ${TLV_C[1]}|awk -F' ' '{print $1}'`
CNT1=`echo ${TLV_C[0]}|awk -F' ' '{print $2}'`
CNT2=`echo ${TLV_C[1]}|awk -F' ' '{print $2}'`

IFS=$' '
ARR1=($(echo $CNT1 | sed -e 's/,/ /g'))
IFS=" "
C_COUNT=${#ARR1[@]}
for (( c=0; c < "${C_COUNT}"; c++ ))
    do
    echo 0x"${ARR1[$c]}" | xxd -r >> /etc/psme/certs/ca.crt
done

IFS=$' '
ARR2=($(echo $CNT2 | sed -e 's/,/ /g'))
IFS=" "
C_COUNT=${#ARR2[@]}
for (( c=0; c < "${C_COUNT}"; c++ ))
    do
    echo 0x"${ARR2[$c]}" | xxd -r >> /etc/psme/certs/ca.crt
done

