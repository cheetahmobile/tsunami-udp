#!/bin/sh

rm parsetest
gcc parse_evn_filename.c -DUNIT_TEST -o parsetest -lm

echo "To successfully pass validation:" 
echo "  - DEBUG: ... fixed_mktime()=...  matches 'expected' value" 
echo "  - ef->data_start_time equals fixed_mktime() result, OR" 
echo "  - ef->data_start_time equals 0.0 when start time is "
echo "    in the past" 
echo "  - system time zone does not alter fixed_mktime() result" 
echo "  - valid or invalid file name is reported" 
echo 

./parsetest gre53_Ef_scan035_154d12h43m10s.vsi
echo "  expect:  1180874590 (in 2007)"
echo
./parsetest gre53_Ef_scan035_154124310.vsi
echo "  expect:  1180874590 (in 2007)"
echo
./parsetest R1262_On_037-1240b_2007037124050_flen=5408000000.evn
echo "  expect:  1170765650"
echo
./parsetest R1262_On_037-1240b_2007037_dl=5408000000.vsi           
echo "  expect:  1170720000"
echo
./parsetest R1262_On_037-1240b_2007y037d_dl=5408000000.vsi
echo "  expect:  1170720000"
echo
./parsetest R1262_On_037-1240b_2007y037d12h6m1s_dl=5408000000.vsi  
echo "  expect:  1170763561"
echo
./parsetest gre53_Ef_scan035_2006-11-21T08:45:00_dl=14400000.vsi   
echo "  expect:  1164098700"
echo
./parsetest dummy_Mh_scan01_134330_dl=1500.vsi
TMPSTR=`date -u --date "13:43:30" +%s`
echo "  expect:  $TMPSTR (per 'date -u --date '13:43:30' +%s')"
echo
./parsetest dummy_Mh_scan01_124500.00_dl=1500.vsi
TMPSTR=`date -u --date "12:45:00" +%s`
echo "  expect:  $TMPSTR (fractional parse not working yet)"
echo


