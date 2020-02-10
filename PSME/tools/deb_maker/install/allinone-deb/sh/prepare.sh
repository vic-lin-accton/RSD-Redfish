#!/bin/bash
CUR_PATH=$(cd "$(dirname "$0")"; pwd)
PROJ_PATH=$CUR_PATH/../../../../
PSME_PROJ_PATH=$CUR_PATH/../../../
#For PSME install
PSME_ACC_SW_DIR="$PROJ_PATH/agent/network/acc_sw"

if [ "$1" = "arm" ];then
	LIB_PREINSTALL=/usr/lib/arm-linux-gnueabi
else
	LIB_PREINSTALL=/usr/lib/x86_64-linux-gnu
fi

function del_file()
{
    for del in $1*
    do
        if [ -f $del ]
        then
            rm -rf $del
        fi
    done
}

ITEM_PATH=$CUR_PATH/../psme-allinone

mkdir -p $ITEM_PATH/usr/local/bin
mkdir -p $ITEM_PATH/usr/local/sbin
mkdir -p $ITEM_PATH/usr/local/lib
mkdir -p $ITEM_PATH/etc
mkdir -p $ITEM_PATH/etc/logrotate.d
del_file $ITEM_PATH/usr/local/bin/
del_file $ITEM_PATH/usr/local/lib/
del_file $ITEM_PATH/etc/

mkdir -p $ITEM_PATH/etc/psme/certs   
mkdir -p $ITEM_PATH/broadcom   

cp ${PSME_ACC_SW_DIR}/psme.sh                   $ITEM_PATH/usr/local/bin
cp ${PSME_ACC_SW_DIR}/collect_mem_info.sh       $ITEM_PATH/usr/local/bin
cp ${PSME_ACC_SW_DIR}/collect_drv_info.sh	$ITEM_PATH/usr/local/bin
cp ${PSME_ACC_SW_DIR}/collect_cpu_util_info.sh	$ITEM_PATH/usr/local/bin
cp ${PSME_ACC_SW_DIR}/collect_mem_util_info.sh	$ITEM_PATH/usr/local/bin
cp ${PSME_ACC_SW_DIR}/collect_disk_util_info.sh	$ITEM_PATH/usr/local/bin
cp ${PSME_ACC_SW_DIR}/ipv6_status.sh		$ITEM_PATH/usr/local/bin
cp ${PSME_ACC_SW_DIR}/mgmt_protocol.sh		$ITEM_PATH/usr/local/bin
cp ${PSME_ACC_SW_DIR}/mgmt_vlan.sh		$ITEM_PATH/usr/local/bin
cp ${PSME_ACC_SW_DIR}/logsrv.sh  		$ITEM_PATH/usr/local/bin
cp ${PSME_ACC_SW_DIR}/mkpasswd                  $ITEM_PATH/usr/local/bin
cp ${PSME_ACC_SW_DIR}/Kafka_agent.py            $ITEM_PATH/usr/local/bin
cp ${PSME_ACC_SW_DIR}/Kafka_PM_agent.py         $ITEM_PATH/usr/local/bin
cp -rf ${PSME_ACC_SW_DIR}/mod_conf              $ITEM_PATH/usr/local/bin
cp ${PSME_ACC_SW_DIR}/HW_NODE_VM                $ITEM_PATH/etc/psme
cp ${PSME_ACC_SW_DIR}/HW_NODE_AS5916_54XM       $ITEM_PATH/etc/psme
cp ${PSME_ACC_SW_DIR}/HW_NODE_AS5916_54XKS       $ITEM_PATH/etc/psme
cp ${PSME_ACC_SW_DIR}/HW_NODE_AS7712_32X        $ITEM_PATH/etc/psme
cp ${PSME_ACC_SW_DIR}/HW_NODE_AS6812_32X        $ITEM_PATH/etc/psme
cp ${PSME_ACC_SW_DIR}/HW_NODE_AS6712_32X        $ITEM_PATH/etc/psme
cp ${PSME_ACC_SW_DIR}/HW_NODE_AS5912_54X        $ITEM_PATH/etc/psme
cp ${PSME_ACC_SW_DIR}/HW_NODE_AS5712_54X        $ITEM_PATH/etc/psme
cp ${PSME_ACC_SW_DIR}/HW_NODE_AS5812_54T        $ITEM_PATH/etc/psme
cp ${PSME_ACC_SW_DIR}/HW_NODE_AS5812_54X        $ITEM_PATH/etc/psme
cp ${PSME_ACC_SW_DIR}/HW_NODE_AS7816_64X        $ITEM_PATH/etc/psme
cp ${PSME_ACC_SW_DIR}/HW_NODE_ASXVOLT16         $ITEM_PATH/etc/psme
cp ${PSME_ACC_SW_DIR}/HW_NODE_ASGVOLT64         $ITEM_PATH/etc/psme
cp ${PSME_ACC_SW_DIR}/HW_NODE_AS7316_26XB       $ITEM_PATH/etc/psme
cp ${PSME_ACC_SW_DIR}/HW_NODE_AS7726_32X        $ITEM_PATH/etc/psme
cp ${PSME_ACC_SW_DIR}/HW_NODE_AS7326_56X        $ITEM_PATH/etc/psme
cp ${PSME_ACC_SW_DIR}/HW_NODE_AS4610_54T        $ITEM_PATH/etc/psme
cp ${PSME_ACC_SW_DIR}/HW_NODE_AS7926_80XK       $ITEM_PATH/etc/psme
cp ${PSME_ACC_SW_DIR}/HW_NODE_AS7926_40XKE      $ITEM_PATH/etc/psme
cp ${PSME_ACC_SW_DIR}/HW_NODE_AS9716_32D        $ITEM_PATH/etc/psme
cp ${PSME_ACC_SW_DIR}/HW_NODE_SAU5081           $ITEM_PATH/etc/psme
cp ${PSME_ACC_SW_DIR}/C_PINFO                   $ITEM_PATH/etc/psme
cp ${PSME_ACC_SW_DIR}/HW_TYPE                   $ITEM_PATH/etc/psme
cp ${PSME_ACC_SW_DIR}/sessions                  $ITEM_PATH/etc/psme
cp ${PSME_ACC_SW_DIR}/rfsrvlog                  $ITEM_PATH/etc/logrotate.d/

