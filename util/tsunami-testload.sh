#!/bin/bash

# Testing of realtime Tsunami transfer, default semi-lossy mode 15s window

RTSERVER=didius.metsahovi.fi              # server with real-time Tsunami server, default passwd
RTRATE=200                                # server side Mbit/s (VSI rate)
RTLEN=$((RTRATE * 1000000 * 60 * 3 / 8))  # 3 minutes of data
RAIDROOT=/raid/s
TSBIN=`which tsunami`

while true; do

  timestr=`date -u --date "now + 20 seconds" +"%Y%j%H%M%S"` # YYYYdddhhhmmss + 50s
  usc="_"
  dlfile=dummy${usc}mh${usc}scan01${usc}${timestr}${usc}dl=${RTLEN}.vsi

  echo $dlfile
  pushd ${RAIDROOT}
  $TSBIN connect ${RTSERVER} \
   set rate 500m \
   set transcript yes \
   set lossless no \
   set losswindow 15000 \
   get $dlfile \
   close \
   quit
  # rm $dlfile
  popd

  sleep 10

done
