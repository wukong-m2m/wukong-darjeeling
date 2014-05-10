#!/bin/sh
mkdir -p /usr/local/darjeeling
cp service/start_service.sh darjeeling.elf lib_infusions.dja app_infusion.dja /usr/local/darjeeling
chmod a+rx /usr/local/darjeeling/start_service.sh /usr/local/darjeeling/darjeeling.elf
cp service/darjeeling.sh /etc/init.d
touch /var/log/darjeeling.log
chown root /var/log/darjeeling.log
update-rc.d "darjeeling.sh" defaults
/etc/init.d/darjeeling.sh start
