#!/bin/sh
export LD_LIBRARY_PATH=/usr/local/lib/
export mavproxy=1
cd /home/pi/picontrolserver/
/home/pi/picontrolserver/picontrolserver > /tmp/picontrolserver.out 
cd -
