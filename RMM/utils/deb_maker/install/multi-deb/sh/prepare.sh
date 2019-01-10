#!/bin/bash
CUR_PATH=$(cd "$(dirname "$0")"; pwd)
PROJ_PATH=$CUR_PATH/../../../..
PSME_PROJ_PATH=$CUR_PATH/../../../../../PSME/build

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

#1
ITEM_PATH=$CUR_PATH/../rmm-base

mkdir -p $ITEM_PATH/usr/local/bin
mkdir -p $ITEM_PATH/usr/local/sbin
mkdir -p $ITEM_PATH/usr/local/lib
mkdir -p $ITEM_PATH/usr/local/etc  #Vic Added 

#for arm platform <-- 
if [ "$1" = "arm" ];then
mkdir -p $ITEM_PATH/root/.ssh
chmod 700 $ITEM_PATH/root/.ssh
cp $PROJ_PATH/build/utils/cfg/open_src/authorized_keys   $ITEM_PATH/root/.ssh
fi
#for arm platform --> 

if [ "$1" = "AS5812-54" ];then
mkdir -p $ITEM_PATH/root/.ssh
chmod 700 $ITEM_PATH/root/.ssh
cp $PROJ_PATH/build/utils/cfg/id_dsa   $ITEM_PATH/root/.ssh
fi



mkdir -p $ITEM_PATH/etc
mkdir -p $ITEM_PATH/etc/rmm/
mkdir -p $ITEM_PATH/etc/psme/certs   #Nick Added
mkdir -p $ITEM_PATH/etc/udev/rules.d
mkdir -p $ITEM_PATH/etc/stunnel
mkdir -p $ITEM_PATH/var/rmm/
mkdir -p $ITEM_PATH/var/rmm/redfish
del_file $ITEM_PATH/usr/local/bin/
del_file $ITEM_PATH/usr/local/lib/
del_file $ITEM_PATH/etc/rmm
del_file $ITEM_PATH/etc/udev/rules.d/
del_file $ITEM_PATH/etc/
del_file $ITEM_PATH/var/rmm/redfish
del_file $ITEM_PATH/etc/stunnel

#cp $PROJ_PATH/build/lib/*.so* $ITEM_PATH/usr/local/lib
cp $PROJ_PATH/build/lib/libptassdk.so $ITEM_PATH/usr/local/lib > /dev/null 2>&1
cp $PROJ_PATH/build/lib/librmm_wrap.so* $ITEM_PATH/usr/local/lib
cp $PROJ_PATH/build/lib/librmm_json.so* $ITEM_PATH/usr/local/lib
cp $PROJ_PATH/build/lib/librmm_cfg.so* $ITEM_PATH/usr/local/lib
cp $PROJ_PATH/build/lib/librmm_encrypter.so* $ITEM_PATH/usr/local/lib
cp $PROJ_PATH/build/lib/librmm_redfish.so* $ITEM_PATH/usr/local/lib
cp $PROJ_PATH/build/lib/librmm_utils.so* $ITEM_PATH/usr/local/lib
cp $PROJ_PATH/build/lib/librmm_securec.so* $ITEM_PATH/usr/local/lib
cp $PROJ_PATH/build/lib/librmm_log.so* $ITEM_PATH/usr/local/lib
#cp $PROJ_PATH/build/lib/libcurl.so* $ITEM_PATH/usr/local/lib
#cp $PROJ_PATH/build/lib/libuuid.so* $ITEM_PATH/usr/local/lib
cp $PROJ_PATH/build/lib/librmm_init.so* $ITEM_PATH/usr/local/lib
cp $PROJ_PATH/build/lib/librmm_jsonrpc.so* $ITEM_PATH/usr/local/lib
cp $PROJ_PATH/build/lib/librmm_jsonrpcapi.so* $ITEM_PATH/usr/local/lib
#cp $PROJ_PATH/build/lib/libssl.so* $ITEM_PATH/usr/local/lib
#cp $PROJ_PATH/build/lib/libcrypto.so* $ITEM_PATH/usr/local/lib
cp $PROJ_PATH/build/utils/redfish/MR.json $ITEM_PATH/var/rmm/redfish
cp $PROJ_PATH/build/utils/cfg/rmm.cfg $ITEM_PATH/etc/rmm
cp $PROJ_PATH/build/utils/cfg/pwm_rpm.cfg $ITEM_PATH/etc/rmm
cp $PROJ_PATH/build/utils/encrypt_text/keyfile $ITEM_PATH/etc/rmm
cp $PROJ_PATH/src/VERSION $ITEM_PATH/etc/rmm

