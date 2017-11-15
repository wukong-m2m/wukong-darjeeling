#!/bin/zsh
alias gdj="gradle -b ../../build.gradle"

benchmarks=(bsort32 hsort32 binsrch32 fft xxtea rc5 md5 coremk)

gdj clean

# MAIN GRAPHS
# for benchmark in ${benchmarks}
# do
#     gdj avrora_store_trace -Paotbm=${benchmark} -Paotstrat=baseline         -Puseconstantshiftoptimisation=false -Puse16bitarrayindex=false -Pusesimul=false
#     # gdj avrora_store_trace -Paotbm=${benchmark} -Paotstrat=baseline         -Puseconstantshiftoptimisation=true  -Puse16bitarrayindex=false -Pusesimul=false
#     # gdj avrora_store_trace -Paotbm=${benchmark} -Paotstrat=baseline         -Puseconstantshiftoptimisation=true  -Puse16bitarrayindex=true  -Pusesimul=false
#     # gdj avrora_store_trace -Paotbm=${benchmark} -Paotstrat=baseline         -Puseconstantshiftoptimisation=true  -Puse16bitarrayindex=true  -Pusesimul=true

#     gdj avrora_store_trace -Paotbm=${benchmark} -Paotstrat=improvedpeephole -Puseconstantshiftoptimisation=false -Puse16bitarrayindex=false -Pusesimul=false
#     # gdj avrora_store_trace -Paotbm=${benchmark} -Paotstrat=improvedpeephole --useconstantshiftoptimisation=true  -PPuse16bitarrayinde=false  Pusesimulx=false
#     # gdj avrora_store_trace -Paotbm=${benchmark} -Paotstrat=improvedpeephole -Puseconstantshiftoptimisation=true  -Puse16bitarrayindex=true  -Pusesimul=true
#     gdj avrora_store_trace -Paotbm=${benchmark} -Paotstrat=simplestackcache -Puseconstantshiftoptimisation=false -Puse16bitarrayindex=false -Pusesimul=false
#     # gdj avrora_store_trace -Paotbm=${benchmark} -Paotstrat=simplestackcache --useconstantshiftoptimisation=true  -PPuse16bitarrayinde=false  Pusesimulx=false
#     # gdj avrora_store_trace -Paotbm=${benchmark} -Paotstrat=simplestackcache -Puseconstantshiftoptimisation=true  -Puse16bitarrayindex=true  -Pusesimul=true
#     gdj avrora_store_trace -Paotbm=${benchmark} -Paotstrat=poppedstackcache -Puseconstantshiftoptimisation=false -Puse16bitarrayindex=false -Pusesimul=false
#     # gdj avrora_store_trace -Paotbm=${benchmark} -Paotstrat=poppedstackcache --useconstantshiftoptimisation=true  -PPuse16bitarrayinde=false  Pusesimulx=false
#     # gdj avrora_store_trace -Paotbm=${benchmark} -Paotstrat=poppedstackcache -Puseconstantshiftoptimisation=true  -Puse16bitarrayindex=true  -Pusesimul=true

#     gdj avrora_store_trace -Paotbm=${benchmark} -Paotstrat=markloop         -Puseconstantshiftoptimisation=false -Puse16bitarrayindex=false -Pusesimul=false
#     gdj avrora_store_trace -Paotbm=${benchmark} -Paotstrat=markloop         -Puseconstantshiftoptimisation=true  -Puse16bitarrayindex=false -Pusesimul=false
#     gdj avrora_store_trace -Paotbm=${benchmark} -Paotstrat=markloop         -Puseconstantshiftoptimisation=true  -Puse16bitarrayindex=true  -Pusesimul=false
#     gdj avrora_store_trace -Paotbm=${benchmark} -Paotstrat=markloop         -Puseconstantshiftoptimisation=true  -Puse16bitarrayindex=true  -Pusesimul=true
# done

