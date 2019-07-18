## RSD Redfish PSME Test Automation ##

* Reference from [OpenBMC Test Automation Redfish part](https://github.com/openbmc/openbmc-test-automation)

**Interface Feature List**
* REST
* DMTF Redfish

## Installation Setup Guide ##

* Under Ubuntu 16.04lts version.

* [Robot Framework Install Instruction](https://github.com/robotframework/robotframework/blob/master/INSTALL.rst)

* Pre-install
Packages required to be installed for RF test Automation.
Install the packages and it's dependencies via `pip3`

    ```
    $ sudo apt-get install python3-pip
    $ sudo pip3 install robotframework
    $ sudo pip3 install -U requests
    $ sudo pip3 install -U robotframework-requests
    $ sudo pip3 install -U robotframework-httplibrary
    $ sudo pip3 install redfish
    $ sudo pip3 install robotframework-sshlibrary
    $ sudo pip3 install robotframework-scplibrary
    $ sudo pip3 install beautifulsoup4>=4.6.0
    $ sudo pip3 install lxml
    ```

## Test item ##

`redfish/`:  Test cases for DMTF Redfish-related feature supported.

`redfish_eit/`: Test cases for ONLP EIT test.

* How to run onl EIT test:

    ONLP EIT:
    
    Because of some test item need the user to remove hardware like FAN/PSU/SFP, 
    
    it needs to use this EIT test to check if hardware module states are OK or Not.
    
    ```
    $ eit.sh
    ```

    All test:
    
    These test based on some BASELINE service defined by OCP .
    
    Please modify all.sh
    
     ```
    ####################################

    rfip=172.17.8.122:8888                  <== DUT IP 

    listener=172.17.10.60:8889              <== Listener IP

    FirmwareVersion=2.1.3.59.21             <== PSME version

    ####################################

    ```   
    Start up PSME on device first and then
    
    ```
    $ all.sh
    ```
* How to run CI test:

### [CI Build and Test Architecture Model](CI.md)

```
+-----------------------+            +----------------------+
|                       |   1.       |                      |
|  172.17.10.60         +------------>   172.17.10.64       |
|Agent                  |            | Agent Build_RSD_PSME |
|Robot_PSME_Auto_Test   |            |                      |
|                       |            |                      |
|  Jenkins Service      |            |                      |
|       :8080           |            |                      |
+-------------------+---+            +-----+----------------+
                    |                      |
                    |                      |
                    |3.                    | 2.
                    |                      |
                    |                      |
                 +--v----------------------v---+
                 |    Target: Switch or OLT    |
                 |    172.17.10.x              |
                 |                             |
                 |                             |
                 +-----------------------------+


 1. Start Build on Agent Build_RSD_PSME

 2. After build finished, deploy PSME package to Target and start up RF service.

 3. Start robot framework automation test to target.


```    
    
 ### Triger Jenkins build and deploy to device and run all.sh test: ###
 
```
    http://172.17.10.60:8080/job/Build_ONL_RF/build?token=05a8ab5 
```
