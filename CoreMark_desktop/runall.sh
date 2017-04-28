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
	java -jar java/build/libs/pg_coremk_ori.jar | grep -E 'Iteration|Total' >> output.txt
	echo '' >> output.txt
}

# opt
for i in `seq 1 10`; {
	echo 'opt' $i >> output.txt
	java -jar java/build/libs/pg_coremk_opt.jar | grep -E 'Iteration|Total' >> output.txt
	echo '' >> output.txt
}

# cht
for i in `seq 1 10`; {
	echo 'cht' $i >> output.txt
	java -jar java/build/libs/pg_coremk_cht.jar | grep -E 'Iteration|Total ticks' >> output.txt
	echo '' >> output.txt
}
