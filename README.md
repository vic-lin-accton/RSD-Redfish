# Redfish for Edgecore bare-metal Switch/vOLT

Repository source provide Redfish API on Edgecore Networks' Switch/vOLT with a choice of open software of [NOS](https://github.com/opencomputeproject/OpenNetworkLinux).
Framework modified and based on Intel Rack Scale Design Software. 
For more details on Intel Rack Scale Design, please visit [Official Intel Rack Scale Design Project Website](http://intel.com/intelRSD).

# Intel Rack Scale Design Reference Software

Intel Rack Scale Design Software is a logical architecture that disaggregates compute, storage, and network resources, 
and introduces the ability to more efficiently pool and utilize these resources. 
Intel Rack Scale Design Software APIs simplify resource management and provide the ability to dynamically compose resources 
based on workload-specific demands.

More detailed information can be found at [official Intel Rack Scale Design Site](http://intel.com/intelRSD).

## How To Build PSME Redfish Service for Edgecore bare-metal devices

### Build NOS OpenNetworkLinux -

 Require host PC OS Ubuntu 16.04 TLS with container supported environment.
  
 How To Install Docker on Ubuntu 16.04
```  
$ sudo apt-get update
$ sudo apt-key adv --keyserver hkp://p80.pool.sks-keyservers.net:80 --recv-keys 58118E89F3A912897C070ADBF76221572C52609D
$ sudo apt-add-repository 'deb https://apt.dockerproject.org/repo ubuntu-xenial main'
$ sudo apt-get update
$ apt-cache policy docker-engine
$ sudo apt-get install -y docker-engine
$ sudo usermod -aG docker $(whoami)
```  
 How To Build ONL in Docker on Ubuntu 16.04
```    
$ cd ~
$ git clone https://github.com/opencomputeproject/OpenNetworkLinux.git 
$ cd OpenNetworkLinux
$ git checkout 72b95a7 
$ sudo docker/tools/onlbuilder -8
$ apt-cacher-ng
$ source setup.env
$ make amd64  2>&1 | tee  onl.log   
```    
### Build RSD -

 Git clone RSD source code.
  
```   
$ cd ~/OpenNetworkLinux
$ git clone https://github.com/edge-core/RSD-Redfish.git
$ cd RSD-Redfish/PSME/build/
```  
 Install necessary packages only at first time and start build
```  
$ ./pre-inst-X86-pkgs.sh
```  
 Build all
  
```  
$ ./Build_all.sh
``` 
 Output file will be in ~/OpenNetworkLinux/RSD-Redfish/PSME/build/install/allinone-deb/bin/psme-allinone.deb

 If want to partial build for any modification for PSME source.
``` 
$ cd ~/OpenNetworkLinux/RSD-Redfish/PSME/build/
$ make all; psme_release.sh;
``` 
 then will create new psme-allinone.deb

 Clean Build
``` 
$ ./Build_all.sh C
$ ./Build_all.sh
``` 

## Installing
cp psme-allinone.deb to Switch or vOLT ~/ directory and
```
dpkg -i psme-allinone.deb
```
## Start PSME Redfish service

```
$ service psme start
```

## Start PSME at boot time
```   
$ update-rc.d psme defaults    
```
And you can start to use PSME Redfish Service now.


## Do basic API query.

On vOLT side:

Assume vOLT's management IP is 172.17.10.9

On client side:

Use curl command to get basic information.
```
curl --insecure -v https://172.17.10.9:8888/redfish/v1/ | json_pp
```
Then you can get json data respones like:
```
{
   "EventService" : {
      "@odata.id" : "/redfish/v1/EventService"
   },
   "Description" : "Service Root description",
   "Managers" : {
      "@odata.id" : "/redfish/v1/Managers"
   },
   "Links" : {
      "Sessions" : {
         "@odata.id" : "/redfish/v1/SessionService/Sessions"
      }
   },
   "Chassis" : {
      "@odata.id" : "/redfish/v1/Chassis"
   },
   "AccountService" : {
      "@odata.id" : "/redfish/v1/AccountService"
   },
   "@odata.context" : "/redfish/v1/$metadata#ServiceRoot.ServiceRoot",
   "@odata.id" : "/redfish/v1",
   "Oem" : {
      "Intel_RackScale" : {
         "ApiVersion" : "2.0.0",
         "EthernetSwitches" : {
            "@odata.id" : "/redfish/v1/EthernetSwitches"
         },
         "@odata.type" : "#Intel.Oem.ServiceRoot"
      }
   },
   "SessionService" : {
      "@odata.id" : "/redfish/v1/SessionService"
   },
   "Registries" : {
      "@odata.id" : "/redfish/v1/Registries"
   },
   "Systems" : {
      "@odata.id" : "/redfish/v1/Systems"
   },
   "Id" : "RootService",
   "UUID" : "cec9c1d0-30e6-11e9-820c-53995703a902",
   "RedfishVersion" : "1.0.2",
   "Name" : "PSME Service Root",
   "@odata.type" : "#ServiceRoot.v1_1_1.ServiceRoot"
}
``` 

If want to stop / remvoe PSME service

## Stop PSME Redfish service

```
$ service psme stop
```
## Remove auto start PSME at boot time
```   
$ update-rc.d -f psme remove   
```
## Remvoe psme-allinone.deb package
```
$ dpkg -r psme-allinone
```

## Support on 

PSME current tested on ONL(OpenNetworkLinux) system. 
   
	+-----------------------------|--------------------+
	|            Platform         | ONL kernel 4.14.x  |
	|-----------------------------|--------------------|                 
	|x86-64-accton-as5712-54x     |        O           |         
	|-----------------------------|--------------------|                 
	|x86-64-accton-as5812-54t     |        O           |         
	|-----------------------------|--------------------|                 
	|x86-64-accton-as5812-54x     |        O           |         
	|-----------------------------|--------------------|                 
	|x86-64-accton-as5912-54x     |        O           |         
	|-----------------------------|--------------------|                 
	|x86-64-accton-as5916-54xk    |        O           |         
	|-----------------------------|--------------------|                 
	|x86-64-accton-as5916-54xks   |        O           |         
	|-----------------------------|--------------------|                 
	|x86-64-accton-as5916-54xm    |        O           |         
	|-----------------------------|--------------------|                 
	|x86-64-accton-as6712-32x     |        o           |         
	|-----------------------------|--------------------|                 
	|x86-64-accton-as6812-32x     |        o           |         
	|-----------------------------|--------------------|                 
	|x86-64-accton-as7312-54x     |        o           |         
	|-----------------------------|--------------------|                 
	|x86-64-accton-as7316-26xb    |        o           |         
	|-----------------------------|--------------------|                 
	|x86-64-accton-as7326-56x     |        o           |         
	|-----------------------------|--------------------|                 
	|x86-64-accton-as7712-32x     |        o           |         
	|-----------------------------|--------------------|                 
	|x86-64-accton-as7726-32x     |        o           |         
	|-----------------------------|--------------------|                 
	|x86-64-accton-as7816-64x     |        o           |         
	|-----------------------------|--------------------|                 
	|x86-64-accton-as7926-40xke   |        o           |         
	|-----------------------------|--------------------|                 
	|x86-64-accton-as7926-80xk    |        O           |         
	|-----------------------------|--------------------|                 
	|x86-64-accton-as9716-32d     |        o           |         
	|-----------------------------|--------------------|                 
	|x86-64-accton-asxvolt16      |        O           |         
	|-----------------------------|--------------------|                 
	|x86-64-accton-asgvolt64      |        O           |         
	+--------------------------------------------------+
