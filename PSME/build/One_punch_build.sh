#!/bin/bash
ContainerName="RSD_213"
QEMU_CONTAINER=`docker ps -a | grep ${ContainerName} | awk '{print $1}'`

USERNAME=`whoami`
uid=`id -u`
gid=`id -g`
PWD=`pwd`

echo "USERNAME[$USERNAME]"

if [ "${QEMU_CONTAINER}" == "" ];then

    echo  "Do not have docker image, pull one!!!!"

    # do not have docker image, pull it.
    docker run --privileged -i -t -e DOCKER_IMAGE=opennetworklinux/builder8:1.9 --name ${ContainerName} -d -v /lib/modules:/lib/modules -v ${PWD}:${PWD} -e USER=${USERNAME} --net host -w ${PWD} -e HOME=/home/${USERNAME} -v /home/${USERNAME}:/home/${USERNAME} opennetworklinux/builder8:1.9 /bin/docker_shell --user ${USERNAME}:${gid} -c bash

    echo  "Build ONL first!!!!"
    docker exec ${ContainerName} /bin/docker_shell -c "./Build_ONL.sh"
    echo  "Pre install needed packages!!!!"
    docker exec ${ContainerName} /bin/docker_shell -c "./pre-inst-X86-pkgs.sh"
    echo  "Build PSME!!!!"
    docker exec ${ContainerName} /bin/docker_shell -c "./Build_all.sh;sudo chown -R ${USERNAME}:${gid} ../"
else
    echo  "Already have container, start build...!!!!"
    docker start ${ContainerName}
    docker exec ${ContainerName} /bin/docker_shell -c "./Build_all.sh;sudo chown -R ${USERNAME}:${gid} ../"
fi
