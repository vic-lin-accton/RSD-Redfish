grep -rl Werror . | grep flags.make | xargs sed -i 's/-Werror//g'
make all -j8
./psme_release.sh
cp ./install/allinone-deb/bin/psme-allinone.deb ~/
