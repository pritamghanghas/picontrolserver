#!/bin/bash

# This script is invoked by ardupilot.service and starts
# http server that is used to manage the camera and ardupilot
# start with proper udp boardcast parameter to client


################start of media management code########################
#make sure that we have the dirctories for media storage
mkdir -p /home/pi/media/videos

cd /home/pi/media/videos

#convert all existing recordings to mp4
allH264Files=`ls -1 *.h264 | tr "\n" " "`
for h264file in $allH264Files
do
    MP4Box -add $h264file "$h264file.mp4"
    rm $h264file
done

# while free space is less than 2GB and there are still files to delete
freeGB=`df | grep "/dev/root" | tr -s " " |  cut -d " " -f 4`
echo "free space before cleanup is $freeGB\n"
mediaFiles=`ls -clt | tr -s " " | cut -d " " -f 9 | tr "\n" " "`
echo "current files are $mediaFiles\n"
while [ $freeGB -lt 2097152 ]  &&  [ ! -z $mediaFiles ]
do
    for file in $mediaFiles
    do
        echo "removing file $file\n"
        rm $file
        break;
    done
    freeGB=`df | grep "/dev/root" | tr -s " " |  cut -d " " -f 4`
    mediaFiles=`ls -clt | tr -s " " | cut -d " " -f 9 | tr "\n" " "`
done

cd -
#########################end of media management code#######################


AP_BIN_DIR="/home/pi/ardupilot/bin/"
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

export unique_id=`cat /proc/cpuinfo | grep Serial | cut -d ' ' -f 2`
date
while :; do
         cd $AP_BIN_DIR 
         $AP_BIN_DIR/picontrolserver
         cd -
done >> /home/pi/ardupilot/info/picontrolserver.log 2>&1
