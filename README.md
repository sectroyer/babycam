# babycam
OpenSource implementation of iBaby M2/ M3/ M3S network protocol. It can also work on other cameras. You can use in two ways either by replacing orignal application on camera itself or by impersonating clients.

## Client application impersonation
There are two clients (camera2.py and camera3.py) for both Python 2 and Python 3. For both of them you need special version of xxtea library. Regular one uses PKCS padding which is incompatible with packets sent by camera. You can install it by running this command:

```
pip install xxtea==0.1.5
```

After that once client is executed it connects to camera and starts dumping frames to out.jpg file which for example can be hosted via regular HTTP server. I attach very simple example of HTML file that refreshes image and can work on any device. To start you simply edit the source of camera2/camera3 and start it like this:
```
./camera3.py
```

Or to execute in a loop (for example to make auto connect every time camera is up) run this:
```
./loop_camera.sh
```
## Original application replacement
As models I tested allow for remote access on telnet (using admin/admin credentials) you can simply replace original application by putting your webserver address in conf.sh file and running:
```
./install.sh IP_OF_CAMERA
```
After some setup (and 60 seconds delay) camera will start hosting out.jpg file with latest frame.
