#!/bin/bash

BUILD_DIR="../../../../../../../"
OPENOLTDEVICE="asfvolt16"
BAL_DIR="${BUILD_DIR}/${OPENOLTDEVICE}-bal"
BAL3_DIR="${BUILD_DIR}/bal-api-3.1"
echo "BAL_3DIR[$BAL3_DIR]"
echo "BAL_DIR[$BAL_DIR]"

#BAL_INC="-I./ -I${BAL_DIR}/host_driver/bal/bal_include  \
BAL_INC="-I${BAL_DIR}/host_driver/bal/bal_include  \
    -I${BAL_DIR}/host_driver/topology  \
    -I${BAL_DIR}/host_driver/utils  \
    -I${BAL_DIR}/host_driver/api  \
    -I${BAL_DIR}/host_driver/transport/  \
    -I${BAL_DIR}/host_customized/os_abstraction \
    -I${BAL_DIR}/host_customized/os_abstraction/posix \
    -I${BAL_DIR}/host_customized/config \
    -I${BAL_DIR}/host_driver/api_conn_mgr  \
    -I${BAL_DIR}/host_driver/conn_mgr \
    -I${BAL_DIR}/host_driver/system_types \
    -I${BAL_DIR}/host_driver/api/host/topology  \
    -I${BAL_DIR}/host_reference/cli  \
    -I${BAL_DIR}/host_reference/api_cli  \
    -I${BAL_DIR}/host_reference/dev_log  \
    -I${BAL_DIR}/host_reference/host_api/  \
    -I${BAL_DIR}/lib/cmdline \
    "
g++ -g -O2 -I/usr/local/include -std=c++11 -o unitest_acc_bal3_api_dist unitest_acc_bal3_api_dist.cpp acc_bal3_api_dist_helper.cpp  -DBCMOS_MSG_QUEUE_DOMAIN_SOCKET -DBCMOS_MSG_QUEUE_UDP_SOCKET -DBCMOS_MEM_CHECK  -DBCMOS_SYS_UNITTEST -DENABLE_LOG -DENABLE_CLI  -DBAL32 ${BAL_INC} -L${BUILD_DIR}/asfvolt16-bal/build/host_reference/host_api -L/usr/local/lib  -lbal_host_api -ldl

scp unitest_acc_bal3_api_dist root@172.17.10.7:/broadcom


#g++ -g -O2 -I/usr/local/include -fpermissive -Wno-literal-suffix -std=c++11 -o unitest_acc_bal3_api_dist unitest_acc_bal3_api_dist.cpp acc_bal3_api_dist_helper.cpp  -DBCMOS_MSG_QUEUE_DOMAIN_SOCKET -DBCMOS_MSG_QUEUE_UDP_SOCKET -DBCMOS_MEM_CHECK  -DBCMOS_SYS_UNITTEST -DENABLE_LOG -DENABLE_CLI  ${BAL_INC} -L${BUILD_DIR}/asfvolt16-bal/build/host_reference/host_api -L/usr/local/lib  -lbal_host_api -ldl
#g++ -O2 -std=c++11 -g -o unitest_acc_bal_api_dist acc_bal_api_dist_helper.c  -DBCMOS_MSG_QUEUE_DOMAIN_SOCKET -DBCMOS_MSG_QUEUE_UDP_SOCKET -DBCMOS_MEM_CHECK -DBCMOS_SYS_UNITTEST -DENABLE_LOG  ${BAL_INC} -L./build/asfvolt16-bal/bal_release/build/core/src/apps/bal_api_dist_shared_lib -lbal_api_dist
#gcc -E acc_bal_api_dist_helper.cpp  -o acc_bal_api_dist_helper.i ${BAL_INC} -L${BUILD_DIR} -lbal_api_dist
#cp unitest_acc_bal_api_dist ~/

