all:
	cd ..; ant arduino
	-mkdir ../../../../../../hardware/arduino/wudevice
	cp ../darjeeling.a ../../../../../../hardware/arduino/wudevice/libdarjeeling.a
	rm -rf libraries/WukongVM/wkpf
	-mkdir libraries/WukongVM/wkpf
	find ../../.. -name '*.h' -exec cp \{} libraries/WukongVM/wkpf \;

install: all
	cp -a * ../../../../../../hardware/arduino/wudevice
		
