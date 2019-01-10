#!/bin/bash
ONL_KERN_VER_MAJOR="4.14"
ONL_KERN_VER_MINOR="49"
ONL_KERN_VER="${ONL_KERN_VER_MAJOR}.${ONL_KERN_VER_MINOR}"

BUILD_DIR="build"
OPENOLTDEVICE="asfvolt16"
BAL_DIR="${BUILD_DIR}/${OPENOLTDEVICE}-bal"

BAL_INC="-I./ -I${BAL_DIR}/bal_release/src/common/os_abstraction \
    -I${BAL_DIR}/bal_release/src/common/os_abstraction/posix \
    -I${BAL_DIR}/bal_release/src/common/config \
    -I${BAL_DIR}/bal_release/src/core/platform \
    -I${BAL_DIR}/bal_release/src/core/main \
    -I${BAL_DIR}/bal_release/src/common/include \
    -I${BAL_DIR}/bal_release/src/lib/libbalapi \
    -I${BAL_DIR}/bal_release/src/balapiend \
    -I${BAL_DIR}/bal_release/src/common/dev_log \
    -I${BAL_DIR}/bal_release/src/common/bal_dist_utils  \
    -I${BAL_DIR}/bal_release/src/lib/libtopology \
    -I${BAL_DIR}/bal_release/src/lib/libcmdline \
    -I${BAL_DIR}/bal_release/src/lib/libutils \
    -I${BAL_DIR}/bal_release/3rdparty/maple/sdk/host_driver/utils \
    -I${BAL_DIR}/bal_release/3rdparty/maple/sdk/host_driver/model \
    -I${BAL_DIR}/bal_release/3rdparty/maple/sdk/host_driver/api \
    -I${BAL_DIR}/bal_release/3rdparty/maple/sdk/host_reference/cli \
    "
g++ -O2 -std=c++11 -g -o unitest_acc_bal_api_dist unitest_acc_bal_api_dist.cpp acc_bal_api_dist_helper.cpp  -DBCMOS_MSG_QUEUE_DOMAIN_SOCKET -DBCMOS_MSG_QUEUE_UDP_SOCKET -DBCMOS_MEM_CHECK  ${BAL_INC} -L${BUILD_DIR} -lbal_api_dist
#g++ -O2 -std=c++11 -g -o unitest_acc_bal_api_dist acc_bal_api_dist_helper.c  -DBCMOS_MSG_QUEUE_DOMAIN_SOCKET -DBCMOS_MSG_QUEUE_UDP_SOCKET -DBCMOS_MEM_CHECK -DBCMOS_SYS_UNITTEST -DENABLE_LOG  ${BAL_INC} -L./build/asfvolt16-bal/bal_release/build/core/src/apps/bal_api_dist_shared_lib -lbal_api_dist
gcc -E acc_bal_api_dist_helper.cpp  -o acc_bal_api_dist_helper.i ${BAL_INC} -L${BUILD_DIR} -lbal_api_dist
cp unitest_acc_bal_api_dist ~/

