#!/bin/sh

#start Ardupilot
#/home/pi/ardupilot/ArduCopter/ArduCopter.elf -A udp:127.0.0.1:1440 -B /dev/ttyAMA0 > /home/pi/arducopter.log 2>&1 &

# start control server
export LD_LIBRARY_PATH=/usr/local/lib/
export mavproxy=1
export picam=1
cd /home/pi/ardupilot/picontrolserver/
/home/pi/ardupilot/picontrolserver/picontrolserver > /home/pi/ardupilot/logs/picontrolserver.out 2>&1 &
cd -
