#!/bin/bash
rm ../agent/storage/include/storage_config.hpp
rm -rf ../application/include/psme/rest/metadata/include-generated/
rm ../application/include/version.hpp
rm ../common/agent-framework/include/agent-framework/version.hpp

mv pre-inst-arm-pkgs.sh ../
mv pre-inst-X86-pkgs.sh ../
mv psme_release.sh ../
mv Build_all.sh ../
mv Build_ONL.sh ../
mv One_punch_build.sh ../
mv a.sh ../
cd ../
rm -rf build/*
mv pre-inst-arm-pkgs.sh build
mv pre-inst-X86-pkgs.sh build
mv psme_release.sh build
mv Build_all.sh build
mv Build_ONL.sh build  
mv One_punch_build.sh build
mv a.sh build
cd build

if [ "$1" != "C" ];then
    echo "Build all"
    if [ "$1" != "arm" ];then
	if [ "$1" == "volt" ];then
            echo "Build for volt platform!"
	    cp ../CMakeLists.txt ../CMakeLists.txt-org
	    `sed -i 's/-DONLP)/-DVOLT -DBCMOS_MSG_QUEUE_DOMAIN_SOCKET -DBCMOS_MSG_QUEUE_UDP_SOCKET -DBCMOS_MEM_CHECK &\nset(CUSE_ACC_BAL_DISTLIB "TRUE")/' ../CMakeLists.txt`
	fi
        echo "Build for x86 platform!"
        cmake ../
        grep -rl Werror . | grep flags.make | xargs sed -i 's/-Werror//g'
        #make unittest_psme-chassis_onlp
        #make unittest_psme-chassis_acc_api_bal_dist_test 
        make all 2>&1 | tee  onl.log    
	find ./ -name control  | xargs sed -i 's/armel/amd64/g'
	find ../tools/deb_maker/install/allinone-deb/psme-allinone/DEBIAN -name control  | xargs sed -i 's/armel/amd64/g'
        ./psme_release.sh
	cp ../CMakeLists.txt-org ../CMakeLists.txt
	rm ../CMakeLists.txt-org
    else
        echo "Build for arm platform!"
        mkdir pkgconfig
        cp /usr/lib/arm-linux-gnueabi/pkgconfig/*  pkgconfig
        #Build third party src and install
        cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/Platform/Linux-gcc-arm.cmake ..
        #Create Makefile
        cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/Platform/Linux-gcc-arm.cmake ..
        grep -rl Werror . | grep flags.make | xargs sed -i 's/-Werror//g'
	make psme-rest-server -j8 ;make psme-chassis -j8
	find ./ -name control  | xargs sed -i 's/amd64/armel/g'
	find ../tools/deb_maker/install/allinone-deb/psme-allinone/DEBIAN -name control  | xargs sed -i 's/amd64/armel/g'
        ./psme_release.sh $1
    fi

    echo "cp to home!!!!"
    cp ./install/allinone-deb/bin/psme-allinone.deb ~/
    echo "done!!!!"
fi
