#!/bin/bash
# Build ONL 
PATH=$PATH:$HOME/bin:./:/bin/:/usr/bin/
export PATH
cd ../../../
source setup.env
make amd64 -j8
