#!/bin/bash

if [ "$1" == "" ]; then
  echo
  echo " ./tsuc2gnuplot tsunamiClientTranscript.tsuc "
  echo 
  echo "Takes a Tsunami client side transcript file, extracts "
  echo "the data into a temp file, and plots the rates vs time"
  echo "of the transfer using Octave."
  echo
  exit
fi

# -- find data portion of the log
linenrs=$(grep -n -e "START" -e "STOP" $1 | cut -f1 -d: | head -2)
read lbegin lend << HERE
  $(echo $linenrs)
HERE
lbegin=$(($lbegin + 1))
lend=$(($lend - 1))

# -- extract data and reformat the presentation a bit for Matlab/Octave/GNUplot
TMPF=`mktemp /tmp/tsuc2gpData.XXXXXXXXXX` || exit 1
SEDF=`mktemp /tmp/tsuc2gpSed.XXXXXXXXXX`  || exit 1
echo -e " s/Mbps//g \n s/M//g \n s/G//g \n s/:/ /g \n s/\%//g" > $SEDF
sed -n "$lbegin,${lend}p" $1 | sed -f $SEDF > $TMPF
rm $SEDF

# example data:
# 000000.357  550   0.00  403.7   0.0     550    0.0  403.6   0.0     0     0  457214        0        0
# interval, packets, retransmitted MB, receive rate Mbit/s, loss %, ...

OCTF=`mktemp /tmp/tsuc2gpPlot.XXXXXXXXXX` || exit 1
echo "load '${TMPF}';" > $OCTF
echo "xsecs = tsuc2gpData(:,3) + 60*(tsuc2gpData(:,2) + 60*tsuc2gpData(:,1));" >> $OCTF
echo "figure(1), hold on, grid on, title('$1')" >> $OCTF
echo "plot(xsecs, tsuc2gpData(:,6), ';receive rate;')" >> $OCTF
echo "plot(xsecs, tsuc2gpData(:,5), ';loss rate;')" >> $OCTF
echo "plot(xsecs, tsuc2gpData(:,10), ';cumulative rate;')" >> $OCTF
echo "xlabel('seconds'), ylabel('Mbit/s')" >> $OCTF
echo "disp('Any key to close the plot...')" >> $OCTF
echo "pause" >> $OCTF
octave $OCTF

rm $OCTF
rm $TMPF