cp $PROJ_PATH/build/bin/ptasd $ITEM_PATH/usr/local/bin/
cp $PROJ_PATH/build/bin/OEM_coe_file $ITEM_PATH/var/rmm/
cp $PROJ_PATH/build/bin/Memdbd $ITEM_PATH/usr/local/bin/
cp $PROJ_PATH/build/bin/IPMIModule $ITEM_PATH/usr/local/bin
cp $PROJ_PATH/build/bin/Assetd $ITEM_PATH/usr/local/bin
cp $PROJ_PATH/build/bin/AssetModule $ITEM_PATH/usr/local/bin
cp $PROJ_PATH/build/bin/Registerd $ITEM_PATH/usr/local/bin
cp $PROJ_PATH/build/bin/RMMLogd $ITEM_PATH/usr/local/bin
cp $PROJ_PATH/build/bin/Upgraded $ITEM_PATH/usr/local/bin
cp $PROJ_PATH/build/bin/SNMPSubagentd $ITEM_PATH/usr/local/bin
cp $PROJ_PATH/build/bin/cm_reset $ITEM_PATH/usr/local/bin
cp $PROJ_PATH/build/bin/snmpd $ITEM_PATH/usr/local/bin
cp $PROJ_PATH/build/bin/stunnel $ITEM_PATH/usr/local/bin
cp $PROJ_PATH/build/bin/rmm_post_install $ITEM_PATH/usr/local/bin
cp $PROJ_PATH/build/bin/rmm_super $ITEM_PATH/usr/local/bin


#Nick Add for PSME 
cp $PROJ_PATH/build/utils/cfg/psme.sh                  $ITEM_PATH/usr/local/bin
cp $PROJ_PATH/build/utils/cfg/collect_mem_info.sh     $ITEM_PATH/usr/local/bin
cp $PROJ_PATH/build/utils/cfg/collect_drv_info.sh     $ITEM_PATH/usr/local/bin
cp $PROJ_PATH/build/utils/cfg/mkpasswd                 $ITEM_PATH/usr/local/bin
cp $PROJ_PATH/build/utils/cfg/HW_NODE_VM              $ITEM_PATH/etc/psme
cp $PROJ_PATH/build/utils/cfg/HW_NODE_AS5916_54XM     $ITEM_PATH/etc/psme
cp $PROJ_PATH/build/utils/cfg/HW_NODE_AS7712_32X      $ITEM_PATH/etc/psme
cp $PROJ_PATH/build/utils/cfg/HW_NODE_AS5912_54X      $ITEM_PATH/etc/psme
cp $PROJ_PATH/build/utils/cfg/HW_NODE_AS5712_54X      $ITEM_PATH/etc/psme
cp $PROJ_PATH/build/utils/cfg/HW_NODE_AS5812_54T      $ITEM_PATH/etc/psme
cp $PROJ_PATH/build/utils/cfg/HW_NODE_AS5812_54X      $ITEM_PATH/etc/psme
cp $PROJ_PATH/build/utils/cfg/HW_NODE_AS7816_64X      $ITEM_PATH/etc/psme
cp $PROJ_PATH/build/utils/cfg/HW_NODE_ASXVOLT16       $ITEM_PATH/etc/psme
cp $PROJ_PATH/build/utils/cfg/HW_NODE_AS4610_54T      $ITEM_PATH/etc/psme
cp $PROJ_PATH/build/utils/cfg/HW_NODE_SAU5081         $ITEM_PATH/etc/psme
cp $PROJ_PATH/build/utils/cfg/C_PINFO                 $ITEM_PATH/etc/psme


