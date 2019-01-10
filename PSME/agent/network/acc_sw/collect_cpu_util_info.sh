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
	top5_cpu_util_a=($(ps -eo pid,pcpu,user,args | sort -k 2 -nr | head -5))
	a_num=${#top5_cpu_util_a[@]}

	if [ "$HW_type" = "arm-accton-as4610-54-r0" ];then
		echo todo
	else
		if [ -f /tmp/top5cputil ];then
			rm /tmp/top5cputil
		fi

		for (( i=0; i < ${a_num} ; i++ ))
		do 
			echo "${top5_cpu_util_a[$i]}" >> /tmp/top5cputil
		done
	fi
	top5_cpu_util_at=($(cat /tmp/top5cputil))
	echo ${#top5_cpu_util_at[@]}
;;

"INDEX")
	IFS=$'\n'
	ID=$(($2-1))
	top5_cpu_util_at=($(cat /tmp/top5cputil))
	echo ${top5_cpu_util_at[$ID]}
;;

esac

