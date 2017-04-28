rm -f output.txt
date >> output.txt

cd java
rm -rf build
gradle -Pcm_version=ori
gradle -Pcm_version=opt
gradle -Pcm_version=cht
cd ..

ITERATIONS=3

# java ori
for i in `seq 1 $ITERATIONS`; {
	echo 'java ori' $i >> output.txt
	java -jar java/build/libs/pg_coremk_ori.jar | grep -E 'Iteration|Total ticks' >> output.txt
	echo '' >> output.txt
}

# java opt
for i in `seq 1 $ITERATIONS`; {
	echo 'java opt' $i >> output.txt
	java -jar java/build/libs/pg_coremk_opt.jar | grep -E 'Iteration|Total ticks' >> output.txt
	echo '' >> output.txt
}

# java cht
for i in `seq 1 $ITERATIONS`; {
	echo 'java cht' $i >> output.txt
	java -jar java/build/libs/pg_coremk_cht.jar | grep -E 'Iteration|Total ticks' >> output.txt
	echo '' >> output.txt
}

# c ori
for i in `seq 1 $ITERATIONS`; {
	cd c/coremark_v1.0
	echo 'c ori' $i >> ../../output.txt
	./runme.sh
	cat run1.log | grep -E 'Iteration|Total ticks' >> ../../output.txt
	echo '' >> ../../output.txt
	cd ../..
}

# c cht
for i in `seq 1 $ITERATIONS`; {
	cd c/dj_cht_version
	echo 'c cht' $i >> ../../output.txt
	./runme.sh | grep -E 'Iteration|Total ticks' >> ../../output.txt
	echo '' >> ../../output.txt
	cd ../..
}

