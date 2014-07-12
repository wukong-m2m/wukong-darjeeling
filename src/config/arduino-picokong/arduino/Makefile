all:
	cd ..; ant arduino
	-mkdir ../../../../../../hardware/arduino/picodevice
	cp ../darjeeling.a libdarjeeling.a
	rm -rf libraries/WukongVM/wkpf
	-mkdir libraries/WukongVM/wkpf
	find ../../.. -name '*.h' -exec cp \{} libraries/WukongVM/wkpf \; 2> /dev/null

install: all
	cp -a * ../../../../../../hardware/arduino/picodevice > /dev/null
	echo "Deploy successfully"
	cp arduino-wukong ../../../../../..
		
