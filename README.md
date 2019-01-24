# Redfish for Edgecore bare-metal Switch/vOLT

Repository source provide Redfish API on Edgecore Networks' Switch/vOLT with a choice of open software of [NOS](https://github.com/opencomputeproject/OpenNetworkLinux).
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

  Need pre-build OpenOLT agent library bal_api_dist.
  
  vOLT platform need build with bal_api_dist library to support port statistics information.
  
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

## Build For Edgecore vOLT

Enter ONL build docker environment in openolt 

Currentlly, support OpenOLT agent with [tag voltha-1.4.0](https://github.com/opencord/openolt.git)

Change to openolt built directory that has built out bal_api_dist library.
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
cp psme-allinone.deb to Switch or vOLT ~/ directory and
```
dpkt -i psme-allinone.deb
```
## Starting/Stopping PSME (Redfish) service

For Edgecore bare-metal Switch

```
service psme start

```
For Edgecore vOLT

After openolt agent start ...then

```
service psme start
```
If want stop PSME agent

```
service psme stop
```
## Support on 

|            Platform        | ONL kernel 3.x |ONL kernel 4.14.49|
|----------------------------|----------------|------------------|
x86-64-accton-as7712-32x-r0  |        O       |        O                 
x86-64-accton-as5916-54xm-r0 |        O       |        O                  
x86-64-accton-as5912-54x-r0  |        O       |        O         
x86-64-accton-as5712-54x-r0  |        O       |        X         
x86-64-accton-as5812-54t-r0  |        O       |        O         
x86-64-accton-as5812-54x-r0  |        O       |        O                      
x86-64-accton-asxvolt16-r0   |        O       |        O        
x86-64-accton-as7816-64x-r0  |        O       |        X          
x86-64-accton-as6712-32X-r0  |        O       |        X          
x86-64-accton-as6812-32X-r0  |        O       |        X  
x86-64-accton-as7316-26xb-r0 |        X       |        O
x86-64-accton-as7726-32x-r0  |        X       |        O  
x86-64-accton-as7326-56x-r0  |        X       |        O  
x86-64-accton-as7926-80xk-r0 |        X       |        O    
x86-64-accton-as9716-32d-r0  |        X       |        O 
