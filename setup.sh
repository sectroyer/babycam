#!/bin/sh
cd /tmp/
mkdir myroot
cd myroot
wget http://$HOSTIP/camera/busybox-mipsel
wget http://$HOSTIP/camera/webcam_capture
wget http://$HOSTIP/camera/tcpdump
wget http://$HOSTIP/camera/up.sh
wget http://$HOSTIP/camera/ckill.sh
wget http://$HOSTIP/camera/myreboot.sh
chmod +x ./*
mkdir /tmp/bin/
mkdir /tmp/sbin
mv ./busybox-mipsel /tmp/bin/busybox
chmod +x /tmp/bin/*
mkdir /tmp/lib
/tmp/bin/busybox --install -s /tmp/bin
/tmp/bin/busybox tcpsvd -vE 0.0.0.0 21 /tmp/bin/busybox ftpd -t 600 -T 600000 -w /tmp/myroot &
/tmp/bin/busybox httpd
wget http://$HOSTIP/camera/index.html
echo "#!/bin/sh" > /tmp/init.sh
echo "export PATH=\"/tmp/bin:$PATH\"" >> /tmp/init.sh
echo "export HOSTIP=\"$HOSTIP\"" >> /tmp/init.sh
echo "cd /tmp/myroot" >> /tmp/init.sh
echo "/tmp/bin/ash"  >> /tmp/init.sh
chmod +x /tmp/init.sh
echo "DONE"
