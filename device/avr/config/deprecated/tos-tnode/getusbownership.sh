#!/bin/bash
lsusb | grep Future | awk -F" |:" '{ print  "sudo chown $(whoami) /dev/bus/usb/"$2"/"$4}' | sh
