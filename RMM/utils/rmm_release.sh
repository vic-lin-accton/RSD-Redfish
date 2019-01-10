#! /bin/bash

CUR_DIR=$PWD
TOP_DIR="$PWD/../"
BUILD_DIR="$PWD/../build/"
TOOLCHAIN_DIR="/usr/bin"
BUILD_FLAG="$PWD/../build/build_arm"

#For PSME install
PSME_ACC_SW_DIR="$PWD/../../PSME/agent/network/acc_sw"

/bin/mkdir -p ${BUILD_DIR}

if [ "$1" = "clean" ];then
	rm -rf ${BUILD_DIR}/* && exit -1
fi

if [ "$1" = "arm" ];then

	find ./ -name control  | xargs sed -i 's/amd64/armel/g'

	if [ -e $TOOLCHAIN_DIR ]; then
		echo "toolchain found in opt"
	else
		sudo tar -xvf tools/cc.tar.gz -C /opt || exit -1
	fi

	echo $PATH|grep -q -w "/usr/bin" || export PATH=$PATH:/usr/bin

	if [ -e $BUILD_FLAG ]; then
		cd ${BUILD_DIR} && export CC=/usr/bin/arm-linux-gnueabi-gcc && cmake .. -DARM=yes && make && make install || exit -1
	else
		cd ${BUILD_DIR} && rm -rf * && touch $BUILD_FLAG && export CC=/usr/bin/arm-linux-gnueabi-gcc && cmake .. -DARM=yes && make && make install || exit -1
	fi
else

	find ./ -name control  | xargs sed -i 's/armel/amd64/g'

	if [ -e $BUILD_FLAG ]; then
		cd ${BUILD_DIR} && rm -rf * && export CC= && cmake ..  && make && make install || exit -1
	else
		cd ${BUILD_DIR} && cmake ..  && export CC= && make && make install || exit -1
	fi
	rm -f $BUILD_FLAG
fi

SRC_DIR="$TOP_DIR/src"
DEB_TOOL_DIR="$TOP_DIR/utils/deb_maker"
DEB_PACKAGE_DIR="$TOP_DIR/build/install/multi-deb/bin"

RMM_RELEASE_DIR="$TOP_DIR/build/release"
if [ -e ${RMM_RELEASE_DIR} ]; then
	/bin/rm -rf ${RMM_RELEASE_DIR}
fi
/bin/mkdir -p ${RMM_RELEASE_DIR}

DEB_TOOL="$DEB_TOOL_DIR/gener_deb.sh $1"
VERSION=$(cat $SRC_DIR/VERSION)

cp ${PSME_ACC_SW_DIR}/psme.sh                ${BUILD_DIR}utils/cfg/ 
cp ${PSME_ACC_SW_DIR}/collect_mem_info.sh    ${BUILD_DIR}utils/cfg/ 
cp ${PSME_ACC_SW_DIR}/collect_drv_info.sh    ${BUILD_DIR}utils/cfg/ 
cp ${PSME_ACC_SW_DIR}/mkpasswd               ${BUILD_DIR}utils/cfg/ 
cp ${PSME_ACC_SW_DIR}/C_PINFO               ${BUILD_DIR}utils/cfg/ 
cp ${PSME_ACC_SW_DIR}/psme.conf             ${BUILD_DIR}utils/cfg/ 
cp ${PSME_ACC_SW_DIR}/HW_NODE_VM            ${BUILD_DIR}utils/cfg/ 
cp ${PSME_ACC_SW_DIR}/HW_NODE_AS5916_54XM   ${BUILD_DIR}utils/cfg/ 
cp ${PSME_ACC_SW_DIR}/HW_NODE_AS7712_32X    ${BUILD_DIR}utils/cfg/ 
cp ${PSME_ACC_SW_DIR}/HW_NODE_AS5912_54X    ${BUILD_DIR}utils/cfg/
cp ${PSME_ACC_SW_DIR}/HW_NODE_AS5712_54X    ${BUILD_DIR}utils/cfg/
cp ${PSME_ACC_SW_DIR}/HW_NODE_AS5812_54T    ${BUILD_DIR}utils/cfg/
cp ${PSME_ACC_SW_DIR}/HW_NODE_AS5812_54X    ${BUILD_DIR}utils/cfg/
cp ${PSME_ACC_SW_DIR}/HW_NODE_AS7816_64X    ${BUILD_DIR}utils/cfg/
cp ${PSME_ACC_SW_DIR}/HW_NODE_ASXVOLT16     ${BUILD_DIR}utils/cfg/ 
cp ${PSME_ACC_SW_DIR}/HW_NODE_AS4610_54T    ${BUILD_DIR}utils/cfg/ 
cp ${PSME_ACC_SW_DIR}/HW_NODE_SAU5081       ${BUILD_DIR}utils/cfg/ 

##For lldp
if [ "$1" = "arm" ];then
    mkdir -p ${BUILD_DIR}utils/cfg/open_src
    cp ${PSME_ACC_SW_DIR}/arm_lldp/liblldpctl.so.4         ${BUILD_DIR}utils/cfg/open_src 
    cp ${PSME_ACC_SW_DIR}/arm_lldp/lldpcli                 ${BUILD_DIR}utils/cfg/open_src 
    cp ${PSME_ACC_SW_DIR}/arm_lldp/lldpd                   ${BUILD_DIR}utils/cfg/open_src 
    cp ${PSME_ACC_SW_DIR}/arm_lldp/lldpd.conf              ${BUILD_DIR}utils/cfg/open_src 
    cp ${PSME_ACC_SW_DIR}/arm_lldp/authorized_keys         ${BUILD_DIR}utils/cfg/open_src
    
    mkdir -p ${BUILD_DIR}utils/cfg/bcmsdk
    cp ${PSME_ACC_SW_DIR}/arm_bcm_sdk/script/*    ${BUILD_DIR}utils/cfg/bcmsdk 
    #CP Accton Bin
    mkdir -p ${BUILD_DIR}utils/cfg/acc_bin
    cp ${PSME_ACC_SW_DIR}/arm_acc_bin/*                   ${BUILD_DIR}utils/cfg/acc_bin
    
else
    if [ "$1" = "AS5812-54" ];then
        # RSD PSME need client certificate
        cp ${PSME_ACC_SW_DIR}/psme_rsd.conf                    ${BUILD_DIR}utils/cfg/psme.conf 

        # CP private key 
        cp ${PSME_ACC_SW_DIR}/id_dsa        	               ${BUILD_DIR}utils/cfg/ 

        mkdir -p ${BUILD_DIR}utils/cfg/bcm_sdk
        cp ${PSME_ACC_SW_DIR}/x_86_bcm_sdk/AS5812-54/*    ${BUILD_DIR}utils/cfg/bcm_sdk 

        #CP Accton Bin
        mkdir -p ${BUILD_DIR}utils/cfg/acc_bin
        cp ${PSME_ACC_SW_DIR}/x_86_acc_bin/*                   ${BUILD_DIR}utils/cfg/acc_bin

        #CP Accton script 
        mkdir -p ${BUILD_DIR}utils/cfg/acc_script
        cp -rf ${PSME_ACC_SW_DIR}/acc_script/*                     ${BUILD_DIR}utils/cfg/acc_script

        #CP Open srouce related utility 
        mkdir -p ${BUILD_DIR}utils/cfg/open_src
        cp ${PSME_ACC_SW_DIR}/x_86_lldp/*                      ${BUILD_DIR}utils/cfg/open_src 
        cp ${PSME_ACC_SW_DIR}/x_86_uci/*                       ${BUILD_DIR}utils/cfg/open_src 
        #CP uci config 
    fi
fi 
#Server self-signed certificate and private key
cp ${PSME_ACC_SW_DIR}/server.crt     	    ${BUILD_DIR}utils/cfg/ 
cp ${PSME_ACC_SW_DIR}/server.key    	    ${BUILD_DIR}utils/cfg/ 

# generate deb packages
$DEB_TOOL

cd $DEB_PACKAGE_DIR
if [ "$1" = "arm" ];then
    cp ${PSME_ACC_SW_DIR}/arm_bcm_sdk/*.deb       ${DEB_PACKAGE_DIR}/
    cp ${PSME_ACC_SW_DIR}/arm_acc_bin/*.sh        ${DEB_PACKAGE_DIR}/
fi
#time_stamp=`date "+%Y%m%d%H%M%S"`
#deb_tar_name="rmm-$time_stamp.tar.gz"

deb_tar_name="rmm-bin-${VERSION}.tar.gz"
deb_bin_name="rmm-${VERSION}.bin"
deb_signed_bin_name="rmm-${VERSION}.signed.bin"
tar -zcf $RMM_RELEASE_DIR/$deb_bin_name rmm*.deb 
(cd $TOP_DIR/utils/tools/sign_image/ && make && ./img_sign $RMM_RELEASE_DIR/$deb_bin_name $RMM_RELEASE_DIR/$deb_signed_bin_name)
RELEASE_PACKAGE_DIR="rmm-bin-${VERSION}"
if [ -e $RELEASE_PACKAGE_DIR ]; then
	/bin/rm -rf ${RELEASE_PACKAGE_DIR}/*
else
	/bin/mkdir -p ${RELEASE_PACKAGE_DIR}
fi

cp -a rmm*.deb *.sh  ${RELEASE_PACKAGE_DIR}
tar -zcf $RMM_RELEASE_DIR/$deb_tar_name ${RELEASE_PACKAGE_DIR}
