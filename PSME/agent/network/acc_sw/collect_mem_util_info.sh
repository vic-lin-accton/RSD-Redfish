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
	top5_mem_util_a=($(ps -eo pid,pmem,vsize,rss,cmd | sort -k 3 -nr | head -5))
	a_num=${#top5_mem_util_a[@]}

	if [ "$HW_type" = "arm-accton-as4610-54-r0" ];then
		echo todo
	else
		if [ -f /tmp/top5memutil ];then
			rm /tmp/top5memutil
		fi

		for (( i=0; i < ${a_num} ; i++ ))
		do 
			echo "${top5_mem_util_a[$i]}" >> /tmp/top5memutil
		done
	fi
	top5_mem_util_at=($(cat /tmp/top5memutil))

	free_status=`free | grep Mem:`
	echo $free_status > /tmp/freemem
	echo ${#top5_mem_util_at[@]}
;;

"INDEX")
	IFS=$'\n'
	ID=$(($2-1))
	top5_mem_util_at=($(cat /tmp/top5memutil))
	echo ${top5_mem_util_at[$ID]}
;;

"FREE")
	#total       used       free     shared    buffers     cached
	FREE=`cat /tmp/freemem`
	echo $FREE
;;



esac

