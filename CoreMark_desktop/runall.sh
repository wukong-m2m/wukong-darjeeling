cd java
rm -rf build
gradle -Pcm_version=ori
gradle -Pcm_version=opt
gradle -Pcm_version=cht
cd ..
rm -f output.txt
date >> output.txt

# ori
for i in `seq 1 10`; {
	echo 'ori' $i >> output.txt
	java -jar java/build/libs/coremk_ori.jar-1.0.jar | grep -E 'Iteration|Total' >> output.txt
	echo '' >> output.txt
}

# opt
for i in `seq 1 10`; {
	echo 'opt' $i >> output.txt
	java -jar java/build/libs/coremk_opt.jar-1.0.jar | grep -E 'Iteration|Total' >> output.txt
	echo '' >> output.txt
}

# cht
for i in `seq 1 10`; {
	echo 'cht' $i >> output.txt
	java -jar java/build/libs/coremk_cht.jar-1.0.jar | grep -E 'Iteration|Total ticks' >> output.txt
	echo '' >> output.txt
}
