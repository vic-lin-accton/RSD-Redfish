#!/bin/bash
PIDSERVICE=`pidof netserve`
if [ "$PIDSERVICE" != "" ];then
    kill -9 $PIDSERVICE
fi

PIDBCM=`pidof example_drivshell`
if [ "$PIDBCM" != "" ];then
    kill -9 $PIDBCM
fi

RESULT=`lsmod | grep linux_bcm_knet -m1`
if [ "$RESULT" != "" ];then
    rmmod linux_bcm_knet
fi

RESULT=`lsmod | grep linux_user_bde -m1`
if [ "$RESULT" != "" ];then
    rmmod linux_user_bde
fi

RESULT=`lsmod | grep linux_kernel_bde -m1`
if [ "$RESULT" != "" ];then
    rmmod linux_kernel_bde
fi

service psme stop
