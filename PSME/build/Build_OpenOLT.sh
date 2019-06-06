#!/bin/bash
cd ../../../../../../

rm Makefile
rm configure.ac
rm config.log

./configure
 
sed -i 's/ onl //g'     Makefile
sed -i 's/clang//g'     Makefile
sed -i 's/docker.io//g' Makefile

make OPENOLTDEVICE=asfvolt16 prereq
make OPENOLTDEVICE=asfvolt16 2>&1 | tee openolt.log

cd build
mkdir -p lib/x86_64-linux-gnu/
mkdir -p usr/local/bin/openolt

cp /usr/local/lib/libgpr.so.6                                        lib/x86_64-linux-gnu/
cp /usr/local/lib/libprotobuf.so.15                                  lib/x86_64-linux-gnu/
cp /usr/local/lib/libgrpc++.so.1                                     lib/x86_64-linux-gnu/
cp /usr/local/lib/libgrpc.so.6                                       lib/x86_64-linux-gnu/
cp asfvolt16-bal/build/host_driver/bal/stub/libbal_core.so           lib/x86_64-linux-gnu/
cp asfvolt16-bal/build/host_reference/host_api/libbal_host_api.so    lib/x86_64-linux-gnu/

cp openolt           usr/local/bin/openolt
cp dev_mgmt_daemon   usr/local/bin/openolt

rm release_asfvolt16_V3.0.3.201904171432.tar.gz
cp asfvolt16-bal/build/fs/asfvolt16/release/release_asfvolt16_V3.0.3.201904171432.tar.gz ./

tar zcvf openolt.tar.gz release_asfvolt16_V3.0.3.201904171432.tar.gz lib/x86_64-linux-gnu/ usr/local/bin/openolt
