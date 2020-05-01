#!/bin/sh
#mipsel-mstar-linux-gnu-g++ ./webcam_capture.cpp -o ./webcam_capture -Xlinker -I/lib/ld-uClibc.so.0 -static
mipsel-mstar-linux-gnu-gcc ./webcam_capture.c -o ./webcam_capture -Xlinker -I/lib/ld-uClibc.so.0 -static
#mipsel-ramips-linux-uclibc-gcc ./webcam_capture.c -o ./webcam_capture
