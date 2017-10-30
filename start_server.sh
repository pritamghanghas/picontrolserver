#!/bin/sh

# not using this script anymore. We will remove it soon
#start Ardupilot
#/home/pi/ardupilot/ArduCopter/ArduCopter.elf -A udp:127.0.0.1:1440 -B /dev/ttyAMA0 > /home/pi/arducopter.log 2>&1 &

# start control server
export LD_LIBRARY_PATH=/usr/local/lib/
export mavudp=1
export picam=1
#export uvccam=1
export mavtcp=1
export hostapd=1
#export sik=1
#export lepton=1
#export seek=1
export interfaces=lan0 # comma seperated list of interfaces, no spaces
export beacon_interval=3
cd /home/pi/ardupilot/picontrolserver/
/home/pi/ardupilot/picontrolserver/picontrolserver > /home/pi/ardupilot/logs/picontrolserver.out 2>&1 &
cd -
