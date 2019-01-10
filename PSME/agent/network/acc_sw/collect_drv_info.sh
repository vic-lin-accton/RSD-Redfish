#!/bin/bash


PlatformPath="/etc/onl/platform"

if [ -f "${PlatformPath}" ];then
HW_type=`cat  ${PlatformPath}`
else
HW_type=`/usr/local/bin/sonic-cfggen -v platform`
fi

cmd_array=("NAME" "MSIZE" "SERIAL" "MODEL_ID" "REVISION")

if [ "$HW_type" = "arm-accton-as4610-54-r0" ];then
    c=1
    eval mem$c[1]=sda #id 1 for drv name 
    let mem$c[2]=8
    eval mem$c[3]="n/a" #id 3 for drv SERIAL 
    eval mem$c[4]="n/a"  #id 4 for drv ID_MODEL 
    eval mem$c[5]="n/a"  #id 5 for drv ID_REVISION 

else

device_num=`lsblk --nodeps -o name | wc -l`
device_num=$(($device_num-1))
#echo num[$device_num]

memory_index=($(lsblk --nodeps -o name))

for (( c=1; c <= $device_num; c++ ))
do  
    id=${memory_index[$c]}
    content=`fdisk -l`    
    content2=`udevadm info --query=all --name /dev/$id`
# Show total content
# echo "[$content2]" 
    MSIZE=`echo "[$content]" |  grep "Disk /dev/$id" | awk -F': ' '{print $2}' | awk -F' ' '{print $3}'`
    eval mem$c[1]='${id}' #id 1 for drv name 
    let mem$c[2]=${MSIZE}
#echo ${MSIZE}

    SERIAL=`echo "[$content2]" | grep ID_SERIAL= | awk -F'=' '{print $2}'` 
    eval mem$c[3]='${SERIAL}' #id 3 for drv SERIAL 
#echo ${SERIAL}

    ID_MODEL=`echo "[$content2]" | grep ID_MODEL_ID= | awk -F'=' '{print $2}'` 
    eval mem$c[4]='${ID_MODEL}' #id 4 for drv ID_MODEL 
#echo ${ID_MODEL}

    ID_REVISION=`echo "[$content2]" | grep ID_REVISION= | awk -F'=' '{print $2}'` 
    eval mem$c[5]='${ID_REVISION}' #id 5 for drv ID_REVISION 
#echo ${ID_REVISION}

done

fi

help_p()
{
    for (( i=0; i < ${#cmd_array[@]} ; i++ ))
    do 
        echo "collect_drv_info.sh" "[1..4]" ${cmd_array[$i]}
    done
}

if [ "$#" -eq 1 ]; then
    case "$1" in
    "MCOUNT")
    echo $device_num;
    exit
    ;;
    esac
fi

case "$2" in

"NAME")
var=mem$1[1]
echo ${!var}
;;

"MSIZE")
var=mem$1[2]
echo ${!var}
;;
"SERIAL")
var=mem$1[3]
echo ${!var}
;;

"MODEL_ID")
var=mem$1[4]
echo ${!var}
;;

"REVISION")
var=mem$1[5]
echo ${!var}
;;




*)
help_p
;;

esac


