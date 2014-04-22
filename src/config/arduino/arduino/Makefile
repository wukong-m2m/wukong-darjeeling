all:
	cd ..; ant arduino
	cp ../darjeeling.a libdarjeeling.a
	rm -rf libraries/WukongVM/wkpf
	mkdir libraries/WukongVM/wkpf
	cp `find ../../.. -name '*.h'` ./libraries/WukongVM/wkpf
