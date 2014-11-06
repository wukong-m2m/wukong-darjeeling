#!/bin/sh
if [ -n "$1" ]; then
	DJ_BOOT_PARAMETERS=$1
else
	DJ_BOOT_PARAMETERS="-u 2=/dev/ttyACM0"
fi

mkdir -p /usr/local/darjeeling

if [ -f /etc/init.d/darjeeling.sh ]; then
    /etc/init.d/darjeeling.sh stop
fi

cp service/start_service.sh darjeeling.elf lib_infusions.dja app_infusion.dja /usr/local/darjeeling
chmod a+rx /usr/local/darjeeling/start_service /usr/local/darjeeling/darjeeling.elf
sed -i "s/REPLACE_THIS_WITH_BOOT_PARAMETERS/$(echo $DJ_BOOT_PARAMETERS | sed -e 's/\\/\\\\/g' -e 's/\//\\\//g' -e 's/&/\\\&/g')/g" /usr/local/darjeeling/start_service.sh

cp service/darjeeling.sh /etc/init.d
touch /var/log/darjeeling.log
chown root /var/log/darjeeling.log
update-rc.d "darjeeling.sh" defaults
/etc/init.d/darjeeling.sh start
