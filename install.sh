#!/bin/sh
source ./conf.sh
if [ -z $1 ]
then
	echo "No camera address specified!!!"
	exit -1
fi

(sleep 1;
echo "admin";
sleep 1; 
echo -e "admin
export HOSTIP=$HOSTIP
wget http://$HOSTIP/camera/setup.sh -O - | sh; /tmp/init.sh
#./ckill.sh
sleep 60
#./webcam_capture > /dev/null 
sleep 3600
exit") | nc -t "$1" 23
