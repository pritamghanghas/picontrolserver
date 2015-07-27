#!/bin/sh
export LD_LIBRARY_PATH=/usr/local/lib/
cd /home/pi/picontrolserver/
/home/pi/picontrolserver/picontrolserver > /tmp/picontrolserver.out 
cd -
