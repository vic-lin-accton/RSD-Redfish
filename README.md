# Redfish for Edgecore bare-metal Switch/VOLT

Repository source provide Redfish API on Edgecore Networks' Switch/VOLT with a choice of open software of [NOS](https://github.com/opencomputeproject/OpenNetworkLinux).
Framework modified and based on IntelR Rack Scale Design Software. 
For more details on IntelR Rack Scale Design, please visit [Official IntelR Rack Scale Design Project Website]
(http://intel.com/intelRSD).
**Keep in mind this code is reference software only.** 
It is expected that developers will take this reference software and make it their own. 

# IntelR Rack Scale Design Reference Software

IntelR Rack Scale Design Software is a logical architecture that disaggregates compute, storage, and network resources, 
and introduces the ability to more efficiently pool and utilize these resources. 
IntelR Rack Scale Design Software APIs simplify resource management and provide the ability to dynamically compose resources 
based on workload-specific demands.

More detailed information can be found at [official IntelR Rack Scale Design Site](http://intel.com/intelRSD).

## How to build

### Prerequisites
- ONL(OpenNetworkLinux) docker bulid environment and well built ONLP library.

  To get peripherals information from hardware device,excution file need link with ONLP library. 
  
- OpenOLT adapter bal_api_dist library.(Optional)

  VOLT platform need build with bal_api_dist library to support port statistics information.
  
## Build For Edgecore bare-metal Switch

Enter ONL build docker environment
```
$ cd OpenNetworkLinux
$ git clone https://github.com/edge-core/RSD-Redfish.git
$ cd RSD-Redfish/PSME/build/
```
Install necessary packages only at first time and start build
```
$ ./pre-inst-X86-pkgs.sh
$ ./Build_all.sh
```

## Build For Edgecore VOLT

Enter ONL build docker environment in openolt 

Currentlly, only support [tag voltha-1.4.0](https://github.com/opencord/openolt.git)
```
$ cd openolt/build/asfvolt16-onl/OpenNetworkLinux
$ git clone https://github.com/edge-core/RSD-Redfish.git
$ cd RSD-Redfish/PSME/build/
```
Install necessary packages only at first time and start build
```
$ ./pre-inst-X86-pkgs.sh
$ ./Build_all.sh volt
```
Output file will be in ~/psme-allinone.deb

## Installing
cp psme-allinone.deb to Switch/VOLT ~/ and
```
dpkt -i psme-allinone.deb
```
## Starting/Stopping PSME (Redfish) service
```
service psme start/stop
```
