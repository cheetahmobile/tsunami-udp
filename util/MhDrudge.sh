#!/bin/sh

if [ "$1" == "" ]; then
  echo -e "Usage:  MhDrudge.sh experimentname"
  echo -e "   Calls drudg to create .snp and .prc"
  echo -e "   out of 'experimentname'.skd\n"
  exit
fi

# -- purge
basef=${1%.skd}
skdf="${basef}.skd"
prcf="/usr2/proc/${basef}mh.prc"
snpf="/usr2/sched/${basef}mh.snp"
if [ -e $prcf ]; then
   echo -n "Proc $prcf exists (y to delete): "
   read action
   if [ $action == "y" ]; then
      rm -f $prcf
   else
      exit 1
   fi
fi
if [ -e $snpf ]; then
   echo -n "Snap $snpf exists (y to delete): "
   read action
   if [ $action == "y" ]; then
      rm -f $snpf
   else
      exit 1
   fi
fi

# -- drudg the files
#  Mh\n
#  11\n -- equipment type
#  8 10 0 0\n -- VLBA4 Mark5A n/c n/c
#  12\n \n -- make .prc, use default TPI setting
#  3\n  -- make .snp
# ( 5\n -- print summary )
#  0\n -- exit
keystr="Mh\n11\n8 10 0 0\n12\n \n3\n0\n";
echo -e $keystr | drudg $skdf
sleep 2

# -- create PC-EVN scripts
./makeRecexpt.sh $snpf ${basef} Mh
./makeRecexpt-tsunami.sh $snpf ${basef} Mh

  # TODO: add Gerhards samplerate detect for ./recpass

# -- summary
echo
echo "Done drudg'ing and created recexpt scripts"
echo "Start experiment in fieldsystem with: "
echo "> proc=${basef}mh"
echo "> setupsx    (assuming geo-VLBI...)"
echo "> schedule=${basef}mh"
echo "Start recexpt scripts on client PC."
echo

# scp "recexptsunami_${basef}*" jwagner@kurp:/home/jwagner/public_html/expscripts
# http://www.metsahovi.fi/~jwagner/expscripts/
