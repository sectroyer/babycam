#!/bin/env python3
# copyright sectroyer 2020
import socket
import binascii
import xxtea
import struct
import time
import os
import sys
from datetime import datetime
from PIL import Image, ImageDraw, ImageFont, ImageFile
from io import BytesIO

key="1234567890123456"
A_C_INIT="4942424d8c000100000000000000000000000000000000"
A_C_RESPONSE=b"4942424d8e000100000000000000001000000000000000"
A_A_REQ="4942424d96000100000000000000000900000000000000d20400004633314303"
A_V_REQUEST="4942424d96000100000000000000000900000000000000d20400004633314302"
A_HB="4942424d9c000100000000000000000600000000000000000000000101"

A_SOME_REQ="4942424d96000100000000000000000900000000000000d20400004633314301"
A_RES_HIGH_REQ="4942424d91010100000000000000000b00000000000000d204000000070201000000"
A_RES_MID_REQ="4942424d91010100000000000000000b00000000000000d204000000070202000000"
A_RES_LOW_REQ="4942424d91010100000000000000000b00000000000000d204000000070203000000"

BUFFER_SIZE = 1024
HEADER_SIZE = 23
IMG_SIZE = HEADER_SIZE + 5
IMG_DATA = IMG_SIZE + 4
JFIF_POS= IMG_DATA + 6
num_max=60
draw_time=True

V_REQUEST=binascii.unhexlify(A_V_REQUEST)
C_INIT=binascii.unhexlify(A_C_INIT)

def recvCameraPacket(sock):
    data=bytearray()
    cur_num=0
    while len(data) < HEADER_SIZE and cur_num < num_max:
        cur_num+=1
        data+=sock.recv(HEADER_SIZE)
        if len(data) == 0:
            return data
    current_size=0
    if struct.unpack(">I",data[:4])[0] != 0x4942424d:
        #print "received data: ", binascii.hexlify(data) 
        cur_num=0
        while data.find(b'\x49\x42\x42\x4d') < 0 and cur_num < num_max:
            cur_num+=1
            data+=sock.recv(BUFFER_SIZE)
        ibbm_pos=data.find(b'\x49\x42\x42\x4d')
        old_data=data[:ibbm_pos]
        #f = open('out.jpg', 'r+b')
        #f.seek(0,2)
        #f.write(old_data)
        #f.close()
        #print "Found IBBM at: ", ibbm_pos 
        #print "old data: ", binascii.hexlify(old_data) 
        data=data[ibbm_pos:]
        #print "new data: ", binascii.hexlify(data) 
        current_size=len(data)-HEADER_SIZE
    cur_num=0
    while len(data) < HEADER_SIZE and cur_num < num_max:
        cur_num+=1
        data+=sock.recv(HEADER_SIZE)
    packet_size=struct.unpack("<I",data[15:19])[0]
    #print "received data: ", binascii.hexlify(data) 
    #print "Found packet with size: ",packet_size
    packet=data
    if packet_size > 80000:
        #return packet
        #return data[0:1]
        while len(data) > 0 and len(packet) < 80000:
            data=sock.recv(BUFFER_SIZE)
            packet+=data
        print("\nreceived (packet_size: ", str(packet_size), ") data: ", binascii.hexlify(packet)) 
        now = datetime.now()
        print(now.strftime("%d/%m/%Y %H:%M:%S"))
        #sys.exit(-1)
    cur_num=0
    while current_size < packet_size and cur_num < num_max:
        cur_num+=1
        data=sock.recv(BUFFER_SIZE)
        if len(data) == 0:
            return packet
        packet+=data
        current_size+=len(data)
    return packet

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.settimeout(1)
socket.setdefaulttimeout(1)
s.connect(("c2",10002))
s.send(C_INIT)

#data = s.recv(BUFFER_SIZE)
data=recvCameraPacket(s)
print("received data: ", binascii.hexlify(data)) 
xxTeaRequest=binascii.hexlify(data)[-32:]
print("xxTeaRequest:  ", xxTeaRequest)

xxTeaResponse = binascii.hexlify(xxtea.decrypt(xxTeaRequest,key))
print("xxTeaResponse: ", xxTeaResponse)

C_RES=binascii.unhexlify(A_C_RESPONSE+xxTeaResponse)

