#!/bin/bash
rm ../agent/storage/include/storage_config.hpp
rm -rf ../application/include/psme/rest/metadata/include-generated/
rm ../application/include/version.hpp
rm ../common/agent-framework/include/agent-framework/version.hpp

mv pre-inst-X86-pkgs.sh ../
mv psme_release.sh ../
mv Build_all.sh ../
mv Build_ONL.sh ../
mv Build_BAL_SDK_ALL.sh ../
cd ../
rm -rf build/*
mv pre-inst-X86-pkgs.sh build
mv psme_release.sh build
mv Build_all.sh build
mv Build_ONL.sh build  
mv Build_BAL_SDK_ALL.sh build
cd build

if [ "$1" != "C" ];then
    echo "Build all"
    if [ "$1" == "bal" ];then
	echo "Build for latest bal sdk platform!"
	cp ../CMakeLists.txt ../CMakeLists.txt-org
	`sed -i 's/-DONLP)/-DBAL -DBCMOS_MSG_QUEUE_DOMAIN_SOCKET -DBCMOS_MSG_QUEUE_UDP_SOCKET -DBCMOS_MEM_CHECK  -DBCMOS_SYS_UNITTEST -DENABLE_LOG -DENABLE_CLI &\nset(CUSE_ACC_BAL3_DISTLIB "TRUE")/' ../CMakeLists.txt`
    fi

    cmake ../
    grep -rl Werror . | grep flags.make | xargs sed -i 's/-Werror//g'
    #make unittest_psme-chassis_onlp
    #make unittest_psme-chassis_acc_api_bal_dist_test 
    make all 2>&1 | tee  onl.log    
    if [ "$?" == 1 ];then
        echo "Build_all.sh error!!"
        exit 1
    fi

    find ./ -name control  | xargs sed -i 's/armel/amd64/g'
    find ../tools/deb_maker/install/allinone-deb/psme-allinone/DEBIAN -name control  | xargs sed -i 's/armel/amd64/g'
    ./psme_release.sh
    cp ../CMakeLists.txt-org ../CMakeLists.txt
    rm ../CMakeLists.txt-org
fi
