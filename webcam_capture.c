#include <stdio.h>
#include <stdlib.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/v4l2-common.h>
#include <linux/v4l2-controls.h>
#include <linux/videodev2.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <string.h>


int main() {
    // 1.  Open the device
    int fd; // A file descriptor to the video device
	puts("Compiler test");
	exit(-1);
    fd = open("/dev/video0",O_RDWR);
    if(fd < 0){
        perror("Failed to open device 0, OPEN");
        fd = open("/dev/video1",O_RDWR);
        if(fd < 0){
            perror("Failed to open device1, OPEN");
            return 1;
        }
        else
            puts("Device 1 open");
    }
    else
        puts("Device 0 open");

	struct v4l2_fmtdesc fmt;
       memset(&fmt, 0, sizeof(fmt));
       fmt.index = 0;
       fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
       while (( ioctl(fd, VIDIOC_ENUM_FMT,&fmt)) == 0) {
              fmt.index++;
              printf("{ pixelformat =''%c%c%c%c'', description = ''%s'' }\n",
                            fmt.pixelformat& 0xFF, (fmt.pixelformat >> 8) & 0xFF,
                            (fmt.pixelformat>> 16) & 0xFF, (fmt.pixelformat >> 24) & 0xFF,
                            fmt.description);
       }	

    // 2. Ask the device if it can capture frames
    struct v4l2_capability capability;
    if(ioctl(fd, VIDIOC_QUERYCAP, &capability) < 0){
        // something went wrong... exit
        perror("Failed to get device capabilities, VIDIOC_QUERYCAP");
        return 1;
    }
	struct v4l2_cropcap cropcap;
	memset(&cropcap,0,sizeof(cropcap));
	cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl(fd, VIDIOC_CROPCAP, &cropcap) < 0){
        // something went wrong... exit
        perror("Failed to get cropcap, VIDIOC_CROPCAP");
        return 1;
    }
	else
		puts("VIDIOC_CROPCAP worked");
   
   struct v4l2_crop crop;
   int ret;
   	memset(&crop,0,sizeof(crop));
  	printf("v4l2_crop size: 0x%08x\n",sizeof(crop));
  	printf("V4L2_BUF_TYPE_VIDEO_CAPTURE: 0x%08x\n",V4L2_BUF_TYPE_VIDEO_CAPTURE);
 	crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
   //crop.c = cropcap.defrect; /* reset to default */
    //if(ioctl(fd, VIDIOC_S_CROP, &crop) < 0){
    if(ret=ioctl(fd, 0x8014563C, &crop) < 0){
        // something went wrong... exit
        perror("Failed to set crop, VIDIOC_S_CROP");
		if(ret==1)
			puts("Size to use: 640x480\n");
		else
			printf("ret: %d\n",ret);
    }
	else
		puts("VIDIOC_S_CROP worked");


    // 3. Set Image format
    struct v4l2_format imageFormat;
    imageFormat.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    imageFormat.fmt.pix.width = 640;
    imageFormat.fmt.pix.height = 480;
    imageFormat.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    imageFormat.fmt.pix.field = V4L2_FIELD_NONE;
    // tell the device you are using this format
    if(ioctl(fd, VIDIOC_S_FMT, &imageFormat) < 0){
        perror("Device could not set format, VIDIOC_S_FMT");
        return 1;
    }


    // 4. Request Buffers from the device
    struct v4l2_requestbuffers requestBuffer = {0};
    requestBuffer.count = 1; // one request buffer
    requestBuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; // request a buffer wich we an use for capturing frames
    requestBuffer.memory = V4L2_MEMORY_MMAP;

    if(ioctl(fd, VIDIOC_REQBUFS, &requestBuffer) < 0){
        perror("Could not request buffer from device, VIDIOC_REQBUFS");
        return 1;
    }


    // 5. Quety the buffer to get raw data ie. ask for the you requested buffer
    // and allocate memory for it
    struct v4l2_buffer queryBuffer = {0};
    queryBuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    queryBuffer.memory = V4L2_MEMORY_MMAP;
    queryBuffer.index = 0;
    if(ioctl(fd, VIDIOC_QUERYBUF, &queryBuffer) < 0){
        perror("Device did not return the buffer information, VIDIOC_QUERYBUF");
        return 1;
    }
    // use a pointer to point to the newly created buffer
    // mmap() will map the memory address of the device to
    // an address in memory
    char* buffer = (char*)mmap(NULL, queryBuffer.length, PROT_READ | PROT_WRITE, MAP_SHARED,
                        fd, queryBuffer.m.offset);
    memset(buffer, 0, queryBuffer.length);


    // 6. Get a frame
    // Create a new buffer type so the device knows whichbuffer we are talking about
    struct v4l2_buffer bufferinfo;
    memset(&bufferinfo, 0, sizeof(bufferinfo));
    bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufferinfo.memory = V4L2_MEMORY_MMAP;
    bufferinfo.index = 0;

	usleep(10000);
