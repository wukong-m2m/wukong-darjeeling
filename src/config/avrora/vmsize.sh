#!/bin/zsh
alias gdj="gradle -b ../../build.gradle"

rm -f vmsize.txt

echo -n "results_0BASE_R___P__CS0    " >> vmsize.txt
gdj vm_size -Paotbm=md5 -Paotstrat=baseline -Paotstackcachesize=11 -Paotmarkloopregs=7 -Paotconstshiftoptimisation=none             | grep "AOT compiler" | head -n 1 >> vmsize.txt

echo -n "results_0PEEP_R___P__CS0    " >> vmsize.txt
gdj vm_size -Paotbm=md5 -Paotstrat=improvedpeephole -Paotstackcachesize=11 -Paotmarkloopregs=7 -Paotconstshiftoptimisation=none     | grep "AOT compiler" | head -n 1 >> vmsize.txt

echo -n "results_1SMPL_R11_P__CS0    " >> vmsize.txt
gdj vm_size -Paotbm=md5 -Paotstrat=simplestackcache -Paotstackcachesize=11 -Paotmarkloopregs=7 -Paotconstshiftoptimisation=none     | grep "AOT compiler" | head -n 1 >> vmsize.txt

echo -n "results_2POPD_R11_P__CS0    " >> vmsize.txt
gdj vm_size -Paotbm=md5 -Paotstrat=poppedstackcache -Paotstackcachesize=11 -Paotmarkloopregs=7 -Paotconstshiftoptimisation=none     | grep "AOT compiler" | head -n 1 >> vmsize.txt

echo -n "results_3MARK_R11_P7_CS0    " >> vmsize.txt
gdj vm_size -Paotbm=md5 -Paotstrat=markloop         -Paotstackcachesize=11 -Paotmarkloopregs=7 -Paotconstshiftoptimisation=none     | grep "AOT compiler" | head -n 1 >> vmsize.txt

echo -n "results_3MARK_R11_P7_CS4    " >> vmsize.txt
gdj vm_size -Paotbm=md5 -Paotstrat=markloop         -Paotstackcachesize=11 -Paotmarkloopregs=7 -Paotconstshiftoptimisation=gcc_like | grep "AOT compiler" | head -n 1 >> vmsize.txt



#echo -n "results_0BASE_R___P__CS1    " >> vmsize.txt
#gdj vm_size -Paotbm=md5 -Paotstrat=baseline -Paotstackcachesize=11 -Paotmarkloopregs=7 -Paotconstshiftoptimisation=by1                  | grep "AOT compiler" | head -n 1 >> vmsize.txt

#echo -n "results_0BASE_R___P__CS2    " >> vmsize.txt
#gdj vm_size -Paotbm=md5 -Paotstrat=baseline -Paotstackcachesize=11 -Paotmarkloopregs=7 -Paotconstshiftoptimisation=all_only_shift       | grep "AOT compiler" | head -n 1 >> vmsize.txt

#echo -n "results_0BASE_R___P__CS3    " >> vmsize.txt
#gdj vm_size -Paotbm=md5 -Paotstrat=baseline -Paotstackcachesize=11 -Paotmarkloopregs=7 -Paotconstshiftoptimisation=all_move_and_shift   | grep "AOT compiler" | head -n 1 >> vmsize.txt

# echo -n "results_0BASE_R___P__CS4    " >> vmsize.txt
# gdj vm_size -Paotbm=md5 -Paotstrat=baseline -Paotstackcachesize=11 -Paotmarkloopregs=7 -Paotconstshiftoptimisation=gcc_like             | grep "AOT compiler" | head -n 1 >> vmsize.txt

# echo -n "results_1SMPL_R11_P__CS4    " >> vmsize.txt
# gdj vm_size -Paotbm=md5 -Paotstrat=simplestackcache -Paotstackcachesize=11 -Paotmarkloopregs=7 -Paotconstshiftoptimisation=gcc_like     | grep "AOT compiler" | head -n 1 >> vmsize.txt

# echo -n "results_2POPD_R11_P__CS4    " >> vmsize.txt
# gdj vm_size -Paotbm=md5 -Paotstrat=poppedstackcache -Paotstackcachesize=11 -Paotmarkloopregs=7 -Paotconstshiftoptimisation=gcc_like     | grep "AOT compiler" | head -n 1 >> vmsize.txt
