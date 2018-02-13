#!/bin/zsh
alias gdj="gradle -b ../../build.gradle"

rm -f vmsize.txt

gdj vm_size -Paotbm=md5 -Pno-rtc-guards -Paotstrat=baseline         -Puseconstantshiftoptimisation=false -Puse16bitarrayindex=false -Pusesimul=false -Puselightweightmethods=false >> vmsize_0BASE_R___P__C0_A0_S0_G1_NOLW.txt
gdj vm_size -Paotbm=md5 -Pno-rtc-guards -Paotstrat=improvedpeephole -Puseconstantshiftoptimisation=false -Puse16bitarrayindex=false -Pusesimul=false -Puselightweightmethods=false >> vmsize_1PEEP_R___P__C0_A0_S0_G1_NOLW.txt
gdj vm_size -Paotbm=md5 -Pno-rtc-guards -Paotstrat=simplestackcache -Puseconstantshiftoptimisation=false -Puse16bitarrayindex=false -Pusesimul=false -Puselightweightmethods=false >> vmsize_2SMPL_R11_P__C0_A0_S0_G1_NOLW.txt
gdj vm_size -Paotbm=md5 -Pno-rtc-guards -Paotstrat=poppedstackcache -Puseconstantshiftoptimisation=false -Puse16bitarrayindex=false -Pusesimul=false -Puselightweightmethods=false >> vmsize_3POPD_R11_P__C0_A0_S0_G1_NOLW.txt
gdj vm_size -Paotbm=md5 -Pno-rtc-guards -Paotstrat=markloop         -Puseconstantshiftoptimisation=false -Puse16bitarrayindex=false -Pusesimul=false -Puselightweightmethods=false >> vmsize_4MARK_R11_P6_C0_A0_S0_G1_NOLW.txt
gdj vm_size -Paotbm=md5 -Pno-rtc-guards -Paotstrat=markloop         -Puseconstantshiftoptimisation=true  -Puse16bitarrayindex=false -Pusesimul=false -Puselightweightmethods=false >> vmsize_4MARK_R11_P6_C1_A0_S0_G1_NOLW.txt
gdj vm_size -Paotbm=md5 -Pno-rtc-guards -Paotstrat=markloop         -Puseconstantshiftoptimisation=true  -Puse16bitarrayindex=true  -Pusesimul=false -Puselightweightmethods=false >> vmsize_4MARK_R11_P7_C1_A1_S0_G1_NOLW.txt
gdj vm_size -Paotbm=md5 -Pno-rtc-guards -Paotstrat=markloop         -Puseconstantshiftoptimisation=true  -Puse16bitarrayindex=true  -Pusesimul=true  -Puselightweightmethods=false >> vmsize_4MARK_R11_P7_C1_A1_S1_G1_NOLW.txt
gdj vm_size -Paotbm=md5 -Pno-rtc-guards -Paotstrat=markloop         -Puseconstantshiftoptimisation=true  -Puse16bitarrayindex=true  -Pusesimul=true  -Puselightweightmethods=true >> vmsize_4MARK_R11_P7_C1_A1_S1_G1.txt

echo -n "vmsize_0BASE_R___P__C0_A0_S0_G1_NOLW    " >> vmsize.txt
grep "AOT compiler" vmsize_0BASE_R___P__C0_A0_S0_G1_NOLW.txt | head -n 1 >> vmsize.txt
echo -n "vmsize_1PEEP_R___P__C0_A0_S0_G1_NOLW    " >> vmsize.txt
grep "AOT compiler" vmsize_1PEEP_R___P__C0_A0_S0_G1_NOLW.txt | head -n 1 >> vmsize.txt
echo -n "vmsize_2SMPL_R11_P__C0_A0_S0_G1_NOLW    " >> vmsize.txt
grep "AOT compiler" vmsize_2SMPL_R11_P__C0_A0_S0_G1_NOLW.txt | head -n 1 >> vmsize.txt
echo -n "vmsize_3POPD_R11_P__C0_A0_S0_G1_NOLW    " >> vmsize.txt
grep "AOT compiler" vmsize_3POPD_R11_P__C0_A0_S0_G1_NOLW.txt | head -n 1 >> vmsize.txt
echo -n "vmsize_4MARK_R11_P6_C0_A0_S0_G1_NOLW    " >> vmsize.txt
grep "AOT compiler" vmsize_4MARK_R11_P6_C0_A0_S0_G1_NOLW.txt | head -n 1 >> vmsize.txt
echo -n "vmsize_4MARK_R11_P6_C1_A0_S0_G1_NOLW    " >> vmsize.txt
grep "AOT compiler" vmsize_4MARK_R11_P6_C1_A0_S0_G1_NOLW.txt | head -n 1 >> vmsize.txt
echo -n "vmsize_4MARK_R11_P7_C1_A1_S0_G1_NOLW    " >> vmsize.txt
grep "AOT compiler" vmsize_4MARK_R11_P7_C1_A1_S0_G1_NOLW.txt | head -n 1 >> vmsize.txt
echo -n "vmsize_4MARK_R11_P7_C1_A1_S1_G1_NOLW    " >> vmsize.txt
grep "AOT compiler" vmsize_4MARK_R11_P7_C1_A1_S1_G1_NOLW.txt | head -n 1 >> vmsize.txt
echo -n "vmsize_4MARK_R11_P7_C1_A1_S1_G1         " >> vmsize.txt
grep "AOT compiler" vmsize_4MARK_R11_P7_C1_A1_S1_G1.txt      | head -n 1 >> vmsize.txt
