rm -rf build
mkdir build
cp -R ../coremark_v1.0/* build
rm build/core_list_join.c build/core_main.c build/core_matrix.c build/core_state.c build/core_util.c build/coremark.h

BM_DIR=../../../src/lib/coremk_c_opt/c/common
cp $BM_DIR/core_list_join.c $BM_DIR/core_main.c $BM_DIR/core_matrix.c $BM_DIR/core_state.c $BM_DIR/core_util.c ./build
cp $BM_DIR/../../include/common/core_optimisations.h ./build
cp *.h ./build
cp -R macos_dj ./build

cd build
make clean PORT_DIR=macos_dj
make PORT_DIR=macos_dj
cat run1.log
cd ..
