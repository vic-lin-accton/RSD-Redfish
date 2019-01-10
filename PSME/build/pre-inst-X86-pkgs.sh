#!/bin/bash

#Install required packages

echo ""
echo "Installing dependency Packages"
echo ""

sudo apt-get update

PACKAGES=("clang" "libgcrypt20-dev" "libncurses5-dev" "libnl-3-dev" "libudev-dev" "libglibmm-2.4-dev" "libglib3.0-cil-dev"
          "libxml++2.6-dev" "libgnutls28-dev" "libnl-route-3-dev" "flex" "bison" "doxygen" "cpp" "ccache" "build-essential"
	  "linux-libc-dev" "libmpc-dev" "libstdc++6" "libcurl4-openssl-dev" "libmicrohttpd-dev" "libjsoncpp-dev"
          "lcov" "libossp-uuid-dev" "libnl-route-3-200" "libsysfs-dev" "libpopt-dev" "libusb-dev" "patch" "libdevmapper-dev" 
	  "liblvm2-dev" "unzip" "libnl-genl-3-dev" "libblkid-dev" "debsigs" "debsig-verify" "gnupg" "ipmitool"
          )

count="${#PACKAGES[@]}"	  

for (( c=0; c < "${count}"; c++ ))
do
    PK_NAME=${PACKAGES[$c]}
    echo "install PK_NAME[$PK_NAME]"
    RES=`sudo apt-get install "$PK_NAME" --yes`
    echo "RES[$RES]"
done

sudo apt-get install -f

#Install cmake version to  3.5.2
CMAKE_VER=`cmake -version | grep "cmake version"  | awk '{print $3}'`
if [ "$CMAKE_VER" != "3.5.2" ]; then
    sudo apt-get purge cmake
    wget https://cmake.org/files/v3.5/cmake-3.5.2.tar.gz
    tar xzvf cmake-3.5.2.tar.gz
    cd cmake-3.5.2/
    ./bootstrap
    make -j4
    sudo make install
    if [ -e /usr/bin/cmake ]; then
	sudo rm /usr/bin/cmake
    fi
    sudo ln -s /usr/local/bin/cmake /usr/bin/cmake
fi