/***************************** Begin looping here *********************/
    // Queue the buffer
    if(ioctl(fd, VIDIOC_QBUF, &bufferinfo) < 0){
        perror("Could not queue buffer, VIDIOC_QBUF");
        return 1;
    }
	usleep(10000);

    // Activate streaming
    int type = bufferinfo.type;
    if(ioctl(fd, VIDIOC_STREAMON, &type) < 0){
        perror("Could not start streaming, VIDIOC_STREAMON");
        return 1;
    }
	while(1)
	{
	usleep(10000);
    // Dequeue the buffer
    if(ioctl(fd, VIDIOC_DQBUF, &bufferinfo) < 0){
        perror("Could not dequeue the buffer, VIDIOC_DQBUF");
        return 1;
    }
    // Frames get written after dequeuing the buffer
    
	printf("Buffer has: %f KBytes of data\n",(double)bufferinfo.bytesused / 1024);
    
	// Write the data out to file
    FILE *fp=fopen("tmp.jpg","wb");
	if(!fp)
	{
		perror("Unable to open output file");
		return 1;
	}




    int bufPos = 0, outFileMemBlockSize = 0;  // the position in the buffer and the amoun to copy from
                                        // the buffer
    int remainingBufferSize = bufferinfo.bytesused; // the remaining buffer size, is decremented by
                                                    // memBlockSize amount on each loop so we do not overwrite the buffer
    char* outFileMemBlock = NULL;  // a pointer to a new memory block
    int itr = 0; // counts thenumber of iterations
    while(remainingBufferSize > 0) {
        bufPos += outFileMemBlockSize;  // increment the buffer pointer on each loop
                                        // initialise bufPos before outFileMemBlockSize so we can start
                                        // at the begining of the buffer

        outFileMemBlockSize = 1024;    // set the output block size to a preferable size. 1024 :)
        outFileMemBlock = malloc(sizeof(char) * outFileMemBlockSize);

        // copy 1024 bytes of data starting from buffer+bufPos
        memcpy(outFileMemBlock, buffer+bufPos, outFileMemBlockSize);
    	
		fwrite(outFileMemBlock, outFileMemBlockSize, 1, fp);
    	fflush(fp);

        // calculate the amount of memory left to read
        // if the memory block size is greater than the remaining
        // amount of data we have to copy
        if(outFileMemBlockSize > remainingBufferSize)
            outFileMemBlockSize = remainingBufferSize;

        // subtract the amount of data we have to copy
        // from the remaining buffer size
        remainingBufferSize -= outFileMemBlockSize;

        // display the remaining buffer size
		printf("%d Remaining bytes: %d\n",itr,remainingBufferSize);
        
        free(outFileMemBlock);
    }

    // Close the file
    fclose(fp);
	system("mv tmp.jpg out.jpg");
	// Queue the buffer
    if(ioctl(fd, VIDIOC_QBUF, &bufferinfo) < 0){
        perror("Could not queue buffer, VIDIOC_QBUF");
        return 1;
    }
	usleep(10000);
	}

/******************************** end looping here **********************/

    // end streaming
    if(ioctl(fd, VIDIOC_STREAMOFF, &type) < 0){
        perror("Could not end streaming, VIDIOC_STREAMOFF");
        return 1;
    }

    close(fd);
    return 0;
}

