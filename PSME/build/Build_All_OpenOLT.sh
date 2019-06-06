#/bin/bash

#####
# Put 3 files "Build_All_OpenOLT.sh" "Build_OpenOLT.sh" "softlink.sh"
# in Jenkins workspace directory.
####

TAG=$1

ContainerName="BAL30_RSD_213"

BAL30_RSD_213_CONTAINER=`docker ps -a | grep ${ContainerName} | awk '{print $1}'`

USERNAME=`whoami`

echo "USERNAME[$USERNAME]"

uid=`id -u`
gid=`id -g`
PWD=`pwd`

BUILD_AGENT_DIR=$PWD/openolt/agent
TOP_DIR=$PWD

echo "PWD[$PWD]"

if [ ! -d "openolt" ]; then

    git clone -b bal30-dev https://gerrit.opencord.org/openolt

    cd openolt;cd agent

    cp $TOP_DIR/../softlink.sh download

    cd download;./softlink.sh;cd ../

    mkdir build;cd build

    mkdir asfvolt16-onl;cd asfvolt16-onl

    git clone https://github.com/opencomputeproject/OpenNetworkLinux.git; 

    echo "NETAUTO=dhcp" >> OpenNetworkLinux/builds/amd64/installer/installed/builds/boot-config;
  
    cd OpenNetworkLinux/

    git checkout $1 

    git clone https://github.com/edge-core/RSD-Redfish.git

    cd RSD-Redfish/PSME/build

    cp $TOP_DIR/../Build_OpenOLT.sh ./

else

    cd openolt/agent/build/asfvolt16-onl/OpenNetworkLinux/RSD-Redfish/PSME/build

fi

if [ ! "$(docker ps -a | grep BAL30_RSD_213)" ];then 

    echo  "Do not have docker image, pull one!!!!"

    docker run --privileged -i -t -e DOCKER_IMAGE=opennetworklinux/builder8:1.10 --name ${ContainerName} -d -v /lib/modules:/lib/modules -v ${PWD}:${PWD} -e USER=${USERNAME} --net host -w ${PWD} -e HOME=/home/${USERNAME} -v /home/${USERNAME}:/home/${USERNAME} opennetworklinux/builder8:1.10 /bin/docker_shell --user ${USERNAME}:${gid} -c bash

    docker exec ${ContainerName} /bin/docker_shell -c "./pre-inst-X86-pkgs.sh"

    docker exec ${ContainerName} /bin/docker_shell -c "./Build_ONL.sh"

    docker exec ${ContainerName} /bin/docker_shell -c "./Build_OpenOLT.sh ;sudo chown -R ${USERNAME}:${gid} ${TOP_DIR}"

else


    [ ! "$(docker ps -a | grep BAL30_RSD_213)" ]  && docker start $ContainerName
    docker exec ${ContainerName} /bin/docker_shell -c "./Build_OpenOLT.sh ;sudo chown -R ${USERNAME}:${gid} ${TOP_DIR}"

fi
