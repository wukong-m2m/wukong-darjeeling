all:
	cd ..; gradle -b ../../build.gradle clean
	cd ..; gradle -b ../../build.gradle

	-mkdir ../../../../../../hardware/arduino/wudevice
	cp ../../../build/arduino/binaries/darjeelingStaticLibrary/libdarjeeling.a libdarjeeling.a
	rm -rf libraries/WukongVM/wkpf
	-mkdir libraries/WukongVM/wkpf
	find ../../.. -name '*.h' -exec cp \{} libraries/WukongVM/wkpf \; 2> /dev/null
	cp ../include/config.h libraries/WukongVM/wkpf

install: all
	if [ ! -d ../../../../../../hardware/arduino/avr ]; then\
		echo "The arduino environment is not available. Please put the wukong-darjeering in the tools directory of the Arduino distribution";\
		false;\
	fi

	cp -a ../../../../../../hardware/arduino/avr/* ../../../../../../hardware/arduino/wudevice
	cp -a boards.txt platform.txt libdarjeeling.a ../../../../../../hardware/arduino/wudevice > /dev/null
	cp -a libraries/WukongVM ../../../../../../hardware/arduino/wudevice/libraries > /dev/null
	echo "Deploy successfully"
	cp arduino-wukong ../../../../../..
		
