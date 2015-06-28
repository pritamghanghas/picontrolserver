#!/bin/sh
export LD_LIBRARY_PATH=/usr/local/lib/
export thermal_cam=1
cd /home/pi/picontrolserver/
/home/pi/picontrolserver/picontrolserver > /home/pi/picontrolserver/picontrolserver.out 
cd -
