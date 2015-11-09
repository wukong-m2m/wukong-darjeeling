export PYTHONPATH=../..
if [ ! -f email.txt ]; then
	echo "Please define the email.txt firstly. You can duplicate it from the email.txt.dist"
	exit -1
fi
python device-email.py
