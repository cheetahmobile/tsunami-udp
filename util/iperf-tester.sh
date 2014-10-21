#!/bin/bash
#
# (C) Jan Wagner 2007
#
# Runs a two-way iperf UDP test with various packet sizes
# and transmit rates, using the Tsunami UDP port 46224.
#

SPEEDS=(	# the speeds...
 64m 90m 128m 200m 256m 300m 400m 512m 600m 700m 896m 940m
)
PSIZES=(	# UDP packet sizes to try at each speed
 1430 4096 8192 16384 32768
)
TIME=15		# iperf checking time in seconds


if [ "$1" == "" ]; then
	echo "Running as server"
	iperf -s -u -t $TIME -l 32768 -p 46224 -w 256k -i 1 -m
else
	echo "Running as client, connecting to $1"
	echo `date -u`
	traceroute $1
	for spdIdx in $(seq 0 $((${#SPEEDS[@]} - 1))); do
		for szIdx in $(seq 0 $((${#PSIZES[@]} - 1))); do
			echo "#==== speed ${SPEEDS[$spdIdx]}  packetsize ${PSIZES[$szIdx]}  time $TIME sec"
			OUTFILE=iperf${SPEEDS[$spdIdx]}speed-${PSIZES[$szIdx]}size.log
			iperf -c $1 -u -p 46224 -w 256k -t $TIME -b ${SPEEDS[$spdIdx]} -l ${PSIZES[$szIdx]} -i 1 -m -d > $OUTFILE
		done
	done
fi
