#!/bin/sh
mkdir -p /usr/local/darjeeling
cp service/start_service.sh darjeeling.elf lib_infusions.dja app_infusion.dja /usr/local/darjeeling

cp service/darjeeling.sh /etc/init.d
touch /var/log/darjeeling.log
chown root /var/log/darjeeling.log
update-rc.d "darjeeling.sh" defaults
/etc/init.d/darjeeling.sh start