#Disable log service as default
touch ${PSME_ACC_SW_DIR}/nosrvlog
cp ${PSME_ACC_SW_DIR}/nosrvlog                  $ITEM_PATH/etc/psme/

if [ "$1" = "AS5812-54" ];then
    mkdir -p $ITEM_PATH/etc/bcm_sdk
    cp ${PSME_ACC_SW_DIR}/x_86_bcm_sdk/AS5812-54/*.sh  $ITEM_PATH/etc/bcm_sdk
    cp ${PSME_ACC_SW_DIR}/x_86_bcm_sdk/AS5812-54/netserve  $ITEM_PATH/etc/bcm_sdk

    #Add for Acc bin/script
    cp ${PSME_ACC_SW_DIR}/x_86_acc_bin/*               $ITEM_PATH/usr/local/bin
    cp -rf ${PSME_ACC_SW_DIR}/acc_script/*             $ITEM_PATH/usr/local/bin

    #Prepare UCI related files
    mkdir -p $ITEM_PATH/etc/config
    cp ${PSME_ACC_SW_DIR}/acc_script/etc/config/*      $ITEM_PATH/etc/config
    cp ${PSME_ACC_SW_DIR}/x_86_uci/libubox.so          $ITEM_PATH/usr/local/lib
    cp ${PSME_ACC_SW_DIR}/x_86_uci/libuci.so           $ITEM_PATH/usr/local/lib
    cp ${PSME_ACC_SW_DIR}/x_86_uci/uci                 $ITEM_PATH/usr/local/bin

    #Add for lldp ACCTON RSD ARCH. 
    cp ${PSME_ACC_SW_DIR}/x_86_lldp/liblldpctl.so.4    $ITEM_PATH/usr/local/lib
    cp ${PSME_ACC_SW_DIR}/x_86_lldp/lldpcli            $ITEM_PATH/usr/local/sbin
    cp ${PSME_ACC_SW_DIR}/x_86_lldp/lldpd              $ITEM_PATH/usr/local/sbin
    cp ${PSME_ACC_SW_DIR}/psme_rsd.conf                $ITEM_PATH/etc/psme/psme.conf 
else
    cp ${PSME_ACC_SW_DIR}/psme.conf                    $ITEM_PATH/etc/psme
fi

cp $PSME_PROJ_PATH/bin/psme-rest-server               $ITEM_PATH/usr/local/bin
cp $PSME_PROJ_PATH/bin/psme-chassis                   $ITEM_PATH/usr/local/bin
cp $PSME_PROJ_PATH/lib/libjsoncpp.so.999              $ITEM_PATH/usr/local/lib
cp $PSME_PROJ_PATH/lib/libjsonrpccpp-server.so.999    $ITEM_PATH/usr/local/lib
cp $PSME_PROJ_PATH/lib/libjsonrpccpp-client.so.999    $ITEM_PATH/usr/local/lib
cp $PSME_PROJ_PATH/lib/libjsonrpccpp-common.so.999    $ITEM_PATH/usr/local/lib

if [ "$1" = "arm" ];then
    cp $PSME_PROJ_PATH/lib/libuuid.so.16                 $ITEM_PATH/usr/local/lib
    cp $PSME_PROJ_PATH/lib/libuuid++.so.16               $ITEM_PATH/usr/local/lib
else
    cp $PSME_PROJ_PATH/bin/tests/unittest_psme-chassis_onlp $ITEM_PATH/usr/local/bin

    if [ -f "$PSME_PROJ_PATH/bin/tests/unittest_psme-chassis_acc_api_bal_dist_test" ];then
        cp $PSME_PROJ_PATH/bin/tests/unittest_psme-chassis_acc_api_bal_dist_test $ITEM_PATH/usr/local/bin
    fi

    if [ -f "$PSME_PROJ_PATH/bin/tests/unittest_psme-chassis_acc_api_bal3_dist_test" ];then
        cp $PSME_PROJ_PATH/bin/tests/unittest_psme-chassis_acc_api_bal3_dist_test $ITEM_PATH/broadcom
    fi

    cp ${PSME_ACC_SW_DIR}/onu_cfg                         $ITEM_PATH/broadcom
    cp ${PSME_ACC_SW_DIR}/rm_g_onu_cfg                    $ITEM_PATH/broadcom
    cp ${PSME_ACC_SW_DIR}/xgspon_unit_test_onu_cfg        $ITEM_PATH/broadcom
    cp ${PSME_ACC_SW_DIR}/gpon_unit_test_onu_cfg          $ITEM_PATH/broadcom
    cp $LIB_PREINSTALL/libossp-uuid.so.16                 $ITEM_PATH/usr/local/lib
    cp $LIB_PREINSTALL/libossp-uuid++.so.16               $ITEM_PATH/usr/local/lib
fi
#  For Accton Server
cp $LIB_PREINSTALL/libgnutls-deb0.so.28               $ITEM_PATH/usr/local/lib
cp $LIB_PREINSTALL/libnettle.so.4                     $ITEM_PATH/usr/local/lib
cp $LIB_PREINSTALL/libhogweed.so.2                    $ITEM_PATH/usr/local/lib
cp $LIB_PREINSTALL/libmicrohttpd.so.10                $ITEM_PATH/usr/local/lib
cp $LIB_PREINSTALL/libcurl.so.4                       $ITEM_PATH/usr/local/lib
cp $LIB_PREINSTALL/librtmp.so.1                       $ITEM_PATH/usr/local/lib
cp $LIB_PREINSTALL/libssh2.so.1                       $ITEM_PATH/usr/local/lib
cp ${PSME_ACC_SW_DIR}/server.key                      $ITEM_PATH/etc/psme/certs
cp ${PSME_ACC_SW_DIR}/server.crt                      $ITEM_PATH/etc/psme/certs


$CUR_PATH/modify_ver.sh $ITEM_PATH
