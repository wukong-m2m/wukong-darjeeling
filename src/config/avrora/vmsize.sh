#!/bin/zsh
alias gdj="gradle -b ../../build.gradle"

rm -f vmsize.txt

echo -n "results_0BASE_R___P__C0_A0_S0_G1    " >> vmsize.txt
gdj vm_size -Paotbm=md5 -Pno-rtc-guards -Paotstrat=baseline         -Puseconstantshiftoptimisation=false -Puse16bitarrayindex=false -Pusesimul=false | grep "AOT compiler" | head -n 1 >> vmsize.txt

echo -n "results_1PEEP_R___P__C0_A0_S0_G1    " >> vmsize.txt
gdj vm_size -Paotbm=md5 -Pno-rtc-guards -Paotstrat=improvedpeephole -Puseconstantshiftoptimisation=false -Puse16bitarrayindex=false -Pusesimul=false | grep "AOT compiler" | head -n 1 >> vmsize.txt

echo -n "results_2SMPL_R11_P__C0_A0_S0_G1    " >> vmsize.txt
gdj vm_size -Paotbm=md5 -Pno-rtc-guards -Paotstrat=simplestackcache -Puseconstantshiftoptimisation=false -Puse16bitarrayindex=false -Pusesimul=false | grep "AOT compiler" | head -n 1 >> vmsize.txt

echo -n "results_3POPD_R11_P__C0_A0_S0_G1    " >> vmsize.txt
gdj vm_size -Paotbm=md5 -Pno-rtc-guards -Paotstrat=poppedstackcache -Puseconstantshiftoptimisation=false -Puse16bitarrayindex=false -Pusesimul=false | grep "AOT compiler" | head -n 1 >> vmsize.txt

echo -n "results_4MARK_R11_P6_C0_A0_S0_G1    " >> vmsize.txt
gdj vm_size -Paotbm=md5 -Pno-rtc-guards -Paotstrat=markloop         -Puseconstantshiftoptimisation=false -Puse16bitarrayindex=false -Pusesimul=false | grep "AOT compiler" | head -n 1 >> vmsize.txt

echo -n "results_4MARK_R11_P6_C1_A0_S0_G1    " >> vmsize.txt
gdj vm_size -Paotbm=md5 -Pno-rtc-guards -Paotstrat=markloop         -Puseconstantshiftoptimisation=true  -Puse16bitarrayindex=false -Pusesimul=false | grep "AOT compiler" | head -n 1 >> vmsize.txt

echo -n "results_4MARK_R11_P7_C1_A1_S0_G1    " >> vmsize.txt
gdj vm_size -Paotbm=md5 -Pno-rtc-guards -Paotstrat=markloop         -Puseconstantshiftoptimisation=true  -Puse16bitarrayindex=true  -Pusesimul=false | grep "AOT compiler" | head -n 1 >> vmsize.txt

echo -n "results_4MARK_R11_P7_C1_A1_S1_G1    " >> vmsize.txt
gdj vm_size -Paotbm=md5 -Pno-rtc-guards -Paotstrat=markloop         -Puseconstantshiftoptimisation=true  -Puse16bitarrayindex=true  -Pusesimul=true  | grep "AOT compiler" | head -n 1 >> vmsize.txt











