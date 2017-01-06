#!/bin/sh
#This script tries to flash the darjeeling image into all /dev/ttyUSB* devices
#using lnode-bsl. It also sets the following node ids: Lowest ttyUSB* is
#assigned nodeId 1, all others the number of the ttyUSB port plus 10. Note that
#the output is a bit messy, but allows you to see whether an error occurred.
#TODO: automatically check if all process finish correct, and only display the
#progress of node 1.
FIRST=0
for DEVICE in /dev/ttyUSB*; do

				#extract number from /dev/ttyUSB* string
        NUM=${DEVICE:11}
				#assign node id
        NODEID=$((NUM+10))

        if [ $FIRST -eq 0 ] ; then
                FIRSTDEVICE=$DEVICE
                FIRST=1
        else
                #create hex file with node id
                tos-set-symbols --objcopy msp430-objcopy --objdump msp430-objdump --target ihex darjeeling.elf install-$NODEID.hex TOS_NODE_ID=$NODEID
                lnode-bsl -c $DEVICE -r -e -I -p install-$NODEID.hex &
        fi
done
if [ $FIRST -ne 0 ] ; then
        NODEID=1
        tos-set-symbols --objcopy msp430-objcopy --objdump msp430-objdump --target ihex darjeeling.elf install-$NODEID.hex TOS_NODE_ID=$NODEID
        lnode-bsl -c $FIRSTDEVICE -r -e -I -p install-$NODEID.hex
fi
