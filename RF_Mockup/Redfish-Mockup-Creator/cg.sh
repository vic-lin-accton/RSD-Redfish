#!/bin/bash
DEVICE="ASGVOLT64-MOCKUP"
rc=`rm -rf $HOME/DMTF_TOOLS/$DEVICE`
rc=`mkdir $HOME/DMTF_TOOLS/$DEVICE`
IP="172.17.10.6"
DATE=`date`
python3.5 ./redfishMockupCreate.py  -r $IP:8888/ -S  -D "$HOME/DMTF_TOOLS/$DEVICE" \ -d  "Mockup from real ASGVOLT64"