print("generated res: ", binascii.hexlify(C_RES))
s.send(C_RES)
#data = s.recv(BUFFER_SIZE)
data=recvCameraPacket(s)
print("received data: ", binascii.hexlify(data)) 

SIZE_REQ=binascii.unhexlify(A_RES_LOW_REQ)
s.send(SIZE_REQ)
print("generated req: ", binascii.hexlify(SIZE_REQ))

data=recvCameraPacket(s)
print("received data: ", binascii.hexlify(data)) 

SIZE_REQ=binascii.unhexlify(A_RES_HIGH_REQ)
s.send(SIZE_REQ)
print("generated req: ", binascii.hexlify(SIZE_REQ))

data=recvCameraPacket(s)
print("received data: ", binascii.hexlify(data)) 

V_REQ=binascii.unhexlify(A_V_REQUEST)
s.send(V_REQ)
print("generated req: ", binascii.hexlify(V_REQ))

#data = s.recv(BUFFER_SIZE)
data=recvCameraPacket(s)
print("received data: ", binascii.hexlify(data)) 

packet_count=0
small_packet_count=0
HB_SENT=0
ImageFile.LOAD_TRUNCATED_IMAGES = True
now = datetime.now()
dt_string = now.strftime("%d/%m/%Y %H:%M:%S")
print("\n"+dt_string)
print("Starting camera capture...")
while True:
    #data = s.recv(HEADER_SIZE)
    data=recvCameraPacket(s)
    #print "received data: ", binascii.hexlify(data[:HEADER_SIZE]) 
    if len(data) < (JFIF_POS+4):
        small_packet_count+=1
        if small_packet_count > 100:
            print("\nToo many small packets. Exiting...")
            sys.exit(-1)
        continue
    small_packet_count=0
    packet_type=struct.unpack("<BBBBB",data[:5])
    #if len(data) > 200 and data[JFIF_POS:JFIF_POS+4].decode('ascii') == "JFIF":
    if len(data) > 200 and  struct.unpack(">I",data[JFIF_POS:JFIF_POS+4])[0] == 0x4a464946:
        packet_count+=1
        #print "packet data: ", binascii.hexlify(data[HEADER_SIZE:0x40]) 
        image_size=struct.unpack("<I",data[IMG_SIZE:IMG_SIZE+4])[0]
        HB_SENT=0
        sys.stdout.write("\rFound image packet. HB: "+str(HB_SENT)+". Image size: "+str(image_size))
        sys.stdout.flush()
        if image_size < 80000:
            #f = open('tmp.jpg', 'wb')
            image_data=data[IMG_DATA:]
            file_jpgdata = BytesIO(image_data)
            image = Image.open(file_jpgdata)

            if draw_time:
                now = datetime.now()
                current_time=now.strftime("%H:%M:%S")
                #color = 'rgb(0, 255, 255)' # white color
                font = ImageFont.load_default() 
                time_im = Image.new("L", font.getsize(current_time), 255) 
                draw = ImageDraw.Draw(time_im)
                draw.rectangle(((0,0),time_im.size), fill = "black" )
                draw.text((0, 0), current_time, fill="yellow")
                time_im = time_im.resize((time_im.width * 2, time_im.height * 2)) 
                del draw
                hor_margin=20
                top_margin=40
                bottom_margin=40
                #draw = ImageDraw.Draw(image)
                (x, y) = (hor_margin, top_margin)
                image.paste(time_im,(x,y))
                #draw.text((x, y), current_time, fill=color)
                (x, y) = (image.width-hor_margin-time_im.width, top_margin)
                #draw.text((x, y), current_time, fill=color)
                image.paste(time_im,(x,y))
                hor_margin=20
                (x, y) = (image.width-hor_margin-time_im.width, image.height-time_im.height-bottom_margin)
                image.paste(time_im,(x,y))
            image.save("tmp.jpg")
            #f.write(image_data)
            #print "Got image."
            #f.close()
            os.rename("tmp.jpg","out.jpg")
    time.sleep(0.01)
    if packet_count%10 == 0:
        HB=binascii.unhexlify(A_HB)
        s.send(HB)
        HB_SENT=1
            #print "\nsent heartbeat : ", binascii.hexlify(HB)
s.close()
