#!/bin/bash

PlatformPath="/etc/onl/platform"

if [ -f "${PlatformPath}" ];then
	HW_type=`cat  ${PlatformPath}`
else
	HW_type=`/usr/local/bin/sonic-cfggen -v platform`
fi

case "$1" in

"COUNT")
	IFS=$'\n'
	disk_util_a=($( df -h | grep '/dev/sd.'))
	a_num=${#disk_util_a[@]}

	if [ "$HW_type" = "arm-accton-as4610-54-r0" ];then
		echo todo
	else
		if [ -f /tmp/disk_util ];then
			rm /tmp/disk_util
		fi

		for (( i=0; i < ${a_num} ; i++ ))
		do 
			echo "${disk_util_a[$i]}" >> /tmp/disk_util
		done
	fi
	disk_util_at=($(cat /tmp/disk_util))

	free_status=`free | grep Mem:`
	echo $free_status > /tmp/freemem
	echo ${#disk_util_at[@]}
;;

"INDEX")
	IFS=$'\n'
	ID=$(($2-1))
	disk_util_at=($(cat /tmp/disk_util))
	echo ${disk_util_at[$ID]}
;;
esac

