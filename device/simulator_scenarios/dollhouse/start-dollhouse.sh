# Simple script to run all VMs.
# Should normally be done by network_ui.jar,
# but it may be useful to be able to run the VMs independently from the UI.
./stop-dollhouse.sh
cd ../../../src/config/native-simulator/
./darjeeling.elf -i 2 -d ./dollhouse -e ../../../wukong/simulator_scenarios/dollhouse/enabled_wuclasses_bedroom1.xml &
./darjeeling.elf -i 3 -d ./dollhouse -e ../../../wukong/simulator_scenarios/dollhouse/enabled_wuclasses_bedroom2.xml &
./darjeeling.elf -i 4 -d ./dollhouse -e ../../../wukong/simulator_scenarios/dollhouse/enabled_wuclasses_bedroom3.xml &
./darjeeling.elf -i 5 -d ./dollhouse -e ../../../wukong/simulator_scenarios/dollhouse/enabled_wuclasses_kitchen.xml &
./darjeeling.elf -i 6 -d ./dollhouse -e ../../../wukong/simulator_scenarios/dollhouse/enabled_wuclasses_entrance1.xml &
cd -
