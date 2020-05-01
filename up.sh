#!/bin/sh
cd /tmp/myroot
rm ./webcam*
rm ./led
wget http://$HOSTIP/camera/webcam_capture
wget http://$HOSTIP/camera/led
chmod +x webcam_capture
chmod +x led
