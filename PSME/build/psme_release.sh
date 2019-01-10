#!/bin/bash

TOP_DIR="$PWD/.."
PSME_RELEASE_DIR="$TOP_DIR/build/release"
PSME_INSTALL_DIR="$TOP_DIR/build/install"
PSME_ACC_SW_DIR="$TOP_DIR/agent/network/acc_sw"
DEB_PACKAGE_DIR="$TOP_DIR/build/install/allinone-deb/bin"
DEB_TOOL="$TOP_DIR/build/gener_deb.sh $1"
BUILD_DIR="$TOP_DIR/build"


ver=$(grep VER_STRING $TOP_DIR/common/agent-framework/Version.cmake| awk 'BEGIN {FS="\""}; {print $2}' )

if [ -e ${PSME_RELEASE_DIR} ]; then
	/bin/rm -rf ${PSME_RELEASE_DIR}
fi

if [ -e ${PSME_INSTALL_DIR} ]; then
	/bin/rm -rf ${PSME_INSTALL_DIR}
fi

/bin/mkdir -p ${PSME_RELEASE_DIR}

cp -rf $TOP_DIR/tools/deb_maker/* $TOP_DIR/build

$DEB_TOOL

cp -a ${PSME_ACC_SW_DIR}/x_86_dmidecode/*.deb      		 ${DEB_PACKAGE_DIR}/

if [ "$1" = "AS5812-54" ];then
	cp ${PSME_ACC_SW_DIR}/x_86_bcm_sdk/AS5812-54/opennsl/*.deb       ${DEB_PACKAGE_DIR}/
fi


deb_tar_name="psme-bin-${ver}.tar.gz"
cd $DEB_PACKAGE_DIR

tar -zcf $PSME_RELEASE_DIR/$deb_tar_name *
