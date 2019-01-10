#!/bin/bash
sudo service acc_bcm_rmm stop
sudo service rmm stop
sudo service psme stop
sudo dpkg --purge rmm-all
sudo dpkg --purge rmm-consolecontrol
sudo dpkg --purge rmm-api
sudo dpkg --purge rmm-base
sudo dpkg --purge opennsl-accton