# BEST CASE ONLY
for benchmark in ${benchmarks}
do
    gdj avrora_store_trace -Paotbm=${benchmark} -Paotstrat=markloop         -Puseconstantshiftoptimisation=true  -Puse16bitarrayindex=true  -Pusesimul=true
done

# MARKLOOP + XXTEA FOR DIFF NUMBERS OF PINNED REGISTERS
# for benchmark in ${benchmarks}
# do
#     markloopregs=(1 2 3 4 5 6 7)
#     for aotmarkloopregs in ${markloopregs}
#     do
#         gdj avrora_store_trace -Paotbm=${benchmark} -Paotmarkloopregs=${aotmarkloopregs}
#     done
# done

# # CoreMark manual optimisations
# 	# Absolute baseline: not optimised code, no lightweight methods, baseline translation, no instructionset optimisations
# 	gdj avrora_store_trace -Paotbm=coremk_or0 -Paotstrat=baseline   -Puseconstantshiftoptimisation=false -Puse16bitarrayindex=false -Pusesimul=false -Pnolightweightmethods=true

# 	# Using our optimisations the original code can get as good as:
# 	gdj avrora_store_trace -Paotbm=coremk_or0
# 	# Step by step manual optimisation
# 	gdj avrora_store_trace -Paotbm=coremk_or1
# 	gdj avrora_store_trace -Paotbm=coremk_or2
# 	gdj avrora_store_trace -Paotbm=coremk_or3
# 	gdj avrora_store_trace -Paotbm=coremk_or4
# 	gdj avrora_store_trace -Paotbm=coremk_or5
# 	# Should be identical to normal CoreMark version
# 	gdj avrora_store_trace -Paotbm=coremk
# 	# Two 'unfair' optimisations
# 	gdj avrora_store_trace -Paotbm=coremk_ch1
# 	gdj avrora_store_trace -Paotbm=coremk_ch2


# # Breakdown of executed JVM instructions (executions, not cycles. for the top of the performance table). Get these with 16 bit indexes enabled, otherwise we get lots of I2S conversions since the code uses shorts.
# for benchmark in ${benchmarks}
# do
# 	gdj avrora_store_trace -Paotbm=${benchmark} -Paotstrat=baseline         -Puseconstantshiftoptimisation=false -Puse16bitarrayindex=true -Pusesimul=false
# done

# # Run all without lightweight methods
# for benchmark in ${benchmarks}
# do
# 	gdj avrora_store_trace -Paotbm=${benchmark} -Puselightweightmethods=false
# done

# # Data for lightweight method evaluation
# 	# Normal versions
# 	gdj avrora_store_trace -Paotbm=coremk 
# 	gdj avrora_store_trace -Paotbm=fft
# 	gdj avrora_store_trace -Paotbm=hsort32
# 	gdj avrora_store_trace -Paotbm=coremk_lw     # Coremk with ee_isdigit as a lightweight instead of inlined
# 	gdj avrora_store_trace -Paotbm=coremk_fn     # Coremk with only functions (ee_isdigit not inlined)
# 	gdj avrora_store_trace -Paotbm=coremk_f2     # Coremk with only functions (but ee_isdigit still inlined)
# 	gdj avrora_store_trace -Paotbm=fft_lw        # FFT with FIX__MPY as a hardcoded Lightweight method, and SIN8/COS8 inlined by ProGuard
# 	gdj avrora_store_trace -Paotbm=fft_fn        # FFT with FIX__MPY as a normal method, and SIN8/COS8 inlined by ProGuard

# 	# Heap sort with 
# 	gdj avrora_store_trace -Paotbm=hsort32_fn    # Heap sort with siftDown as a normal method
# 	gdj avrora_store_trace -Paotbm=hsort32_cht   # Heap sort with siftDown manually inlined



# Special case: CoreMark is too big to fit both Java and C versions in memory at the same time. Run the coremk_c config for the native CoreMark results, and then copy the result here.
cd ../coremk_c
rm -rf results_coremk_c
gdj avrora_store_trace
cd -
rm -rf results_coremk_c
cp -r ../coremk_c/results_coremk_c .

./analyseall.sh