if [ "$1" = "AS5812-54" ];then
    #Add for Brcm OPENNSL 
    mkdir -p $ITEM_PATH/etc/bcm_sdk
    cp $PROJ_PATH/build/utils/cfg/bcm_sdk/*           $ITEM_PATH/etc/bcm_sdk

    #Add for Acc bin/script
    cp $PROJ_PATH/build/utils/cfg/acc_bin/*           $ITEM_PATH/usr/local/bin
    cp $PROJ_PATH/build/utils/cfg/acc_script/*        $ITEM_PATH/usr/local/bin

    #Prepare UCI related files
    mkdir -p $ITEM_PATH/etc/config
    cp $PROJ_PATH/build/utils/cfg/acc_script/etc/config/*    $ITEM_PATH/etc/config

    cp $PROJ_PATH/build/utils/cfg/open_src/libubox.so        $ITEM_PATH/usr/local/lib
    cp $PROJ_PATH/build/utils/cfg/open_src/libuci.so         $ITEM_PATH/usr/local/lib
    cp $PROJ_PATH/build/utils/cfg/open_src/uci               $ITEM_PATH/usr/local/bin
#Add for lldp ACCTON RSD ARCH. 
    cp $PROJ_PATH/build/utils/cfg/open_src/liblldpctl.so.4         $ITEM_PATH/usr/local/lib
    cp $PROJ_PATH/build/utils/cfg/open_src/lldpcli                 $ITEM_PATH/usr/local/sbin
    cp $PROJ_PATH/build/utils/cfg/open_src/lldpd                   $ITEM_PATH/usr/local/sbin
fi

if [ "$1" = "arm" ];then
    cp $PROJ_PATH/build/utils/cfg/open_src/lldpd.conf              $ITEM_PATH/usr/local/etc
    cp $PROJ_PATH/build/utils/cfg/open_src/liblldpctl.so.4         $ITEM_PATH/usr/local/lib
    cp $PROJ_PATH/build/utils/cfg/open_src/lldpcli                 $ITEM_PATH/usr/local/sbin
    cp $PROJ_PATH/build/utils/cfg/open_src/lldpd                   $ITEM_PATH/usr/local/sbin
    mkdir -p $ITEM_PATH/etc/bcmsdk
    cp $PROJ_PATH/build/utils/cfg/bcmsdk/*            $ITEM_PATH/etc/bcmsdk
    cp $PROJ_PATH/build/utils/cfg/acc_bin/*           $ITEM_PATH/usr/local/bin
fi

cp $PSME_PROJ_PATH/bin/psme-rest-server               $ITEM_PATH/usr/local/bin
#cp $PSME_PROJ_PATH/bin/psme-network-stubs            $ITEM_PATH/usr/local/bin
cp $PSME_PROJ_PATH/bin/psme-chassis                   $ITEM_PATH/usr/local/bin
cp $PSME_PROJ_PATH/lib/libjsoncpp.so.999              $ITEM_PATH/usr/local/lib
cp $PSME_PROJ_PATH/lib/libjsonrpccpp-server.so.999    $ITEM_PATH/usr/local/lib
cp $PSME_PROJ_PATH/lib/libjsonrpccpp-client.so.999    $ITEM_PATH/usr/local/lib
cp $PSME_PROJ_PATH/lib/libjsonrpccpp-common.so.999    $ITEM_PATH/usr/local/lib
cp $LIB_PREINSTALL/libossp-uuid.so.16                 $ITEM_PATH/usr/local/lib
cp $LIB_PREINSTALL/libossp-uuid++.so.16               $ITEM_PATH/usr/local/lib

#  For Accton Server
cp $LIB_PREINSTALL/libgnutls-deb0.so.28               $ITEM_PATH/usr/local/lib
cp $LIB_PREINSTALL/libnettle.so.4                     $ITEM_PATH/usr/local/lib
cp $LIB_PREINSTALL/libhogweed.so.2                    $ITEM_PATH/usr/local/lib


#for arm platform <-- 
if [ "$1" = "arm" ];then
cp $PSME_PROJ_PATH/lib/libuuid.so.16                      $ITEM_PATH/usr/local/lib
cp $PSME_PROJ_PATH/lib/libuuid++.so.16                    $ITEM_PATH/usr/local/lib
fi
#for arm platform --> 

cp $LIB_PREINSTALL/libmicrohttpd.so.10                $ITEM_PATH/usr/local/lib
cp $LIB_PREINSTALL/libcurl.so.4                       $ITEM_PATH/usr/local/lib
cp $LIB_PREINSTALL/librtmp.so.1                       $ITEM_PATH/usr/local/lib
cp $LIB_PREINSTALL/libssh2.so.1                       $ITEM_PATH/usr/local/lib
cp $PROJ_PATH/build/utils/cfg/server.key              $ITEM_PATH/etc/psme/certs
cp $PROJ_PATH/build/utils/cfg/server.crt              $ITEM_PATH/etc/psme/certs
cp $PROJ_PATH/build/utils/cfg/psme.conf               $ITEM_PATH/etc/psme

cp $PROJ_PATH/build/utils/cfg/80-persistent-tty.rules $ITEM_PATH/etc/udev/rules.d/
cp -r $PROJ_PATH/src/core/restd/web $ITEM_PATH/var/rmm/
cp $PROJ_PATH/src/core/restd/stunnel.conf $ITEM_PATH/etc/stunnel/
cp $PROJ_PATH/src/core/restd/stunnel.pem $ITEM_PATH/etc/stunnel/
cp $PROJ_PATH/src/core/restd/key.pub $ITEM_PATH/etc/stunnel/
cp $PROJ_PATH/src/core/restd/podm.cert                $ITEM_PATH/etc/rmm/

#cp $PROJ_PATH/build/bin/in.tftpd $ITEM_PATH/usr/local/sbin/
$CUR_PATH/modify_ver.sh $ITEM_PATH

#2
ITEM_PATH=$CUR_PATH/../rmm-api

mkdir -p $ITEM_PATH/usr/local/bin
del_file $ITEM_PATH/usr/local/bin/
cp $PROJ_PATH/build/bin/Restd $ITEM_PATH/usr/local/bin/
cp $PROJ_PATH/build/bin/Redfishd $ITEM_PATH/usr/local/bin/
$CUR_PATH/modify_ver.sh $ITEM_PATH


#3
ITEM_PATH=$CUR_PATH/../rmm-consolecontrol

mkdir -p $ITEM_PATH/usr/local/bin
del_file $ITEM_PATH/usr/local/bin/
cp $PROJ_PATH/build/utils/tools/set_puid.py $ITEM_PATH/usr/local/bin

cp $PROJ_PATH/build/bin/dumpmemdb $ITEM_PATH/usr/local/bin
$CUR_PATH/modify_ver.sh $ITEM_PATH

#4
ITEM_PATH=$CUR_PATH/../rmm-all
$CUR_PATH/modify_ver.sh $ITEM_PATH

#5
ITEM_PATH=$CUR_PATH/../net-snmp

mkdir -p $ITEM_PATH/usr/local/bin
del_file $ITEM_PATH/usr/local/bin/

mkdir -p $ITEM_PATH/etc/snmp
del_file $ITEM_PATH/etc/snmp/

mkdir -p $ITEM_PATH/usr/local/lib
del_file $ITEM_PATH/usr/local/lib/

#mkdir -p $ITEM_PATH/usr/local/share/snmp/mibs
#del_file $ITEM_PATH/usr/local/share/snmp/mibs

cp $PROJ_PATH/build/lib/libnet* $ITEM_PATH/usr/local/lib
cp $PROJ_PATH/build/bin/snmp* $ITEM_PATH/usr/local/bin/
#copy snmp config file
cp $PROJ_PATH/src/core/snmp_subagentd/config/*.conf $ITEM_PATH/etc/snmp/
#copy mibs file
#cp $PROJ_PATH/src/core/snmp_subagentd/*.txt $ITEM_PATH/usr/local/share/snmp/mibs/
#cp $PROJ_PATH/src/dep/snmp/net-snmp-5.7.3/mibs/*.txt $ITEM_PATH/usr/local/share/snmp/mibs/
#add env 
#export MIBDIRS="/usr/local/share/snmp/mibs"
$CUR_PATH/modify_ver.sh $ITEM_PATH
