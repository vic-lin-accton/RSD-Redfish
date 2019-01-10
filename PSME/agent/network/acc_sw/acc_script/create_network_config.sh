Oray_Config_File="NETWORK"

if [ -f /etc/config/NETWORK ];then
    rm /etc/config/NETWORK 
fi

MAX_PORT=`psme.sh  get max_port_num`

make_mgmt_network_if_conf()
{
echo "
config 'interface' 'ma$1'
    option 'ifname' 'eth$1'
    option 'proto' 'static'
    option 'ipaddr' '192.168.1.$1'
    option 'netmask' '255.255.255.0'
" > ${Oray_Config_File}
}


make_network_default_sw_port_conf()
{
echo "
config 'switch_port' 'sp$1'
    option port '$1'
    option pvid '1'
    option tag         '0'
    option speed       '10000'
    option duplex      'FD'
    option auto        '1'
    option framesize   '9412'
    option operate     '1'

" >> ${Oray_Config_File}
}

make_network_default_vlan_conf()
{
echo "
config 'switch_vlan' 'vlan$1'
   option 'port' '0x000000000000000000000000000000000000000000000000007fffffffffffff'
   option 'untag_port' '0x000000000000000000000000000000000000000000000000007fffffffffffff'
" >> ${Oray_Config_File}
}


make_network_if_conf()
{
echo "
config 'interface' 'knet$1'
   option 'ifname' 'eth$1'
   option 'proto' ''
   option 'ipaddr' ''
   option 'netmask' ''
" >> ${Oray_Config_File}
}


make_mgmt_network_if_conf 1
make_network_default_vlan_conf 1 

x=1
while [ $x -le $MAX_PORT ]
do
  make_network_if_conf $x
  make_network_default_sw_port_conf $x
  x=$(( $x + 1 ))
done



